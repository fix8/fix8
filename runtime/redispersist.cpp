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

#if defined FIX8_HAVE_LIBHIREDIS
//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
bool HiredisPersister::initialise(const f8String& host, unsigned port, unsigned connect_timeout,
	const f8String& key_base, bool purge)
{
	if (_cache)
		return true;
	_key_base = key_base;
	const timeval timeout { connect_timeout, 0 }; // seconds
	if (!(_cache = redisConnectWithTimeout(host.c_str(), port, timeout)) || _cache->err)
	{
		if (_cache->err)
		{
			glout_error << "redis error connect: " << _cache->errstr << " for " << _key_base;
		}
		return false;
	}

	if (purge)
	{
		redisReply *reply(static_cast<redisReply*>(redisCommand(_cache, "ZREMRANGEBYRANK %s 0 -1", _key_base.c_str())));
		if (reply->type == REDIS_REPLY_ERROR)
		{
			glout_error << "redis error purge (ZREMRANGEBYRANK): " << reply->str << " for " << _key_base;
		}
		freeReplyObject(reply);
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
HiredisPersister::~HiredisPersister()
{
	if (_cache)	// redisFree doesn't check for nullptr
	{
		redisFree(_cache);
		_cache = 0;
	}
}

//-------------------------------------------------------------------------------------------------
unsigned HiredisPersister::get_last_seqnum(unsigned& sequence) const
{
	unsigned result(0);
	redisReply *reply(static_cast<redisReply*>(redisCommand(_cache, "ZRANGE %s -1 -1 WITHSCORES", _key_base.c_str())));
	if (reply->type == REDIS_REPLY_ERROR)
	{
		glout_error << "redis error ZRANGE: " << reply->str << " for " << _key_base;
	}
	else if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2)	// we expect two records
		result = fast_atoi<unsigned>((*(reply->element + 1))->str);	// 3nd element is score (seqnum)
	else
	{
		glout_error << "redis error ZRANGE: unexpected type: " << reply->type << " for " << _key_base;
	}

	freeReplyObject(reply);
	return sequence = result;
}

//-------------------------------------------------------------------------------------------------
unsigned HiredisPersister::get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*callback)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const
{
	unsigned last_seq(0);
	get_last_seqnum(last_seq);
	unsigned recs_sent(0), startSeqNum(from);
	const unsigned finish(to == 0 ? last_seq : to);
	Session::RetransmissionContext rctx(from, to, session.get_next_send_seq());

	if (!startSeqNum || from > finish)
	{
		glout_warn << "No records found";
		rctx._no_more_records = true;
		(session.*callback)(Session::SequencePair(0, ""), rctx);
		return 0;
	}

	redisReply *reply(static_cast<redisReply*>(redisCommand(_cache, "ZRANGEBYSCORE %s %u %u WITHSCORES", _key_base.c_str(), startSeqNum, finish)));
	if (reply->type == REDIS_REPLY_ERROR)
	{
		glout_error << "redis error ZRANGEBYSCORE: " << reply->str << " for " << _key_base;
	}
	else if (reply->type == REDIS_REPLY_ARRAY)
	{
		//cerr << "last_seq=" << last_seq << " reply->elements=" << reply->elements << endl;

		for (unsigned ii(0); ii < reply->elements; ii += 2)
		{
			Session::SequencePair txresult(fast_atoi<unsigned>((*(reply->element + ii + 1))->str),
				f8String((*(reply->element + ii))->str, (*(reply->element + ii))->len));
			++recs_sent;
			if (!(session.*callback)(txresult, rctx))
				break;
		}
	}
	else
	{
		glout_error << "redis error ZRANGEBYSCORE: unexpected type: " << reply->type << " for " << _key_base;
	}

	freeReplyObject(reply);

	rctx._no_more_records = true;
	(session.*callback)(Session::SequencePair(0, ""), rctx);

	return recs_sent;
}

//-------------------------------------------------------------------------------------------------
bool HiredisPersister::put(const unsigned sender_seqnum, const unsigned target_seqnum)
{
	if (!_cache)
		return false;
	bool result(true);
	// we need to remove the exiting control record and readd it
	redisReply *reply(static_cast<redisReply*>(redisCommand(_cache, "ZREMRANGEBYSCORE %s 0 0", _key_base.c_str())));
	freeReplyObject(reply);
	reply = static_cast<redisReply*>(redisCommand(_cache, "ZADD %s 0 %u:%u", _key_base.c_str(), sender_seqnum, target_seqnum));
	if (reply->type == REDIS_REPLY_ERROR)
	{
		glout_error << "redis error ZADD: " << reply->str << " for " << _key_base;
		result = false;
	}

	freeReplyObject(reply);
	return result;
}

//-------------------------------------------------------------------------------------------------
bool HiredisPersister::put(const unsigned seqnum, const f8String& what)
{
	if (!_cache || !seqnum)
		return false;

	bool result(true);
	redisReply *reply(static_cast<redisReply*>(redisCommand(_cache, "ZADD %s %u %b", _key_base.c_str(),
		seqnum, what.data(), what.size())));
	if (reply->type == REDIS_REPLY_ERROR)
	{
		glout_error << "redis error ZADD: " << reply->str << " for " << _key_base;
		result = false;
	}

	freeReplyObject(reply);
	return result;
}

//-------------------------------------------------------------------------------------------------
bool HiredisPersister::put(const f8String& key, const f8String& what)
{
	if (!_cache)
		return false;

	bool result(true);
	redisReply *reply(static_cast<redisReply*>(redisCommand(_cache, "SET %s%s %b", _key_base.c_str(),
		key.c_str(), what.data(), what.size())));
	if (reply->type == REDIS_REPLY_ERROR)
	{
		glout_error << "redis error SET: " << reply->str << " for " << _key_base << ':' << key;
		result = false;
	}

	freeReplyObject(reply);
	return result;
}

//-------------------------------------------------------------------------------------------------
bool HiredisPersister::get(unsigned& sender_seqnum, unsigned& target_seqnum) const
{
	if (!_cache)
		return false;

	bool result(false);
	redisReply *reply(static_cast<redisReply*>(redisCommand(_cache, "ZRANGEBYSCORE %s 0 0", _key_base.c_str())));
	if (reply->type == REDIS_REPLY_ERROR)
	{
		glout_error << "redis error ZRANGEBYSCORE: " << reply->str << " for " << _key_base;
	}
	else if (reply->type == REDIS_REPLY_ARRAY)
	{
		if (reply->elements == 1)
		{
			istringstream istr((*reply->element)->str);
			istr >> sender_seqnum;
			istr.ignore();
			istr >> target_seqnum;
			result = true;
		}
	}
	else
	{
		glout_error << "redis error ZRANGEBYSCORE: unexpected type: " << reply->type << " for " << _key_base;
	}

	freeReplyObject(reply);
	return result;
}

//-------------------------------------------------------------------------------------------------
bool HiredisPersister::get(const unsigned seqnum, f8String& to) const
{
	if (!_cache || !seqnum)
		return false;

	bool result(false);
	redisReply *reply(static_cast<redisReply*>(redisCommand(_cache, "ZRANGEBYSCORE %s %u %u", _key_base.c_str(), seqnum, seqnum)));
	if (reply->type == REDIS_REPLY_ERROR)
	{
		glout_error << "redis error ZRANGEBYSCORE: " << reply->str << " for " << _key_base;
	}
	else if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 1)	// we expect one record
	{
		to.assign((*reply->element)->str, (*reply->element)->len);
		result = true;
	}
	else
	{
		glout_error << "redis error ZRANGEBYSCORE: unexpected type: " << reply->type << " for " << _key_base;
	}

	freeReplyObject(reply);
	return result;
}

//-------------------------------------------------------------------------------------------------
bool HiredisPersister::get(const f8String& key, f8String& to) const
{
	if (!_cache)
		return false;

	bool result(false);
	redisReply *reply(static_cast<redisReply*>(redisCommand(_cache, "GET %s%s", _key_base.c_str(), key.c_str())));
	if (reply->type == REDIS_REPLY_ERROR)
	{
		glout_error << "redis error GET: " << reply->str << " for " << _key_base << ':' << key;
	}
	else if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 1)	// we expect one record
	{
		to.assign((*reply->element)->str, (*reply->element)->len);
		result = true;
	}
	else
	{
		glout_error << "redis error GET: unexpected type: " << reply->type << " for " << _key_base << ':' << key;
	}

	freeReplyObject(reply);
	return result;
}

//-------------------------------------------------------------------------------------------------
bool HiredisPersister::del(const f8String& key)
{
	if (!_cache)
		return false;

	bool result(false);
	redisReply *reply(static_cast<redisReply*>(redisCommand(_cache, "DEL %s%s", _key_base.c_str(), key.c_str())));
	if (reply->type == REDIS_REPLY_ERROR)
	{
		glout_error << "redis error DEL: " << reply->str << " for " << _key_base << ':' << key;
	}
	else if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 1)	// we expect one record
	{
		result = true;
	}
	else
	{
		glout_error << "redis error DEL: unexpected type: " << reply->type << " for " << _key_base << ':' << key;
	}

	freeReplyObject(reply);
	return result;
}

//---------------------------------------------------------------------------------------------------
unsigned HiredisPersister::find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const
{
	if (last)
	{
		string target;
		for (unsigned startseqnum(requested); startseqnum <= last; ++startseqnum)
			if (get(startseqnum, target))
				return startseqnum;
	}

	return 0;
}

#endif // FIX8_HAVE_LIBHIREDIS

