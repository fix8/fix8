#ifndef _FIX8_CONNECTION_HPP_
#define _FIX8_CONNECTION_HPP_


#include <Poco/Net/StreamSocket.h>
#include <Poco/Timespan.h>
#include <Poco/Net/NetException.h>

//----------------------------------------------------------------------------------------
namespace FIX8 {

class Session;

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

    virtual void start() {}
    virtual void quit() {}

    Poco::Net::StreamSocket *socket() { return NULL; }
    int join() { return -1; }
};

//----------------------------------------------------------------------------------------
/// Fix message reader
class FIXReader : public AsyncSocket<f8String>
{
    int callback_processor();

    size_t _bg_sz;
    bool read(f8String& to);

    int sockRead(char *where, size_t sz)
    {
    }

protected:
    int operator()();

public:
    FIXReader(Poco::Net::StreamSocket *sock, Session& session, const bool pipelined=true)
        : AsyncSocket<f8String>(sock, session, pipelined), _bg_sz()
    {
        set_preamble_sz();
    }

    virtual ~FIXReader() {}

    virtual void start()
    {
    }

    virtual void quit()
    {
    }

    virtual void stop()
    {
    }

    void set_preamble_sz();

    bool is_socket_error() const { return false; }
};

//----------------------------------------------------------------------------------------
/// Fix message writer
class FIXWriter : public AsyncSocket<Message *>
{
protected:
    int operator()();

public:
    FIXWriter(Poco::Net::StreamSocket *sock, Session& session, const bool pipelined=true)
    : AsyncSocket<Message *>(sock, session, pipelined)
    {}

    virtual ~FIXWriter() {}
    bool write(Message *from)
    {
        return true;
    }

    bool write(Message& from)
    {
        return true;
    }
    int send(const f8String& msg)
    {
        return msg.length();
    }

    virtual void start()
    {}

    virtual void stop() {}
};

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
    Connection(Poco::Net::StreamSocket *sock, Session &session, const bool pipelined)   // client
        : _connected(false), _session(session), _role(cn_initiator),
        _hb_interval(10), _started(false){}

    Connection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval, const bool pipelined) // server
        : _connected(true), _session(session), _role(cn_acceptor), _hb_interval(hb_interval),
        _hb_interval20pc(hb_interval + hb_interval / 5), _started(false){}

    virtual ~Connection() {}

    Role get_role() const { return _role; }

    void start();
    void stop();

    virtual bool connect() { return _connected; }

    virtual bool write(Message *from)
    {
        f8String output;
        from->encode(output);
        _output.push_back(output);
        return true;
    }

    int send(const f8String& from)
    {
        _output.push_back(from);
        //std::cout << *_output << std::endl;
        return from.length();
    }

    void set_hb_interval(const unsigned hb_interval)
        {
            _hb_interval = hb_interval; _hb_interval20pc = hb_interval + hb_interval / 5;
        }

    unsigned get_hb_interval() const { return _hb_interval; }

    unsigned get_hb_interval20pc() const { return _hb_interval20pc; }

    int join() { return 0; }

    bool is_socket_error() const { return false; }

    static void set_recv_buf_sz(const unsigned sz, Poco::Net::Socket *sock)
    {
        const unsigned current_sz(sock->getReceiveBufferSize());
        sock->setReceiveBufferSize(sz);
        std::ostringstream ostr;
        ostr << "ReceiveBufferSize old:" << current_sz << " requested:" << sz << " new:" << sock->getReceiveBufferSize();
        GlobalLogger::log(ostr.str());
    }

    static void set_send_buf_sz(const unsigned sz, Poco::Net::Socket *sock)
    {
        const unsigned current_sz(sock->getSendBufferSize());
        sock->setSendBufferSize(sz);
        std::ostringstream ostr;
        ostr << "SendBufferSize old:" << current_sz << " requested:" << sz << " new:" << sock->getSendBufferSize();
        GlobalLogger::log(ostr.str());
    }
    void set_recv_buf_sz(const unsigned sz) const {}

    void set_send_buf_sz(const unsigned sz) const {}

    Session& get_session() { return _session; }
};
//-------------------------------------------------------------------------------------------------
class ClientConnection : public Connection
{
    Poco::Net::SocketAddress _addr;
    const bool _no_delay;

public:
    ClientConnection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr, Session &session, const bool pipelined=true,
            const bool no_delay=true)
        : Connection(sock, session, pipelined), _addr(addr), _no_delay(no_delay) {}

    virtual ~ClientConnection() {}
    bool connect()
    {
        _connected  = true;
        return true;
    };
};

//-------------------------------------------------------------------------------------------------
class ServerConnection : public Connection
{
public:
    ServerConnection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval, const bool pipelined=true,
            const bool no_delay=true) :
        Connection(sock, session, hb_interval, pipelined)
    {
    }

    virtual ~ServerConnection() {}
};

//-------------------------------------------------------------------------------------------------

}

#endif

