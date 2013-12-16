//-----------------------------------------------------------------------------------------
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
/*
mockConnection.hpp and mockConnection.cpp are used to supply a mock connection object for unit tests
*/
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_CONNECTION_HPP_
#define _FIX8_CONNECTION_HPP_

#include <Poco/Net/StreamSocket.h>
#include <Poco/Timespan.h>
#include <Poco/Net/NetException.h>

//----------------------------------------------------------------------------------------
namespace FIX8 {

class Session;

/// mock AsyncSocket
template <typename T>
class AsyncSocket
{

protected:
    Session& _session;
	 ProcessModel _pmodel;

public:
    AsyncSocket(Poco::Net::StreamSocket *sock, Session& session, const ProcessModel pmodel=pm_pipeline)
        :  _session(session), _pmodel(pmodel) {}

    virtual ~AsyncSocket() {}

    size_t queued() const { return 0; }
    virtual int operator()() = 0;

    ///empty function
    virtual void start() {}

     ///empty function
    virtual void quit() {}

    Poco::Net::StreamSocket *socket() { return 0; }
    int join() { return -1; }
};

//----------------------------------------------------------------------------------------
/// mock Fix message reader
class FIXReader : public AsyncSocket<f8String>
{
    int callback_processor();

    bool read(f8String& to);

    int sockRead(char *where, size_t sz) { return 0; }

protected:
    int operator()();

public:
    /// Ctor
    FIXReader(Poco::Net::StreamSocket *sock, Session& session, const ProcessModel pmodel=pm_pipeline)
        : AsyncSocket<f8String>(sock, session, pmodel)
    {
        set_preamble_sz();
    }

    /// Dtor
    virtual ~FIXReader() {}

    ///empty function
    virtual void start() {}

    ///empty function
    virtual void quit() {}

    ///empty function
    virtual void stop() {}

    ///empty function
    void set_preamble_sz();

    /*!empty function
          \return always return false*/
    bool is_socket_error() const { return false; }
};

//----------------------------------------------------------------------------------------
/// mock Fix message writer
class FIXWriter : public AsyncSocket<Message *>
{
protected:
    int operator()();

public:

    /// Ctor
    FIXWriter(Poco::Net::StreamSocket *sock, Session& session, const ProcessModel pmodel=pm_pipeline)
    : AsyncSocket<Message *>(sock, session, pmodel)
    {}

    /// Dtor
    virtual ~FIXWriter() {}

    /*!empty function
          \return always return true*/
    bool write(Message *from, bool)
    {
        return true;
    }

    /*!empty function
          \return always return true*/
	size_t write_batch(const std::vector<Message *>& msgs, bool destroy)
	{
		return msgs.size();
	}

    /*!empty function
          \return always return true*/
    bool write(Message& from)
    {
        return true;
    }

    /*!empty function
          \param msg message string to be sent
          \return return the message length*/
    int send(const f8String& msg)
    {
        return msg.length();
    }

    ///empty function
    virtual void start() {}

    ///empty function
    virtual void stop() {}
};

/// mock connection
class Connection
{
public:
    enum Role { cn_acceptor, cn_initiator, cn_unknown };
    std::vector<f8String>  _output;

protected:
    bool _connected;
    Session& _session;
    Role _role;
	 ProcessModel _pmodel;
    unsigned _hb_interval, _hb_interval20pc;
    bool _started;
	 Poco::Net::SocketAddress _addr;

public:

    /// Ctor
    Connection(Poco::Net::StreamSocket *sock, Session &session, const ProcessModel pmodel)   // client
        : _connected(false), _session(session), _role(cn_initiator), _pmodel(pmodel),
        _hb_interval(10), _started(false){}

    /// Ctor
    Connection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval, const ProcessModel pmodel) // server
        : _connected(true), _session(session), _role(cn_acceptor), _pmodel(pmodel), _hb_interval(hb_interval),
        _hb_interval20pc(hb_interval + hb_interval / 5), _started(false){}

    /// Dtor
    virtual ~Connection() {}

    /*! Get the role for this connection.
        \return the role */
    Role get_role() const { return _role; }

	/*! Determine if this session is actually connected
	  \return true if connected */
	bool is_connected() const { return _connected; }

	/*! Get the process model
	  \return the process model */
	 ProcessModel get_pmodel() const { return _pmodel; }

    ///set the state to started
    void start();

    ///set the state to stopped
    void stop();


    /*! Get the connection state.
        \return true if connected */

    virtual bool connect() { return _connected; }

    /*!helper to unit test, cache the message in string format
          \param from message to be sent
          \return always return true*/

    virtual bool write(Message *from, bool)
    {
		  char output[MAX_MSG_LENGTH + HEADER_CALC_OFFSET], *ptr(output);
        from->encode(&ptr);
        _output.push_back(ptr);
        return true;
    }

    /*!helper to unit test, cache the messages in string format
          \param from message to be sent
          \return always return true*/
	size_t write_batch(const std::vector<Message *>& msgs, bool destroy)
	{
		char output[MAX_MSG_LENGTH + HEADER_CALC_OFFSET], *ptr(output);
		for(std::vector<Message*>::const_iterator cit=msgs.begin(); cit != msgs.end(); ++cit)
		{
			(*cit)->encode(&ptr);
        _output.push_back(ptr);
		}
		return msgs.size();
	}

    /*!helper to unit test, cache the message in string format
          \param from message to be sent
          \return  always return true*/

    virtual bool write(Message& from)
    {
		  char output[MAX_MSG_LENGTH + HEADER_CALC_OFFSET], *ptr(output);
        from.encode(&ptr);
        _output.push_back(ptr);
        return true;
    }

    /*!helper to unit test, cache the message in string format
          \param from message string to be sent
          \return return the length of the message*/

    int send(const f8String& from)
    {
        _output.push_back(from);
        return from.length();
    }

    /*!helper to unit test, cache the message in string format
          \param from message string to be sent
          \return return the length of the message*/

    int send(const char *from, size_t sz)
    {
        _output.push_back(f8String(from, sz));
        return sz;
    }

    /*! Set the heartbeat interval for this connection.
        \param hb_interval heartbeat interval */

    void set_hb_interval(const unsigned hb_interval)
    {
        _hb_interval = hb_interval; _hb_interval20pc = hb_interval + hb_interval / 5;
    }

    /*! Get the heartbeat interval for this connection.
        \return the heartbeat interval */

    unsigned get_hb_interval() const { return _hb_interval; }


    /*! Get the heartbeat interval + %20 for this connection.
        \return the heartbeat interval + %20 */

    unsigned get_hb_interval20pc() const { return _hb_interval20pc; }

    /*!empty function
          \return always return 0*/

    int join() { return 0; }

    /*!empty function
          \return always return false*/

    bool is_socket_error() const { return false; }

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

    /// empty function
    void set_recv_buf_sz(const unsigned sz) const {}

    /// empty function
    void set_send_buf_sz(const unsigned sz) const {}

    /// empty function
    void set_tcp_cork_flag(bool way) const {}

    ///Get the session associated with this connection
    Session& get_session() { return _session; }

	const Poco::Net::SocketAddress& get_socket_address() const { return _addr; }
	const Poco::Net::SocketAddress get_peer_socket_address() const { return Poco::Net::SocketAddress(); }
};
//-------------------------------------------------------------------------------------------------
///  mock Client (initiator) specialisation of Connection.
class ClientConnection : public Connection
{
    Poco::Net::SocketAddress _addr;

public:
    /// Ctor
    ClientConnection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr, Session &session,
		 const ProcessModel pmodel=pm_pipeline, const bool no_delay=true)
        : Connection(sock, session, pmodel), _addr(addr) {}

    /// Dtor
    virtual ~ClientConnection() {}


    /*! Set the state to connected
        \return always return true */
    bool connect()
    {
        _connected  = true;
        return true;
    };
};

//-------------------------------------------------------------------------------------------------
/// mock Server (acceptor) specialisation of Connection.

class ServerConnection : public Connection
{
public:

    /// Ctor
    ServerConnection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval,
		 const ProcessModel pmodel=pm_pipeline, const bool no_delay=true) :
        Connection(sock, session, hb_interval, pmodel) {}

    /// Dtor
    virtual ~ServerConnection() {}
};

//-------------------------------------------------------------------------------------------------

}

#endif // _FIX8_CONNECTION_HPP_

