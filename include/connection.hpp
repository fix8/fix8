//-------------------------------------------------------------------------------------------------
#if 0

FIX8 is released under the New BSD License.

Copyright (c) 2010-11, David L. Dight <fix@fix8.org>
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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

---------------------------------------------------------------------------------------------------
$Id: session.hpp 540 2010-11-05 21:25:33Z davidd $
$Date: 2010-11-06 08:25:33 +1100 (Sat, 06 Nov 2010) $
$URL: svn://catfarm.electro.mine.nu/usr/local/repos/fix8/include/session.hpp $

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
template <typename T>
class AsyncHalfDuplexSocket
{
	Thread<AsyncHalfDuplexSocket> _thread;

protected:
	Poco::Net::StreamSocket *_sock;
	tbb::concurrent_bounded_queue<T> _msg_queue;
	Session& _session;

public:
	AsyncHalfDuplexSocket(Poco::Net::StreamSocket *sock, Session& session)
		: _thread(ref(*this)), _sock(sock), _session(session) {}
	virtual ~AsyncHalfDuplexSocket() {}
	size_t queued() const { return _msg_queue.size(); }
	virtual int operator()() = 0;
	virtual void start() { _thread.Start(); }
	virtual void quit() { _thread.Kill(1); }
	Poco::Net::StreamSocket *socket() { return _sock; }
	int join() { return _thread.Join(); }
};

//----------------------------------------------------------------------------------------
class FIXReader : public AsyncHalfDuplexSocket<f8String>
{
	static RegExp _hdr;
	static const size_t _max_msg_len = 1024, _chksum_sz = 7;

	Thread<FIXReader> _callback_thread;
	int callback_processor();

	size_t _bg_sz; // 8=FIXx.x^A9=x

	bool read(f8String& to);
	int sockRead(char *where, size_t sz);

protected:
	int operator()();

public:
	FIXReader(Poco::Net::StreamSocket *sock, Session& session)
		: AsyncHalfDuplexSocket<f8String>(sock, session), _callback_thread(ref(*this), &FIXReader::callback_processor), _bg_sz()
	{
		set_preamble_sz();
	}

	virtual ~FIXReader() {}

	virtual void start()
	{
		AsyncHalfDuplexSocket<f8String>::start();
		_callback_thread.Start();
	}

	virtual void quit() { _callback_thread.Kill(1); AsyncHalfDuplexSocket<f8String>::quit(); }
	virtual void stop() { const f8String from; _msg_queue.try_push(from); }

	void set_preamble_sz();
};

//----------------------------------------------------------------------------------------
class FIXWriter : public AsyncHalfDuplexSocket<Message *>
{
protected:
	int operator()();

public:
	FIXWriter(Poco::Net::StreamSocket *sock, Session& session) : AsyncHalfDuplexSocket<Message *>(sock, session) {}
	virtual ~FIXWriter() {}

	bool write(Message *from) { return _msg_queue.try_push(from); }
	int send(const f8String& msg) { return _sock->sendBytes(msg.data(), msg.size()); }
	virtual void stop() { _msg_queue.try_push(0); }
};

//----------------------------------------------------------------------------------------
class Connection
{
public:
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
	Connection(Poco::Net::StreamSocket *sock, Session &session)	// client
		: _sock(sock), _connected(), _session(session), _role(cn_initiator),
		_hb_interval(10), _reader(sock, session), _writer(sock, session) {}

	Connection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval) // server
		: _sock(sock), _connected(true), _session(session), _role(cn_acceptor), _hb_interval(hb_interval),
		_hb_interval20pc(hb_interval + hb_interval / 5),
		  _reader(sock, session), _writer(sock, session) {}

	virtual ~Connection() {}

	const Role get_role() const { return _role; }
	void start();
	void stop();
	virtual bool connect() { return _connected; }
	virtual bool write(Message *from) { return _writer.write(from); }
	int send(const f8String& from) { return _writer.send(from); }
	void set_hb_interval(const unsigned hb_interval)
		{ _hb_interval = hb_interval; _hb_interval20pc = hb_interval + hb_interval / 5; }
	unsigned get_hb_interval() const { return _hb_interval; }
	unsigned get_hb_interval20pc() const { return _hb_interval20pc; }
	int join() { return _reader.join(); }
	Session& get_session() { return _session; }
};

//-------------------------------------------------------------------------------------------------
class ClientConnection : public Connection
{
	Poco::Net::SocketAddress _addr;

public:
	ClientConnection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr, Session &session)
		: Connection(sock, session), _addr(addr) {}
	virtual ~ClientConnection() {}
	bool connect();
};

//-------------------------------------------------------------------------------------------------
class ServerConnection : public Connection
{

public:
	ServerConnection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval) :
		Connection(sock, session, hb_interval) {}
	virtual ~ServerConnection() {}
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_CONNECTION_HPP_
