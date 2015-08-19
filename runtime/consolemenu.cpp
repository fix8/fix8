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
#include <fix8/consolemenu.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
const f8String ConsoleMenu::_opt_keys("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()_+-={}[]:\";'<>,?/|");
const f8String ConsoleMenu::_fld_prompt("Press ENTER for next page, '.' to Quit or type an option> ");

//-------------------------------------------------------------------------------------------------
const BaseMsgEntry *ConsoleMenu::SelectMsg() const
{
   for(;;)
   {
		const MsgTable::Pair *pp(_ctx._bme.begin());
		char opt(0);
      _os << endl;
      _os << "--------------------------------------------------" << endl;
      _os << " Select message to create" << endl;
      _os << "--------------------------------------------------" << endl;

		int page(0);
      for (unsigned nlines(0); pp != _ctx._bme.end(); ++pp)
      {
         _os << '[' << _opt_keys[nlines] << "]  " << pp->_value._name << '(' << pp->_key << ')' << endl;

			++nlines;
			if (nlines % _lpp == 0 || (nlines + _lpp * page) == _ctx._bme.size())
			{
				_os << "Page " << (page + 1) << '/' << (1 + (_ctx._bme.size() / _lpp)) << ' ';
            if ((opt = get_key(_fld_prompt, true)))
               break;
				++page;
				nlines = 0;
				_os << endl;
			}
      }

		size_t idx;
		if (opt)
		{
			if (opt == '.')
				return nullptr;

			if ((idx = _opt_keys.find_first_of(opt)) != f8String::npos)
			{
				idx += (page * _lpp);
				if (idx < _ctx._bme.size())
				{
					const MsgTable::Pair *pr(_ctx._bme.at(idx));
					if (pr)
						return &pr->_value;
				}
			}
		}
   }

	return nullptr;
}

//-------------------------------------------------------------------------------------------------
const FieldTable::Pair *ConsoleMenu::SelectField(const Message *msg, int grpid) const
{
	ostringstream ostr;
	if (grpid)
		ostr << msg->get_msgtype() << " (" << grpid << ')';
	else
		ostr << _ctx._bme.find_ptr(msg->get_msgtype().c_str())->_name;

   for(;;)
   {
		Presence::const_iterator itr(msg->get_fp().get_presence().begin());
		char opt(0);
		_os << endl;
		_os << "--------------------------------------------------" << endl;
		_os << ' ' << ostr.str() << ": Select field to add (*:mandatory +:present)" << endl;
		_os << "--------------------------------------------------" << endl;

		int page(0);
		for (unsigned nlines(0); itr != msg->get_fp().get_presence().end(); ++itr)
		{
			const BaseEntry *tbe(_ctx.find_be(itr->_fnum));
			_os << '[' << _opt_keys[nlines] << "] ";
			if (msg->have(itr->_fnum))
			{
				_os << '+';
				msg->print_field(itr->_fnum, _os);
				_os << endl;
			}
			else
				_os << (msg->get_fp().is_mandatory(itr->_fnum) ? '*' : ' ')
					<< tbe->_name << '(' << itr->_fnum << ')' << endl;

			++nlines;

			if (nlines % _lpp == 0 || (nlines + _lpp * page) == msg->get_fp().get_presence().size())
			{
				_os << "Page " << (page + 1) << '/' << (1 + (msg->get_fp().get_presence().size() / _lpp)) << ' ';
				if ((opt = get_key(_fld_prompt, true)))
					break;
				++page;
				nlines = 0;
				_os << endl;
			}
		}

		size_t idx;
		if (opt)
		{
			if (opt == '.')
				return nullptr;

			if ((idx = _opt_keys.find_first_of(opt)) != f8String::npos)
			{
				idx += (page * _lpp);
				Presence::const_iterator fitr(msg->get_fp().get_presence().at(idx));
				if (fitr != msg->get_fp().get_presence().end())
					return _ctx._be.find_pair_ptr(fitr->_fnum);
			}
		}
	}

	return nullptr;
}

//-------------------------------------------------------------------------------------------------
int ConsoleMenu::SelectRealm(const unsigned short fnum, const RealmBase *rb) const
{
	const BaseEntry *be(_ctx.find_be(fnum));

	for(;;)
	{
		int pp(0);
		char opt(0);

		_os << endl;
		_os << "--------------------------------------------------" << endl;
		_os << ' ' << be->_name << ": Select realm value to add" << endl;
		_os << "--------------------------------------------------" << endl;

		int page(0);
		for (int nlines(0); pp < rb->_sz; ++pp)
		{
			_os << '[' << _opt_keys[nlines] << "]  " << *(rb->_descriptions + pp) << " (";
			rb->print(_os, pp);
			_os << ')' << endl;

			++nlines;
			if (nlines % _lpp == 0 || (nlines + _lpp * page) == rb->_sz)
			{
				_os << "Page " << (page + 1) << '/' << (1 + (rb->_sz / _lpp)) << ' ';
				if ((opt = get_key(_fld_prompt, true)))
					break;
				++page;
				nlines = 0;
				_os << endl;
			}
		}

		int idx;
		if (opt)
		{
			if (opt == '.')
				return 0;

			if (static_cast<size_t>((idx = (static_cast<int>(_opt_keys.find_first_of(opt))))) != f8String::npos)
			{
				idx += (page * _lpp);
				if (idx < rb->_sz)
					return idx;
			}
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
Message *ConsoleMenu::SelectFromMsg(MsgList& lst) const
{
	if (lst.empty())
		return nullptr;

   for(;;)
   {
		MsgList::const_iterator itr(lst.begin());
		char opt(0);
		_os << endl;
		_os << "--------------------------------------------------" << endl;
		_os << "Select from " << lst.size() << " messages" << endl;
		_os << "--------------------------------------------------" << endl;

		int page(0);
		for (unsigned nlines(0); itr != lst.end(); ++itr)
		{
			const MsgTable::Pair *tbme(_ctx._bme.find_pair_ptr((*itr)->get_msgtype().c_str()));
			text txt;
			(*itr)->get(txt);
         _os << '[' << _opt_keys[nlines] << "]  " << tbme->_value._name << '(' << tbme->_key << ")\t" << txt() << endl;

			++nlines;
			if (nlines % _lpp == 0 || (nlines + _lpp * page) == lst.size())
			{
				_os << "Page " << (page + 1) << ' ';
				if ((opt = get_key(_fld_prompt, true)))
					break;
				++page;
				nlines = 0;
				_os << endl;
			}
		}

		size_t idx;
		if (opt)
		{
			if (opt == '.')
				return nullptr;

			if ((idx = _opt_keys.find_first_of(opt)) != f8String::npos)
			{
				idx += (page * _lpp);
				if (idx < lst.size())
					return lst[idx];
			}
		}
	}

	return nullptr;
}

//-------------------------------------------------------------------------------------------------
int ConsoleMenu::CreateMsgs(tty_save_state& tty, MsgList& lst) const
{
	for (;;)
	{
		const BaseMsgEntry *mc(SelectMsg());
		if (!mc)
			break;
		Message *msg(mc->_create._do(true));
		const FieldTable::Pair *fld;
		while((fld = SelectField(msg)))
			EditMsg(tty, fld, msg);
		_os << endl << endl << *static_cast<MessageBase *>(msg) << endl;
		if (get_yn("Add to list? (y/n):", true))
			lst.push_back(msg);
	}

	return static_cast<int>(lst.size());
}

//-------------------------------------------------------------------------------------------------
f8String& ConsoleMenu::GetString(tty_save_state& tty, f8String& to) const
{
	char buff[128] {};
	tty.unset_raw_mode();
	_is.getline(buff, sizeof(buff));
	tty.set_raw_mode();
	return to = buff;
}

//-------------------------------------------------------------------------------------------------
void ConsoleMenu::EditMsg(tty_save_state& tty, const FieldTable::Pair *fld, Message *msg) const
{
	string txt;
	int rval(-1);
	if (fld->_value._rlm)
		rval = SelectRealm(fld->_key, fld->_value._rlm);
	else
	{
		_os << endl << fld->_value._name << ": " << flush;
		GetString(tty, txt);
		if (msg->get_fp().is_group(fld->_key))
		{
			int cnt(stoi(txt));
			GroupBase *gb(msg->find_group(fld->_key));
			if (gb && cnt)
			{
				for (int ii(0); ii < cnt; ++ii)
				{
					Message *gmsg(static_cast<Message *>(gb->create_group(true)));
					const FieldTable::Pair *fld;
					while((fld = SelectField(gmsg, ii + 1)))
						EditMsg(tty, fld, gmsg);
					_os << endl << endl << *static_cast<MessageBase *>(gmsg) << endl;
					if (get_yn("Add group to msg? (y/n):", true))
						*gb += gmsg;
				}
			}
		}
	}

	BaseField *bf(fld->_value._create._do(txt.c_str(), fld->_value._rlm, rval));
	msg->add_field(bf->get_tag(), msg->get_fp().get_presence().end(), 0, bf, true);
}

//-------------------------------------------------------------------------------------------------
int ConsoleMenu::EditMsgs(tty_save_state& tty, MsgList& lst) const
{
	for (;;)
	{
		Message *msg(SelectFromMsg(lst));
		if (!msg)
			break;
		const FieldTable::Pair *fld;
		while((fld = SelectField(msg)))
			EditMsg(tty, fld, msg);
		_os << endl << endl << *static_cast<MessageBase *>(msg) << endl;
	}

	return static_cast<int>(lst.size());
}

//-------------------------------------------------------------------------------------------------
Message *ConsoleMenu::RemoveMsg(tty_save_state& tty, MsgList& lst) const
{
	Message *msg(SelectFromMsg(lst));
	if (msg)
	{
		for (MsgList::iterator itr(lst.begin()); itr != lst.end(); ++itr)
		{
			if (*itr == msg)
			{
				_os << endl;
				if (get_yn("Remove msg from list? (y/n, n=return a copy):", true))
				{
					lst.erase(itr);
					return msg;
				}
				return msg->clone();
			}
		}
	}

	return nullptr;;
}

//-------------------------------------------------------------------------------------------------
int ConsoleMenu::DeleteAllMsgs(tty_save_state& tty, MsgList& lst) const
{
	if (lst.size() && get_yn("Delete all msgs? (y/n):", true))
	{
		for_each(lst.begin(), lst.end(), [](const Message *pp){ delete pp; });
		lst.clear();
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
int ConsoleMenu::DeleteMsgs(tty_save_state& tty, MsgList& lst) const
{
	for (;;)
	{
		Message *msg(SelectFromMsg(lst));
		if (!msg)
			break;
		for (MsgList::iterator itr(lst.begin()); itr != lst.end(); ++itr)
		{
			if (*itr == msg)
			{
				if (get_yn(" Delete msg? (y/n):", true))
				{
					delete *itr;
					lst.erase(itr);
				}
				break;
			}
		}
	}

	return static_cast<int>(lst.size());
}

