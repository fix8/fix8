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
/// Base session wrapper.
struct SessionConfig : public Configuration
{
	const F8MetaCntx& _ctx;
	const XmlEntity *_ses;

	/// Ctor. Loads configuration, obtains session details, sets up logfile flags.
	SessionConfig (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name) :
		Configuration(conf_file, true),
		_ctx(ctx),
		_ses(find_session(session_name))
	{
		if (!_ses)
			throw InvalidConfiguration(session_name);
	}

	/// Dtor.
	virtual ~SessionConfig () {}
};

//-------------------------------------------------------------------------------------------------
/// Client wrapper.
/*!  \tparam T your derived session class */
template<typename T>
class ClientSession : public SessionConfig
{
protected:
	Logger *_log, *_plog;
	sender_comp_id _sci;
	target_comp_id _tci;
	const SessionID _id;
	Persister *_persist;
	T *_session;
	Poco::Net::StreamSocket *_sock;
	Poco::Net::SocketAddress _addr;
	ClientConnection *_cc;

public:
	/// Ctor. Prepares session for connection as an initiator.
	ClientSession (const F8MetaCntx& ctx, const std::string& conf_file,
		const std::string& session_name, bool init_con_later=false) :
		SessionConfig(ctx, conf_file, session_name),
		_log(create_logger(_ses, session_log)),
		_plog(create_logger(_ses, protocol_log)),
		_sci(get_sender_comp_id(_ses)), _tci(get_target_comp_id(_ses)),
		_id(_ctx._beginStr, _sci, _tci),
		_persist(create_persister(_ses)),
		_session(new T(_ctx, _id, _persist, _log, _plog)),
		_sock(init_con_later ? 0 : new Poco::Net::StreamSocket),
		_addr(get_address(_ses)),
		_cc(init_con_later ? 0 : new ClientConnection(_sock, _addr, *_session))
	{
		_session->set_login_parameters(get_retry_interval(_ses), get_retry_count(_ses));
	}

	/// Dtor.
	virtual ~ClientSession ()
	{
		delete _persist;
		delete _session;
		delete _log;
		delete _plog;
	}

	/*! Get a pointer to the session
	  \return the session pointer */
	T *session_ptr() { return _session; }

	/*! Start the session - initiate the connection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning
	  \param send_seqnum if supplied, override the send login sequence number, set next send to seqnum+1
	  \param recv_seqnum if supplied, override the receive login sequence number, set next recv to seqnum+1 */
	virtual void start(bool wait, const unsigned send_seqnum=0, const unsigned recv_seqnum=0)
		{ _session->start(_cc, wait, send_seqnum, recv_seqnum); }

	/// Convenient scoped pointer for your session
	typedef scoped_ptr<ClientSession<T> > Client_ptr;
};

//-------------------------------------------------------------------------------------------------
/// Reliable Client wrapper. This client attempts to recover from disconnects and login rejects.
/*! \tparam T your derived session class */
template<typename T>
class ReliableClientSession : public ClientSession<T>
{
	Thread<ReliableClientSession<T> > _thread;
	unsigned _login_retry_interval, _login_retries;

public:
	/// Ctor. Prepares session for connection as an initiator.
	ReliableClientSession (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name)
		: ClientSession<T>(ctx, conf_file, session_name, true), _thread(ref(*this))
	{
		this->_session->get_login_parameters(_login_retry_interval, _login_retries);
	}

	/// Dtor.
	virtual ~ReliableClientSession () {}

	/*! Start the session - initiate the connection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning */
	virtual void start(bool wait, const unsigned send_seqnum=0, const unsigned recv_seqnum=0)
	{
		if (!wait)
			_thread.Start();
		else
			(*this)();
	}

	/*! The reliability thread entry point.
	    \return 0 on success */
	int operator()()
	{
		unsigned attempts(0);

		for (; ++attempts < _login_retries; )
		{
			try
			{
				//std::cout << "operator()():try" << std::endl;

				this->_sock = new Poco::Net::StreamSocket,
				this->_cc = new ClientConnection(this->_sock, this->_addr, *this->_session);
				this->_session->start(this->_cc, true);
			}
			catch(f8Exception& e)
			{
				//std::cout << "ReliableClientSession: f8exception" << std::endl;
			}

			//std::cout << "operator()():out of try" << std::endl;
			delete this->_cc;
			delete this->_sock;
			millisleep(_login_retry_interval);
		}

		return 0;
	}

	/// Convenient scoped pointer for your session
	typedef scoped_ptr<ReliableClientSession<T> > ReliableClient_ptr;
};

//-------------------------------------------------------------------------------------------------
/// Server wrapper.
/*! \tparam T your derived session class */
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

/// Server session instance.
/*! \tparam T your derived session class */
template<typename T>
class SessionInstance
{
	Logger *_log, *_plog;
	Persister *_persist;
	Poco::Net::SocketAddress _claddr;
	Poco::Net::StreamSocket *_sock;
	T *_session;
	ServerConnection _sc;

public:
	/// Ctor. Prepares session instance with inbound connection.
	SessionInstance (ServerSession<T>& sf) :
		_log(sf.create_logger(sf._ses, Configuration::session_log)),
		_plog(sf.create_logger(sf._ses, Configuration::protocol_log)),
		_persist(sf.create_persister(sf._ses)),
		_sock(new Poco::Net::StreamSocket(sf.accept(_claddr))),
		_session(new T(sf._ctx, _persist, _log, _plog)),
		_sc(_sock, *_session, sf.get_heartbeat_interval(sf._ses))
	{}

	/// Dtor.
	virtual ~SessionInstance ()
	{
		delete _persist;
		delete _session;
		delete _sock;
		delete _log;
		delete _plog;
	}

	/*! Get a pointer to the session
	  \return the session pointer */
	T *session_ptr() { return _session; }

	/*! Start the session - accept the connection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning
	  \param send_seqnum if supplied, override the send login sequence number, set next send to seqnum+1
	  \param recv_seqnum if supplied, override the receive login sequence number, set next recv to seqnum+1 */
	void start(bool wait, const unsigned send_seqnum=0, const unsigned recv_seqnum=0)
		{ _session->start(&_sc, wait, send_seqnum, recv_seqnum); }

	/// Stop the session. Cleanup.
	void stop() { _session->stop(); }

	/// Convenient scoped pointer for your session instance.
	typedef scoped_ptr<SessionInstance<T> > Instance_ptr;
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_SESSIONWRAPPER_HPP_
