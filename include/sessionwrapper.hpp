//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-13 David L. Dight <fix@fix8.org>

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
	const XmlElement *_ses;
	LoginParameters _loginParameters;

	/// Ctor. Loads configuration, obtains session details, sets up logfile flags.
	SessionConfig (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name) :
		Configuration(conf_file, true),
		_ctx(ctx),
		_ses(find_session(session_name))
	{
		if (!_ses)
			throw InvalidConfiguration(session_name);

		LoginParameters lparam(get_retry_interval(_ses), get_retry_count(_ses),
			get_default_appl_ver_id(_ses), get_reset_sequence_number_flag(_ses),
			get_tcp_recvbuf_sz(_ses), get_tcp_sendbuf_sz(_ses));
		_loginParameters = lparam;
	}

	/// Dtor.
	virtual ~SessionConfig () {}

	/*! Get a pointer to the active session XmlElement to permit extraction of other XML attributes
	  \return the session element */
	const XmlElement *get_session_element() const { return _ses; }
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
		_session->set_login_parameters(_loginParameters);
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
	  \param recv_seqnum if supplied, override the receive login sequence number, set next recv to seqnum+1
	  \param davi default appl version id (FIXT) */
	virtual void start(bool wait, const unsigned send_seqnum=0, const unsigned recv_seqnum=0, const f8String davi=f8String())
		{ _session->start(_cc, wait, send_seqnum, recv_seqnum, davi); }

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
	unsigned _send_seqnum, _recv_seqnum;

public:
	/// Ctor. Prepares session for connection as an initiator.
	ReliableClientSession (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name)
		: ClientSession<T>(ctx, conf_file, session_name, true), _thread(ref(*this)), _send_seqnum(), _recv_seqnum() {}

	/// Dtor.
	virtual ~ReliableClientSession () {}

	/*! Start the session - initiate the connection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning
	  \param send_seqnum next send seqnum (not used here)
	  \param recv_seqnum next recv seqnum (not used here)
	  \param davi default appl version id (FIXT) */
	virtual void start(bool wait, const unsigned send_seqnum=0, const unsigned recv_seqnum=0, const f8String davi=f8String())
	{
		_send_seqnum = send_seqnum;
		_recv_seqnum = recv_seqnum;
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

		for (; ++attempts < this->_loginParameters._login_retries; )
		{
			try
			{
				//std::cout << "operator()():try" << std::endl;

				this->_sock = new Poco::Net::StreamSocket,
				this->_cc = new ClientConnection(this->_sock, this->_addr, *this->_session);
				this->_session->start(this->_cc, true, _send_seqnum, _recv_seqnum, this->_loginParameters._davi());
				_send_seqnum = _recv_seqnum = 0; // only set seqnums for the first time round
			}
			catch(f8Exception& e)
			{
				//std::cout << "ReliableClientSession: f8exception" << std::endl;
				this->_session->log(e.what());
			}
			catch (std::exception& e)	// also catches Poco::Net::NetException
			{
				//cout << "process:: std::exception" << endl;
				this->_session->log(e.what());
			}

			//std::cout << "operator()():out of try" << std::endl;
			this->_session->stop();
			delete this->_cc;
			delete this->_sock;
			millisleep(this->_loginParameters._login_retry_interval);
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
	{
		if (_loginParameters._recv_buf_sz)
			Connection::set_recv_buf_sz(_loginParameters._recv_buf_sz, &_server_sock);
		if (_loginParameters._send_buf_sz)
			Connection::set_send_buf_sz(_loginParameters._send_buf_sz, &_server_sock);
	}

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
		_sc(_sock, *_session, sf.get_heartbeat_interval(sf._ses), sf.get_tcp_nodelay(sf._ses))
	{
		_session->set_login_parameters(sf._loginParameters);
	}

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
