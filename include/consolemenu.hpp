//-----------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of
	 	conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list
	 	of conditions and the following disclaimer in the documentation and/or other
		materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to
	 	endorse or promote products derived from this software without specific prior
		written permission.
    * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
	   in their name without written permission from fix8.org

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_CONSOLEMENU_HPP_
#define _FIX8_CONSOLEMENU_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
typedef std::deque<FIX8::Message *> MsgList;

//-------------------------------------------------------------------------------------------------
/// Console test harness menu
class ConsoleMenu
{
	const F8MetaCntx& _ctx;
	Session *_ses;
	std::istream& _is;
	std::ostream& _os;
	const int _lpp;
	static const f8String _opt_keys, _fld_prompt;

public:
	/*! Ctor
	 \param ctx - reference to generated metadata
	 \param ses - pointer to session
	 \param is - reference to input stream
	 \param os - reference to output stream
	 \param lpp - lines to print per page */
	ConsoleMenu (const F8MetaCntx& ctx, Session *ses, std::istream& is=std::cin, std::ostream& os=std::cout, const int lpp=20)
		: _ctx(ctx), _ses(ses), _is(is), _os(os), _lpp(lpp) {}

	/// Dtor.
	virtual ~ConsoleMenu () {}

	virtual const BaseMsgEntry *SelectMsg() const;
	virtual const FieldTable::Pair *SelectField(const Message *msg, int groupid=0) const;
	virtual int SelectRealm(const unsigned short fnum, const RealmBase *rb) const;
	Message *SelectFromMsg(MsgList& lst) const;
	virtual int CreateMsgs(tty_save_state& tty, MsgList& lst) const;
	void EditMsg(tty_save_state& tty, const FieldTable::Pair *fld, Message *msg) const;
	virtual int EditMsgs(tty_save_state& tty, MsgList& lst) const;
	virtual int DeleteMsgs(tty_save_state& tty, MsgList& lst) const;

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
	f8String& GetString(tty_save_state& tty, f8String& to) const;
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_CONSOLEMENU_HPP_
