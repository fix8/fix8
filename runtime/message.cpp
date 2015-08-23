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

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
#if defined FIX8_CODECTIMING
codec_timings Message::_encode_timings, Message::_decode_timings;
#endif
unsigned MessageBase::_tabsize = defaults::tabsize;

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::extract_header(const f8String& from, char *len, char *mtype)
{
	const char *dptr(from.data());
	const unsigned flen(static_cast<unsigned>(from.size()));
	char tag[MAX_MSGTYPE_FIELD_LEN], val[FIX8_MAX_FLD_LENGTH];
	unsigned s_offset(0), result;

	if ((result = extract_element(dptr, flen, tag, val)))
	{
		if (*tag != '8')
			return 0;
		s_offset += result;
		if ((result = extract_element(dptr + s_offset, flen - s_offset, tag, len)))
		{
			if (*tag != '9')
				return 0;
			s_offset += result;
			if ((result = extract_element(dptr + s_offset, flen - s_offset, tag, mtype)))
			{
				if (*tag != '3' || *(tag + 1) != '5')
					return 0;
				s_offset += result;
			}
		}
	}
	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::extract_trailer(const f8String& from, f8String& chksum)
{
	f8String tag;
	return extract_element(from.data() + from.size() - 7, 6, tag, chksum);
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode(const f8String& from, unsigned s_offset, unsigned ignore, bool permissive_mode)
{
	const unsigned fsize(static_cast<unsigned>(from.size()) - ignore), npos(0xffffffff);
	unsigned pos(static_cast<unsigned>(_pos.size())), last_valid_pos(npos);
	const char *dptr(from.data());
	char tag[FIX8_MAX_FLD_LENGTH], val[FIX8_MAX_FLD_LENGTH];
	size_t last_valid_offset(0);

	for (unsigned result; s_offset <= fsize && (result = extract_element(dptr + s_offset, fsize - s_offset, tag, val));)
	{
		const unsigned short tv(fast_atoi<unsigned short>(tag));
		Presence::const_iterator itr(_fp.get_presence().find(tv));
		if (itr == _fp.get_presence().end())
		{
			if (permissive_mode)
			{
				if (last_valid_pos == npos)
				{
					last_valid_pos = pos;
					last_valid_offset = s_offset;
				}
				_unknown.append(dptr + s_offset, result);
				s_offset += result;
				continue;
			}
			break;
		}
		s_offset += result;
		if (itr->_field_traits.has(FieldTrait::present))
		{
			if (!itr->_field_traits.has(FieldTrait::automatic))
				throw DuplicateField(tv);
		}
		else
		{
			const BaseEntry *be(_ctx.find_be(tv));
			if (!be)
				throw UnknownField(tv);
			BaseField *bf(be->_create._do(val, be->_rlm, -1));
			add_field_decoder(tv, ++pos, bf);
			itr->_field_traits.set(FieldTrait::present);
			// check if repeating group and num elements > 0
			if (itr->_field_traits.has(FieldTrait::group) && has_group_count(bf))
				s_offset = decode_group(nullptr, tv, from, s_offset, ignore);
		}
	}

	const unsigned short missing(_fp.find_missing());
	if (missing)
	{
		const BaseEntry *tbe(_ctx.find_be(missing));
		ostringstream ostr;
		ostr << tbe->_name << " (" << missing << ')';
		throw MissingMandatoryField(ostr.str());
	}

	return permissive_mode && last_valid_pos == pos ? static_cast<unsigned>(last_valid_offset) : s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode_group(GroupBase *grpbase, const unsigned short fnum, const f8String& from,
	unsigned s_offset, unsigned ignore)
{
	unsigned result;
	if (!(grpbase = find_add_group(fnum, grpbase)))
		throw InvalidRepeatingGroup(fnum, FILE_LINE);
	const unsigned fsize(static_cast<unsigned>(from.size()) - ignore);
	const char *dptr(from.data());
	char tag[FIX8_MAX_FLD_LENGTH], val[FIX8_MAX_FLD_LENGTH];

	for (bool ok(true); ok && s_offset < fsize; )
	{
		unique_ptr<MessageBase> grp(grpbase->create_group(false)); // shallow create

		for (unsigned pos(0); s_offset < fsize && (result = extract_element(dptr + s_offset, fsize - s_offset, tag, val));)
		{
			const unsigned tv(fast_atoi<unsigned>(tag));
			Presence::const_iterator itr(grp->_fp.get_presence().end());
			if (grp->_fp.get(tv, itr, FieldTrait::present))	// already present; next group?
				break;
			if (pos == 0 && grp->_fp.getPos(tv, itr) != 1)	// first field in group is mandatory
				throw MissingRepeatingGroupField(tv);
			const BaseEntry *be(_ctx.find_be(tv));
			if (!be || !grp->_fp.has(tv, itr))	// unknown field or field not found in sub-group - end of repeats?
			{
				ok = false;
				break;
			}
			s_offset += result;
			BaseField *bf(be->_create._do(val, be->_rlm, -1));
			grp->add_field(tv, itr, ++pos, bf, false);
			grp->_fp.set(tv, itr, FieldTrait::present);	// is present
			// nested group (check if not zero elements)
			if (grp->_fp.is_group(tv, itr) && has_group_count(bf))
				s_offset = grp->decode_group(grpbase, tv, from, s_offset, ignore);
		}

		const unsigned short missing(grp->_fp.find_missing());
		if (missing)
		{
			const BaseEntry *tbe(_ctx.find_be(missing));
			ostringstream ostr;
			ostr << tbe->_name << " (" << missing << ')';
			throw MissingMandatoryField(ostr.str());
		}
		*grpbase << grp.release();
	}

	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::check_positions()
{
	return 0; // TODO
}

//-------------------------------------------------------------------------------------------------
Message *Message::factory(const F8MetaCntx& ctx, const f8String& from, bool no_chksum, bool permissive_mode)
{
	char mtype[MAX_MSGTYPE_FIELD_LEN] {}, len[MAX_MSGTYPE_FIELD_LEN] {};
	const unsigned hlen(extract_header(from, len, mtype));

	if (!hlen)
	{
		glout_debug << "Message::factory throwing";
		throw InvalidMessage(from, FILE_LINE);
	}

	const unsigned mlen(fast_atoi<unsigned>(len));
	const BaseMsgEntry *bme(ctx._bme.find_ptr(mtype));
	if (!bme)
		throw InvalidMessage(mtype, FILE_LINE);
	Message *msg(bme->_create._do(false)); // shallow create
#if defined FIX8_CODECTIMING
	IntervalTimer itm;
#endif
	msg->decode(from, hlen, 7, permissive_mode); // skip already decoded mandatory 8, 9, 35 and 10
#if defined FIX8_CODECTIMING
	_decode_timings._cpu_used += itm.Calculate().AsDouble();
	++_decode_timings._msg_count;
#endif

	msg->_header->get_body_length()->set(mlen);
	msg->_header->get_msg_type()->set(mtype);
#if defined FIX8_POPULATE_METADATA
	msg->check_set_rlm(fitr->second);
#endif

	const char *pp(from.data() + from.size() - 7);	 // FIXME: assumes supplied string is one complete message only
	if (*pp != '1' || *(pp + 1) != '0') // 10=XXX^A
		throw InvalidMessage(from, FILE_LINE);
	if (!no_chksum) // permit chksum calculation to be skipped
	{
		const f8String chksum(pp + 3, 3);
		msg->_trailer->get_check_sum()->set(chksum);
		const unsigned chkval(fast_atoi<unsigned>(chksum.c_str())), mchkval(calc_chksum(from, 0, static_cast<unsigned>(from.size()) - 7));
		if (chkval != mchkval)
			throw BadCheckSum(mchkval);
	}

	return msg;
}

//-------------------------------------------------------------------------------------------------
// copy all fields from this message to 'to' where the field is legal for 'to' and it is not
// already present in 'to'; includes repeating groups;
// a deep constructed target message is required
// if force, copy all fields regardless, replacing any existing, adding any new
unsigned MessageBase::copy_legal(MessageBase *to, bool force) const
{
	unsigned copied{};
	for (const auto& pp : _fp.get_presence())
	{
		if (pp._field_traits & FieldTrait::present && (force || (to->_fp.has(pp._fnum) && !to->_fp.get(pp._fnum))))
		{
			GroupBase *gb;
			if (pp._field_traits & FieldTrait::group && (gb = find_group(pp._fnum)))
			{
				GroupBase *gb1(to->find_group(pp._fnum));
				for (const auto *qq : gb->_msgs)
				{
					MessageBase *grc(gb1->create_group(true));
					copied += qq->copy_legal(grc, force);
					*gb1 += grc;
				}
			}

			BaseField *nf(get_field(pp._fnum)->copy());
#if defined FIX8_POPULATE_METADATA
			to->check_set_rlm(nf);
#endif
			Presence::const_iterator fpitr(to->_fp.get_presence().end());
			if (force && to->_fp.get(pp._fnum, fpitr, FieldTrait::present))
				delete to->replace(pp._fnum, fpitr, nf);
			else
				to->add_field(nf);
			++copied;
		}
	}

	return copied;
}

//-------------------------------------------------------------------------------------------------
// move all fields from this message to 'to' where the field is legal for 'to' and it is not
// already present in 'to'; includes repeating groups;
// if force, move all fields regardless, replacing any existing
// not thread safe
unsigned MessageBase::move_legal(MessageBase *to, bool force)
{
	unsigned moved{};
	for (const auto& pp : _fp.get_presence())
	{
		if (pp._field_traits & FieldTrait::present && (force || (to->_fp.has(pp._fnum) && !to->_fp.get(pp._fnum))))
		{
			if (pp._field_traits & FieldTrait::group)
			{
				auto gitr(_groups.find(pp._fnum));
				GroupBase *gb1(to->find_group(pp._fnum));
				if (gb1)
					delete to->replace(pp._fnum, gitr->second);
				else
					*to += gitr->second;
				gitr->second = nullptr;
			}

			auto itr(_fields.find(pp._fnum));
			Presence::const_iterator fpitr(_fp.get_presence().end());
			if (force && to->_fp.get(pp._fnum, fpitr, FieldTrait::present))
				delete to->replace(pp._fnum, fpitr, itr->second);
			else
				to->add_field(itr->second);
			itr->second = nullptr;
			++moved;
		}
	}
	clear_positions();

	return moved;
}

//-------------------------------------------------------------------------------------------------
size_t MessageBase::encode(char *to) const
{
	const char *where(to);
	for (const auto& pp : _pos)
	{
#if defined FIX8_POPULATE_METADATA
		check_set_rlm(pp.second);
#endif
		Presence::const_iterator fpitr(_fp.get_presence().end());
		if (!_fp.get(pp.second->_fnum, fpitr, FieldTrait::suppress))	// some fields are not encoded until unsuppressed (eg. checksum)
		{
			to += pp.second->encode(to);
			if (fpitr->_field_traits.has(FieldTrait::group) && has_group_count(pp.second))
				to += encode_group(pp.second->_fnum, to);
		}
	}

	if (_unknown.size())
		to += _unknown.copy(to, _unknown.size());

	return to - where;
}

//-------------------------------------------------------------------------------------------------
size_t MessageBase::encode(ostream& to) const
{
	const std::ios::pos_type where(to.tellp());
	for (const auto& pp : _pos)
	{
#if defined FIX8_POPULATE_METADATA
		check_set_rlm(pp.second);
#endif
		Presence::const_iterator fpitr(_fp.get_presence().end());
		if (!_fp.get(pp.second->_fnum, fpitr, FieldTrait::suppress))	// some fields are not encoded until unsuppressed (eg. checksum)
		{
			pp.second->encode(to);
			if (fpitr->_field_traits.has(FieldTrait::group) && has_group_count(pp.second))
				encode_group(pp.second->_fnum, to);
		}
	}

	if (_unknown.size())
		to << _unknown;

	return to.tellp() - where;
}

//-------------------------------------------------------------------------------------------------
size_t MessageBase::encode_group(const unsigned short fnum, char *to) const
{
	const char *where(to);
	GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum, FILE_LINE);
	for (const auto *pp : grpbase->_msgs)
		to += pp->encode(to);
	return to - where;
}

//-------------------------------------------------------------------------------------------------
size_t MessageBase::encode_group(const unsigned short fnum, std::ostream& to) const
{
	const std::ios::pos_type where(to.tellp());
	GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum, FILE_LINE);
	for (const auto *pp : grpbase->_msgs)
		pp->encode(to);
	return to.tellp() - where;
}

//-------------------------------------------------------------------------------------------------
/// Encode message with minimal copying
size_t Message::encode(char **hmsg_store) const
{
	char *moffs(*hmsg_store + HEADER_CALC_OFFSET), *msg(moffs);

#if defined FIX8_CODECTIMING
	IntervalTimer itm;
#endif

	if (!_header)
		throw MissingMessageComponent("header");
	_header->get_msg_type()->set(_msgType);

#if defined FIX8_RAW_MSG_SUPPORT
	msg += (_begin_payload = _header->encode(msg)); // start
#if defined FIX8_PREENCODE_MSG_SUPPORT
	if (_preencode_len)
	{
		::memcpy(msg, _preencode.data(), _payload_len =_preencode_len);
		msg += _preencode_len;
	}
	else
#endif
		msg += (_payload_len = MessageBase::encode(msg));
#else
	msg += _header->encode(msg); // start
#if defined FIX8_PREENCODE_MSG_SUPPORT
	if (_preencode_len)
	{
		::memcpy(msg, _preencode.data(), _preencode_len);
		msg += _preencode_len;
	}
	else
#endif
		msg += MessageBase::encode(msg);
#endif

	if (!_trailer)
		throw MissingMessageComponent("trailer");
	msg += _trailer->encode(msg);
	const size_t msgLen(msg - moffs); // checksummable msglength
	const size_t hlen(_ctx._preamble_sz +
		(msgLen < 10 ? 1 : msgLen < 100 ? 2 : msgLen < 1000 ? 3 : msgLen < 10000 ? 4 :
		 msgLen < 100000 ? 5 : msgLen < 1000000 ? 6 : 7));
	char *hmsg(moffs - hlen);
	*hmsg_store = hmsg;

	if (!_header->get_begin_string())
		throw MissingMandatoryField(Common_BeginString);
	_header->_fp.clear(Common_BeginString, FieldTrait::suppress);
	hmsg += _header->get_begin_string()->encode(hmsg);

	if (!_header->get_body_length())
		throw MissingMandatoryField(Common_BodyLength);
	_header->_fp.clear(Common_BodyLength, FieldTrait::suppress);

	_header->get_body_length()->set(static_cast<int>(msgLen));
	hmsg += _header->get_body_length()->encode(hmsg);

	if (!_trailer->get_check_sum())
		throw MissingMandatoryField(Common_CheckSum);
	_trailer->get_check_sum()->set(fmt_chksum(calc_chksum(moffs - hlen, msgLen + hlen)));
	_trailer->_fp.clear(Common_CheckSum, FieldTrait::suppress);
	msg += _trailer->get_check_sum()->encode(msg);

#if defined FIX8_CODECTIMING
	_encode_timings._cpu_used += itm.Calculate().AsDouble();
	++_encode_timings._msg_count;
#endif

	*msg = 0;
	const size_t rlen(msg - *hmsg_store);
#if defined FIX8_RAW_MSG_SUPPORT
	_rawmsg.assign(*hmsg_store, rlen);
#endif
	return rlen;
}

//-------------------------------------------------------------------------------------------------
size_t Message::encode(f8String& to) const
{
	char output[FIX8_MAX_MSG_LENGTH + HEADER_CALC_OFFSET], *ptr(output);
	const size_t msgLen(encode(&ptr));
	to.assign(ptr, msgLen);
	return to.size();
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print(ostream& os, int depth) const
{
	const string dspacer((depth + 1) * _tabsize, ' ');
   const BaseMsgEntry *tbme(_ctx._bme.find_ptr(_msgType.c_str()));
   if (tbme)
      os << string(depth * _tabsize, ' ') << tbme->_name << " (\"" << _msgType << "\")" << endl;
	for (const auto& pp : _pos)
	{
		const BaseEntry *tbe(_ctx.find_be(pp.second->_fnum));
		if (!tbe)
			throw InvalidField(pp.second->_fnum);
		os << dspacer << tbe->_name;
		const unsigned short comp(_fp.getComp(pp.second->_fnum));
		if (comp)
			os << " [" << _ctx._cn[comp] << ']';
		os << " (" << pp.second->_fnum << "): ";
		int idx;
		if (pp.second->_rlm && (idx = (pp.second->get_rlm_idx())) >= 0)
			os << pp.second->_rlm->_descriptions[idx] << " (" << *pp.second << ')' << endl;
		else
			os << *pp.second << endl;
		if (_fp.is_group(pp.second->_fnum) && has_group_count(pp.second))
			print_group(pp.second->_fnum, os, depth);
	}
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print_group(const unsigned short fnum, ostream& os, int depth) const
{
	const GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum, FILE_LINE);

	++depth;
	const string dspacer(depth * _tabsize, ' ');
	size_t cnt(1);
	for (const auto *pp : grpbase->_msgs)
	{
		os << dspacer << pp->_msgType << " (Repeating group " << cnt << '/' << grpbase->_msgs.size() << ')' << endl;
		pp->print(os, depth);
		++cnt;
	}
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print_field(const unsigned short fnum, ostream& os) const
{
	Fields::const_iterator fitr(_fields.find(fnum));
	if (fitr != _fields.end())
	{
		const BaseEntry *tbe(_ctx.find_be(fnum));
		if (!tbe)
			throw InvalidField(fnum);
		os << tbe->_name << " (" << fnum << "): ";
		int idx;
		if (fitr->second->_rlm && (idx = (fitr->second->get_rlm_idx())) >= 0)
			os << fitr->second->_rlm->_descriptions[idx] << " (" << *fitr->second << ')';
		else
			os << *fitr->second;
		if (_fp.is_group(fnum) && has_group_count(fitr->second))
			print_group(fnum, os, 0);
	}
}

//-------------------------------------------------------------------------------------------------
BaseField *MessageBase::replace(const unsigned short fnum, BaseField *with)
{
	BaseField *old(nullptr);
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
		_pos.insert({pos, with});
		itr->second = with;
		_fp.set(fnum, FieldTrait::present);
	}
	return old;
}

//-------------------------------------------------------------------------------------------------
BaseField *MessageBase::replace(const unsigned short fnum, Presence::const_iterator fitr, BaseField *with)
{
	BaseField *old(nullptr);
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
		_pos.insert({pos, with});
		itr->second = with;
		_fp.set(fnum, fitr, FieldTrait::present);
	}
	return old;
}

//-------------------------------------------------------------------------------------------------
BaseField *MessageBase::remove(const unsigned short fnum)
{
	BaseField *old(nullptr);
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
	BaseField *old(nullptr);
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
	const BaseMsgEntry& bme(_ctx._bme.find_ref(_msgType.c_str()));
	Message *msg(bme._create._do(true));
	// important not to pass force as true with copy_legal here
	copy_legal(msg);
	_header->copy_legal(msg->_header);
	_trailer->copy_legal(msg->_trailer);
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

//-------------------------------------------------------------------------------------------------
#if defined FIX8_CODECTIMING
void Message::format_codec_timings(const f8String& str, ostream& os, codec_timings& ct)
{
	os << str << ": " << setprecision(9) << ct._cpu_used << " secs, "
		<< setw(8) << right << ct._msg_count << " msgs, "
		<< (ct._cpu_used / ct._msg_count) << " secs/msg, "
		<< setprecision(2) << (ct._msg_count / ct._cpu_used) << " msgs/sec";
}

void Message::report_codec_timings(const f8String& tag)
{
	ostringstream ostr;
	ostr.setf(std::ios::showpoint);
	ostr.setf(std::ios::fixed);

	ostr << tag << ' ';
	format_codec_timings("Encode", ostr, _encode_timings);
	glout_info << ostr.str();

	ostr.str("");
	ostr << tag << ' ';
	format_codec_timings("Decode", ostr, _decode_timings);
	glout_info << ostr.str();
}
#endif

//-------------------------------------------------------------------------------------------------
GroupBase *MessageBase::replace(const unsigned short fnum, GroupBase *with)
{
	GroupBase *old(nullptr);
	auto itr(_groups.find(fnum));
	if (itr != _groups.end())
	{
		old = itr->second;
		itr->second = with;
	}
	return old;
}

