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
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_CONSOLEMENU_HPP_
#define FIX8_CONSOLEMENU_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
using MsgList = std::deque<FIX8::Message *>;

//-------------------------------------------------------------------------------------------------
/// Console test harness menu
class ConsoleMenu
{
	const F8MetaCntx& _ctx;
	std::istream& _is;
	std::ostream& _os;
	int _lpp;
	static const f8String _opt_keys, _fld_prompt;

public:
	/*! Ctor
	 \param ctx - reference to generated metadata
	 \param is - reference to input stream
	 \param os - reference to output stream
	 \param lpp - lines to print per page */
	ConsoleMenu (const F8MetaCntx& ctx, std::istream& is=std::cin, std::ostream& os=std::cout, int lpp=20)
		: _ctx(ctx), _is(is), _os(os), _lpp(lpp) {}

	/// Dtor.
	virtual ~ConsoleMenu () {}

	F8API virtual const BaseMsgEntry *SelectMsg() const;
	F8API virtual const FieldTable::Pair *SelectField(const Message *msg, int groupid = 0) const;
	F8API virtual int SelectRealm(const unsigned short fnum, const RealmBase *rb) const;
	F8API Message *SelectFromMsg(MsgList& lst) const;
	F8API virtual int CreateMsgs(tty_save_state& tty, MsgList& lst) const;
	F8API Message *RemoveMsg(tty_save_state& tty, MsgList& lst) const;
	F8API void EditMsg(tty_save_state& tty, const FieldTable::Pair *fld, Message *msg) const;
	F8API virtual int EditMsgs(tty_save_state& tty, MsgList& lst) const;
	F8API virtual int DeleteMsgs(tty_save_state& tty, MsgList& lst) const;
	F8API virtual int DeleteAllMsgs(tty_save_state& tty, MsgList& lst) const;

	int get_lpp() const { return _lpp; }
	void set_lpp(int lpp) { _lpp = lpp; }

	bool get_yn(const f8String& prompt, bool echo=false) const { return toupper(get_key(prompt, echo)) == 'Y'; }
	char get_key(const f8String& prompt=std::string(), bool echo=false) const
	{
		char ch(0);
		_is.clear();
		if (!prompt.empty())
			_os << prompt << std::flush;
		_is.get(ch);
		if (!_is.bad() && ch != 0x3 && ch != 0xa)
		{
			if (echo)
				_os << ch;
			return ch;
		}
		return 0;
	}
	F8API f8String& GetString(tty_save_state& tty, f8String& to) const;
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // FIX8_CONSOLEMENU_HPP_
