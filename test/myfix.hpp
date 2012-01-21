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
class myfix_session_client;

/*! Example client message router. Derives from fix8 generated router class.
    Your application must define a class similar to this in order to receive
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

/*! Example client session. Derives from FIX8::Session.
    Your application must define a class similar to this in order to create and connect a client.
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
	myfix_session_client(const FIX8::F8MetaCntx& ctx, const FIX8::SessionID& sid, FIX8::Persister *persist=0,
		FIX8::Logger *logger=0, FIX8::Logger *plogger=0) : Session(ctx, sid, persist, logger, plogger), _router(*this) {}

	/*! Application message callback.
	    This method is called by the framework when an application message has been received and decoded. You
	  	 should implement this method and call the supplied Message::process.
	    \param seqnum Fix sequence number of the message
		 \param msg Mesage decoded (base ptr)
		 \return true on success */
	bool handle_application(const unsigned seqnum, const FIX8::Message *msg);

#if defined PERMIT_CUSTOM_FIELDS
	/*! Post message constructor.
	    If you have enabled custom field capability then you need to provde an implementation for this method.
	    It is called immediately after the framework has constructed a message from the metadata. You should then add your
		 custom field traits.
		 \param msg Mesage decoded (base ptr)
		 \return true on success */
	bool post_msg_ctor(FIX8::Message *msg);
#endif
};

//-----------------------------------------------------------------------------------------
class myfix_session_server;

/*! Example server message router. Derives from fix8 generated router class.
    Your application must define a class similar to this in order to receive
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
	    \param msg NewOrderSingle report message session */
	virtual bool operator() (const FIX8::TEX::NewOrderSingle *msg) const;
};

class myfix_session_server : public FIX8::Session
{
	tex_router_server _router;

public:
	/*! Ctor. Acceptor.
	    \param ctx reference to generated metadata
		 \param persist persister for this session
		 \param logger logger for this session
		 \param plogger protocol logger for this session */
	myfix_session_server(const FIX8::F8MetaCntx& ctx, FIX8::Persister *persist=0,
		FIX8::Logger *logger=0, FIX8::Logger *plogger=0) : Session(ctx, persist, logger, plogger), _router(*this) {}

	/*! Application message callback. This method is called by the framework when an application message has been received and decoded.
	    You should implement this method and call the supplied Message::process.
	    \param seqnum Fix sequence number of the message
		 \param msg Mesage decoded (base ptr)
		 \return true on success */
	bool handle_application(const unsigned seqnum, const FIX8::Message *msg);

#if defined PERMIT_CUSTOM_FIELDS
	/*! Post message constructor. If you have enabled custom field capability then you need to provde an implementation for this method.
	    It is called immediately after the framework has constructed a message from the metadata. You should then add your
		 custom field traits.
		 \param msg Mesage decoded (base ptr)
		 \return true on success */
	bool post_msg_ctor(FIX8::Message *msg);
#endif
};

//---------------------------------------------------------------------------------------------------
// The following classes are not part of FIX8.
//---------------------------------------------------------------------------------------------------
class fdinbuf : public std::streambuf
{
   static const int _buffer_size = 16;

protected:
   char _buffer[_buffer_size];
   int _fd;

   virtual int_type underflow()
   {
      if (gptr() < egptr())
         return *gptr();
      int put_back_cnt(gptr() - eback());
      if (put_back_cnt > 4)
         put_back_cnt = 4;
		std::memcpy(_buffer + (4 - put_back_cnt), gptr() - put_back_cnt, put_back_cnt);
      int num_read(read (_fd, _buffer + 4, _buffer_size - 4));
      if (num_read <= 0)
         return EOF;
      setg(_buffer + (4 - put_back_cnt), _buffer + 4, _buffer + 4 + num_read);
      return *gptr();
   }

public:
   fdinbuf(int infd) : _buffer(), _fd(infd) { setg(_buffer + 4, _buffer + 4, _buffer + 4); }
};

//-------------------------------------------------------------------------------------------------
class MyMenu
{
	bool _raw_mode;
	termio _tty_state;

	struct MenuItem
	{
		const char _key;
		const std::string _help;

		MenuItem(const char key, const std::string& help) : _key(key), _help(help) {}
		MenuItem() : _key(), _help() {}
		bool operator() (const MenuItem& a, const MenuItem& b) const { return a._key < b._key; }
	};

	myfix_session_client& _session;
	int _fd;
	std::istream _istr;
	std::ostream& _ostr;

	typedef FIX8::StaticTable<const MenuItem, bool (MyMenu::*)(), MenuItem> Handlers;
	Handlers _handlers;

public:
	MyMenu(myfix_session_client& session, int infd, std::ostream& ostr)
		: _raw_mode(), _tty_state(), _session(session), _fd(infd), _istr(new fdinbuf(infd)), _ostr(ostr) {}
	virtual ~MyMenu() {}

	std::istream& get_istr() { return _istr; }
	std::ostream& get_ostr() { return _ostr; }
	bool process(const char ch) { return (this->*_handlers.find_ref(MenuItem(ch, std::string())))(); }

	bool new_order_single();
	bool new_order_single_50();
	bool help();
	bool nothing() { return true; }
	bool do_exit() { return false; }
	bool do_logout();

	void unset_raw_mode()
	{
		if (_raw_mode)
		{
			if (ioctl(_fd, TCSETA, &_tty_state) < 0)
				std::cerr << FIX8::Str_error(errno, "Cannot reset ioctl") << std::endl;
			else
				_raw_mode = false;
		}
	}

	void set_raw_mode()
	{
		if (!_raw_mode)
		{
			if (ioctl(_fd, TCGETA, &_tty_state) < 0)
			{
				std::cerr << FIX8::Str_error(errno, "Cannot set ioctl") << std::endl;
				return;
			}
			termio tty_state(_tty_state);
			tty_state.c_lflag = 0;
			tty_state.c_cc[VTIME] = 0;
			tty_state.c_cc[VMIN] = 1;
			if (ioctl(_fd, TCSETA, &tty_state) < 0)
				std::cerr << FIX8::Str_error(errno, "Cannot reset ioctl") << std::endl;
			else
				_raw_mode = true;
		}
	}

	friend class FIX8::StaticTable<const MenuItem, bool (MyMenu::*)(), MenuItem>;
};

//-----------------------------------------------------------------------------------------
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

