//-------------------------------------------------------------------------------------------------
#if 0

FIX8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <fix@fix8.org>
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
class AsyncHalfDuplexSocket
{
	Thread<AsyncHalfDuplexSocket> _thread;

protected:
	Poco::Net::StreamSocket *_sock;
	tbb::concurrent_bounded_queue<f8String> _msg_queue;

public:
	AsyncHalfDuplexSocket(Poco::Net::StreamSocket *sock)  : _thread(ref(*this)), _sock(sock) {}
	virtual ~AsyncHalfDuplexSocket() {}
	size_t queued() const { return _msg_queue.size(); }
	virtual int operator()() = 0;
	virtual void start() { _thread.Start(); }
	Poco::Net::StreamSocket *socket() { return _sock; }
};

//----------------------------------------------------------------------------------------
class FIXReader : public AsyncHalfDuplexSocket
{
	static RegExp _hdr;
	static const size_t _max_msg_len = 1024, _chksum_sz = 7;

	Thread<FIXReader> _callback_thread;
	int callback_processor();

	size_t _bg_sz; // 8=FIXx.x^A9=x
	Session& _session;
	bool (Session::*_callback)(const f8String&);

	bool read(f8String& to);

protected:
	int operator()();

public:
	FIXReader(Poco::Net::StreamSocket *sock, Session& session, bool (Session::*callback)(const f8String&))
		: AsyncHalfDuplexSocket(sock),
		_callback_thread(ref(*this), &FIXReader::callback_processor), _bg_sz(),
		_session(session), _callback(callback)
	{
		set_preamble_sz();
	}

	virtual ~FIXReader() {}

	virtual void start()
	{
		AsyncHalfDuplexSocket::start();
		_callback_thread.Start();
	}

	void set_preamble_sz();
	Session& get_session() { return _session; }
};

//----------------------------------------------------------------------------------------
class FIXWriter : public AsyncHalfDuplexSocket
{
protected:
	int operator()();

public:
	FIXWriter(Poco::Net::StreamSocket *sock) : AsyncHalfDuplexSocket(sock) {}
	virtual ~FIXWriter() {}

	bool write(const f8String& from);
};

//----------------------------------------------------------------------------------------
class Connection
{
public:
	enum Role { cn_acceptor, cn_initiator };

protected:
	Poco::Net::StreamSocket *_sock;
	bool _connected;
	Role _role;
	unsigned _hb_interval;

	FIXReader *_reader;
	FIXWriter _writer;

public:
	Connection(Poco::Net::StreamSocket *sock, FIXReader *reader)	// client
		: _sock(sock), _connected(), _role(cn_initiator),
		_hb_interval(), _reader(reader), _writer(sock) {} // takes owership

	Connection(Poco::Net::StreamSocket *sock, FIXReader *reader, const unsigned hb_interval) // server
		: _sock(sock), _connected(true), _role(cn_acceptor), _hb_interval(hb_interval),
		  _reader(reader), _writer(sock) {} // takes owership

	virtual ~Connection() { delete _reader; }

	const Role get_role() const { return _role; }
	void start();
	virtual bool connect() { return _connected; }
	virtual bool write(const f8String& from) { return _writer.write(from); }
	void set_hb_interval(const unsigned hb_interval) { _hb_interval = hb_interval; }
	unsigned get_hb_interval() const { return _hb_interval; }
};

//-------------------------------------------------------------------------------------------------
class ClientConnection : public Connection
{
	Poco::Net::SocketAddress _addr;

public:
	ClientConnection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr, FIXReader *reader)
		: Connection(sock, reader), _addr(addr) {}
	virtual ~ClientConnection() {}
	bool connect();
};

//-------------------------------------------------------------------------------------------------
class ServerConnection : public Connection
{

public:
	ServerConnection(Poco::Net::StreamSocket *sock, FIXReader *reader, const unsigned hb_interval) :
		Connection(sock, reader, hb_interval) {}
	virtual ~ServerConnection() {}
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_CONNECTION_HPP_
