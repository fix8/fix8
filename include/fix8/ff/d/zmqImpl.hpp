/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/**
 *  \link
 *  \file zmqImpl.hpp
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief This file describes the communication patterns of distributed
 *  FastFlow using ØMQ.
 */

/* ***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************
 */

#ifndef FF_ZMQIMPL_HPP
#define FF_ZMQIMPL_HPP


#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>
#include <zmq.hpp>
#include <fix8/ff/svector.hpp>
#include <fix8/ff/d/zmqTransport.hpp>

namespace ff {

/**
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  @{
 */

#define CONNECTION_PROTOCOL 1
//#undef CONNECTION_PROTOCOL
#if defined(DEBUG)
 #define zmqTDBG(x) x
#else
 #define zmqTDBG(x)
#endif

/* ---------------- Pattern macro definitions -------------------- */

#define UNICAST                          commPattern<zmq1_1>
#define UNICAST_DESC(name,trasp,P)       UNICAST::descriptor(name,1,trasp,P)
#define BROADCAST                        commPattern<zmqBcast>
#define BROADCAST_DESC(name,n,trasp,P)   BROADCAST::descriptor(name,n,trasp,P)
#define ALLGATHER                        commPattern<zmqAllGather>
#define ALLGATHER_DESC(name,n,trasp,P)   ALLGATHER::descriptor(name,n,trasp,P)
#define FROMANY                          commPattern<zmqFromAny>
#define FROMANY_DESC(name,n,trasp,P)     FROMANY::descriptor(name,n,trasp,P)
#define SCATTER                          commPattern<zmqScatter>
#define SCATTER_DESC(name,n,trasp,P)     SCATTER::descriptor(name,n,trasp,P)
#define ONETOMANY                        commPattern<zmq1_N>
#define ONETOMANY_DESC(name,n,trasp,P)   ONETOMANY::descriptor(name,n,trasp,P)
#define ONDEMAND                         commPattern<zmqOnDemand>
#define ONDEMAND_DESC(name,n,trasp,P)    ONDEMAND::descriptor(name,n,trasp,P)

/* TODO */
#define MANYTOONE                        commPattern<zmqN_1>
#define MANYTOONE_DESC(name,n,trasp,P)   MANYTOONE::descriptor(name,n,trasp,P)

//**************************************
// One to Many communication
//**************************************

/**
 *  \struct descriptor1_N
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief This struct is used in several communication patterns; Unicast,
 *  Broadcast, Scatter, On-Demand, OneToMany, FromAny etc.
 *
 *  This struct is defined in zmqImpl.hpp
 *
 */
struct descriptor1_N {
    typedef zmqTransportMsg_t  msg_t;

    /**
     * Constructor.
     *
     * It Creates a One-to-Many descriptor, that is, instantiate a
     * transport layer using the first node as a router.
     *
     * \param name is the name of the descriptor.
     * \param peers is the number of peers involved (receiving nodes).
     * \param transport is a pointer to the transport layer object of class
     * zmqTransport.
     * \param P is a flag that identifies whether the descriptor refers to a \p
     * sender (i.e. a Router) or to a \p receiver.
     *
     */
    descriptor1_N(const std::string name, const int peers, zmqTransport* const transport, const bool P):
        name(name), socket(NULL), transport(transport), P(P), peers(P ? peers : 1), zmqHdrSet(false), transportIDs(peers)
    {
        if (transport)
            socket = transport->newEndPoint(!P);
            // NOTE: we want to use ZMQ_ROUTER at producer side !P means ROUTER
            // => create a socket (a new end-point) acting as a ROUTER -- REW,
            // P always true?

        if (P) {
            std::stringstream identity;
            for(int i = 0; i < peers; ++i) {
                identity.str("");
                identity << name << ":" << i;
                transportIDs[i]=identity.str();
            }
        }
    }

    /**
     * Destructor
     */
    ~descriptor1_N() { close(); socket=NULL; }

    /**
     * It closes active connection and delete the existing socket.
     *
     * \return It delets the end point from socket and return NULL. If it is
     * not succefful then a negative value is returned.
     */
    inline int close() {
        if (socket) return transport->deleteEndPoint(socket);
        return -1;
    }

    /**
     *  It initialises a socket and estabilishes a connection to the given address.
     *
     *  \param addr is the IP address in the form 'ip/host:port'
     *  \param nodeId is the node identifier in the range [0..inf[
     *
     *  \exception TODO
     *  \return TODO
     */
    inline int init(const std::string& addr, const int nodeId=-1) {
        if (!socket) return -1;

        if (!P) {   // if ROUTER: routing of messages to a specific connection
            std::stringstream identity;
            identity.str("");
            identity << name << ":" << ((nodeId!=-1) ? nodeId:transport->getProcId());
            // set durable identity. it must be set before connecting the socket
            socket->setsockopt(ZMQ_IDENTITY, identity.str().c_str(), identity.str().length() + 1);

            std::stringstream address;
            address.str("");
            address << "tcp://" << addr;
            socket->connect(address.str().c_str());

#if defined(CONNECTION_PROTOCOL)
            sendHelloRecvHi();
#endif
        } else {    // if DEALER: fair-queuing on input and load-balancing on output
            int i;
            if ((i=addr.find(":"))<0) {
                printf("init: ERROR -- wrong IP address format for %s\n",addr.c_str());
                return -1;
            }
            int port = atoi((addr.substr(i+1)).c_str());
            std::stringstream address;
            address << "tcp://*:" << port;
            try {
                socket->bind(address.str().c_str());
            } catch(std::exception& e) {
                printf("initTransport: ERROR -- binding address (%s): %s\n",
                       address.str().c_str(), e.what());
                return -1;
            }
#if defined(CONNECTION_PROTOCOL)
            recvHelloSendHi();
#endif
        }
        return 0;
    }

    /**
     *  It broadcast a message to all connected peers.
     *
     *  \param msg is a eference to the message to be sent
     *  \param flags is a flag indcating whether the operation should be in
     *  non-blocking mode or if the message is a multi-part message.
     *  See <a href="http://api.zeromq.org/2-1:_start"> ØMQ's API </a> for details.
     *
     *  \return TODO
     *
     *  \exception TODO
     */
    inline bool send(const msg_t& msg, int flags = 0) {
        for(int i=0; i < peers; ++i) {
            msg_t _msg;
            _msg.copy(const_cast<msg_t&>(msg));
            zmq::message_t to(const_cast<char*>(transportIDs[i].c_str()), transportIDs[i].length()+1,0,0);
            try {
                if (!zmqHdrSet) if (!socket->send(to,ZMQ_SNDMORE)) return false;
                if (!socket->send(_msg, flags)) return false;
            } catch (std::exception& e) {
                return false;
            }
        }
        zmqHdrSet=false;
        return true;
    }

    /**
     *  It broadcasts other parts of a multi-parts message to all connected peers.
     *  This method exits if the flag is not set to ZMQ_SNDMORE.
     *
     *  \param msg eference to the message to be sent
     *  \param flags a flag indcating whether the operation should be in
     *  non-blocking mode or if the message is a multi-part message.
     *  See <a href="http://api.zeromq.org/2-1:_start"> ØMQ's API </a> for details.
     *
     *  \return It returns the message.
     */
    inline bool sendmore(const msg_t& msg, int flags) {
        assert(flags==ZMQ_SNDMORE);
        bool v=send(msg,flags);
        zmqHdrSet=true;
        return v;
    }

    /**
     *  It unicasts a message to a specified destination
     *
     *  \param msg is a reference to the message to be sent
     *  \param dest is the ID of the receiving peer.
     *  \param flags is a flag indcating whether the operation should be in
     *  non-blocking mode or if the message is a multi-part message.
     *  See <a href="http://api.zeromq.org/2-1:_start"> ØMQ's API </a> for details.
     *
     *  \exception TODO
     *  \return TODO
     *
     */
    inline bool send(const msg_t& msg, const int dest, int flags) {
        assert(dest<peers);
        zmq::message_t to(const_cast<char*>(transportIDs[dest].c_str()), transportIDs[dest].length()+1,0,0);
        try {
            if (!zmqHdrSet) if (!socket->send(to,ZMQ_SNDMORE)) return false;
            if (!socket->send(const_cast<msg_t&>(msg), flags)) return false;
        } catch (std::exception& e) {
            return false;
        }
        zmqHdrSet=false;
        return true;
    }

    /**
     *  It unicasts other parts of a multi-parts message to a specified
     *  destination. This method exits if the flag is not set to ZMQ_SNDMORE.
     *
     *  \param msg is a reference to the message to be sent
     *  \param dest is the ID of the receiving peer.
     *  \param flags is a flag indcating whether the operation should be in
     *  non-blocking mode or if the message is a multi-part message.
     *  See <a href="http://api.zeromq.org/2-1:_start"> ØMQ's API </a> for details.
     *
     *  \return The contents of the message is returned.
     */
    inline bool sendmore(const msg_t& msg, const int dest, int flags) {
        assert(flags==ZMQ_SNDMORE);
        bool v=send(msg,dest,flags);
        zmqHdrSet=true;
        return v;
    }

    /**
     * It receives a message header
     *
     * \return Contents of the message.
     */
    inline bool recvhdr(msg_t& msg) {
        return recv(msg);
    }

    /**
     * It receives a message after having received the header.
     *
     * \exceptionn TODO
     * \return TODO
     */
    inline bool recv(msg_t& msg) {
        try {
            if(!socket->recv(&msg)) {
                zmqTDBG(printf("recv RETURN -1\n"));
                return false;
            }
        } catch (std::exception & e) {
                return false;
        }
        return true;
        }

    /**
     * It returns the total number of peers (NOTE: if !P,  peers is 1).
     *
     * \return The number of peers.
     */
    inline int getPeers() const { return peers;}

    // variables
    const std::string        name;          // name of descriptor (?)
    zmq::socket_t*           socket;        // zmq socket object
    zmqTransport * const     transport;
    const bool               P;
    const int                peers;
    bool                     zmqHdrSet;
    std::vector<std::string> transportIDs;

#if defined(CONNECTION_PROTOCOL)
    /**
     * It defines the sending of a message.
     */
    inline void sendHelloRecvHi() {
        const char hello[] = "HELLO";
        // sending Hello
        zmqTDBG(printf("%s sending HELLO\n", name.c_str()));
        zmqTransportMsg_t req(hello,6);
        sendreq(req);
        // receive Hi
        zmqTransportMsg_t msg;
        recvreply(msg);
        zmqTDBG(printf("%s received %s\n", name.c_str(), static_cast<char*>(msg.getData())));
    }

    /**
     * It defines the sending of requests.
     *
     * \exception TODO
     * \return TODO
     */
    inline bool sendreq(const msg_t& msg) {
        try {
            if (!socket->send(const_cast<msg_t&>(msg))) return false;
        } catch (std::exception& e) {
            return false;
        }
        return true;
    }

    /**
     * It defines the receiving of reply.
     *
     * \exception TODO
     * \return TODO
     */
    inline int recvreply(msg_t& msg) {
        try {
            if(!socket->recv(&msg)) {
                zmqTDBG(printf("recv RETURN -1\n"));
                return false;
            }
        } catch (std::exception & e) {
            return false;
        }
        return true;
    }

    /**
     * It receives a message from a ZMQ_ROUTER
     *
     * \exception TODO
     * \return TODO
     */
    inline bool recvreq(msg_t& msg) {
        msg_t from;
        for(int i = 0; i < peers; ++i) {
            try {
                if(!socket->recv(&from)) {
                    zmqTDBG(printf("recvreq RETURN -1\n"));
                    return false;
                }
                if(!socket->recv(&msg)) {
                    zmqTDBG(printf("recvreq RETURN -1\n"));
                    return false;
                }
                zmqTDBG(printf("%s received from %s %s\n",name.c_str(),
                               static_cast<char*>(from.getData()),
                               static_cast<char*>(msg.getData())));
            } catch (std::exception & e) {
                return false;
            }
        }
        return true;
    }

    /**
     * It is used in the on-demand pattern by the producer (i.e. the on-demand consumer).
     *
     * \return The contents of the message.
     */
    inline bool sendReq(const msg_t& msg) {
        return sendreq(msg);
    }
    /**
     * It is used in the on-demand pattern by the consumer (i.e. the on-demand producer).
     *
     * \param flags can be ZMQ_NOBLOCK
     *
     * \exception TODO
     * \return TODO
     */
    inline bool recvReq(int& peer, int flags=0) {
        zmq::message_t from;
        zmq::message_t msg;
        try {
            if(!socket->recv(&from, flags)) return false;
            const std::string & f=static_cast<char*>(from.data());
            int i;
            if ((i=f.find(":")) < 0) {
                printf("recvhdr: ERROR: wrong header\n");
                return false;
            }
            peer = atoi((f.substr(i+1)).c_str());
            if(!socket->recv(&msg)) {
                zmqTDBG(printf("recvreq RETURN -1\n"));
                return false;
            }
        } catch (std::exception & e) {
            return false;
        }
        return true;
    }

    /**
     * It is used for sending back the message to the sender.
     *
     * \exception TODO
     * \return TODO
     */
    inline bool sendreply(const msg_t& msg) {
        for(int i = 0; i < peers; ++i) {
            msg_t _msg;
            _msg.copy(const_cast<msg_t&>(msg));
            zmq::message_t to(const_cast<char*>(transportIDs[i].c_str()), transportIDs[i].length()+1,0,0);
            try {
                if (!socket->send(to,ZMQ_SNDMORE)) return false;
                if (!socket->send(_msg)) return false;
            } catch (std::exception& e) {
                return false;
            }
        }
        return true;
    }

    /**
     * It defines the receiving of hi.
     */
    inline void recvHelloSendHi() {
        static const char hi[] = "HI"; // need to be static
        // receives Hello and sends Hi
        zmqTransportMsg_t req;
        recvreq(req);
        zmqTransportMsg_t rep(hi,3);
        send(rep);
        zmqTDBG(printf("%s sending HI\n", name.c_str()));
    }
#endif
}; // end descriptor1_N

//**************************************
// Many to One Communication
//
// used in ALLgather, collect fromANY, ManyToOne
//**************************************

/**
 *  \struct descriptorN_1
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief Descriptor used in several communication patterns: AllGather,
 *  Collect FromAny, ManyToOne.
 *
 *  This class is defined in zmqImpl.hpp
 *
 */
struct descriptorN_1 {
    typedef zmqTransportMsg_t  msg_t;

    /**
     * Constructor.
     *
     * It creates a Many-to-One descriptor, that is, instantiate a transport
     * layer using the first node as a router.
     *
     * \param name is the name of the descriptor
     * \param peers is the number of peers involved (sending nodes)
     * \param transport is the pointer to the transport layer object of class
     * zmqTransport
     * \param P is the flag that identifies whether the descriptor refers to a \p sender
     * (i.e. a Router) or to a \p receiver.
     */
    descriptorN_1(const std::string name, const int peers, zmqTransport* const  transport, const bool P ) :
            name(name), socket(NULL), transport(transport), P(P), peers(P?1:peers), recvHdr(true), transportIDs(peers)
    {
        if (transport) socket = transport->newEndPoint(P);

        if (!P) {
            std::stringstream identity;
            for(int i = 0; i < peers; ++i) {
                identity.str("");
                identity << name << ":" << i;
                transportIDs[i] = identity.str();
            }
        }
    }

    /**
     * Destructor
     */
    ~descriptorN_1() { close(); socket=NULL; }

    /**
     * It closes active connection and delete the existing socket.
     *
     * \return Deletes the end point and a null pointer is returned, otherwise
     * a negative value is returned.
     */
    inline int close() {
        if (socket) return transport->deleteEndPoint(socket);
        return -1;
    }

    /**
     *  It initialises a socket and estabilish a connection to the given
     *  address.
     *
     *  \param addr is the IP address in the form 'ip/host:port'.
     *  \param nodeId is the node identifier in the range [0..inf[.
     *
     *  \exception TODO
     *  \return TODO
     */
    inline int init(const std::string& addr, const int nodeId = -1) {
        if (!socket) return -1;

        if (P) {
            std::stringstream identity;
            identity.str("");
            identity << name << ":" << ((nodeId != -1) ? nodeId:transport->getProcId());
            // set durable identity. it must be set before connecting the socket
            socket->setsockopt(ZMQ_IDENTITY, identity.str().c_str(), identity.str().length() + 1);

            std::stringstream address;
            address.str("");
            address << "tcp://" << addr;
            socket->connect(address.str().c_str());

#if defined(CONNECTION_PROTOCOL)
            sendHelloRecvHi();
#endif
        } else {
            int i;
            if ((i = addr.find(":")) < 0) {
                printf("init: ERROR -- wrong IP address format for %s\n",addr.c_str());
                return -1;
            }
            int port = atoi((addr.substr(i + 1)).c_str());
            std::stringstream address;
            address << "tcp://*:" << port;
            try {
                socket->bind(address.str().c_str());
            } catch(std::exception& e) {
                printf("initTransport: ERROR -- binding address (%s): %s\n",
                       address.str().c_str(), e.what());
                return -1;
            }
#if defined(CONNECTION_PROTOCOL)
            recvHelloSendHi();
#endif
        }
        return 0;
    }

    /**
     * It sends a message.
     *
     *  \param msg is a reference to the message to be sent.
     *  \param flags is a flag indicating whether the operation should be in
     *  non-blocking mode or if the message is a multi-part message.
     *  See <a href="http://api.zeromq.org/2-1:_start"> ØMQ's API </a> for details.
     *
     *  \exception TODO
     *  \return TODO
     */
    inline bool send(const msg_t& msg, int flags = 0) {
        try {
            if (!socket->send(const_cast<msg_t&>(msg), flags)) return false;
        } catch (std::exception& e) {
            return false;
        }
        return true;
    }

    /**
     * It receives a message header.
     *
     * \exception TODO
     * \return TODO
     */
    inline bool recvhdr(msg_t& msg, int& peer) {
        zmq::message_t from;
        try {
            if(!socket->recv(&from)) {
                zmqTDBG(printf("recvhdr RETURN -1\n"));
                return false;
            }
            const std::string & f = static_cast<char*>(from.data());
            int i;
            if ((i = f.find(":")) < 0) {
                printf("recvhdr: ERROR -- wrong header\n");
                return false;
            }
            peer = atoi((f.substr(i + 1)).c_str());
            recvHdr = false;
            if(!socket->recv(&msg)) {
                zmqTDBG(printf("recvhdr RETURN -1\n"));
                return false;
            }
        } catch (std::exception & e) {
            return false;
        }
        return true;
    }

    /**
     * \p It receives a message after having received the header.
     *
     * \parm msg is a reference to the message to be sent.
     * \param peers is the number of peers involved (sending nodes).
     *
     * \exception TODO
     * \return TODO
     */
    inline bool recv(msg_t& msg, int& peer) {
        try {
            if (recvHdr) {
                zmq::message_t from;
                if(!socket->recv(&from)) {
                    zmqTDBG(printf("recv RETURN -1\n"));
                    return false;
                }

                const std::string & f=static_cast<char*>(from.data());
                int i;
                if ((i = f.find(":")) < 0) {
                    printf("recvhdr: ERROR: wrong header\n");
                    return false;
                    }
                peer = atoi((f.substr(i + 1)).c_str());
            }

            if(!socket->recv(&msg)) {
                zmqTDBG(printf("recv RETURN -1\n"));
                return false;
            }
        } catch (std::exception & e) {
            return false;
        }
        return true;
    }

    /**
     * It receives the request.
     *
     * \exception TODO
     * \return TODO
     */
    inline bool recvreq() {
        try {
            msg_t useless;
            if(!socket->recv(&useless)) {
                zmqTDBG(printf("recvreq RETURN -1\n"));
                return false;
            }
        } catch (std::exception & e) {
            return false;
        }
        return true;
    }

    /**
     * It resets recvHdr.
     *
     * \exception TODO
     * \return TODO
     */
    inline bool done(bool sendready=false) {
        if (sendready) {
            static const char ready[] = "READY";
            for(int i = 0; i < peers; ++i) {
                msg_t msg(ready, 6);
                zmq::message_t to(const_cast<char*>(transportIDs[i].c_str()), transportIDs[i].length()+1,0,0);
                try {
                    if (!socket->send(to,ZMQ_SNDMORE)) return false;
                    if (!socket->send(msg)) return false;
                } catch (std::exception& e) {
                    return false;
                }
            }
        }
        recvHdr=true;
        return true;
    }

    /**
     * It returns the number of partners of the single communication (NOTE: if P, peers is 1).
     *
     * \return The number of peers are returned.
     */
    inline int getPeers() const { return peers;}

    const std::string        name;
    zmq::socket_t*           socket;
    zmqTransport * const     transport;
    const bool               P;
    const int                peers;
    bool                     recvHdr;
    std::vector<std::string> transportIDs;

#if defined(CONNECTION_PROTOCOL)

    /**
     * It sends hello to receiving node.
     */
    inline void sendHelloRecvHi() {
        const char hello[] = "HELLO";
        // sending Hello
        zmqTDBG(printf("%s sending HELLO\n", name.c_str()));
        zmqTransportMsg_t req(hello, 6);
        send(req);
        // receive Hi
        zmqTransportMsg_t msg;
        recvreply(msg);
        zmqTDBG(printf("%s received %s\n", name.c_str(), static_cast<char*>(msg.getData())));
    }

    /**
     * It receives a reply.
     *
     * \exception TODO
     * \return TODO
     */
    inline bool recvreply(msg_t& msg) {
        try {
            if(!socket->recv(&msg)) {
                zmqTDBG(printf("recv RETURN -1\n"));
                return false;
            }
        } catch (std::exception & e) {
            return false;
        }
        return true;
    }

    /**
     * It sends a reply.
     *
     * \exception TODO
     * \return TODO
     */
    inline bool sendreply(const msg_t& msg) {
        for(int i = 0; i < peers; ++i) {
            msg_t _msg;
            _msg.copy(const_cast<msg_t&>(msg));
            zmq::message_t to(const_cast<char*>(transportIDs[i].c_str()), transportIDs[i].length()+1,0,0);
            try {
                if (!socket->send(to,ZMQ_SNDMORE)) return false;
                if (!socket->send(_msg)) return false;
            } catch (std::exception& e) {
                return false;
            }
        }
        return true;
    }

    /**
     * It receive hello to sening hi.
     */
    inline void recvHelloSendHi() {
        static const char hi[] = "HI"; // need to be static
        // receives Hello and sends Hi
        int useless;
        for(int i = 0; i < peers; ++i) {
            zmqTransportMsg_t req;
            recv(req,useless);
            zmqTDBG(printf("%s received %s\n",name.c_str(), static_cast<char*>(req.getData())));
        }
        zmqTransportMsg_t rep(hi,3);
        sendreply(rep);
        zmqTDBG(printf("%s sending HI\n", name.c_str()));
    }
#endif
}; // end descriptorN_1

//**************************************
// One to one communication
//**************************************

/*!
 *  \class zmq1_1
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief This class provides ZeroMQ implementation of the 1 to 1
 *  communication pattern.
 *
 *  This class is defined in zmqImpl.hpp
 *
 */
class zmq1_1 {
public:
    typedef zmqTransportMsg_t  msg_t;           /// a ØMQ message (extends zmq::message_t)
    typedef zmqTransport       TransportImpl;   /// to the transpoort layer
    typedef msg_t              tosend_t;        /// ut supra
    typedef msg_t              torecv_t;        /// ut supra
    typedef descriptor1_N      descriptor;      ///< descriptor used in Unicast, Bcast, Scatter,
                                                ///< On-Demand, OneToMany

    enum {MULTIPUT = 0};

    /**
     * Constructor
     */
    zmq1_1():desc(NULL),active(false) {}

    /**
     * Constructor (2)
     *
     * \param D is a pointer to a 1_N descriptor object.
     */
    zmq1_1(descriptor* D):desc(D),active(false) {}

    /*
     * It sets the descriptor.
     *
     * \param D is the descriptor.
     */
    inline void setDescriptor(descriptor* D) {
        if (desc)  desc->close();
        desc = D;
    }

    /**
     * It gets the descriptor.
     *
     * \return It returns the descriptor.
     */
    inline  descriptor* getDescriptor() { return desc; }

    /**
     * It initializes the communication pattern.
     *
     * \return TODO
     */
    inline bool init(const std::string& address,const int nodeId=-1) {
        if (active) return false;
        // we force 0 to be the nodeId for the consumer
        if(!desc->init(address,(desc->P)?nodeId:0)) active = true;
        return active;
    }

    /**
     * It sends one message.
     *
     * \param msg is a reference to the message to be sent.
     *
     * \return TODO
     */
    inline bool put(const tosend_t& msg) { return desc->send(msg, 0, 0); }

    /**
     * It sends other parts of a multi-part message.
     *
     * \param msg is a reference to the message to be sent.
     *
     * \return TODO
     */
    inline bool putmore(const tosend_t& msg) { return desc->sendmore(msg,0,ZMQ_SNDMORE);}

    /**
     * TODO
     *
     * \param msg TODO
     * \parm x TODO
     *
     * \return TODO
     */
    inline bool put(const msg_t& msg, const int) { return desc->send(msg, 0, 0); }

    /**
     * TODO
     *
     * \parm msg TODO
     * \parm x TODO
     *
     * \return TODO
     */
    inline bool putmore(const msg_t& msg, const int) { return desc->send(msg, 0, ZMQ_SNDMORE); }

    /**
     * It receives the message header (should be called before get).
     *
     * \parm msg TODO
     * \parm peer TODO
     *
     * \return TODO
     */
    inline bool gethdr(torecv_t& msg, int& peer) { peer=0; return desc->recvhdr(msg); }

    /*
     * It returns the number of disctint messages from one single
     * communication.
     *
     * \parm msg TODO
     *
     * \return TODO
     */
    inline bool get(torecv_t& msg, int=0) { return desc->recv(msg); }

    /*
     * It put the node in waiting state.
     *
     * \return TODO
     */
    inline int getToWait() const { return 1;}

    /*
     * It put the node to performing state.
     *
     * \return TODO
     */
    inline int putToPerform() const { return 1;}

    /*
     * The communicaiton is finishe.
     */
    inline void done() { }

    /*
     * It closes the communication pattern.
     *
     * \return TODO
     */
    inline bool close() {
        if (!active) return false;
        if (!desc->close()) return false;
        active = false;
        return true;
    }

protected:
    descriptor* desc;
    bool        active;
};

//**************************************
// Broadcast communication
//**************************************

/*!
 *  \class zmqBcast
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief It implements the broadcast communication pattern of
 *  FastFlow in ZeroMQ.
 *
 *  This class is defined in \ref zmqImpl.hpp
 */
class zmqBcast {
public:
    typedef zmqTransportMsg_t  msg_t;           /// a ØMQ message (extends zmq::message_t)
    typedef zmqTransport       TransportImpl;   /// to the transpoort layer
    typedef msg_t              tosend_t;        /// ut supra
    typedef msg_t              torecv_t;        /// ut supra
    typedef descriptor1_N      descriptor;      ///< descriptor used in Unicast, Bcast, Scatter,
                                                ///< On-Demand, OneToMany

    enum {MULTIPUT = 0};

    /**
     * Constructor
     */
    zmqBcast():desc(NULL),active(false) {}

    /**
     *  Constructor (2)
     *
     *  \param D pointer to a 1_N descriptor object.
     */
    zmqBcast(descriptor* D):desc(D),active(false) {}

    /**
     * It sets the descriptor.
     *
     * \parm D is the descriptor.
     */
    inline void setDescriptor(descriptor* D) {
        if (desc)  desc->close();
        desc = D;
    }

    /**
     * It returns the descriptor.
     *
     * \return It returns the descriptor.
     */
    inline  descriptor* getDescriptor() { return desc; }

    /**
     * It initializes the communication pattern.
     *
     * \param addr is the IP address in the form 'ip/host:port'
     * \param nodeId is the node identifier in the range [0..inf[
     *
     * \return TODO
     */
    inline bool init(const std::string& address,const int nodeId=-1) {
        if (active) return false;
        if(!desc->init(address,nodeId)) active = true;
        return active;
    }

    /**
     * It sends one message.
     *
     * \param msg is a reference to the message to be sent.
     *
     * \return TODO
     */
    inline bool put(const tosend_t& msg) { return desc->send(msg, 0); }

    /**
     * It sends other parts of a multi-part message.
     *
     * \param msg is a reference to the message to be sent.
     *
     * \return TODO
     */
    inline bool putmore(const tosend_t& msg) { return desc->sendmore(msg,ZMQ_SNDMORE);}

    /**
     * It sens the message.
     *
     * \parm msg is the reference to the message.
     * \parm to is the address of the node where the message should be sent.
     *
     * \return TODO
     */
    inline bool put(const msg_t& msg, const int to) {
        return desc->send(msg, to, 0);
    }

    /**
     * It sen the message to more nodes.
     *
     * \parm msg is the reference to the message.
     * \parm to is the address of the node where the message should be sent.
     *
     * \return TODO
     */
    inline bool putmore(const msg_t& msg, const int to) {
        return desc->sendmore(msg,to,ZMQ_SNDMORE);
    }

    /**
     * It receives the message header (should be called before get).
     *
     * \parm msg is the reference to the message.
     * \parm peer is the number of peers.
     *
     * \return TODO
     */
    inline bool gethdr(torecv_t& msg, int& peer) { peer=0; return desc->recvhdr(msg); }

    /**
     * It receives one message.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool get(torecv_t& msg, int=0) { return desc->recv(msg); }

    /**
     * It puts the node in the waiting state.
     *
     * \return TODO
     */
    inline int getToWait() const { return 1;}

    /**
     * It puts the node to peforming state.
     *
     * \return TODO
     */
    inline int putToPerform() const { return 1;}

    /**
     * It closes the communication pattern.
     *
     * \return TODO
     */
    inline bool close() {
        if (!active) return false;
        if (!desc->close()) return false;
        active=false;
        return true;
    }

    /**
     * It finishes the communication.
     */
    inline void done() {}

protected:
    descriptor* desc;
    bool        active;
};


//**************************************
// Communication Transport interface
//
//  ZeroMQ implementation of the ALL_GATHER communication patter
//**************************************

/*!
 *  \class zmqAllGather
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief It provides implementation of the ALL_GATHER communication pattern.
 *
 *  This class is defined in \ref zmqImpl.hpp
 *
 */
class zmqAllGather {
public:
    typedef zmqTransportMsg_t  msg_t;
    typedef zmqTransport       TransportImpl;
    typedef msg_t              tosend_t;
    typedef svector<msg_t>     torecv_t;
    typedef descriptorN_1      descriptor;

    enum {MULTIPUT = 0};

    /**
     * Constructor (1)
     */
    zmqAllGather():desc(NULL),active(false) {}

    /**
     * Constructor (2)
     *
     * \parm D is the descriptor.
     */
    zmqAllGather(descriptor* D):desc(D),active(false) {}

    /**
     * It sets the descriptor.
     */
    inline void setDescriptor(descriptor* D) {
        if (desc)  desc->close();
        desc = D;
    }

    /**
     * It returns the descriptor.
     *
     * \return It returns the descriptor.
     */
    inline  descriptor* getDescriptor() { return desc; }

    /**
     * It initializes communication pattern.
     *
     * \return TODO
     */
    inline bool init(const std::string& address,const int nodeId=-1) {
        if (active) return false;
        if(!desc->init(address,nodeId)) active = true;
        if (!desc->P) done();
        return active;
    }

    /**
     * It sends one message.
     *
     * \parm msg is the reference of the message.
     *
     * \return TODO
     */
    inline bool put(const tosend_t& msg) {
        if (!desc->recvreq()) return false;
        return desc->send(msg, 0);
    }

    /**
     * It puts more messages in the sening queue.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool putmore(const tosend_t& msg) { return desc->send(msg,ZMQ_SNDMORE);}

    /**
     * It puts single message.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool put(const msg_t& msg, const int) {
        return put(msg);
    }

    /**
     * It put more messages.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool putmore(const tosend_t& msg, const int) {
        return putmore(msg);
    }

    /**
     * It receives the message header ONLY from one peer (should be called before get).
     *
     * \parm msg is the reference to the message.
     * \param peer is the number of peers.
     *
     * \return TODO
     */
    inline bool gethdr(msg_t& msg, int& peer) {
        return desc->recvhdr(msg, peer);
    }

    /**
     * It receives one message ONLY from one peer (gethdr should be called before).
     *
     * \param msg is the reference to the message.
     * \param peer is the number of peers.
     *
     * \return TODO
     */
    inline bool get(msg_t& msg, int& peer) {
        return desc->recv(msg,peer);
    }

    /**
     * It receives one message.
     *
     * \return TODO
     */
    inline bool get(torecv_t& msg) {
        const int peers = desc->getPeers();
        int useless;
        msg.resize(peers);
        for(int i = 0; i < peers; ++i)
            if (!desc->recv(msg[i], useless)) return false;
        done();
        return true;
    }

    /**
     * It returns the number of distinct messages for one single communication.
     *
     * \return TODO
     */
    inline int getToWait() const { return desc->getPeers();}

    /**
     * It puts the node in performing state.
     *
     * \return TODO
     */
    inline int putToPerform() const { return 1;}

    /**
     * It finishes the communication process.
     */
    inline void done() { desc->done(true); }

    /**
     * It closes communication pattern.
     *
     * \return TODO
     */
    inline bool close() {
        if (!active) return false;
        if (desc->P) if (!desc->recvreq()) return false;
        if (!desc->close()) return false;
        active=false;
        return true;
    }

protected:
    descriptor* desc;
    bool        active;
};

//**************************************
// FromAny communication pattern
//**************************************

/*!
 *  \class zmqFromAny
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief It provides implementation of the collect from ANY communication
 *  pattern.
 *
 *  This class is defined in \ref zmqImpl.hpp
 *
 */
class zmqFromAny {
public:
    typedef zmqTransportMsg_t  msg_t;
    typedef zmqTransport       TransportImpl;
    typedef msg_t              tosend_t;
    typedef msg_t              torecv_t;
    typedef descriptorN_1      descriptor;

    enum {MULTIPUT = 0};

    zmqFromAny():desc(NULL),active(false) {}
    zmqFromAny(descriptor* D):desc(D),active(false) {}

    /**
     * It sets the descriptor.
     *
     * \parm D is the descriptor.
     */
    inline void setDescriptor(descriptor* D) {
        if (desc)  desc->close();
        desc = D;
    }

    /**
     * It returns the descriptor.
     *
     * \return TODO
     */
    inline  descriptor* getDescriptor() { return desc; }

    /**
     * It initializes communication pattern.
     *
     * \parm address
     * \param nodeID
     *
     * \return TODO
     */
    inline bool init(const std::string& address,const int nodeId=-1) {
        if (active) return false;
        if(!desc->init(address,nodeId)) active = true;
        return active;
    }

    /**
     * It sends one message.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool put(const tosend_t& msg) {
        return desc->send(msg, 0);
    }

    /**
     * It put more messages.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool putmore(const tosend_t& msg) {
        return desc->send(msg,ZMQ_SNDMORE);
    }

    /**
     * It places a message in the node.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool put(const msg_t& msg, const int) {
        return desc->send(msg, 0);
    }

    /**
     * It places more messages in the node.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool putmore(const tosend_t& msg, const int) {
        return desc->send(msg,ZMQ_SNDMORE);
    }

    /**
     * It receives the message header (should be called before get).
     *
     * \parm msg is the reference to the message.
     * \parm peer is the number of peers.
     *
     * \return TODO
     */
    inline bool gethdr(msg_t& msg, int& peer) {
        return desc->recvhdr(msg,peer);
    }

    /**
     * It receives one message.
     *
     * \parm msg is the reference to the message.
     * \parm peer is the number of peers.
     *
     * \return TODO
     */
    inline bool get(msg_t& msg, int& peer) {
        return desc->recv(msg,peer);
    }

    /**
     * It receives one message.
     *
     * \param msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool get(msg_t& msg) {
        int useless = 0;
        return desc->recv(msg,useless);
    }

    /**
     * It places the node in the waiting state.
     *
     * \return TODO
     */
    inline int getToWait() const { return 1;}

    /**
     * It places the node in the waiting state.
     *
     * \return TODO
     */
    inline int putToPerform() const { return 1;}

    /**
     * It performs the completion operation.
     */
    inline void done() { desc->done(); }

    /**
     * It closes communication pattern.
     *
     * \return TODO
     */
    inline bool close() {
        if (!active) return false;
        if (!desc->close()) return false;
        active = false;
        return true;
    }

protected:
    descriptor* desc;
    bool        active;
};

//**************************************
// Scatter communication pattern
//**************************************

/*!
 *  \class zmqScatter
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief It provides implementation of the SCATTER communication pattern.
 *
 *  This class is defined in \ref zmqImpl.hpp
 *
 */
class zmqScatter {
public:
    typedef zmqTransportMsg_t  msg_t;
    typedef zmqTransport       TransportImpl;
    typedef svector<msg_t>     tosend_t;
    typedef msg_t              torecv_t;
    typedef descriptor1_N      descriptor;
    enum {MULTIPUT = 1};

    /**
     * Constructor (1)
     *
     * It creates an empty scatter.
     */
    zmqScatter():desc(NULL),active(false) {}

    /**
     * Constructor (2)
     *
     * It creates a scatter with a scriptor.
     *
     * \parm D is the descriptor.
     */
    zmqScatter(descriptor* D):desc(D),active(false) {}

    /**
     * It sets the descriptor.
     *
     * \parm D is the descriptor.
     */
    inline void setDescriptor(descriptor* D) {
        if (desc) desc->close();
        desc = D;
    }

    /**
     * It returns the descriptor.
     *
     * \return It returns the descriptor.
     */
    inline  descriptor* getDescriptor() { return desc; }

    /**
     * It initializes communication pattern.
     *
     * \parm address TOOD
     * \parm nodeId TODO
     *
     * \return TODO
     */
    inline bool init(const std::string& address,const int nodeId=-1) {
        if (active) return false;
        if(!desc->init(address,nodeId)) active = true;
        return active;
    }

    /**
     * It sends one message.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool put(const tosend_t& msg) {
        const int peers = desc->getPeers();
        assert(msg.size()==(size_t)peers);
        for(int i = 0; i < peers; ++i)
            if (!desc->send(msg[i], i, 0)) return false;
        return true;
    }

    /**
     * It places more messages.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool putmore(const tosend_t& msg) {
        return -1;
    }

    /**
     * It places message.
     *
     * \parm msg is the reference to the message.
     * \parm to is the address of the receiving node.
     *
     * \return TODO
     */
    inline bool put(const msg_t& msg, const int to) {
        return desc->send(msg, to, 0);
    }

    /**
     * It sends a message.
     *
     * \parm msg is the reference to message
     *
     * \return A negative value is returned.
     */
    inline bool put(const msg_t& msg) {
        return -1;
    }

    /**
     * It places many messages.
     *
     * \parm msg is the reference to the message.
     * \parm to is the address of the receiving node.
     *
     * \return TODO
     */
    inline bool putmore(const msg_t& msg, const int to) {
        return desc->sendmore(msg,to,ZMQ_SNDMORE);
    }

    /**
     * It places more messages to the node.
     *
     * \parm msg is the reference to the message.
     *
     * \return A negative value is returned.
     */
    inline bool putmore(const msg_t& msg) {
        return -1;
    }

    /**
     * It receives the message header (should be called before get).
     *
     * \parm msg is the reference to the message.
     * \parm peer is the number of peers.
     *
     * \return TODO
     */
    inline bool gethdr(torecv_t& msg, int& peer) { peer=0; return desc->recvhdr(msg); }

    /**
     * It receives one message.
     *
     * \return TODO
     */
    inline bool get(torecv_t& msg, int=0) { return desc->recv(msg); }

    /**
     * It returns the number of distinct messages for one single communication.
     *
     * \return 1 is always returned
     */
    inline int getToWait() const { return 1;}

    /**
     * \return TODO
     */
    inline int putToPerform() const { return desc->getPeers();}

    /**
     * It closes communication pattern
     *
     * \return TODO
     */
    inline bool close() {
        if (!active) return false;
        if (!desc->close()) return false;
        active = false;
        return true;
    }

    /**
     * The communication finished.
     */
    inline void done() {}

protected:
    descriptor* desc;
    bool        active;
};

//**************************************
// OnDemand communication transport
//**************************************

/*!
 *  \class zmqOnDemand
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief ZeroMQ implementation of the ON-DEMAND communication pattern.
 *
 *  This class is defined in \ref zmqImpl.hpp
 *
 */
class zmqOnDemand {
protected:

    /**
     * It defines the sending request.
     *
     * \return TODO
     */
    inline bool sendReq() {
        static const char ready[] = "R";
        msg_t request(ready,2);
        if (!desc->sendReq(request)) return false;
        requestSent = true;
        return true;
    }

public:
    typedef zmqTransportMsg_t  msg_t;
    typedef zmqTransport       TransportImpl;
    typedef msg_t              tosend_t;
    typedef msg_t              torecv_t;
    typedef descriptor1_N      descriptor;
    enum {MULTIPUT = 0};

    /**
     * Constructur (1)
     *
     */
    zmqOnDemand():desc(NULL),active(false),to(-1),requestsToWait(0),requestSent(false) {}

    /**
     * Constructor (2)
     *
     */
    zmqOnDemand(descriptor* D):desc(D),active(false),to(-1),requestsToWait(0),requestSent(false) {}

    /*
     * \p It sets the descriptor.
     *
     * \parm D is the descriptor.
     */
    inline void setDescriptor(descriptor* D) {
        if (desc) desc->close();
        desc = D;
    }

    /**
     * It returns the descriptor.
     *
     * \return TODO
     */
    inline  descriptor* getDescriptor() { return desc; }

    /**
     * It initializes communication pattern.
     *
     * \parm address if the address of the node.
     * \parm nodeID is the addrecess of the receiving node.
     *
     * \return TODO
     */
    inline bool init(const std::string& address,const int nodeId=-1) {
        if (active) return false;
        if(!desc->init(address,nodeId)) {
            if (!desc->P) if (!sendReq()) return false;
            active = true;
        }

        To.resize(desc->getPeers());
        for(int i = 0; i < desc->getPeers(); ++i) To[i] = false;

        return active;
    }

    /**
     * It sends one message.
     *
     * \parm msg is the reference of the message.
     *
     * \return TODO
     */
    inline bool put(const tosend_t& msg) {
        // FIX: It would be better to use a non-blocking calls to receive requests, and
        //      in case recvReq returns false the msg should be inserted into a local queue.
        //      It is required an extra threads for handling incoming requests.....
        if (to<0) if (!desc->recvReq(to)) return false;
        int r = desc->send(msg, to, 0);
        to=-1;
        return r;
    }

    /**
     * It sends one message, more messages have to come.
     *
     * \parm msg is the reference of the message.
     *
     * \return TODO
     */
    inline bool putmore(const tosend_t& msg) {
        // see FIX comment above
        if (to<0) if (!desc->recvReq(to)) return false;
        return desc->sendmore(msg, to, ZMQ_SNDMORE);
    }

    /**
     * It places a message.
     *
     * \parm msg is the reference of the message.
     * \parm dest is the destination node.
     * \parm flag TODO
     *
     * \return TODO
     */
    inline bool put(const tosend_t& msg, const int dest, int flag=0) {
        if (dest<0) {  // sends the same message to all peers, usefull for termination
            for(int i=0;i<desc->getPeers();++i) {
                to = i;
                tosend_t _msg;
                _msg.copy(const_cast<msg_t&>(msg));
                if (!desc->send(_msg, to, flag)) { to=-1; return false;}
                ++requestsToWait;
            }
            return true;
        }
        if (To[dest]) {
            if (!desc->send(msg, dest, flag)) { to=-1; return false;}
            To[dest]=false;
            return true;
        }
        do {
            if (!desc->recvReq(to)) { to=-1; return false;}
            if (to == dest) {
                if (!desc->send(msg, to, flag)) { to=-1; return false;}
                return true;
            }
            assert(To[to]==false);
            To[to]=true;
        } while(1);

        // not reached
        return false;
    }

    /**
     * It places many messages.
     *
     * \parm msg is the reference of the message.
     * \parm dest is the address of the destination.
     *
     * \return TODO
     */
    inline bool putmore(const tosend_t& msg, const int dest) {
        return put(msg,dest,ZMQ_SNDMORE);
    }

    /**
     * It receives the message header (should be called before get).
     *
     * \parm msg is the reference of the message.
     * \parm peer is the number of peers.
     *
     * \return TODO
     */
    inline bool gethdr(torecv_t& msg, int& peer) {
        peer=0;
        bool r = desc->recvhdr(msg);
        if (r) sendReq();
        return r;
    }

    /**
     * It receives one message part.
     *
     * \parm msg is the reference of the message.
     *
     * \return TODO
     */
    inline bool get(torecv_t& msg) {
        bool r=desc->recv(msg);
        if (!requestSent && r) sendReq();
        return r;
    }
    /**
     * It receives one message part.
     *
     * \parm msg is the reference of the message.
     * \parm peer is the number of peer.
     *
     * \return TODO
     *
     */
    inline bool get(torecv_t& msg,int& peer) {
        peer=0;
        return get(msg);
    }

    /**
     * It closes communication pattern.
     *
     * \return TODO
     */
    inline bool close() {
        if (!active) return false;
        if (desc->P) {
            int useless;
            requestsToWait+=desc->getPeers();
            for(int i=0;i<desc->getPeers();++i)
                if (To[i]) --requestsToWait;
            for(int i=0;i<requestsToWait;++i) {
                if (!desc->recvReq(useless)) return false;
            }
        }
        if (!desc->close()) return false;
        active=false;
        return true;
    }

    /**
     * It returns the number of distinct messages for one single communication.
     *
     * \return 1 is always returned.
     */
    inline int getToWait() const { return 1;}

    /**
     * It completes the communication.
     */
    inline void done() { requestSent=false; }

protected:
    descriptor*   desc;
    bool          active;
    int           to;
    int           requestsToWait;
    bool          requestSent;
    svector<bool> To;
};

//**************************************
// One to many communication
//**************************************

/*
 *  \class zmq1_N
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief It provides implementation of the ONE_TO_MANY communication pattern.
 *
 *  This pattern can be used to dynamically change among UNICAST, BROADCAST and
 *  SCATTER patterns.
 *
 *  This class is define in \ref zmqImpl.hpp
 */
class zmq1_N {
public:
    typedef zmqTransportMsg_t  msg_t;
    typedef svector<msg_t>     tosend_t;
    typedef msg_t              torecv_t;
    typedef descriptor1_N      descriptor;

    enum {MULTIPUT=0 };

    zmq1_N():desc(NULL),active(false) {}
    zmq1_N(descriptor* D):desc(D),active(false) {}

    /**
     * It sets the descriptor.
     *
     * \parm D is the descriptor.
     */
    inline void setDescriptor(descriptor* D) {
        if (desc)  desc->close();
        desc = D;
    }

    /**
     * It returns the descriptor.
     *
     * \return Descriptor of the file is returned.
     */
    inline  descriptor* getDescriptor() { return desc; }

    /**
     * It initializes communication pattern.
     *
     * \parm address if the IP address
     * \parm nodeId is the address of the node
     *
     * \return TODO
     */
    inline bool init(const std::string& address,const int nodeId=-1) {
        if (active) return false;
        if(!desc->init(address,nodeId)) active = true;
        return active;
    }

    /**
     * It sends one message.
     *
     * \parm msg is the reference to the message.
     *
     * \return TODO
     */
    inline bool put(const tosend_t& msg) {
        const int peers=desc->getPeers();
        assert(msg.size()==(size_t)peers);
        for(int i=0;i<peers;++i)
            if (!desc->send(msg[i], i, 0)) return false;
        return true;
    }

    /**
     * It sends one message.
     *
     * \parm msgs is the reference to the message.
     * \parm to is the address of the receiving node.
     *
     * \return TODO
     *
     */
    inline bool put(const msg_t& msg, const int to) {
        return desc->send(msg, to, to);
    }

    /**
     * It sends message to many nodes.
     *
     * \parm msg is the reference to the message.
     * \parm to is the address of the receiving node.
     *
     * \return TODO
     */
    inline bool putmore(const msg_t& msg, const int to) {
        return desc->sendmore(msg,to,ZMQ_SNDMORE);
    }

    /**
     * It receives the message header (should be called before get).
     *
     * \parm msg is the reference to the message.
     * \parm peer is the number of peer.
     *
     * \return TODO
     */
    inline bool gethdr(torecv_t& msg, int& peer) { peer=0; return desc->recvhdr(msg); }

    /**
     * It receives one message.
     *
     * \parm msg is the reference to the message
     *
     * \return TODO
     */
    inline bool get(torecv_t& msg, int=0) { return desc->recv(msg); }

    /**
     * It closes communication pattern.
     *
     * \return TODO
     */
    inline bool close() {
        if (!active) return false;
        if (!desc->close()) return false;
        active=false;
        return true;
    }

    /**
     * It finishes the communicaiton.
     */
    inline void done() {}

protected:
    descriptor* desc;
    bool        active;
};

/*!
 *
 * @}
 * \endlink
 */
}
#endif /* FF_zmqIMPL_HPP */
