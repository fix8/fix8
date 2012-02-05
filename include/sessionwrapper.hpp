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
#ifndef _FIX8_SESSIONWRAPPER_HPP_
#define _FIX8_SESSIONWRAPPER_HPP_

#include <Poco/Net/ServerSocket.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
// Base session wrapper.
struct SessionConfig : public Configuration
{
	const F8MetaCntx& _ctx;
	Logger::LogFlags _logflags, _plogflags;
	std::string _logname, _prot_logname;
	const XmlEntity *_ses;

	/// Ctor. Loads configuration, obtains session details, sets up logfile flags.
	SessionConfig (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name) :
		Configuration(conf_file, true),
		_ctx(ctx),
		_logflags(Logger::LogFlags() << Logger::timestamp << Logger::sequence << Logger::thread),
		_plogflags (Logger::LogFlags() << Logger::append),
		_logname("logname_not_set"), _prot_logname("prot_logname_not_set"),
		_ses(find_session(session_name))
	{
		if (!_ses)
			throw InvalidConfiguration(session_name);
	}

	/// Dtor.
	virtual ~SessionConfig () {}
};

//-------------------------------------------------------------------------------------------------
/*! Client wrapper.
  \tparam T your derived session class */
template<typename T>
class ClientSession : public SessionConfig
{
protected:
	FileLogger _log, _plog;
	sender_comp_id _sci;
	target_comp_id _tci;
	const SessionID _id;
	Persister *_persist;
	T *_session;
	Poco::Net::StreamSocket _sock;
	Poco::Net::SocketAddress _addr;
	ClientConnection _cc;

public:
	/// Ctor. Prepares session for connection as an initiator.
	ClientSession (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name) :
		SessionConfig(ctx, conf_file, session_name),
		_log(get_logname(_ses, _logname), _logflags, get_logfile_rotation(_ses)),
		_plog(get_protocol_logname(_ses, _prot_logname), _plogflags, get_logfile_rotation(_ses)),
		_sci(get_sender_comp_id(_ses)), _tci(get_target_comp_id(_ses)),
		_id(_ctx._beginStr, _sci, _tci),
		_persist(create_persister(_ses)),
		_session(new T(_ctx, _id, _persist, &_log, &_plog)),
		_addr(get_address(_ses)),
		_cc(&_sock, _addr, *_session)
	{
		_session->set_login_parameters(get_retry_interval(_ses), get_retry_count(_ses));
	}

	/// Dtor.
	virtual ~ClientSession ()
	{
		delete _persist;
		delete _session;
	}

	/*! Get a pointer to the session
	  \return the session pointer */
	T *session_ptr() { return _session; }

	/*! Start the session - initiate the conection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning */
	void start(bool wait) { _session->start(&_cc, wait); }

	/// Convenient scoped pointer for your session
	typedef scoped_ptr<ClientSession<T> > Client_ptr;
};

//-------------------------------------------------------------------------------------------------
/*! Reliable Client wrapper. This client attempts to recover from disconnects and login rejects.
  \tparam T your derived session class */
template<typename T>
class ReliableClientSession
{

public:
	/// Ctor. Prepares session for connection as an initiator.
	ReliableClientSession (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name)
	{
	}

	/// Dtor.
	virtual ~ReliableClientSession () {}

	/*! Start the session - initiate the conection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning */
	void start(bool wait);

	/// Convenient scoped pointer for your session
	typedef scoped_ptr<ReliableClientSession<T> > ReliableClient_ptr;
};

//-------------------------------------------------------------------------------------------------
/*! Server wrapper.
  \tparam T your derived session class */
template<typename T>
class ServerSession : public SessionConfig
{
	Poco::Net::SocketAddress _addr;
	Poco::Net::ServerSocket _server_sock;

public:
	/// Ctor. Prepares session for receiving inbbound connections (acceptor).
	ServerSession (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name) :
		SessionConfig(ctx, conf_file, session_name),
		_addr(get_address(_ses)),
		_server_sock(_addr)
	{}

	/// Dtor.
	virtual ~ServerSession () {}

	/*! Check to see if there are any waiting inbound connections.
	  \param span timespan (us) to wait before returning (will return immediately if connection available)
	  \return true if a connection is avaialble */
	bool poll(const Poco::Timespan& span=Poco::Timespan(250000)) const { return _server_sock.poll(span, Poco::Net::Socket::SELECT_READ); }

	/*! Accept an inbound connection and obtain a connected socket
	  \param claddr location to store address of remote connection
	  \return the connected socket */
	Poco::Net::StreamSocket accept(Poco::Net::SocketAddress& claddr) { return _server_sock.acceptConnection(claddr); }

	/// Convenient scoped pointer for your session
	typedef scoped_ptr<ServerSession<T> > Server_ptr;
};

/*! Server session instance.
  \tparam T your derived session class */
template<typename T>
class SessionInstance
{
	FileLogger _log, _plog;
	Persister *_persist;
	Poco::Net::SocketAddress _claddr;
	Poco::Net::StreamSocket _sock;
	T *_session;
	ServerConnection _sc;

public:
	/// Ctor. Prepares session instance with inbound connection.
	SessionInstance (ServerSession<T>& sf) :
		_log(sf.get_logname(sf._ses, sf._logname), sf._logflags, sf.get_logfile_rotation(sf._ses)),
		_plog(sf.get_protocol_logname(sf._ses, sf._prot_logname), sf._plogflags, sf.get_logfile_rotation(sf._ses)),
		_persist(sf.create_persister(sf._ses)),
		_sock(sf.accept(_claddr)),
		_session(new T(sf._ctx, _persist, &_log, &_plog)),
		_sc(&_sock, *_session, sf.get_heartbeat_interval(sf._ses))
	{}

	/// Dtor.
	virtual ~SessionInstance ()
	{
		delete _persist;
		delete _session;
	}

	/*! Get a pointer to the session
	  \return the session pointer */
	T *session_ptr() { return _session; }

	/*! Start the session - initiate the conection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning */
	void start(bool wait) { _session->start(&_sc, wait); }

	/// Stop the session. Cleanup.
	void stop() { _session->stop(); }

	/// Convenient scoped pointer for your session instance.
	typedef scoped_ptr<SessionInstance<T> > Instance_ptr;
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_SESSIONWRAPPER_HPP_
