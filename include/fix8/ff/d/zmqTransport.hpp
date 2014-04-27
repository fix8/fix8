/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

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
 
 /*! 
  * \link 
  * \file zmqTransport.hpp 
  *  \ingroup streaming_network_simple_distributed_memory
  *
  * \brief This file provides the definition of the external transport layer
  * based on ØMQ.
  *
  * TODO: Write a summary of the whole file.
  *
  */

#ifndef FF_ZMQTRANSPORT_HPP
#define FF_ZMQTRANSPORT_HPP

/* 
 * The manual of 0MQ can be found at : http://zguide.zeromq.org/
 * You MUST NOT share ØMQ sockets between threads. ØMQ sockets are not
 * threadsafe. Technically it's possible to do this, but it demands semaphores,
 * locks, or mutexes. This will make your application slow and fragile. The
 * only place where it's remotely sane to share sockets between threads are in
 * language bindings that need to do magic like garbage collection on
 * sockets.
 */

#include <assert.h>
#include <deque>
#include <algorithm>

#include <zmq.hpp>

namespace ff {

/*!
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  @{
 */
 
/*! 
 * \class zmqTransportMsg_t
 *  \ingroup streaming_network_simple_distributed_memory
 *
 * \brief It describes the sructure of 0MQ transport message.
 *
 * This class describes the layout of a message used for inter-thread
 * communications, inter-process communications, TCP/IP and multicast sockets
 * in a distrubuted system running on the FastFlow framework. 
 * 
 * This class is defined in \link zmqTransport.hpp
 *
 */

class zmqTransportMsg_t: public zmq::message_t {
public:
    /**
     * It provides the callback definition.
     */
    typedef void (msg_callback_t) (void *, void *);

    /**
     * \p HEADER_LENGTH is the enumberation of the number of bytes.
     */
    enum {HEADER_LENGTH=4}; // n. of bytes

public:
    /** 
     * Constructor (1)
     *
     * It creates an empty ØMQ message.
     */
    inline zmqTransportMsg_t() {}

    /** 
     * Constructor (2)
     *
     * It creates a structured ØMQ message.
     *
     * \param data has the content of the message.
     * \param size is the amount of memory to be allocated for the message.
     * \param cb is the callback function.
     * \param arg has additional arguments.
     */
    inline zmqTransportMsg_t( const void * data, size_t size, 
                              msg_callback_t cb=0, void * arg=0 ) :
                              zmq::message_t( const_cast<void*>(data), size, cb, arg )
                              { }

    /**
     * It initialises the message object (It is like the constructor but it
     * rebuilds the object from scratch using new values).
     *
     * \param data has the content of the message.
     * \param size is the amount of memory to be allocated for the message.
     * \param cb is the callback function.
     * \param arg has additional arguments.
     */
    inline void init(const void * data, size_t size, msg_callback_t cb=0, void * arg=0) {
        zmq::message_t::rebuild( const_cast<void*>(data), size, cb, arg );
    }

    /**
     * It copies the message.
     *
     * \param msg is the reference to the 0MQ transport message.
     */
    inline void copy(zmq::message_t &msg) {
        zmq::message_t::copy(&msg);
    }

    /**
     * It retrieves the content of the message object.
     *
     * \return The contents of the message object.
     */
    inline void * getData() {
        return zmq::message_t::data();
    }

    /**
     * It retrieves the size in bytes of the content of the message object.
     *
     * \return The size of the contexts of the message in bytes.
     */
    inline size_t size() {
        return zmq::message_t::size();
    }
};

/*! 
 * \class zmqTransport
 *  \ingroup streaming_network_simple_distributed_memory
 *
 * \brief This class describes the transport layer used in a distributed
 * FastFlow environment.
 *
 * This class is defined in zmqTransport.hpp
 */

class zmqTransport {
private:
    /**
     * \p NUM_IO_THREADS is the enumberation of the number of IO threads.
     */
    enum {NUM_IO_THREADS=1};

protected:

    /**
     * It closes all existing connections.
     *
     * \return It returns 0 to show the successful closing of connection.
     */
    inline int closeConnections() {
        /* WARNING: Instead of setting a longer period of some minutes, it
         * would be better to implement a shoutdown protocol and to set the
         * longer period to 0.
         */
        int zero = 1000000; //milliseconds 
        for(unsigned i = 0; i < Socks.size(); ++i) {
            if (Socks[i]) { 
                Socks[i]->setsockopt(ZMQ_LINGER, &zero, sizeof(zero));
                Socks[i]->close();
                delete Socks[i];
                Socks[i]=NULL;
            }
        }
        return 0;
    }

public:
    /**
     * It defines zmq::socket_t.
     */
    typedef zmq::socket_t endpoint_t;

    /**
     * It defines zmqTransportMsg_t.
     */
    typedef zmqTransportMsg_t msg_t;

    /** 
     *  It constructs a transport connection.
     *
     *  \param procId is the process (or thread) ID.
     */
    zmqTransport(const int procId) :
        procId(procId),context(NULL) {
    };

    /** 
     * It closes the transport connection.
     */
    ~zmqTransport() { closeTransport(); }

    /**
     * It initializes the transport layer: creates a new ØMQ context and
     * cleans the list of active end-points.
     *
     * \return If successful 0, otherwise a negative value is returned. 
     */
    int initTransport() {
        if (context) return -1;
        context = new zmq::context_t(NUM_IO_THREADS);        
        if (!context) return -1;
        if (Socks.size()>0) closeConnections();
        return 0;
    }
    
    /** 
     * It closes the transport layer, close all connections to any active
     * end-point and delete the existing context.
     *
     * \returns 0 is returned to show the successful closing of transport
     * connection.
     */
    int closeTransport() {
        closeConnections();
        if (context) {delete context; context=NULL;}        
        return 0;
    }

    /** 
     * It creates a new socket and pushes it into the active sockets list.
     *
     * \param P is a flag to specify the role of the socket: if \p false, the
     * new socket acts as a \p ROUTER (allows routing of messages to specific
     * connections); if \p true the new socket acts as a \p DEALER (used for
     * fair-queuing on input and for performing load-balancing on output
     * toward a pool of collections).
     *
     * \returns If successful a pointer \p s to the newly created endpoint is
     * reutned otherwise NULL is returned.
     */
    endpoint_t * newEndPoint(const bool P) {
        endpoint_t * s = new endpoint_t(*context, (P ? ZMQ_DEALER : ZMQ_ROUTER));
        if (!s) return NULL;
        Socks.push_back(s);
        return s;
    }
    
    /** 
     * It deletes the socket pointed by \p s. It removes the
     * socket from the list of active sockets and destroys the socket.
     *
     * \param s is a pointer to the socket to be deleted.
     * 
     * \returns 0 if successful; otherwise a negative value is returned.
     */
    int deleteEndPoint(endpoint_t *s) {
        if (s) {
            std::deque<endpoint_t*>::iterator it = std::find(Socks.begin(), Socks.end(), s);
            if (it != Socks.end()) {
                Socks.erase(it);

                // WARNING: see closeConnections.
                int zero = 1000000; //milliseconds 
                s->setsockopt(ZMQ_LINGER, &zero, sizeof(zero));
                s->close();
                delete s;
                return 0;
            }
        }
        return -1;
    }

    /** 
     * \p It retrieves the process (or thread) ID.
     *
     * \return the process (or thread) ID
     *
     */
    int getProcId() const { return procId;}

protected:
    const int                procId;    // Process (or thread) ID
    zmq::context_t *         context;   /* A context encapsulates functionality
                                           dealing with the initialisation and
                                           termination of a ØMQ context. */
    std::deque<endpoint_t *> Socks;     // all active end-points (i.e. sockets)
};

/*!
 *  @}
 *  \endlink
 */

} // namespace
#endif /* FF_ZMQTRANSPORT_HPP */
