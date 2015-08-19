//-------------------------------------------------------------------------------------------------
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
#ifndef FIX8_MYFIX_HPP_
#define FIX8_MYFIX_HPP_

//-----------------------------------------------------------------------------------------
class myfix_session_client;

/// Example client message router. Derives from fix8 generated router class.
/*! Your application must define a class similar to this in order to receive
    the appropriate callback when Message::process is called. */
class tex_router_client : public FIX8::TEX::Myfix_Router
{
	myfix_session_client& _session;

public:
	/*! Ctor.
	    \param session client session */
	tex_router_client(myfix_session_client& session) : _session(session) {}

	/*! Execution report handler. Here is where you provide your own methods for the messages you wish to
		 handle. Only those messages that are of interest to you need to be implemented.
	    \param msg Execution report message session */
	virtual bool operator() (const FIX8::TEX::ExecutionReport *msg) const;
};

/// Example client session. Derives from FIX8::Session.
/*! Your application must define a class similar to this in order to create and connect a client.
    You must also implement handle_application in order to receive application messages from the framework. */
class myfix_session_client : public FIX8::Session
{
	tex_router_client _router;

public:
	/*! Ctor. Initiator.
	    \param ctx reference to generated metadata
	    \param sid sessionid of connecting session
		 \param persist persister for this session
		 \param logger logger for this session
		 \param plogger protocol logger for this session */
	myfix_session_client(const FIX8::F8MetaCntx& ctx, const FIX8::SessionID& sid, FIX8::Persister *persist=nullptr,
		FIX8::Logger *logger=nullptr, FIX8::Logger *plogger=nullptr) : Session(ctx, sid, persist, logger, plogger), _router(*this) {}

	/*! Application message callback.
	    This method is called by the framework when an application message has been received and decoded. You
	  	 should implement this method and call the supplied Message::process.
	    \param seqnum Fix sequence number of the message
		 \param msg Mesage decoded (base ptr)
		 \return true on success */
	bool handle_application(const unsigned seqnum, const FIX8::Message *&msg);

	/*! This method id called whenever a session state change occurs
	    \param before previous session state
	    \param after new session state */
	void state_change(const FIX8::States::SessionStates before, const FIX8::States::SessionStates after);
};

//-----------------------------------------------------------------------------------------
class myfix_session_server;

/// Example server message router. Derives from fix8 generated router class.
/*! Your application must define a class similar to this in order to receive
    the appropriate callback when Message::process is called. */
class tex_router_server : public FIX8::TEX::Myfix_Router
{
	myfix_session_server& _session;

public:
	/*! Ctor.
	    \param session server session */
	tex_router_server(myfix_session_server& session) : _session(session) {}

	/*! NewOrderSingle message handler. Here is where you provide your own methods for the messages you wish to
		 handle. Only those messages that are of interest to you need to be implemented.
	    \param msg NewOrderSingle message */
	virtual bool operator() (const FIX8::TEX::NewOrderSingle *msg) const;
};

/// Example server session. Derives from FIX8::Session.
/*! Your application must define a class similar to this in order to receive client connections.
    You must also implement handle_application in order to receive application messages from the framework. */
class myfix_session_server : public FIX8::Session
{
	tex_router_server _router;

public:
	/*! Ctor. Acceptor.
	    \param ctx reference to generated metadata
	    \param sci sender comp id of hosting session
		 \param persist persister for this session
		 \param logger logger for this session
		 \param plogger protocol logger for this session */
	myfix_session_server(const FIX8::F8MetaCntx& ctx, const FIX8::sender_comp_id& sci, FIX8::Persister *persist=nullptr,
		FIX8::Logger *logger=nullptr, FIX8::Logger *plogger=nullptr) : Session(ctx, sci, persist, logger, plogger),
		_router(*this) {}

	/*! Application message callback. This method is called by the framework when an application message has been received and decoded.
	    You should implement this method and call the supplied Message::process.
	    \param seqnum Fix sequence number of the message
		 \param msg Mesage decoded (base ptr)
		 \return true on success */
	bool handle_application(const unsigned seqnum, const FIX8::Message *&msg);

	/*! example scheduler callback function
	  	\return true if ok */
	bool sample_scheduler_callback();

	/*! This method id called whenever a session state change occurs
	    \param before previous session state
	    \param after new session state */
	void state_change(const FIX8::States::SessionStates before, const FIX8::States::SessionStates after);
};

//-------------------------------------------------------------------------------------------------
/// Simple menu system that will work with most term types.
class MyMenu
{
	FIX8::tty_save_state _tty;
	FIX8::MsgList _lst;
	FIX8::ConsoleMenu *_cm;

	/// Individual menu item.
	struct MenuItem
	{
		const char _key = 0;
		const std::string _help;

		MenuItem(char key, const std::string help=std::string()) : _key(key), _help(help) {}
		MenuItem() = default;
		bool operator() (const MenuItem& a, const MenuItem& b) const { return a._key < b._key; }
	};

	myfix_session_client& _session;
	std::istream _istr;
	std::ostream& _ostr;

	using Handlers = std::map<const MenuItem, bool (MyMenu::*)(), MenuItem>;
	static const Handlers _handlers;

public:
	MyMenu(FIX8::Session& session, int infd, std::ostream& ostr, FIX8::ConsoleMenu *cm=nullptr)
		: _tty(infd), _cm(cm), _session(static_cast<myfix_session_client&>(session)),
		_istr(new FIX8::fdinbuf(infd)), _ostr(ostr) {}
	virtual ~MyMenu() {}

	std::istream& get_istr() { return _istr; }
	std::ostream& get_ostr() { return _ostr; }
	bool process(char ch)
	{
		auto itr(_handlers.find({ch}));
		if (itr == _handlers.end())
		{
			_ostr << "Command not found";
			return true;
		}
		return (this->*itr->second)();
	}

	bool new_order_single();
	bool new_order_single_50();
	bool new_order_single_1000();
	bool resend_request();
	bool help();
	bool do_exit() { return false; }
	bool do_logout();
	bool create_msgs();
	bool version_info();
	bool edit_msgs();
	bool delete_msg();
	bool delete_msgs();
	bool print_msgs();
	bool send_msgs();
	bool send_msg();
	bool write_msgs();
	bool read_msgs();
	bool set_lpp();
	bool toggle_heartbeats();
	bool static_probe();
	bool new_order_single_alternate();
	bool new_order_single_recycled();

	bool load_msgs(const std::string& fname);
	FIX8::Message *generate_new_order_single();
	FIX8::Message *generate_new_order_single_alternate();
	void send_lst();
	bool save_msg(const std::string& fname, FIX8::Message *msg);
	unsigned get_msg_cnt() const { return _lst.size(); }

	FIX8::tty_save_state& get_tty() { return _tty; }

	FIX8::f8String& get_string(FIX8::f8String& to)
	{
		char buff[128] {};
		_tty.unset_raw_mode();
		_istr.getline(buff, sizeof(buff));
		_tty.set_raw_mode();
		return to = buff;
	}
};

//-----------------------------------------------------------------------------------------
/// A random number generator wrapper.
struct RandDev
{
	static void init()
	{
		time_t tval(time(0));
#ifdef _MSC_VER
	   srand (static_cast<unsigned>(((tval % _getpid()) * tval)));
#else
		srandom (static_cast<unsigned>(((tval % getpid()) * tval)));
#endif
	}

	template<typename T>
   static T getrandom(const T range=0)
   {
#ifdef _MSC_VER
       T target(rand());
#else
       T target(random());
#endif
       return range ? target / (RAND_MAX / range + 1) : target;
	}
};

#endif // FIX8_MYFIX_HPP_

