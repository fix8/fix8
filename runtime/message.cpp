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

#include <f8exception.hpp>
#include <f8types.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <message.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
RegExp MessageBase::_elmnt("([0-9]+)=([^\x01]+)\x01");

//-------------------------------------------------------------------------------------------------
namespace {
	const string spacer(3, ' ');
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode(const f8String& from, const unsigned offset)
{
	RegMatch match;
	unsigned s_offset(offset);

	for (unsigned pos(0); s_offset < from.size() && _elmnt.SearchString(match, from, 3, s_offset) == 3; )
	{
		f8String tag, val, all;
		_elmnt.SubExpr(match, from, all, s_offset, 0);
		_elmnt.SubExpr(match, from, tag, s_offset, 1);
		_elmnt.SubExpr(match, from, val, s_offset, 2);
		cout << all << endl;
		const unsigned tv(GetValue<unsigned>(tag));
		const BaseEntry *be(_ctx._be.find_ptr(tv));
		if (!be || !_fp.has(tv))
			break;
			//throw InvalidField(tv);
		s_offset += match.SubSize();
		if (_fp.get(tv))
		{
			if (!_fp.get(tv, FieldTrait::automatic))
				throw DuplicateField(tv);
		}
		else
		{
			auto_ptr<BaseField> fld(be->_create(val, be->_dom));
			add_field(tv, ++pos, fld.release());
			if (_fp.is_group(tv))
				s_offset = decode_group(tv, from, s_offset);
		}

		//cout << s_offset << ':' << from.size() << endl;
	}

	//const unsigned short missing(_fp.find_missing());
	//if (missing)
	//	throw MissingMandatoryField(missing);

	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode_group(const unsigned short fnum, const f8String& from, const unsigned offset)
{
	cout << "decode_group" << endl;
	unsigned s_offset(offset);
	GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum);

	bool ok(true);
	for (; ok && s_offset < from.size(); )
	{
		RegMatch match;
		auto_ptr<MessageBase> grp(grpbase->create_group());

		for (unsigned pos(0); s_offset < from.size() && _elmnt.SearchString(match, from, 3, s_offset) == 3; )
		{
			f8String tag, val;
			_elmnt.SubExpr(match, from, tag, s_offset, 1);
			_elmnt.SubExpr(match, from, val, s_offset, 2);
			const unsigned tv(GetValue<unsigned>(tag));
			if (grp->_fp.get(tv))	// already present; next group?
				break;
			if (pos == 0 && grp->_fp.getPos(tv) != 1)	// first field in group is mandatory
				throw MissingRepeatingGroupField(tv);
			const BaseEntry *be(_ctx._be.find_ptr(tv));
			if (!be)	// field not found in sub-group - end of repeats?
			{
				ok = false;
				break;
			}
			s_offset += match.SubSize();
			grp->add_field(tv, ++pos, be->_create(val, be->_dom));
			grp->_fp.set(tv);	// is present
			*grpbase += grp.release();
			cout << "ag:" << tv << endl;
		}

		//const unsigned short missing(grp->_fp.find_missing());
		//if (missing)
		//	throw MissingMandatoryField(missing);
	}

	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::check_positions()
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
unsigned Message::decode(const f8String& from)
{
	unsigned offset(_header->decode(from, 0));
	offset = MessageBase::decode(from, offset);
	offset = _trailer->decode(from, offset);
	return offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::encode(ostream& to)
{
	const std::ios::pos_type where(to.tellp());
	for (Positions::const_iterator itr(_pos.begin()); itr != _pos.end(); ++itr)
	{
		const BaseEntry& tbe(_ctx._be.find_ref(itr->second->_fnum));
		if (tbe._dom && !itr->second->_dom)
			itr->second->_dom = tbe._dom;	// populate dom for outbounds
		if (!_fp.get(itr->second->_fnum, FieldTrait::suppress))	// some fields are not encoded until unsuppressed (eg. checksum)
		{
			itr->second->encode(to);
			if (_fp.get(itr->second->_fnum, FieldTrait::group))
				encode_group(itr->second->_fnum, to);
		}
	}

	return to.tellp() - where;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::encode_group(const unsigned short fnum, std::ostream& to)
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
unsigned Message::encode(f8String& to)
{
	ostringstream msg;
	if (!_header)
		throw MissingMessageComponent("header");
	Fields::const_iterator fitr(_header->_fields.find(Magic_MsgType));
	static_cast<Field<f8String, Magic_MsgType> *>(fitr->second)->set(_msgType);
	_header->encode(msg);
	MessageBase::encode(msg);
	if (!_trailer)
		throw MissingMessageComponent("trailer");
	_trailer->encode(msg);
	const unsigned msgLen(msg.str().size());	// checksummable msglength

	if ((fitr = _trailer->_fields.find(Magic_CheckSum)) == _trailer->_fields.end())
		throw MissingMandatoryField(Magic_CheckSum);
	f8String chksum;
	static_cast<Field<f8String, Magic_CheckSum> *>(fitr->second)->set(PutValue(calc_chksum(msg.str()), chksum));
	_trailer->_fp.clear(Magic_CheckSum, FieldTrait::suppress);
	fitr->second->encode(msg);

	ostringstream hmsg;
	if ((fitr = _header->_fields.find(Magic_BeginString)) == _header->_fields.end())
		throw MissingMandatoryField(Magic_BeginString);
	_header->_fp.clear(Magic_BeginString, FieldTrait::suppress);
	fitr->second->encode(hmsg);

	if ((fitr = _header->_fields.find(Magic_BodyLength)) == _header->_fields.end())
		throw MissingMandatoryField(Magic_BodyLength);
	_header->_fp.clear(Magic_BodyLength, FieldTrait::suppress);
	static_cast<Field<Length, Magic_BodyLength> *>(fitr->second)->set(msgLen);
	fitr->second->encode(hmsg);

	hmsg << msg.str();
	to = hmsg.str();
	return to.size();
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print(ostream& os)
{
	const BaseMsgEntry *tbme(_ctx._bme.find_ptr(_msgType));
	if (tbme)
		os << tbme->_name << " (\"" << _msgType << "\")" << endl;
	for (Positions::const_iterator itr(_pos.begin()); itr != _pos.end(); ++itr)
	{
		const BaseEntry& tbe(_ctx._be.find_ref(itr->second->_fnum));
		os << spacer << tbe._name << " (" << itr->second->_fnum << "): ";
		int idx;
		if (itr->second->_dom && (idx = (itr->second->get_dom_idx())) >= 0)
			os << itr->second->_dom->_descriptions[idx] << " (" << *itr->second << ')' << endl;
		else
			os << *itr->second << endl;
		if (_fp.is_group(itr->second->_fnum))
			print_group(itr->second->_fnum, os);
	}
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print_group(const unsigned short fnum, ostream& os)
{
	GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum);
	for (GroupElement::iterator itr(grpbase->_msgs.begin()); itr != grpbase->_msgs.end(); ++itr)
	{
		os << spacer << spacer << "Repeating group" << endl << spacer << spacer;
		(*itr)->print(os);
	}
}

//-------------------------------------------------------------------------------------------------
void Message::print(ostream& os)
{
	if (_header)
		_header->print(os);
	else
		os << "Null Header" << endl;
	MessageBase::print(os);
	if (_trailer)
		_trailer->print(os);
	else
		os << "Null Trailer" << endl;
}

