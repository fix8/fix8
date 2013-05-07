//-------------------------------------------------------------------------------------------------
#if 0
mockConnection.hpp and mockConnection.cpp are used to supply a mock connection object for unit tests
#endif
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
    bool _pipelined;

public:
    AsyncSocket(Poco::Net::StreamSocket *sock, Session& session, const bool pipelined=true)
        :  _session(session), _pipelined(pipelined) {}

    virtual ~AsyncSocket() {}

    size_t queued() const { return 0; }
    virtual int operator()() = 0;

    ///empty function
    virtual void start() {}

     ///empty function
    virtual void quit() {}

    Poco::Net::StreamSocket *socket() { return NULL; }
    int join() { return -1; }
};

//----------------------------------------------------------------------------------------
/// mock Fix message reader
class FIXReader : public AsyncSocket<f8String>
{
    int callback_processor();

    size_t _bg_sz;
    bool read(f8String& to);

    int sockRead(char *where, size_t sz){}

protected:
    int operator()();

public:
    /// Ctor
    FIXReader(Poco::Net::StreamSocket *sock, Session& session, const bool pipelined=true)
        : AsyncSocket<f8String>(sock, session, pipelined), _bg_sz()
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
    FIXWriter(Poco::Net::StreamSocket *sock, Session& session, const bool pipelined=true)
    : AsyncSocket<Message *>(sock, session, pipelined)
    {}

    /// Dtor
    virtual ~FIXWriter() {}

    /*!empty function
          \return always return true*/
    bool write(Message *from)
    {
        return true;
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
    unsigned _hb_interval, _hb_interval20pc;
    bool _started;

public:

    /// Ctor
    Connection(Poco::Net::StreamSocket *sock, Session &session, const bool pipelined)   // client
        : _connected(false), _session(session), _role(cn_initiator),
        _hb_interval(10), _started(false){}

    /// Ctor
    Connection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval, const bool pipelined) // server
        : _connected(true), _session(session), _role(cn_acceptor), _hb_interval(hb_interval),
        _hb_interval20pc(hb_interval + hb_interval / 5), _started(false){}

    /// Dtor
    virtual ~Connection() {}

    /*! Get the role for this connection.
        \return the role */

    Role get_role() const { return _role; }

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

    virtual bool write(Message *from)
    {
        f8String output;
        from->encode(output);
        _output.push_back(output);
        return true;
    }

    /*!helper to unit test, cache the message in string format
          \param from message to be sent
          \return  always return true*/

    virtual bool write(Message& from)
    {
        f8String output;
        from.encode(output);
        _output.push_back(output);
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

    ///Get the session associated with this connection
    Session& get_session() { return _session; }
};
//-------------------------------------------------------------------------------------------------
///  mock Client (initiator) specialisation of Connection.
class ClientConnection : public Connection
{
    Poco::Net::SocketAddress _addr;
    const bool _no_delay;

public:
    /// Ctor
    ClientConnection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr, Session &session, const bool pipelined=true,
            const bool no_delay=true)
        : Connection(sock, session, pipelined), _addr(addr), _no_delay(no_delay) {}

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
    ServerConnection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval, const bool pipelined=true,
            const bool no_delay=true) :
        Connection(sock, session, hb_interval, pipelined)
    {
    }

    /// Dtor
    virtual ~ServerConnection() {}
};

//-------------------------------------------------------------------------------------------------

}

#endif

