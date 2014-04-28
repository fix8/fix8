/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/**
 *  \link
 *  \file inter.hpp
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  \brief This file defines the interfaces for communication and
 *  transportation patterns in the distributed FastFlow.
 *
 */
 
/* ***************************************************************************
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License version 3 as
 *  published by the Free Software Foundation.
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
 ****************************************************************************
 */

#ifndef FF_COMMINTERFACE_HPP
#define FF_COMMINTERFACE_HPP


namespace ff {

/**
 *  \ingroup streaming_network_simple_distributed_memory
 *
 *  @{
 */

//**************************************
// Communication Pattern interface
//**************************************

/**
 * \class commPattern
 *  \ingroup streaming_network_simple_distributed_memory
 *
 * \brief This is a tempalte class which defines the communication pattern
 * interface. 
 *
 * This class is defined in file \ref inter.hpp
 *
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

    /**
     * Constructor (1): 
     *
     * Creates a an empty communication pattern.
     */
    commPattern():impl() {}

    /**
     * Constructor (2)
     *
     * Creates a communication pattern with a descriptor.
     *
     * \param D is an implementation-based descriptor for the pattern and
     * contains all the low-level implementation details.
     */
    commPattern(descriptor* D):impl(D) {}

    /**
     * It sets the descriptor.
     *
     * \param D is an implementation-based descriptor for the pattern and
     * contains all the low-level implementation details.
     */
    inline void setDescriptor(descriptor* D) {  impl.setDescriptor(D); }

    /**
     * It returns the descriptor.
     */
    inline  descriptor* getDescriptor() { return impl.getDescriptor(); }

    /**
     * It initializes communication pattern.
     *
     * \param address is the IP address of the sender node.
     * \param nodeId is the unique identifier of the calling node in the range [0..inf[.
     */
    inline bool init(const std::string& address,const int nodeId=-1) { return impl.init(address,nodeId); }

    /**
     * It specifies that the message being sent is just a part of the
     * entire message. Further message parts are to follow.
     *
     * \param msg is the message to be sent.
     */
    inline bool putmore(const tosend_t& msg) { return impl.putmore(msg);}

    /**
     * It sends one message to the targetted node.
     *
     * \param msgs is the message to be sent.
     */
    inline bool put(const tosend_t& msg) { return impl.put(msg); }
    
    /**
     * It sends one message to the targetted node.
     *
     * \param msgs is the message to be sent.
     * \param toNode is the address of the node, where the message is intended
     * to be sent.
     */
    inline bool put(const tosend_t& msg, const int toNode) { return impl.put(msg,toNode);}

    /**
     * It receives the message header.
     *
     * \param msg is the pointer of the message to be sent.
     * \param peer is the address of the peer where the message is sent.
     */
    inline bool gethdr(torecv_t& msg, int& peer) { return impl.gethdr(msg,peer); }

    /**
     * It receives one message part.
     *
     * \param msg is the pointer of the message to be sent.
     */
    inline bool get(torecv_t& msg) { return impl.get(msg); }

    /**
     * It receives all messages.
     */
    inline void done() { impl.done(); }

    /**
     * It closes the communication pattern.
     */
    inline bool close() { return impl.close();  }
};

//**************************************
// Communication Transport interface
//**************************************

/**
 * \class commTransport
 *  \ingroup streaming_network_simple_distributed_memory
 *
 * \brief This class defines the transport pattern interface. 
 *
 * This class is defined in file \ref inter.hpp
 *
 */
template <typename Impl>
class commTransport  {
protected:
    Impl impl;
public:
    typedef typename Impl::endpoint_t     endpoint_t;
    typedef typename Impl::msg_t          msg_t;

    /**
     * It is the construcor to create an empty communication trasport.
     *
     * \param procId is the unique id (aka rank) of the calling process.
     */
    commTransport(const int procId): impl(procId) {}

    /**
     * It initializes the communication transport.
     */
    int initTransport() { return impl.initTransport(); }
    
    /**
     * It closes the communication transport.
     */
    int closeTransport() { return impl.closeTransport(); }

    /**
     * It returns a transport specific communication end-point.
     */
    endpoint_t * newEndPoint(const bool P) {
        return impl.newEndPoint(P);
    }
    
    /**
     * It deletes the transport specific communication end-point.
     *
     * \param ep is a pointer to the end-point.
     */
    int deleteEndPoint(endpoint_t* ep) {
        return impl.deleteEndPoint(ep);
    }

    /**
     * It returns the the process identifier of the process.
     */
    int getProcId() const { return impl.getProcId();}
};

/*!
 *
 * @}
 * \endlink
 */


} // namespace
#endif  /* FF_COMMINTERFACE_HPP */
