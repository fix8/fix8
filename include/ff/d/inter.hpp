/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*
 *  \file inter.hpp
 *  \brief Interfaces for communication and transportation patterns
 */
 
#ifndef _FF_COMMINTERFACE_HPP_
#define _FF_COMMINTERFACE_HPP_

/* ***************************************************************************
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version 3 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 ****************************************************************************
 */


namespace ff {

/*
 *  \ingroup zmq
 *
 *  @{
 */

//
// Communication Pattern interface
//

/*
 * \class commPattern
 *
 * \brief Communication Pattern interface
 */

template <typename Impl>
class commPattern  {

protected:
    Impl impl;

public:
    typedef typename Impl::descriptor      descriptor;
    typedef typename Impl::tosend_t        tosend_t;
    typedef typename Impl::torecv_t        torecv_t;
    typedef typename Impl::TransportImpl   TransportImpl;

    // creates a communication pattern
    commPattern():impl() {}

    // @param D is an implementation-based descriptor for the pattern
    // it contains all the low-level implementation details 
    commPattern(descriptor* D):impl(D) {}

    // sets the descriptor
    inline void setDescriptor(descriptor* D) {  impl.setDescriptor(D); }

    // returns the descriptor
    inline  descriptor* getDescriptor() { return impl.getDescriptor(); }

    // initializes communication pattern
    // nodeId is the unique identifier of the calling in the range [0..inf[
    inline bool init(const std::string& address,const int nodeId=-1) { return impl.init(address,nodeId); }

    // Specifies that the message being sent is just a part of the entire message.
    // Further message parts are to follow.
    inline bool putmore(const tosend_t& msg) { return impl.putmore(msg);}

    // sends one message 
    inline bool put(const tosend_t& msg) { return impl.put(msg); }
    inline bool put(const tosend_t& msg, const int toNode) { return impl.put(msg,toNode);}

    // receives the message header (should be called before get)
    inline bool gethdr(torecv_t& msg, int& peer) { return impl.gethdr(msg,peer); }

    // receives one message part
    inline bool get(torecv_t& msg) { return impl.get(msg); }

    // received all messages
    inline void done() { impl.done(); }

    // close pattern
    inline bool close() { return impl.close();  }
};

//
// Communication Transport interface
//

/*
 * \class commTransport
 *
 * \brief Transport Pattern interface
 */
template <typename Impl>
class commTransport  {
protected:
    Impl impl;
public:
    typedef typename Impl::endpoint_t     endpoint_t;
    typedef typename Impl::msg_t          msg_t;

    // creates a communication trasport
    // @param procId is the unique id (aka rank) of the calling process
    commTransport(const int procId): impl(procId) {}

    // initializes the communication transport
    int initTransport() { return impl.initTransport(); }
    
    int closeTransport() { return impl.closeTransport(); }

    // returns a transport specific communication end-point
    endpoint_t * newEndPoint(const bool P) {
        return impl.newEndPoint(P);
    }
    
    int deleteEndPoint(endpoint_t* ep) {
        return impl.deleteEndPoint(ep);
    }

    const int getProcId() const { return impl.getProcId();}
};

/*!
 *
 * @}
 */


} // namespace
#endif  // _FF_COMMINTERFACE_HPP_
