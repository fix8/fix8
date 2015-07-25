//-----------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-15 David L. Dight <fix@fix8.org>

Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-----------------------------------------------------------------------------------------
#include "precomp.hpp"
#include <fix8/f8includes.hpp>

#if defined FIX8_HAVE_LIBMEMCACHED
//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
bool MemcachedPersister::initialise(const f8String& config_str, const f8String& key_base, bool purge)
{
	if (_cache)
		return true;
	_key_base = key_base;
	_cache = memcached(config_str.c_str(), config_str.size());
	if (!(_server_count = memcached_server_count(_cache)))
	{
		glout_error << "Error: no memcached servers were configured for " << _key_base;
		return false;
	}
	return purge ? memcached_success(memcached_flush(_cache, 0)) : true;
}

//-------------------------------------------------------------------------------------------------
MemcachedPersister::~MemcachedPersister()
{
	memcached_free(_cache);
	_cache = 0;
}

//-------------------------------------------------------------------------------------------------
unsigned MemcachedPersister::get_last_seqnum(unsigned& sequence) const
{
	unsigned sender_seqnum, target_seqnum;
	return get(sender_seqnum, target_seqnum) ? sequence = sender_seqnum : 0;
}

//-------------------------------------------------------------------------------------------------
unsigned MemcachedPersister::get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*callback)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const
{
	unsigned last_seq(0);
	get_last_seqnum(last_seq);
	unsigned recs_sent(0), startSeqNum(find_nearest_highest_seqnum (from, last_seq));
	const unsigned finish(to == 0 ? last_seq : to);
	Session::RetransmissionContext rctx(from, to, session.get_next_send_seq());

	if (!startSeqNum || from > finish)
	{
		glout_warn << "No records found";
		rctx._no_more_records = true;
		(session.*callback)(Session::SequencePair(0, ""), rctx);
		return 0;
	}

	for (; startSeqNum <= finish; ++startSeqNum)
	{
		string target, key(generate_seq_key(startSeqNum));
		if (get_from_cache(key, target))
		{
			Session::SequencePair txresult(startSeqNum, f8String(target.c_str(), target.size()));
			++recs_sent;
			if (!(session.*callback)(txresult, rctx))
				break;
		}
	}

	rctx._no_more_records = true;
	(session.*callback)(Session::SequencePair(0, ""), rctx);

	return recs_sent;
}

//-------------------------------------------------------------------------------------------------
bool MemcachedPersister::put(const unsigned sender_seqnum, const unsigned target_seqnum)
{
	if (!_cache)
		return false;
	const string key(generate_seq_key(0)), payload(generate_ctrl_record(sender_seqnum, target_seqnum));
	if (!put_to_cache(key, payload))
	{
		glout_error << "Error: could not write control record to memcached for " << _key_base;
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------
bool MemcachedPersister::put(const unsigned seqnum, const f8String& what)
{
	if (!_cache || !seqnum)
		return false;

	const string key(generate_seq_key(seqnum));
	if (!put_to_cache(key, what))
	{
		glout_error << "Error: could not write record for seqnum " << seqnum << " for " << _key_base;
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool MemcachedPersister::put(const f8String& inkey, const f8String& what)
{
	if (!_cache)
		return false;

	const string key(_key_base + inkey);
	if (!put_to_cache(key, what))
	{
		glout_error << "Error: could not write record for key " << inkey << " for " << _key_base;
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool MemcachedPersister::get(unsigned& sender_seqnum, unsigned& target_seqnum) const
{
	if (!_cache)
		return false;

	const string key(generate_seq_key(0));
	string target;
	if (!get_from_cache(key, target))
	{
		glout_warn << "Warning: memcached does not have control record for " << _key_base;
		return false;
	}

	extract_ctrl_record(target, sender_seqnum, target_seqnum);
	return true;
}

//-------------------------------------------------------------------------------------------------
bool MemcachedPersister::get(const unsigned seqnum, f8String& to) const
{
	if (!_cache || !seqnum)
		return false;
	const string key(generate_seq_key(seqnum));
	if (!get_from_cache(key, to))
	{
		glout_warn << "Warning: could not get message record with seqnum " << seqnum << " for " << _key_base;
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool MemcachedPersister::get(const f8String& inkey, f8String& to) const
{
	if (!_cache)
		return false;
	const string key(_key_base + inkey);
	if (!get_from_cache(key, to))
	{
		glout_warn << "Warning: could not get message record with key " << inkey << " for " << _key_base;
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------------------------------
unsigned MemcachedPersister::find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const
{
	if (last)
	{
		for (unsigned startseqnum(requested); startseqnum <= last; ++startseqnum)
		{
			string target;
			if (get(startseqnum, target))
				return startseqnum;
		}
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------
unsigned MemcachedPersister::find_nearest_seqnum (unsigned requested) const
{
	for (; requested > 0; --requested)
	{
		string target;
		if (get(requested, target))
			return requested;
	}

	return 0;
}

#endif // FIX8_HAVE_LIBMEMCACHED

