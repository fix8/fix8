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
#ifndef FIX8_SESSIONWRAPPER_HPP_
#define FIX8_SESSIONWRAPPER_HPP_

#include <Poco/Net/ServerSocket.h>
#ifdef FIX8_HAVE_OPENSSL
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/SecureServerSocket.h>
#include <Poco/SharedPtr.h>
#include <Poco/Net/PrivateKeyPassphraseHandler.h>
#include <Poco/Net/InvalidCertificateHandler.h>
#include <Poco/Net/SSLManager.h>
#endif

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
#ifdef FIX8_HAVE_OPENSSL

/// A Fix8CertificateHandler is invoked whenever an error occurs verifying the certificate.
/// The certificate is printed to the global logger with an error message
class Fix8CertificateHandler: public Poco::Net::InvalidCertificateHandler
{
public:
   /// Creates the Fix8CertificateHandler.
   Fix8CertificateHandler(bool handleErrorsOnServerSide) : Poco::Net::InvalidCertificateHandler(handleErrorsOnServerSide) {}

   /// Destroys the Fix8CertificateHandler.
   virtual ~Fix8CertificateHandler() {}

	/// Prints the certificate to stdout and waits for user input on the console
	/// to decide if a certificate should be accepted/rejected.
	F8API void onInvalidCertificate(const void* pSender, Poco::Net::VerificationErrorArgs& errorCert);
};

/// An implementation of PrivateKeyPassphraseHandler that
/// prints an error and returns no phrase
/// Passphrases cannot be given when negotiating SSL sessions in Fix8
class Fix8PassPhraseHandler : public Poco::Net::PrivateKeyPassphraseHandler
{
public:
	/// Creates the Fix8PassPhraseHandler.
   Fix8PassPhraseHandler(bool server) : Poco::Net::PrivateKeyPassphraseHandler(server) {}

	/// Destroys the Fix8PassPhraseHandler.
   ~Fix8PassPhraseHandler() {}

	F8API void onPrivateKeyRequested(const void* pSender, std::string& privateKey);
};

//-------------------------------------------------------------------------------------------------
struct PocoSslContext
{
	PocoSslContext(const SslContext& ctx, bool client)
	{
		if (ctx._valid)
		{
			Poco::SharedPtr<Poco::Net::PrivateKeyPassphraseHandler> phrase_handler(new Fix8PassPhraseHandler(!client));
			Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> cert_handler(new Fix8CertificateHandler(!client));
#if POCO_VERSION >= 0x01040000
			Poco::Net::initializeSSL();
#endif
			_context = new Poco::Net::Context(
				client ? Poco::Net::Context::CLIENT_USE : Poco::Net::Context::SERVER_USE,
				ctx._private_key_file, ctx._certificate_file, ctx._ca_location,
				static_cast<Poco::Net::Context::VerificationMode>(ctx._verification_mode), ctx._verification_depth,
				ctx._load_default_cas, ctx._cipher_list);
			if (client)
				Poco::Net::SSLManager::instance().initializeClient(phrase_handler, cert_handler, _context);
			else
				Poco::Net::SSLManager::instance().initializeServer(phrase_handler, cert_handler, _context);
		}
	}

	~PocoSslContext()
	{
#if POCO_VERSION >= 0x01040000
		if (_context)
			Poco::Net::uninitializeSSL();
#endif
	}

	Poco::Net::Context::Ptr _context;
	bool is_secure() const { return _context.get() != 0; }
};
#endif

//-------------------------------------------------------------------------------------------------
/// Base session wrapper.
struct SessionConfig : public Configuration
{
	const F8MetaCntx& _ctx;
	const XmlElement *_ses;
	LoginParameters _loginParameters;
	const std::string _session_name;

	/// Ctor. Loads configuration, obtains session details, sets up logfile flags.
	SessionConfig (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name) :
		Configuration(conf_file, true), _ctx(ctx), _ses(find_group(g_sessions, session_name)), _session_name(session_name)
	{
		if (!_ses)
			throw InvalidConfiguration(session_name);

		_loginParameters =
		{
			get_retry_interval(_ses), get_retry_count(_ses),
			get_default_appl_ver_id(_ses), get_connect_timeout(_ses),
			get_reset_sequence_number_flag(_ses),
			get_always_seqnum_assign(_ses), get_silent_disconnect(_ses),
			get_no_chksum_flag(_ses), get_permissive_mode_flag(_ses), false,
			get_enforce_compids_flag(_ses),
			get_tcp_recvbuf_sz(_ses),
			get_tcp_sendbuf_sz(_ses), get_heartbeat_interval(_ses),
			create_login_schedule(_ses), create_clients(_ses)
		};

		const unsigned ts(get_tabsize(_ses));
		if (ts != defaults::tabsize)	// only set if not default
			MessageBase::set_tabsize(ts);
	}

	/// Dtor.
	virtual ~SessionConfig () {}

	/*! Get a pointer to the active session XmlElement to permit extraction of other XML attributes
	  \return the session element */
	const XmlElement *get_session_element() const { return _ses; }
};

//-------------------------------------------------------------------------------------------------
/// Base Client wrapper.
class ClientSessionBase : public SessionConfig
{
public:
	ClientSessionBase(const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name)
		: SessionConfig(ctx, conf_file, session_name) {}
    // using SessionConfig::SessionConfig;

	/*! If reliable, determine if the maximum no. of reties has been reached
	  \return false for default clientsession */
	virtual bool has_given_up() const { return false; }

	/// Dtor.
	virtual ~ClientSessionBase () {}

	/*! Get a pointer to the session
	  \return the session pointer */
	virtual Session *session_ptr() = 0;

	/*! Start the session - initiate the connection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning
	  \param send_seqnum if supplied, override the send login sequence number, set next send to seqnum+1
	  \param recv_seqnum if supplied, override the receive login sequence number, set next recv to seqnum+1
	  \param davi default appl version id (FIXT) */
	virtual void start(bool wait, const unsigned send_seqnum=0, const unsigned recv_seqnum=0, const f8String davi=f8String()) = 0;
};

//-------------------------------------------------------------------------------------------------
/// Client wrapper.
/*!  \tparam T specialised with your derived session class */
template<typename T>
class ClientSession : public ClientSessionBase
{
protected:
	sender_comp_id _sci;
	target_comp_id _tci;
	const SessionID _id;
	Logger *_log, *_plog;
	Persister *_persist;
	T *_session;
	Poco::Net::StreamSocket *_sock = nullptr;
	Poco::Net::SocketAddress _addr;
	ClientConnection *_cc = nullptr;
#ifdef FIX8_HAVE_OPENSSL
	PocoSslContext _ssl;
#endif

public:
	/// Ctor. Prepares session for connection as an initiator.
	ClientSession (const F8MetaCntx& ctx, const std::string& conf_file,
		const std::string& session_name, bool init_con_later=false) :
		ClientSessionBase(ctx, conf_file, session_name),
		_sci(get_sender_comp_id(_ses)), _tci(get_target_comp_id(_ses)),
		_id(_ctx._beginStr, _sci, _tci),
		_log(create_logger(_ses, session_log, &_id)),
		_plog(create_logger(_ses, protocol_log, &_id)),
		_persist(create_persister(_ses, nullptr, this->_loginParameters._reset_sequence_numbers)),
		_session(new T(_ctx, _id, _persist, _log, _plog)),
		_addr(get_address(_ses))
#ifdef FIX8_HAVE_OPENSSL
		,_ssl(get_ssl_context(_ses), true)
#endif
	{
		if (!init_con_later)
		{
#ifdef FIX8_HAVE_OPENSSL
			bool secured(_ssl.is_secure());
			_sock = secured
				? new Poco::Net::SecureStreamSocket(_ssl._context)
				: new Poco::Net::StreamSocket;
#else
			bool secured(false);
			_sock = new Poco::Net::StreamSocket;
#endif
			_cc = new ClientConnection(_sock, _addr, *_session, this->_loginParameters._hb_int, get_process_model(_ses), true, secured);
		}

		_session->set_login_parameters(this->_loginParameters);
		_session->set_session_config(this);
	}

	/// Dtor.
	virtual ~ClientSession ()
	{
		delete _persist;
		_persist = nullptr;
		delete _session;
		_session = nullptr;
		delete _log;
		_log = nullptr;
		delete _plog;
		_plog = nullptr;
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
	using ClientSession_ptr = std::unique_ptr<ClientSession<T>>;

	using session_type = T;
};

//-------------------------------------------------------------------------------------------------
/// Reliable Client wrapper. This client attempts to recover from disconnects and login rejects.
/*! \tparam T specialised with your derived session class */
template<typename T>
class ReliableClientSession : public ClientSession<T>
{
	f8_thread<ReliableClientSession<T>> _thread;
	unsigned _send_seqnum = 0, _recv_seqnum = 0, _current = 0, _attempts = 0;
	f8_atomic<bool> _giving_up;
	std::vector<Server> _servers;
	const size_t _failover_cnt;
	f8_thread_cancellation_token _cancellation_token;

public:
	/// Ctor. Prepares session for connection as an initiator.
	ReliableClientSession (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name)
		: ClientSession<T>(ctx, conf_file, session_name, true), _thread(std::ref(*this)),
		_failover_cnt(this->get_addresses(this->_ses, _servers))
	{
		_giving_up = false;
		this->_loginParameters._reliable = true;
	}

	enum { no_servers_configured=0xffff };

	/*! If reliable, determine if the maximum no. of reties has been reached
	  \return true if maximum no. of reties reached */
	bool has_given_up() const { return _giving_up; }

	/// Dtor.
	virtual ~ReliableClientSession ()
	{
		_thread.request_stop();
		_thread.join();
	}

	/*! Start the session - initiate the connection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning
	  \param send_seqnum next send seqnum
	  \param recv_seqnum next recv seqnum
	  \param davi default appl version id (FIXT) */
	virtual void start(bool wait, const unsigned send_seqnum=0, const unsigned recv_seqnum=0, const f8String davi=f8String())
	{
		_send_seqnum = send_seqnum;
		_recv_seqnum = recv_seqnum;
		if (!wait)
			_thread.start();
		else
			(*this)();
	}

	/*! Get the number of attempts made so far
	    \return number of attempts */
	size_t get_attemps_cnt() const { return _attempts; }

	/*! Get the number of configured failover servers
	    \return number of servers */
	size_t get_server_cnt() const { return _failover_cnt; }

	/*! Get the Server object for a given server index
	    \param idx of server desired, if not given return current
	    \return ptr to server, 0 if not found */
	const Server *get_server(unsigned idx=no_servers_configured) const
	{
		return idx == no_servers_configured && _failover_cnt ? &_servers[_current]
			: idx < _failover_cnt ? &_servers[idx] : nullptr;
	}

	/*! The reliability thread entry point.
	    \return 0 on success */
	int operator()()
	{
		while(!_cancellation_token)
		{
			++_attempts;

			bool excepted(_failover_cnt == 0);
			try
			{
				if (_failover_cnt)
				{
					std::ostringstream ostr;
					ostr << "Trying " << _servers[_current]._hostname << '(' << (1 + _servers[_current]._retries) << "), "
						<< _attempts << " attempts so far";
					this->_session->log(ostr.str(), Logger::Warn);
					this->_loginParameters._reset_sequence_numbers = _servers[_current]._reset_sequence_numbers; // permit override
				}

				//std::cout << "operator()():try" << std::endl;
#ifdef FIX8_HAVE_OPENSSL
				bool secured(this->_ssl.is_secure());
				this->_sock = secured
					? new Poco::Net::SecureStreamSocket(this->_ssl._context)
					: new Poco::Net::StreamSocket;
#else
				bool secured(false);
				this->_sock = new Poco::Net::StreamSocket;
#endif
				this->_cc = new ClientConnection(this->_sock, _failover_cnt ? this->_addr = _servers[_current]._addr : this->_addr,
					*this->_session, this->_loginParameters._hb_int, this->get_process_model(this->_ses), true, secured);
				this->_session->set_login_parameters(this->_loginParameters);
				this->_session->set_session_config(this);
				this->_session->start(this->_cc, true, _send_seqnum, _recv_seqnum, this->_loginParameters._davi());
				_send_seqnum = _recv_seqnum = 0; // only set seqnums for the first time round
			}
			catch (Poco::TimeoutException& e)
			{
				this->_session->log(e.what(), Logger::Error);
				excepted = true;
			}
			catch (f8Exception& e)
			{
				//	std::cerr << e.what() << std::endl;
				this->_session->log(e.what(), Logger::Error);
				excepted = true;
			}
			catch (Poco::Net::InvalidAddressException& e)
			{
				this->_session->log(e.what(), Logger::Error);
				excepted = true;
			}
#if POCO_VERSION >= 0x01040000
			catch (Poco::Net::InvalidSocketException& e)
			{
				this->_session->log(e.what(), Logger::Error);
				excepted = true;
			}
#endif
			catch (Poco::Net::ServiceNotFoundException& e)
			{
				//std::cerr << e.what() << std::endl;
				this->_session->log(e.what(), Logger::Error);
				excepted = true;
			}
			catch (Poco::Net::ConnectionRefusedException& e)
			{
				this->_session->log(e.what(), Logger::Error);
				excepted = true;
			}
			catch (Poco::Net::DNSException& e)
			{
				this->_session->log(e.what(), Logger::Error);
				excepted = true;
			}
			catch (Poco::Net::InterfaceNotFoundException& e)
			{
				this->_session->log(e.what(), Logger::Error);
				excepted = true;
			}
			catch (Poco::Net::NetException& e)	// catch all other NetExceptions
			{
				this->_session->log(e.what(), Logger::Error);
				excepted = true;
			}
			catch (std::exception& e)
			{
				//std::cout << "process:: std::exception" << endl;
				this->_session->log(e.what(), Logger::Error);
				excepted = true;
			}

			//std::cout << "operator()():out of try" << std::endl;
			try
			{
				this->_session->stop();
			}
			catch (f8Exception& e)
			{
				this->_session->log(e.what(), Logger::Error);
			}
			catch (Poco::Net::NetException& e)
			{
				this->_session->log(e.what(), Logger::Error);
			}
			catch (std::exception& e)
			{
				this->_session->log(e.what(), Logger::Error);
			}

			delete this->_cc;
			this->_cc = nullptr;
			delete this->_sock;
			this->_sock = nullptr;

			if (!excepted || (_failover_cnt == 0 && _attempts > this->_loginParameters._login_retries))
				break;

			if (_failover_cnt)
			{
				++_servers[_current]._retries;

				while (!_cancellation_token) // FIXME possible endless loop condition
				{
					if (_servers[_current]._max_retries && _servers[_current]._retries < _servers[_current]._max_retries)
						break;
					_servers[_current]._retries = 0;	// reset for next time
					++_current %= _failover_cnt;
				}
			}

			hypersleep<h_milliseconds>(this->_loginParameters._login_retry_interval);
		}

		_giving_up = true;

		return 0;
	}

	f8_thread_cancellation_token& cancellation_token() { return _cancellation_token; }

	/// Convenient scoped pointer for your session
	using ReliableClientSession_ptr = std::unique_ptr<ReliableClientSession<T>>;
};

//-------------------------------------------------------------------------------------------------
class SessionInstanceBase;
template<typename T> class SessionInstance;

/// Base Server Session.
class ServerSessionBase : public SessionConfig
{
protected:
	Poco::Net::ServerSocket *_server_sock = nullptr;

public:
    /// Ctor. Prepares session for receiving inbbound connections (acceptor).
	ServerSessionBase(const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name)
		: SessionConfig(ctx, conf_file, session_name) {}
	//using SessionConfig::SessionConfig;	// not supported in all C++11 compilers

	/// Dtor.
	virtual ~ServerSessionBase ()
	{
		delete _server_sock;
		_server_sock = nullptr;
	}

	/*! Create a SessionInstance for the associated Session
	  \return base pointer to new SessionInstance */
	virtual SessionInstanceBase *create_server_instance() = 0;

	/*! Check to see if there are any waiting inbound connections.
	  \param span timespan (us, default 250 ms) to wait before returning (will return immediately if connection available)
	  \return true if a connection is avaialble */
	bool poll(const Poco::Timespan& span=Poco::Timespan(250000)) const
		{ return _server_sock->poll(span, Poco::Net::Socket::SELECT_READ); }

	/*! Accept an inbound connection and obtain a connected socket
	  \param claddr location to store address of remote connection
	  \return the connected socket */
	Poco::Net::StreamSocket accept(Poco::Net::SocketAddress& claddr)
		{ return _server_sock->acceptConnection(claddr); }

	/*! Check if this server session supports secure connections
	  \return true if supported */
	virtual bool is_secure() const { return false; }

	friend class ServerManager;
};

//-------------------------------------------------------------------------------------------------
// Server wrapper.
/*! \tparam T specialised with your derived session class */
template<typename T>
class ServerSession : public ServerSessionBase
{
	Poco::Net::SocketAddress _addr;
#ifdef FIX8_HAVE_OPENSSL
	PocoSslContext _ssl;
#endif

public:
	/// Ctor. Prepares session for receiving inbbound connections (acceptor).
	ServerSession (const F8MetaCntx& ctx, const std::string& conf_file, const std::string& session_name) :
		ServerSessionBase(ctx, conf_file, session_name),
		_addr(get_address(_ses))
#ifdef FIX8_HAVE_OPENSSL
		,_ssl(get_ssl_context(_ses), false)
#endif
	{
		_server_sock =
#ifdef FIX8_HAVE_OPENSSL
			_ssl.is_secure() ? new Poco::Net::SecureServerSocket(_addr, 64, _ssl._context) :
#endif
			new Poco::Net::ServerSocket(_addr);

		if (_loginParameters._recv_buf_sz)
			Connection::set_recv_buf_sz(_loginParameters._recv_buf_sz, _server_sock);
		if (_loginParameters._send_buf_sz)
			Connection::set_send_buf_sz(_loginParameters._send_buf_sz, _server_sock);
	}

	/// Dtor.
	virtual ~ServerSession () {}

	/*! Create a SessionInstance for the associated Session
	  \return base pointer to new SessionInstance */
	virtual SessionInstanceBase *create_server_instance();

	/// Convenient scoped pointer for your session
	using ServerSession_ptr = std::unique_ptr<ServerSession<T>>;

	virtual bool is_secure() const
#ifdef FIX8_HAVE_OPENSSL
		{ return _ssl.is_secure(); }
#else
		{ return false; }
#endif
};

//-------------------------------------------------------------------------------------------------
/// Base Server session instance.
class SessionInstanceBase
{
public:
	/// Ctor. Prepares session instance with inbound connection.
	SessionInstanceBase () = default;

	/// Dtor.
	virtual ~SessionInstanceBase () {}

	/*! Get a pointer to the session
	  \return the session pointer */
	virtual Session *session_ptr() = 0;

	/*! Start the session - accept the connection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning
	  \param send_seqnum if supplied, override the send login sequence number, set next send to seqnum+1
	  \param recv_seqnum if supplied, override the receive login sequence number, set next recv to seqnum+1 */
	virtual void start(bool wait, const unsigned send_seqnum=0, const unsigned recv_seqnum=0) {}

	/// Stop the session. Cleanup.
	virtual void stop() {}
};

//-------------------------------------------------------------------------------------------------
/// Server session instance.
/*! \tparam T specialised with your derived session class */
template<typename T>
class SessionInstance : public SessionInstanceBase
{
	ServerSession<T>& _sf;
	Poco::Net::SocketAddress _claddr;
	Poco::Net::StreamSocket *_sock;
	T *_session;
	ServerConnection *_psc;

public:
	/// Ctor. Prepares session instance with inbound connection.
	SessionInstance (ServerSession<T>& sf) :
		_sf(sf),
		_sock(new Poco::Net::StreamSocket(_sf.accept(_claddr))),
		_session(new T(_sf._ctx, _sf.get_sender_comp_id(_sf._ses))),
		_psc(new ServerConnection(_sock, _claddr, *_session, _sf.get_heartbeat_interval(_sf._ses), _sf.get_process_model(_sf._ses),
		_sf.get_tcp_nodelay(_sf._ses), _sf.get_tcp_reuseaddr(_sf._ses), _sf.get_tcp_linger(_sf._ses),
		_sf.get_tcp_keepalive(_sf._ses),
#ifdef FIX8_HAVE_OPENSSL
		_sf.is_secure()
#else
		false
#endif
			))
	{
		_session->set_login_parameters(_sf._loginParameters);
		_session->set_session_config(&_sf);
	}

	/// Dtor.
	virtual ~SessionInstance ()
	{
		try
		{
			if (_psc != nullptr)
			{
				_psc->stop();
				delete _psc;
				_psc = nullptr;
			}
		}
		catch (f8Exception& e)
		{
			this->_session->log(e.what(), Logger::Error);
		}
		catch (Poco::Exception& e)
		{
			this->_session->log(e.what(), Logger::Error);
		}
		catch (std::exception& e)
		{
			this->_session->log(e.what(), Logger::Error);
		}

		try
		{
			delete _session;
			_session = nullptr;
		}
		catch (f8Exception& e)
		{
			this->_session->log(e.what(), Logger::Error);
		}
		catch (Poco::Exception& e)
		{
			this->_session->log(e.what(), Logger::Error);
		}
		catch (std::exception& e)
		{
			this->_session->log(e.what(), Logger::Error);
		}

		try
		{
			delete _sock;
			_sock = nullptr;
		}
		catch (f8Exception& e)
		{
			this->_session->log(e.what(), Logger::Error);
		}
		catch (Poco::Exception& e)
		{
			this->_session->log(e.what(), Logger::Error);
		}
		catch (std::exception& e)
		{
			this->_session->log(e.what(), Logger::Error);
		}
	}

	/*! Get a pointer to the session
	  \return the session pointer */
	virtual T *session_ptr() override { return _session; }

	/*! Start the session - accept the connection, logon and start heartbeating.
	  \param wait if true wait till session finishes before returning
	  \param send_seqnum if supplied, override the send login sequence number, set next send to seqnum+1
	  \param recv_seqnum if supplied, override the receive login sequence number, set next recv to seqnum+1 */
	virtual void start(bool wait, const unsigned send_seqnum=0, const unsigned recv_seqnum=0) override
		{ _session->start(_psc, wait, send_seqnum, recv_seqnum); }

	/// Stop the session. Cleanup.
	virtual void stop() override { _session->stop(); }

	/// Convenient scoped pointer for your session instance.
	using SessionInstance_ptr = std::unique_ptr<SessionInstance<T>>;
};

template<typename T>
SessionInstanceBase *ServerSession<T>::create_server_instance() { return new SessionInstance<T>(*this); }

} // FIX8

#endif // FIX8_SESSIONWRAPPER_HPP_
