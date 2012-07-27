//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

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
#ifndef _FIX8_MYFIX_HPP_
#define _FIX8_MYFIX_HPP_

//-----------------------------------------------------------------------------------------
class hf_session_client;

/// Example client message router. Derives from fix8 generated router class.
/*! Your application must define a class similar to this in order to receive
    the appropriate callback when Message::process is called. */
class tex_router_client : public FIX8::TEX::Perf_Router
{
	hf_session_client& _session;

public:
	/*! Ctor.
	    \param session client session */
	tex_router_client(hf_session_client& session) : _session(session) {}

	/*! Execution report handler. Here is where you provide your own methods for the messages you wish to
		 handle. Only those messages that are of interest to you need to be implemented.
	    \param msg Execution report message session */
	virtual bool operator() (const FIX8::TEX::ExecutionReport *msg) const;
};

/// Example client session. Derives from FIX8::Session.
/*! Your application must define a class similar to this in order to create and connect a client.
    You must also implement handle_application in order to receive application messages from the framework. */
class hf_session_client : public FIX8::Session
{
	tex_router_client _router;
	typedef std::queue<FIX8::TEX::NewOrderSingle *> Nos_queue;
	Nos_queue _nos_queue;

public:
	/*! Ctor. Initiator.
	    \param ctx reference to generated metadata
	    \param sid sessionid of connecting session
		 \param persist persister for this session
		 \param logger logger for this session
		 \param plogger protocol logger for this session */
	hf_session_client(const FIX8::F8MetaCntx& ctx, const FIX8::SessionID& sid, FIX8::Persister *persist=0,
		FIX8::Logger *logger=0, FIX8::Logger *plogger=0) : Session(ctx, sid, persist, logger, plogger), _router(*this) {}

	/*! Application message callback.
	    This method is called by the framework when an application message has been received and decoded. You
	  	 should implement this method and call the supplied Message::process.
	    \param seqnum Fix sequence number of the message
		 \param msg Mesage decoded (base ptr)
		 \return true on success */
	bool handle_application(const unsigned seqnum, const FIX8::Message *msg);

	void push(FIX8::TEX::NewOrderSingle *nos) { _nos_queue.push(nos); }
	FIX8::TEX::NewOrderSingle *pop()
	{
		if (_nos_queue.size())
		{
			FIX8::TEX::NewOrderSingle *nos(_nos_queue.front());
			_nos_queue.pop();
			return nos;
		}
		return 0;
	}

	int cached() const { return _nos_queue.size(); }
};

//-----------------------------------------------------------------------------------------
class hf_session_server;

/// Example server message router. Derives from fix8 generated router class.
/*! Your application must define a class similar to this in order to receive
    the appropriate callback when Message::process is called. */
class tex_router_server : public FIX8::TEX::Perf_Router
{
	hf_session_server& _session;

public:
	/*! Ctor.
	    \param session server session */
	tex_router_server(hf_session_server& session) : _session(session) {}

	/*! NewOrderSingle message handler. Here is where you provide your own methods for the messages you wish to
		 handle. Only those messages that are of interest to you need to be implemented.
	    \param msg NewOrderSingle message */
	virtual bool operator() (const FIX8::TEX::NewOrderSingle *msg) const;
};

/// Example server session. Derives from FIX8::Session.
/*! Your application must define a class similar to this in order to receive client connections.
    You must also implement handle_application in order to receive application messages from the framework. */
class hf_session_server : public FIX8::Session
{
	tex_router_server _router;

public:
	/*! Ctor. Acceptor.
	    \param ctx reference to generated metadata
		 \param persist persister for this session
		 \param logger logger for this session
		 \param plogger protocol logger for this session */
	hf_session_server(const FIX8::F8MetaCntx& ctx, FIX8::Persister *persist=0,
		FIX8::Logger *logger=0, FIX8::Logger *plogger=0) : Session(ctx, persist, logger, plogger),
		_router(*this) {}

	/*! Application message callback. This method is called by the framework when an application message has been received and decoded.
	    You should implement this method and call the supplied Message::process.
	    \param seqnum Fix sequence number of the message
		 \param msg Mesage decoded (base ptr)
		 \return true on success */
	bool handle_application(const unsigned seqnum, const FIX8::Message *msg);
};

//-------------------------------------------------------------------------------------------------
/// Simple menu system that will work with most term types.
class MyMenu
{
	FIX8::tty_save_state _tty;

	/// Individual menu item.
	struct MenuItem
	{
		const char _key;
		const std::string _help;

		MenuItem(const char key, const std::string& help) : _key(key), _help(help) {}
		MenuItem() : _key(), _help() {}
		bool operator() (const MenuItem& a, const MenuItem& b) const { return a._key < b._key; }
	};

	hf_session_client& _session;
	std::istream _istr;
	std::ostream& _ostr;

	typedef FIX8::StaticTable<const MenuItem, bool (MyMenu::*)(), MenuItem> Handlers;
	Handlers _handlers;

public:
	MyMenu(hf_session_client& session, int infd, std::ostream& ostr)
		: _tty(infd), _session(session), _istr(new FIX8::fdinbuf(infd)), _ostr(ostr) {}
	virtual ~MyMenu() {}

	std::istream& get_istr() { return _istr; }
	std::ostream& get_ostr() { return _ostr; }
	bool process(const char ch) { return (this->*_handlers.find_ref(MenuItem(ch, std::string())))(); }

	bool new_order_single();
	bool preload_new_order_single();
	bool send_all_preloaded();
	bool help();
	bool nothing() { return true; }
	bool do_exit() { return false; }
	bool do_logout();
	bool flush_log();

	FIX8::tty_save_state& get_tty() { return _tty; }

	friend class FIX8::StaticTable<const MenuItem, bool (MyMenu::*)(), MenuItem>;
};

//-----------------------------------------------------------------------------------------
/// A random number generator wrapper.
struct RandDev
{
	static void init()
	{
		time_t tval(time(0));
		srandom (static_cast<unsigned>(((tval % getpid()) * tval)));
	}

	template<typename T>
   static T getrandom(const T range=0)
   {
		T target(random());
		return range ? target / (RAND_MAX / range + 1) : target;
	}
};

#endif // _FIX8_MYFIX_HPP_

