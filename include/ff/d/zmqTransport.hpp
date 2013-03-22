/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-
 * */
/* ***************************************************************************
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 ****************************************************************************
 */
 
 /*! \file zmqTransport.hpp \brief Definition of the external transport layer
  * based on ØMQ
 */

#ifndef _FF_ZMQTRANSPORT_HPP_ 
#define _FF_ZMQTRANSPORT_HPP_

// TODO: better management of error (and exception) conditions.

// NOTE: from 0mq manuals (http://zguide.zeromq.org/)
//   
//   You MUST NOT share ØMQ sockets between threads. ØMQ sockets are not
//   threadsafe. Technically it's possible to do this, but it demands
//   semaphores, locks, or mutexes. This will make your application slow and
//   fragile. The only place where it's remotely sane to share sockets
//   between threads are in language bindings that need to do magic like
//   garbage collection on sockets.

#include <assert.h>
#include <deque>
#include <algorithm>

#include <zmq.hpp>

namespace ff {
/*! \ingroup zmq_runtime
 *
 *  @{
 */
 
/*! \class zmqTransportMsg_t
 *
 * \brief The structure of a ØMQ transport message.
 *
 * This class describes the layout of a message used for inter-thread
 * communications, inter-process communications, TCP/IP and multicast sockets
 * in a distrubuted system running on FastFlow.
 */

class zmqTransportMsg_t: public zmq::message_t {
public:
    // callback definition 
    typedef void (msg_callback_t) (void *, void *);

    enum {HEADER_LENGHT=4}; // n. of bytes

public:
    /** 
     * Default constructor: create an empty ØMQ message
     */
    inline zmqTransportMsg_t() {}

    /** 
     * Default constructor (2): create a structured ØMQ message.
     *
     * \param data has the content of the message
     * \param size is the amount of memory to be allocated for the message
     * \param cb is the callback function
     * \param arg has additional arguments.   
     */
    inline zmqTransportMsg_t( const void * data, size_t size, 
                              msg_callback_t cb=0, void * arg=0 ) :
                              zmq::message_t( const_cast<void*>(data), size, cb, arg ) 
                              { }

    /**
     * Initialise the message object (like the constructor but 
     * it rebuilds the object from scratch using new values). 
     */
    inline void init(const void * data, size_t size, msg_callback_t cb=0, void * arg=0) {
        zmq::message_t::rebuild( const_cast<void*>(data), size, cb, arg );
    }

    /**
     * Copy the message
     */
    inline void copy(zmq::message_t &msg) {
        zmq::message_t::copy(&msg);
    }

    /**
     * Retrieve the message content
     *
     * \returns a pointer to the content of the message object
     */
    inline void * getData() {
        return zmq::message_t::data();
    }

    /**
     * \returns the size in bytes of the content of the message object
     */
    inline size_t size() {
        return zmq::message_t::size();
    }
};

/*!
 *  @}
 */

/*!
 *  \ingroup zmq_runtime
 *
 *  @{
 */

/*! \class zmqTransport
 *
 * \brief The transport layer
 *
 * This class describes the transport layer used in a distributed FastFlow
 * environment.
 */

class zmqTransport {
private:
    enum {NUM_IO_THREADS=1};

protected:

    /** Closes all existing connections */
    inline int closeConnections() {
        //
        // WARNING: Instead of setting a longer period of some minutes, it
        // would be better to implement a shoutdown protocol and to set the
        // longer period to 0.
        //
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
    typedef zmq::socket_t endpoint_t;
    typedef zmqTransportMsg_t msg_t;

    /** 
     *  Default constructor 
     *
     *  \param procId Process (or thread) ID
     */
    zmqTransport(const int procId) :
        procId(procId),context(NULL) {
    };

    /** Default destructor */
    ~zmqTransport() { closeTransport(); }

    /**
     * Initialise the transport layer: creates a new ØMQ context
     * and cleans the list of active end-points.
     *
     * \returns 0 if successful, a negative value otherwise. 
     */
    int initTransport() {
        if (context) return -1;
        context = new zmq::context_t(NUM_IO_THREADS);        
        if (!context) return -1;
        if (Socks.size()>0) closeConnections();
        return 0;
    }
    
    /** Close the transport layer, close all connections to any active
     * end-point and delete the existing context.
     *
     * \returns 0 if successful
     */
    int closeTransport() {
        closeConnections();
        if (context) {delete context; context=NULL;}        
        return 0;
    }

    /** Create a new socket and push it into the active sockets list.
     *
     * \param P flag specifying the role of the socket: if \p false, the new
     * socket acts as a \p ROUTER (allows routing of messages to specific
     * connections); if \p true the new socket acts as a \p DEALER (used for
     * fair-queuing on input and for performing load-balancing on output toward
     * a pool of collections).
     *
     * \returns a pointer to the newly created endpoint.
     */
    endpoint_t * newEndPoint(const bool P) {
        endpoint_t * s = new endpoint_t(*context, (P ? ZMQ_DEALER : ZMQ_ROUTER));
        if (!s) return NULL;
        Socks.push_back(s);
        return s;
    }
    
    /** Delete the socket pointed by \p s. It removes the socket from the list
     * of active sockets and destroys the socket.
     *
     * \param s a pointer to the socket to be deleted
     * 
     * \returns 0 if successful; a negative value otherwise
     */
    int deleteEndPoint(endpoint_t *s) {
        if (s) {
            std::deque<endpoint_t*>::iterator it = std::find(Socks.begin(), Socks.end(), s);
            if (it != Socks.end()) {
                Socks.erase(it);

                // WARNING: see closeConnections.
                //
                int zero = 1000000; //milliseconds 
                s->setsockopt(ZMQ_LINGER, &zero, sizeof(zero));
                s->close();
                delete s;
                return 0;
            }
        }
        return -1;
    }

    /** return the process (or thread) ID
     *
     */
    const int getProcId() const { return procId;}

protected:
    const int                procId;    // Process (or thread) ID
    zmq::context_t *         context;   /* A context encapsulates functionality 
                                        dealing with the initialisation and 
                                        termination of a ØMQ context. */
                                        
    std::deque<endpoint_t *> Socks;     // all active end-points (i.e. sockets)
};

/*!
 *  @}
 */

} // namespace
#endif /* _ZMQTRANSPORT_HPP_INCLUDE_ */
