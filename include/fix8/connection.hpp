//-------------------------------------------------------------------------------------------------
/*

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

*/
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_CONNECTION_HPP_
# define _FIX8_CONNECTION_HPP_

#include <Poco/Net/StreamSocket.h>
#include <Poco/Timespan.h>
#include <Poco/Net/NetException.h>

//----------------------------------------------------------------------------------------
namespace FIX8 {

class Session;

//----------------------------------------------------------------------------------------
/// Half duplex async socket wrapper with thread
/*! \tparam T the object type to queue */
template <typename T>
class AsyncSocket
{
	dthread<AsyncSocket> _thread;

protected:
	coroutine _coro;
	Poco::Net::StreamSocket *_sock;
	f8_concurrent_queue<T> _msg_queue;
	Session& _session;
	ProcessModel _pmodel;

public:
	/*! Ctor.
	    \param sock connected socket
	    \param session session
	    \param pmodel process model */
	AsyncSocket(Poco::Net::StreamSocket *sock, Session& session, const ProcessModel pmodel=pm_pipeline)
		: _thread(FIX8::ref(*this)), _sock(sock), _session(session), _pmodel(pmodel) {}

	/// Dtor.
	virtual ~AsyncSocket() {}

	/*! Get the number of messages queued on this socket.
	    \return number of queued messages */
	size_t queued() const { return _msg_queue.size(); }

	/*! Function operator. Called by thread to process message on queue.
	    \return 0 on success */
	int operator()() { return execute(); }

	/*! Execute the function operator
	    \return result of operator */
	virtual int execute() { return 0; }

	/// Start the processing thread.
	virtual void start() { _thread.start(); }

	/// Stop the processing thread and quit.
	virtual void quit() { _thread.kill(1); }

	/*! Get the underlying socket object.
	    \return the socket */
	Poco::Net::StreamSocket *socket() { return _sock; }

	/*! Wait till processing thead has finished.
	    \return 0 on success */
	int join() { return _thread.join(); }
};

//----------------------------------------------------------------------------------------
/// Fix message reader
class FIXReader : public AsyncSocket<f8String>
{
	enum { _max_msg_len = MAX_MSG_LENGTH, _chksum_sz = 7 };
	f8_atomic<bool> _socket_error;

	dthread<FIXReader> _callback_thread;

    char _read_buffer[_max_msg_len*2];
    char *_read_buffer_rptr, *_read_buffer_wptr;

	/*! Process messages from inbound queue, calls session process method.
	    \return number of messages processed */
	int callback_processor();

	size_t _bg_sz; // 8=FIXx.x^A9=x

	/*! Read a Fix message. Throws InvalidBodyLength, IllegalMessage.
	    \param to string to place message in
	    \return true on success */
	bool read(f8String& to);

    /*! Read bytes from read buffer and then if needed from the socket layer, throws PeerResetConnection.
        \param where buffer to place bytes in
        \param sz number of bytes to read
        \return number of bytes read */
    int sockRead(char *where, size_t sz)
    {
        if (static_cast<size_t>(_read_buffer_wptr - _read_buffer_rptr) < sz)
            realSockRead(sz, _max_msg_len);
        sz = std::min((size_t)(_read_buffer_wptr-_read_buffer_rptr), sz);
        memcpy(where, _read_buffer_rptr, sz);
        _read_buffer_rptr += sz;
        const size_t shift(_max_msg_len);
        if (static_cast<size_t>(_read_buffer_rptr - _read_buffer) >= shift)
        {
				memcpy(_read_buffer, &_read_buffer[shift], sizeof(_read_buffer) - shift);
				_read_buffer_rptr -= shift;
				_read_buffer_wptr -= shift;
        }
        return sz;
    }

	/*! Read bytes from the socket layer, throws PeerResetConnection.
	    \param where buffer to place bytes in
	    \param sz number of bytes to read
	    \param maxsz max number of bytes to read
	    \return number of bytes read */
	int realSockRead(size_t sz, size_t maxsz)
	{
		const size_t max_sz(_read_buffer + sizeof(_read_buffer) - _read_buffer_wptr);
		int maxremaining(std::min(maxsz, max_sz)), remaining(std::min(sz, max_sz));
		char *ptr(_read_buffer_wptr), *eptr(_read_buffer + sizeof(_read_buffer));

		int rdsz(0);
		while (remaining > 0 && _read_buffer_wptr < eptr)
		{
			rdsz = _sock->receiveBytes(_read_buffer_wptr, maxremaining);
			if (rdsz <= 0)
			{
				if (errno == EAGAIN
#if defined EWOULDBLOCK && EAGAIN != EWOULDBLOCK
					|| errno == EWOULDBLOCK
#endif
				)
					continue;
				throw PeerResetConnection("sockRead: connection gone");
			}
			_read_buffer_wptr += rdsz;
			remaining -= rdsz;
			maxremaining -= rdsz;
		}
		return _read_buffer_wptr - ptr;
	}

public:
	/*! Ctor.
	    \param sock connected socket
	    \param session session
	    \param pmodel process model */
	FIXReader(Poco::Net::StreamSocket *sock, Session& session, const ProcessModel pmodel=pm_pipeline)
		: AsyncSocket<f8String>(sock, session, pmodel), _callback_thread(FIX8::ref(*this), &FIXReader::callback_processor)
        , _read_buffer_rptr(_read_buffer), _read_buffer_wptr(_read_buffer)
        , _bg_sz()
	{
		set_preamble_sz();
	}

	/// Dtor.
	virtual ~FIXReader() {}

	/// Start the processing threads.
	virtual void start()
	{
		_socket_error = false;
		if (_pmodel != pm_coro)
			AsyncSocket<f8String>::start();
		if (_pmodel == pm_pipeline)
		{
			if (_callback_thread.start())
				_socket_error = true;
		}
	}

	/// Stop the processing threads and quit.
	virtual void quit()
	{
		if (_pmodel == pm_pipeline)
			_callback_thread.kill(1);
		AsyncSocket<f8String>::quit();
	}

	/// Send a message to the processing method instructing it to quit.
	virtual void stop()
	{
		if (_pmodel == pm_pipeline)
		{
			const f8String from;
			_msg_queue.try_push(from);
		}
	}

	/*! Reader thread method. Reads messages and places them on the queue for processing.
	    Supports pipelined, threaded and coroutine process models.
		 \return 0 on success */
   virtual int execute();

	/*! Wait till writer thread has finished.
	    \return 0 on success */
   int join() { return _pmodel != pm_coro ? AsyncSocket<f8String>::join() : -1; }

	/// Calculate the length of the Fix message preamble, e.g. "8=FIX.4.4^A9=".
	void set_preamble_sz();

	/*! Check to see if the socket is in error
	    \return true if there was a socket error */
	bool is_socket_error() const { return _socket_error; }

	/*! Check to see if there is any data waiting to be read
	    \return true of data ready */
	bool poll() const
	{
		static const Poco::Timespan ts;
		return _sock->poll(ts, Poco::Net::Socket::SELECT_READ);
	}
};

//----------------------------------------------------------------------------------------
/// Fix message writer
class FIXWriter : public AsyncSocket<Message *>
{
	f8_spin_lock _con_spl;

public:
	/*! Ctor.
	    \param sock connected socket
	    \param session session
	    \param pmodel process model */
	FIXWriter(Poco::Net::StreamSocket *sock, Session& session, const ProcessModel pmodel=pm_pipeline)
		: AsyncSocket<Message *>(sock, session, pmodel) {}

	/// Dtor.
	virtual ~FIXWriter() {}

	/*! Place Fix message on outbound message queue.
	    \param from message to send
	    \param destroy if true delete after send
	    \return true in success */
	bool write(Message *from, bool destroy)
	{
		if (_pmodel == pm_pipeline) // pipeline mode ignores destroy flag
			return _msg_queue.try_push(from);
		f8_scoped_spin_lock guard(_con_spl);
		if (destroy)
		{
			scoped_ptr<Message> msg(from);
			return _session.send_process(msg.get());
		}
		return _session.send_process(from);
	}

	/*! Place Fix messages on outbound message queue as a single batch.
	    \param msgs messages to send
	    \param destroy if true delete after send
	    \return count of messages written */
	size_t write_batch(const std::vector<Message *>& msgs, bool destroy)
	{
		if (msgs.empty())
			return 0;
		if (msgs.size() == 1)
			return write(msgs.front(), destroy) ? 1 : 0;
		size_t result(0);
		f8_scoped_spin_lock guard(_con_spl);
		for (std::vector<Message *>::const_iterator itr(msgs.begin()), eitr(msgs.end()), litr(eitr-1); itr != eitr; ++itr)
		{
			Message* msg = *itr;
			msg->set_end_of_batch(itr == litr);
			if (_pmodel == pm_pipeline) // pipeline mode ignores destroy flag
			{
				_msg_queue.try_push(msg);
				++result;
				continue;
			}
			if (_session.send_process(msg))
				++result;
		}
		if (destroy && _pmodel != pm_pipeline)
		{
			for (std::vector<Message *>::const_iterator itr(msgs.begin()), eitr(msgs.end()); itr != eitr; ++itr)
			{
				scoped_ptr<Message> smsg(*itr);
			}
		}
		///@todo: need assert on result==msgs.size()
		return result;
	}
	/*! Wait till writer thead has finished.
	    \return 0 on success */
   int join() { return _pmodel == pm_pipeline ? AsyncSocket<Message *>::join() : -1; }

	/*! Send Fix message directly
	    \param from message to send
	    \return true in success */
	bool write(Message& from)
	{
		if (_pmodel == pm_pipeline) // not permitted if pipeling
			throw f8Exception("cannot send message directly if pipelining");
		f8_scoped_spin_lock guard(_con_spl);
		return _session.send_process(&from);
	}

	/*! Check to see if a write would block
	    \return true if a write would block */
	bool poll() const
	{
		static const Poco::Timespan ts;
		return _sock->poll(ts, Poco::Net::Socket::SELECT_WRITE);
	}

	/*! Send message over socket.
	    \param data char * buffer to send
	    \param remaining number of bytes
	    \return number of bytes sent */
	int send(const char *data, size_t remaining)
	{
		unsigned wrdone(0);

		while (remaining > 0)
		{
			const int wrtSz(_sock->sendBytes(data + wrdone, remaining));
			if (wrtSz < 0)
			{
				if (errno == EAGAIN
#if defined EWOULDBLOCK && EAGAIN != EWOULDBLOCK
					|| errno == EWOULDBLOCK
#endif
				)
					continue;
				throw PeerResetConnection("send: connection gone");
			}

			wrdone += wrtSz;
			remaining -= wrtSz;
		}

		return wrdone;
	}

	/// Start the processing threads.
	virtual void start()
	{
		if (_pmodel == pm_pipeline)
			AsyncSocket<Message *>::start();
	}

	/// Send a message to the processing method instructing it to quit.
	virtual void stop() { _msg_queue.try_push(0); }

    /*! Writer thread method. Reads messages from the queue and sends them over the socket.
        \return 0 on success */
    virtual int execute();
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
	Poco::Net::SocketAddress _addr;
	f8_atomic<bool> _connected;
	Session& _session;
	Role _role;
	ProcessModel _pmodel;
	unsigned _hb_interval, _hb_interval20pc;

	FIXReader _reader;
	FIXWriter _writer;
	bool _secured;

public:
	/*! Ctor. Initiator.
	    \param sock connected socket
	    \param addr sock address structure
	    \param session session
	    \param pmodel process model
	    \param hb_interval heartbeat interval
		 \param secured true for ssl connection
	*/
	Connection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr, Session &session, // client
				  const ProcessModel pmodel, const unsigned hb_interval, bool secured)
		: _sock(sock), _addr(addr), _session(session), _role(cn_initiator), _pmodel(pmodel),
        _hb_interval(hb_interval), _reader(sock, session, pmodel), _writer(sock, session, pmodel),
		  _secured(secured)
	{
		_connected = false;
	}

	/*! Ctor. Acceptor.
	    \param sock connected socket
	    \param addr sock address structure
	    \param session session
	    \param hb_interval heartbeat interval
	    \param pmodel process model
		 \param secured true for ssl connection
	*/
	Connection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr, Session &session, // server
				  const unsigned hb_interval, const ProcessModel pmodel, bool secured)
		: _sock(sock), _addr(addr), _session(session), _role(cn_acceptor), _pmodel(pmodel),
		  _hb_interval(hb_interval), _hb_interval20pc(hb_interval + hb_interval / 5),
		  _reader(sock, session, pmodel), _writer(sock, session, pmodel),
		  _secured(secured)
	{
		_connected = true;
	}

	/// Dtor.
	virtual ~Connection() {}

	/*! Get the role for this connection.
	    \return the role */
	Role get_role() const { return _role; }

	/*! Get the process model
	  \return the process model */
	ProcessModel get_pmodel() const { return _pmodel; }

	/*! Check if this connection is secure
	  \return true if secure */
	bool is_secure() const { return _secured; }

	/// Start the reader and writer threads.
	void start();

	/// Stop the reader and writer threads.
	void stop();

	/*! Get the connection state.
	    \return true if connected */
	virtual bool connect() { return _connected; }

	/*! Determine if this session is actually connected
	  \return true if connected */
	bool is_connected() const { return _connected; }

	/*! Write a message to the underlying socket.
	    \param from Message to write
	    \return true on success */
	virtual bool write(Message *from, bool destroy=true) { return _writer.write(from, destroy); }

	/*! Write a message to the underlying socket. Non-pipelined version.
	    \param from Message to write
	    \return true on success */
	virtual bool write(Message& from) { return _writer.write(from); }

	/*! Write messages to the underlying socket as a single batch.
	    \param from Message to write
	    \return true on success */
	size_t write_batch(const std::vector<Message *>& msgs, bool destroy) { return _writer.write_batch(msgs, destroy); }

	/*! Write a string message to the underlying socket.
	    \param from Message (string) to write
	    \param sz number bytes to send
	    \return number of bytes written */
	int send(const char *from, size_t sz) { return _writer.send(from, sz); }

	/*! Write a string message to the underlying socket.
	    \param from Message (string) to write
	    \return number of bytes written */
	int send(const f8String& from) { return _writer.send(from.data(), from.size()); }

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

	/*! Get the peer socket address
	    \return peer socket address reference */
	const Poco::Net::SocketAddress get_peer_socket_address() const { return _sock->peerAddress(); }

	/*! Get the socket address
	    \return socket address reference */
	const Poco::Net::SocketAddress& get_socket_address() const { return _addr; }

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

	/*! Set the tcp_cork flag
	    \param way boolean true (on) or false(clear) */
	void set_tcp_cork_flag(bool way) const
	{
#if defined HAVE_DECL_TCP_CORK && TCP_CORK != 0
		_sock->setOption(IPPROTO_TCP, TCP_CORK, way ? 1 : 0);
#endif
	}

	/*! Get the session associated with this connection.
	    \return the session */
	Session& get_session() { return _session; }

	/*! Call the FIXreader method
	    \return result of call */
	int reader_execute() { return _reader.execute(); }

	/*! Check if the reader will block
	    \return true if won't block */
	bool reader_poll() const { return _reader.poll(); }

	/*! Call the FIXreader method
	    \return result of call */
	int writer_execute() { return _writer.execute(); }

	/*! Check if the writer will block
	    \return true if won't block */
	bool writer_poll() const { return _writer.poll(); }
};

//-------------------------------------------------------------------------------------------------
/// Client (initiator) specialisation of Connection.
class ClientConnection : public Connection
{
	const bool _no_delay;

public:
	/*! Ctor. Initiator.
	    \param sock connected socket
	    \param addr sock address structure
	    \param session session
	    \param hb_interval heartbeat interval
	    \param pmodel process model
	    \param no_delay set or clear the tcp no delay flag on the socket
		 \param secured true for ssl connection
	*/
    ClientConnection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr,
							Session &session, const unsigned hb_interval, const ProcessModel pmodel=pm_pipeline, const bool no_delay=true,
							bool secured=false)
		 : Connection(sock, addr, session, pmodel, hb_interval, secured), _no_delay(no_delay) {}

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
	    \param addr sock address structure
	    \param session session
	    \param hb_interval heartbeat interval
	    \param pmodel process model
	    \param no_delay set or clear the tcp no delay flag on the socket
		 \param secured true for ssl connection
	*/
	ServerConnection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr,
						  Session &session, const unsigned hb_interval, const ProcessModel pmodel=pm_pipeline, const bool no_delay=true,
						  bool secured=false	) :
		Connection(sock, addr, session, hb_interval, pmodel, secured)
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
