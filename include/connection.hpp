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
#ifndef _FIX8_CONNECTION_HPP_
#define _FIX8_CONNECTION_HPP_

#include <Poco/Net/StreamSocket.h>
#include <Poco/Timespan.h>
#include <Poco/Net/NetException.h>
#include <tbb/concurrent_queue.h>

//----------------------------------------------------------------------------------------
namespace FIX8 {

class Session;

//----------------------------------------------------------------------------------------
/// Half duplex asynch socket wrapper with thread
/*! \tparam T the object type to queue */
template <typename T>
class AsyncSocket
{
	Thread<AsyncSocket> _thread;

protected:
	Poco::Net::StreamSocket *_sock;
	tbb::concurrent_bounded_queue<T> _msg_queue;
	Session& _session;

public:
	/*! Ctor.
	    \param sock connected socket
	    \param session session */
	AsyncSocket(Poco::Net::StreamSocket *sock, Session& session)
		: _thread(ref(*this)), _sock(sock), _session(session)
	{
	}

	/// Dtor.
	virtual ~AsyncSocket() {}

	/*! Get the number of messages queued on this socket.
	    \return number of queued messages */
	size_t queued() const { return _msg_queue.size(); }

	/*! Pure virtual Function operator. Called by thread to process message on queue.
	    \return 0 on success */
	virtual int operator()() = 0;

	/// Start the processing thread.
	virtual void start() { _thread.Start(); }

	/// Stop the processing thread and quit.
	virtual void quit() { _thread.Kill(1); }

	/*! Get the underlying socket object.
	    \return the socket */
	Poco::Net::StreamSocket *socket() { return _sock; }

	/*! Wait till processing thead has finished.
	    \return 0 on success */
	int join() { return _thread.Join(); }
};

//----------------------------------------------------------------------------------------
/// Fix message reader
class FIXReader : public AsyncSocket<f8String>
{
	enum { _max_msg_len = 1024, _chksum_sz = 7 };
	tbb::atomic<bool> _socket_error;

	Thread<FIXReader> _callback_thread;

	/*! Process messages from inbound queue, calls session process method.
	    \return number of messages processed */
	int callback_processor();

	size_t _bg_sz; // 8=FIXx.x^A9=x

	/*! Read a Fix message. Throws InvalidBodyLength, IllegalMessage.
	    \param to string to place message in
	    \return true on success */
	bool read(f8String& to);

	/*! Read bytes from the socket layer, throws PeerResetConnection.
	    \param where buffer to place bytes in
	    \param sz number of bytes to read
	    \return number of bytes read */
	int sockRead(char *where, size_t sz)
	{
		const int result(_sock->receiveBytes(where, sz));
		if (result == 0)
			throw PeerResetConnection("connection gone");
		return result;
	}

protected:
	/*! Reader thread method. Reads messages and places them on the queue for processing.
	    \return 0 on success */
	int operator()();

public:
	/*! Ctor.
	    \param sock connected socket
	    \param session session */
	FIXReader(Poco::Net::StreamSocket *sock, Session& session)
		: AsyncSocket<f8String>(sock, session), _callback_thread(ref(*this), &FIXReader::callback_processor), _bg_sz()
	{
		set_preamble_sz();
	}

	/// Dtor.
	virtual ~FIXReader() {}

	/// Start the processing threads.
	virtual void start()
	{
		_socket_error = false;
		AsyncSocket<f8String>::start();
		if (_callback_thread.Start())
			_socket_error = true;
	}

	/// Stop the processing threads and quit.
	virtual void quit() { _callback_thread.Kill(1); AsyncSocket<f8String>::quit(); }

	/// Send a message to the processing method instructing it to quit.
	virtual void stop() { const f8String from; _msg_queue.try_push(from); }

	/// Calculate the length of the Fix message preamble, e.g. "8=FIX.4.4^A9=".
	void set_preamble_sz();

	/*! Check to see if the socket is in error
	    \return true if there was a socket error */
	bool is_socket_error() const { return _socket_error; }
};

//----------------------------------------------------------------------------------------
/// Fix message writer
class FIXWriter : public AsyncSocket<Message *>
{
protected:
	/*! Writer thread method. Reads messages from the queue and sends them over the socket.
	    \return 0 on success */
	int operator()();

public:
	/*! Ctor.
	    \param sock connected socket
	    \param session session */
	FIXWriter(Poco::Net::StreamSocket *sock, Session& session) : AsyncSocket<Message *>(sock, session) {}

	/// Dtor.
	virtual ~FIXWriter() {}

	/*! Place Fix message on outbound message queue.
	    \param from message to send
	    \return true in success */
	bool write(Message *from) { return _msg_queue.try_push(from); }

	/*! Send message over socket.
	    \param msg message string to send
	    \return number of bytes sent */
	int send(const f8String& msg)
	{
		const int result(_sock->sendBytes(msg.data(), msg.size()));
		if (result <= 0)
			throw PeerResetConnection("connection gone");
		return result;
	}

	/// Send a message to the processing method instructing it to quit.
	virtual void stop() { _msg_queue.try_push(0); }
};

//----------------------------------------------------------------------------------------
/// Complete Fix connection (reader and writer).
class Connection
{
public:
	/// Roles: acceptor, initiator or unknown.
	enum Role { cn_acceptor, cn_initiator, cn_unknown };

protected:
	Poco::Net::StreamSocket *_sock;
	bool _connected;
	Session& _session;
	Role _role;
	unsigned _hb_interval, _hb_interval20pc;

	FIXReader _reader;
	FIXWriter _writer;

public:
	/*! Ctor. Initiator.
	    \param sock connected socket
	    \param session session */
	Connection(Poco::Net::StreamSocket *sock, Session &session)	// client
		: _sock(sock), _connected(), _session(session), _role(cn_initiator),
		_hb_interval(10), _reader(sock, session), _writer(sock, session) {}

	/*! Ctor. Acceptor.
	    \param sock connected socket
	    \param session session
	    \param hb_interval heartbeat interval */
	Connection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval) // server
		: _sock(sock), _connected(true), _session(session), _role(cn_acceptor), _hb_interval(hb_interval),
		_hb_interval20pc(hb_interval + hb_interval / 5),
		  _reader(sock, session), _writer(sock, session) {}

	/// Dtor.
	virtual ~Connection() {}

	/*! Get the role for this connection.
	    \return the role */
	Role get_role() const { return _role; }

	/// Start the reader and writer threads.
	void start();

	/// Stop the reader and writer threads.
	void stop();

	/*! Get the connection state.
	    \return true if connected */
	virtual bool connect() { return _connected; }

	/*! Write a message to the underlying socket.
	    \param from Message to write
	    \return true on success */
	virtual bool write(Message *from) { return _writer.write(from); }

	/*! Write a string message to the underlying socket.
	    \param from Message (string) to write
	    \return number of bytes written */
	int send(const f8String& from) { return _writer.send(from); }

	/*! Set the heartbeat interval for this connection.
	    \param hb_interval heartbeat interval */
	void set_hb_interval(const unsigned hb_interval)
		{ _hb_interval = hb_interval; _hb_interval20pc = hb_interval + hb_interval / 5; }

	/*! Get the heartbeat interval for this connection.
	    \return the heartbeat interval */
	unsigned get_hb_interval() const { return _hb_interval; }

	/*! Get the heartbeat interval + %20 for this connection.
	    \return the heartbeat interval + %20 */
	unsigned get_hb_interval20pc() const { return _hb_interval20pc; }

	/*! Wait till reader thead has finished.
	    \return 0 on success */
	int join() { return _reader.join(); }

	/*! Check to see if the socket is in error
	    \return true if there was a socket error */
	bool is_socket_error() const { return _reader.is_socket_error(); }

	/*! Set the socket recv buffer sz
	    \param sock socket to operate on
	    \param sz new size */
	static void set_recv_buf_sz(const unsigned sz, Poco::Net::Socket *sock)
	{
		const unsigned current_sz(sock->getReceiveBufferSize());
		sock->setReceiveBufferSize(sz);
		std::ostringstream ostr;
		ostr << "ReceiveBufferSize old:" << current_sz << " requested:" << sz << " new:" << sock->getReceiveBufferSize();
		GlobalLogger::log(ostr.str());
	}

	/*! Set the socket send buffer sz
	    \param sock socket to operate on
	    \param sz new size */
	static void set_send_buf_sz(const unsigned sz, Poco::Net::Socket *sock)
	{
		const unsigned current_sz(sock->getSendBufferSize());
		sock->setSendBufferSize(sz);
		std::ostringstream ostr;
		ostr << "SendBufferSize old:" << current_sz << " requested:" << sz << " new:" << sock->getSendBufferSize();
		GlobalLogger::log(ostr.str());
	}
	/*! Set the socket recv buffer sz
	    \param sz new size */
	void set_recv_buf_sz(const unsigned sz) const { set_recv_buf_sz(sz, _sock); }

	/*! Set the socket send buffer sz
	    \param sz new size */
	void set_send_buf_sz(const unsigned sz) const { set_send_buf_sz(sz, _sock); }

	/*! Get the session associated with this connection.
	    \return the session */
	Session& get_session() { return _session; }
};

//-------------------------------------------------------------------------------------------------
/// Client (initiator) specialisation of Connection.
class ClientConnection : public Connection
{
	Poco::Net::SocketAddress _addr;
	const bool _no_delay;

public:
	/*! Ctor. Initiator.
	    \param sock connected socket
	    \param addr sock address structure
	    \param session session
	    \param no_delay set or clear the tcp no delay flag on the socket */
	ClientConnection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr, Session &session, const bool no_delay=true)
		: Connection(sock, session), _addr(addr), _no_delay(no_delay) {}

	/// Dtor.
	virtual ~ClientConnection() {}

	/*! Establish connection.
	    \return true on success */
	bool connect();
};

//-------------------------------------------------------------------------------------------------
/// Server (acceptor) specialisation of Connection.
class ServerConnection : public Connection
{

public:
	/*! Ctor. Initiator.
	    \param sock connected socket
	    \param session session
	    \param hb_interval heartbeat interval
	    \param no_delay set or clear the tcp no delay flag on the socket */
	ServerConnection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval, const bool no_delay=true) :
		Connection(sock, session, hb_interval)
	{
		_sock->setLinger(false, 0);
		_sock->setNoDelay(no_delay);
	}

	/// Dtor.
	virtual ~ServerConnection() {}
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_CONNECTION_HPP_
