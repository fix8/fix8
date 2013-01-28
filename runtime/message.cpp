//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-13 David L. Dight <fix@fix8.org>

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

#endif
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <bitset>

#include <strings.h>
#include <regex.h>

#include <f8includes.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
namespace {
	const string spacer(3, ' ');
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::extract_header(const f8String& from, f8String& len, f8String& mtype)
{
	const char *dptr(from.data());

	char tag[MAX_FLD_LENGTH], val[MAX_FLD_LENGTH];
	unsigned s_offset(0), result;
	if ((result = extract_element(dptr, from.size(), tag, val)))
	{
		if (*tag != '8')
			return 0;
		s_offset += result;
		if ((result = extract_element(dptr + s_offset, from.size() - s_offset, tag, val)))
		{
			if (*tag != '9')
				return 0;
			len = val;
			s_offset += result;
			if ((result = extract_element(dptr + s_offset, from.size() - s_offset, tag, val)))
			{
				if (*tag != '3' || *(tag + 1) != '5')
					return 0;
				mtype = val;
				s_offset += result;
			}
		}
	}
	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::extract_trailer(const f8String& from, f8String& chksum)
{
	unsigned result(0);
	const size_t res(from.find_last_of("10"));
	if (res != f8String::npos)
	{
		f8String tag;
		result = extract_element(from.data() + res - 1, from.size() - res - 1, tag, chksum);
	}
	return result;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode(const f8String& from, const unsigned offset)
{
	unsigned s_offset(offset), result;
	const unsigned fsize(from.size());
	const char *dptr(from.data());
	char tag[MAX_FLD_LENGTH], val[MAX_FLD_LENGTH];

	for (unsigned pos(_pos.size()); s_offset <= fsize && (result = extract_element(dptr + s_offset, fsize - s_offset, tag, val));)
	{
		const unsigned tv(fast_atoi<unsigned>(tag));
		const BaseEntry *be(_ctx._be.find_ptr(tv));
#if defined PERMIT_CUSTOM_FIELDS
		if (!be && (!_ctx._ube || (be = _ctx._ube->find_ptr(tv)) == 0))
#else
		if (!be)
#endif
			throw InvalidField(tv);
		Presence::const_iterator itr(_fp.get_presence().end());
		if (!_fp.has(tv, itr))
			break;
		s_offset += result;
		if (_fp.get(tv, itr, FieldTrait::present))
		{
			if (!_fp.get(tv, itr, FieldTrait::automatic))
				throw DuplicateField(tv);
		}
		else
		{
			add_field(tv, itr, ++pos, be->_create(val, be->_rlm, -1), false);
			if (_fp.is_group(tv, itr))
				s_offset = decode_group(tv, from, s_offset);
		}
	}

	const unsigned short missing(_fp.find_missing());
	if (missing)
	{
		const BaseEntry& tbe(_ctx._be.find_ref(missing));
		ostringstream ostr;
		ostr << tbe._name << " (" << missing << ')';
		throw MissingMandatoryField(ostr.str());
	}

	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode_group(const unsigned short fnum, const f8String& from, const unsigned offset)
{
	unsigned s_offset(offset), result;
	GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum);
	const unsigned fsize(from.size());
	const char *dptr(from.data());
	char tag[MAX_FLD_LENGTH], val[MAX_FLD_LENGTH];

	for (bool ok(true); ok && s_offset < fsize; )
	{
		scoped_ptr<MessageBase> grp(grpbase->create_group());

		for (unsigned pos(0); s_offset < fsize && (result = extract_element(dptr + s_offset, fsize - s_offset, tag, val));)
		{
			const unsigned tv(fast_atoi<unsigned>(tag));
			Presence::const_iterator itr(grp->_fp.get_presence().end());
			if (grp->_fp.get(tv, itr, FieldTrait::present))	// already present; next group?
				break;
			if (pos == 0 && grp->_fp.getPos(tv, itr) != 1)	// first field in group is mandatory
				throw MissingRepeatingGroupField(tv);
			const BaseEntry *be(_ctx._be.find_ptr(tv));
			if (!be)
				throw InvalidField(tv);
			if (!grp->_fp.has(tv, itr))	// field not found in sub-group - end of repeats?
			{
				ok = false;
				break;
			}
			s_offset += result;
			grp->add_field(tv, itr, ++pos, be->_create(val, be->_rlm, -1), false);
			grp->_fp.set(tv, itr, FieldTrait::present);	// is present
			if (grp->_fp.is_group(tv, itr)) // nested group
				s_offset = grp->decode_group(tv, from, s_offset);
		}

		const unsigned short missing(grp->_fp.find_missing());
		if (missing)
		{
			const BaseEntry& tbe(_ctx._be.find_ref(missing));
			ostringstream ostr;
			ostr << tbe._name << " (" << missing << ')';
			throw MissingMandatoryField(ostr.str());
		}
		*grpbase += grp.release();
	}

	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::check_positions()
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
Message *Message::factory(const F8MetaCntx& ctx, const f8String& from)
{
	Message *msg(0);
	f8String len, mtype;
	if (extract_header(from, len, mtype))
	{
		const unsigned mlen(fast_atoi<unsigned>(len.c_str()));
		const BaseMsgEntry *bme(ctx._bme.find_ptr(mtype));
		if (!bme)
			throw InvalidMessage(mtype);
		msg = bme->_create();
#if defined PERMIT_CUSTOM_FIELDS
		if (ctx._ube)
			ctx._ube->post_msg_ctor(msg);
#endif
#if defined CODECTIMING
		ostringstream gerr;
		gerr << "decode(" << mtype << "):";
		IntervalTimer itm;
#endif
		msg->decode(from);
#if defined CODECTIMING
		gerr << itm.Calculate();
		GlobalLogger::log(gerr.str());
#endif

		Fields::const_iterator fitr(msg->_header->_fields.find(Common_BodyLength));
		static_cast<body_length *>(fitr->second)->set(mlen);
		fitr = msg->_header->_fields.find(Common_MsgType);
		static_cast<msg_type *>(fitr->second)->set(mtype);
#if defined POPULATE_METADATA
		msg->check_set_rlm(fitr->second);
#endif

		f8String chksum;
		if (extract_trailer(from, chksum))
		{
			Fields::const_iterator fitr(msg->_trailer->_fields.find(Common_CheckSum));
			static_cast<check_sum *>(fitr->second)->set(chksum);
			const unsigned chkval(fast_atoi<unsigned>(chksum.c_str())), // chksum value
				mchkval(calc_chksum(from, 0)); // chksum pos
			if (chkval != mchkval)
				throw BadCheckSum(mchkval);
		}
	}
	else
	{
		//cerr << "Message::factory throwing" << endl;
		throw InvalidMessage(from);
	}

	return msg;
}

//-------------------------------------------------------------------------------------------------
// copy all fields from this message to 'to' where the field is legal for 'to' and it is not
// already present in 'to'; includes repeating groups;
// if force, copy all fields regardless, replacing any existing, adding any new
unsigned MessageBase::copy_legal(MessageBase *to, bool force) const
{
	unsigned copied(0);
	for (Presence::const_iterator itr(_fp.get_presence().begin()); itr != _fp.get_presence().end(); ++itr)
	{
		if (itr->_field_traits & FieldTrait::present && (force || (to->_fp.has(itr->_fnum) && !to->_fp.get(itr->_fnum))))
		{
			if (itr->_field_traits & FieldTrait::group)
			{
				GroupBase *gb(find_group(itr->_fnum)), *gb1(to->find_group(itr->_fnum));

				for (GroupElement::const_iterator gitr(gb->_msgs.begin()); gitr != gb->_msgs.end(); ++gitr)
				{
					MessageBase *grc(gb1->create_group());
					(*gitr)->copy_legal(grc, force);
					*gb1 += grc;
				}
			}

			BaseField *nf(get_field(itr->_fnum)->copy());
#if defined POPULATE_METADATA
			to->check_set_rlm(nf);
#endif
			Presence::const_iterator fpitr(_fp.get_presence().end());
			if (force && to->_fp.get(itr->_fnum, fpitr, FieldTrait::present))
				delete to->replace(itr->_fnum, fpitr, nf);
			else
				to->add_field(nf);
			++copied;
		}
	}

	return copied;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::encode(char *to, size_t& sz) const
{
	const size_t where(sz);
	for (Positions::const_iterator itr(_pos.begin()); itr != _pos.end(); ++itr)
	{
#if defined POPULATE_METADATA
		check_set_rlm(itr->second);
#endif
		Presence::const_iterator fpitr(_fp.get_presence().end());
		if (!_fp.get(itr->second->_fnum, fpitr, FieldTrait::suppress))	// some fields are not encoded until unsuppressed (eg. checksum)
		{
			itr->second->encode(to, sz);
			if (_fp.get(itr->second->_fnum, fpitr, FieldTrait::group))
				encode_group(itr->second->_fnum, to, sz);
		}
	}

	return sz - where;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::encode(ostream& to) const
{
	const std::ios::pos_type where(to.tellp());
	for (Positions::const_iterator itr(_pos.begin()); itr != _pos.end(); ++itr)
	{
#if defined POPULATE_METADATA
		check_set_rlm(itr->second);
#endif
		Presence::const_iterator fpitr(_fp.get_presence().end());
		if (!_fp.get(itr->second->_fnum, fpitr, FieldTrait::suppress))	// some fields are not encoded until unsuppressed (eg. checksum)
		{
			itr->second->encode(to);
			if (_fp.get(itr->second->_fnum, fpitr, FieldTrait::group))
				encode_group(itr->second->_fnum, to);
		}
	}

	return to.tellp() - where;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::encode_group(const unsigned short fnum, char *to, size_t& sz) const
{
	const size_t where(sz);
	GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum);
	for (GroupElement::iterator itr(grpbase->_msgs.begin()); itr != grpbase->_msgs.end(); ++itr)
		(*itr)->encode(to, sz);
	return sz - where;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::encode_group(const unsigned short fnum, std::ostream& to) const
{
	const std::ios::pos_type where(to.tellp());
	GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum);
	for (GroupElement::iterator itr(grpbase->_msgs.begin()); itr != grpbase->_msgs.end(); ++itr)
		(*itr)->encode(to);
	return to.tellp() - where;
}

//-------------------------------------------------------------------------------------------------
unsigned Message::encode(f8String& to) const
{
	char msg[MAX_MSG_LENGTH], hmsg[MAX_MSG_LENGTH];
	size_t sz(0), hsz(0);

#if defined CODECTIMING
	ostringstream gerr;
	gerr << "encode(" << _msgType << "):";
	IntervalTimer itm;
#endif

	if (!_header)
		throw MissingMessageComponent("header");
	Fields::const_iterator fitr(_header->_fields.find(Common_MsgType));
	static_cast<msg_type *>(fitr->second)->set(_msgType);
	_header->encode(msg, sz);
	MessageBase::encode(msg, sz);
	if (!_trailer)
		throw MissingMessageComponent("trailer");
	_trailer->encode(msg, sz);
	const unsigned msgLen(sz);	// checksummable msglength

	if ((fitr = _header->_fields.find(Common_BeginString)) == _header->_fields.end())
		throw MissingMandatoryField(Common_BeginString);
	_header->_fp.clear(Common_BeginString, FieldTrait::suppress);
	fitr->second->encode(hmsg, hsz);
#if defined MSGRECYCLING
	_header->_fp.set(Common_BeginString, FieldTrait::suppress); // in case we want to reuse
#endif

	if ((fitr = _header->_fields.find(Common_BodyLength)) == _header->_fields.end())
		throw MissingMandatoryField(Common_BodyLength);
	_header->_fp.clear(Common_BodyLength, FieldTrait::suppress);
	static_cast<body_length *>(fitr->second)->set(msgLen);
	fitr->second->encode(hmsg, hsz);
#if defined MSGRECYCLING
	_header->_fp.set(Common_BodyLength, FieldTrait::suppress); // in case we want to reuse
#endif

	::memcpy(hmsg + hsz, msg, sz);
	hsz += sz;

	if ((fitr = _trailer->_fields.find(Common_CheckSum)) == _trailer->_fields.end())
		throw MissingMandatoryField(Common_CheckSum);
	static_cast<check_sum *>(fitr->second)->set(fmt_chksum(calc_chksum(hmsg, hsz)));
	_trailer->_fp.clear(Common_CheckSum, FieldTrait::suppress);
	fitr->second->encode(hmsg, hsz);
#if defined MSGRECYCLING
	_trailer->_fp.set(Common_CheckSum, FieldTrait::suppress); // in case we want to reuse
#endif

#if defined CODECTIMING
	gerr << itm.Calculate();
	GlobalLogger::log(gerr.str());
#endif

	to.assign(hmsg, hsz);
	return to.size();
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print(ostream& os, int depth) const
{
	const string dspacer((depth + 1) * 3, ' ');
	const BaseMsgEntry *tbme(_ctx._bme.find_ptr(_msgType));
	if (tbme)
		os << tbme->_name << " (\"" << _msgType << "\")" << endl;
	for (Positions::const_iterator itr(_pos.begin()); itr != _pos.end(); ++itr)
	{
		const BaseEntry *tbe(_ctx._be.find_ptr(itr->second->_fnum));
		if (!tbe)
#if defined PERMIT_CUSTOM_FIELDS
			if (!_ctx._ube || (tbe = _ctx._ube->find_ptr(itr->second->_fnum)) == 0)
#endif
				throw InvalidField(itr->second->_fnum);
		os << dspacer << tbe->_name << " (" << itr->second->_fnum << "): ";
		int idx;
		if (itr->second->_rlm && (idx = (itr->second->get_rlm_idx())) >= 0)
			os << itr->second->_rlm->_descriptions[idx] << " (" << *itr->second << ')' << endl;
		else
			os << *itr->second << endl;
		if (_fp.is_group(itr->second->_fnum))
			print_group(itr->second->_fnum, os, depth);
	}
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print_group(const unsigned short fnum, ostream& os, int depth) const
{
	const GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum);

	const string dspacer((depth + 1) * 3, ' ');
	size_t cnt(1);
	for (GroupElement::const_iterator itr(grpbase->_msgs.begin()); itr != grpbase->_msgs.end(); ++itr, ++cnt)
	{
		os << dspacer << (*itr)->_msgType << " (Repeating group " << cnt << '/' << grpbase->_msgs.size() << ')' << endl;
		(*itr)->print(os, depth + 1);
	}
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print_field(const unsigned short fnum, ostream& os) const
{
	Fields::const_iterator fitr(_fields.find(fnum));
	if (fitr != _fields.end())
	{
		const BaseEntry *tbe(_ctx._be.find_ptr(fnum));
		if (!tbe)
#if defined PERMIT_CUSTOM_FIELDS
			if (!_ctx._ube || (tbe = _ctx._ube->find_ptr(fnum)) == 0)
#endif
				throw InvalidField(fnum);
		os << tbe->_name << " (" << fnum << "): ";
		int idx;
		if (fitr->second->_rlm && (idx = (fitr->second->get_rlm_idx())) >= 0)
			os << fitr->second->_rlm->_descriptions[idx] << " (" << *fitr->second << ')';
		else
			os << *fitr->second;
		if (_fp.is_group(fnum))
			print_group(fnum, os, 0);
	}
}

//-------------------------------------------------------------------------------------------------
BaseField *MessageBase::replace(const unsigned short fnum, BaseField *with)
{
	BaseField *old(0);
	Fields::iterator itr(_fields.find(fnum));
	if (itr != _fields.end())
	{
		old = itr->second;
		unsigned pos(_fp.getPos(fnum));
		for (Positions::iterator pitr(_pos.begin()); pitr != _pos.end(); ++pitr)
		{
			if (pitr->second == old)
			{
				pos = pitr->first;
				_pos.erase(pitr);
				break;
			}
		}
		_pos.insert(Positions::value_type(pos, with));
		itr->second = with;
		_fp.set(fnum, FieldTrait::present);
	}
	return old;
}

//-------------------------------------------------------------------------------------------------
BaseField *MessageBase::replace(const unsigned short fnum, Presence::const_iterator fitr, BaseField *with)
{
	BaseField *old(0);
	Fields::iterator itr(_fields.find(fnum));
	if (itr != _fields.end())
	{
		old = itr->second;
		unsigned pos(_fp.getPos(fnum, fitr));
		for (Positions::iterator pitr(_pos.begin()); pitr != _pos.end(); ++pitr)
		{
			if (pitr->second == old)
			{
				pos = pitr->first;
				_pos.erase(pitr);
				break;
			}
		}
		_pos.insert(Positions::value_type(pos, with));
		itr->second = with;
		_fp.set(fnum, fitr, FieldTrait::present);
	}
	return old;
}

//-------------------------------------------------------------------------------------------------
BaseField *MessageBase::remove(const unsigned short fnum)
{
	BaseField *old(0);
	Fields::iterator itr(_fields.find(fnum));
	if (itr != _fields.end())
	{
		old = itr->second;
		for (Positions::iterator pitr(_pos.begin()); pitr != _pos.end(); ++pitr)
		{
			if (pitr->second == old)
			{
				_pos.erase(pitr);
				break;
			}
		}
		_fp.clear(fnum, FieldTrait::present);
		_fields.erase(itr);
	}
	return old;
}

//-------------------------------------------------------------------------------------------------
BaseField *MessageBase::remove(const unsigned short fnum, Presence::const_iterator fitr)
{
	BaseField *old(0);
	Fields::iterator itr(_fields.find(fnum));
	if (itr != _fields.end())
	{
		old = itr->second;
		for (Positions::iterator pitr(_pos.begin()); pitr != _pos.end(); ++pitr)
		{
			if (pitr->second == old)
			{
				_pos.erase(pitr);
				break;
			}
		}
		_fp.clear(fnum, fitr, FieldTrait::present);
		_fields.erase(itr);
	}
	return old;
}

//-------------------------------------------------------------------------------------------------
Message *Message::clone() const
{
	const BaseMsgEntry& bme(_ctx._bme.find_ref(_msgType));
	Message *msg(bme._create());
	copy_legal(msg, true);
	_header->copy_legal(msg->_header, true);
	_trailer->copy_legal(msg->_trailer, true);
	return msg;
}

//-------------------------------------------------------------------------------------------------
void Message::print(ostream& os, int) const
{
	if (_header)
		os << *_header;
	else
		os << "Null Header" << endl;
	MessageBase::print(os);
	if (_trailer)
		os << *_trailer;
	else
		os << "Null Trailer" << endl;
}

