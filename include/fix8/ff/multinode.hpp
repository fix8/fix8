/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 * \link
 * \file multinode.hpp
 * \ingroup building_blocks
 *
 * \brief FastFlow ff_minode ff_monode and typed versions.
 *
 * @detail FastFlow multi-input and multi-output nodes.
 *
 */

#ifndef FF_MULTINODE_HPP
#define FF_MULTINODE_HPP

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

#include <fix8/ff/node.hpp>
#include <fix8/ff/lb.hpp>
#include <fix8/ff/gt.hpp>

namespace ff {

/* This file provides the following classes:
 *   ff_minode
 *   ff_monode
 *   ff_minode_t (typed version of the ff_minode -- requires c++11)
 *   ff_monode_t (typed version of the ff_monode -- requires c++11)
 *
 */

/* ************************* Multi-Input node ************************* */

/*!
 * \ingroup building_blocks
 *
 * \brief Multiple input ff_node (the SPMC mediator)
 *
 * The ff_node with many input channels.
 *
 * This class is defined in \ref farm.hpp
 */

class ff_minode: public ff_node {
protected:

    /**
     * \brief Gets the number of input channels
     */
    inline int cardinality(BARRIER_T * const barrier)  {
        gt->set_barrier(barrier);
        return 1;
    }

    /**
     * \brief Creates the input channels
     *
     * \return >=0 if successful, otherwise -1 is returned.
     */
    int create_input_buffer(int nentries, bool fixedsize=true) {
        if (inputNodes.size()==0) {
            int r = ff_node::create_input_buffer(nentries, fixedsize);
            if (r!=0) return r;
            return 1;
        }
        return 0;
    }

    int  wait(/* timeout */) {
        if (gt->wait()<0) return -1;
        return 0;
    }

    // consumer
    virtual inline bool init_input_blocking(pthread_mutex_t   *&m,
                                            pthread_cond_t    *&c,
                                            std::atomic_ulong *&counter) {
        return gt->init_input_blocking(m,c,counter);
    }
    virtual inline void set_input_blocking(pthread_mutex_t   *&m,
                                           pthread_cond_t    *&c,
                                           std::atomic_ulong *&counter) {
        ff_node::set_input_blocking(m,c,counter);
    }

    // producer
    virtual inline bool init_output_blocking(pthread_mutex_t   *&m,
                                             pthread_cond_t    *&c,
                                             std::atomic_ulong *&counter) {
        return gt->init_output_blocking(m,c,counter);
    }
    virtual inline void set_output_blocking(pthread_mutex_t   *&m,
                                            pthread_cond_t    *&c,
                                            std::atomic_ulong *&counter) {
        gt->set_output_blocking(m,c,counter);
        ff_node::set_output_blocking(m,c,counter);
    }

    virtual inline pthread_mutex_t   &get_prod_m()        { return gt->get_prod_m(); }
    virtual inline pthread_cond_t    &get_prod_c()        { return gt->get_prod_c(); }
    virtual inline std::atomic_ulong &get_prod_counter()  { return gt->get_prod_counter();}

public:
    /**
     * \brief Constructor
     */
    ff_minode(int max_num_workers=DEF_MAX_NUM_WORKERS):
        ff_node(), gt(new ff_gatherer(max_num_workers)) { ff_node::setMultiInput(); }

    /**
     * \brief Destructor
     */
    virtual ~ff_minode() {
        if (gt) delete gt;
    }

    /**
     * \brief Assembly input channels
     *
     * Assembly input channelnames to ff_node channels
     */
    virtual inline int set_input(svector<ff_node *> & w) {
        inputNodes += w;
        return 0;
    }

    /**
     * \brief Assembly a input channel
     *
     * Assembly a input channelname to a ff_node channel
     */
    virtual inline int set_input(ff_node *node) {
        inputNodes.push_back(node);
        return 0;
    }

    virtual bool isMultiInput() const { return true;}

    virtual inline void get_out_nodes(svector<ff_node*>&w) {
        w.push_back(this);
    }

    /**
     * \brief Skip first pop
     *
     * Set up spontaneous start
     */
    inline void skipfirstpop(bool sk)   { ff_node::skipfirstpop(sk);}

    /**
     * \brief run
     *
     * \return 0 if successful, otherwise -1 is returned.
     *
     */
    int run(bool skip_init=false) {
        gt->set_filter(this);

        for(size_t i=0;i<inputNodes.size();++i)
            gt->register_worker(inputNodes[i]);
        if (ff_node::skipfirstpop()) gt->skipfirstpop();
        if (gt->run()<0) {
            error("ff_minode, running gather module\n");
            return -1;
        }
        return 0;
    }
    int freeze_and_run(bool=false) {
        gt->freeze();
        return run();
    }
    int run_then_freeze(ssize_t nw=-1) {
        if (gt->isfrozen()) {
            // true means that next time threads are frozen again
            gt->thaw(true, nw);
            return 0;
        }
        gt->freeze();
        return run();
    }

    int wait_freezing() { return gt->wait_freezing(); }

    /**
     * \brief Gets the channel id from which the data has just been received
     *
     */
    ssize_t get_channel_id() const { return gt->get_channel_id();}

    /**
     * \internal
     * \brief Gets the gt
     *
     * It gets the internal gatherer.
     *
     * \return A pointer to the FastFlow gatherer.
     */
    inline ff_gatherer *getgt() const { return gt;}

#if defined(TRACE_FASTFLOW)
    /**
     * \brief Prints the FastFlow trace
     *
     * It prints the trace of FastFlow.
     */
    inline void ffStats(std::ostream & out) {
        gt->ffStats(out);
    }
#endif

private:
    svector<ff_node*> inputNodes;
    ff_gatherer* gt;
};


    /* ************************* Multi-Ouput node ************************* */

/*!
 *  \ingroup building_blocks
 *
 * \brief Multiple output ff_node (the MPSC mediator)
 *
 * The ff_node with many output channels.
 *
 * This class is defined in \ref farm.hpp
 */

class ff_monode: public ff_node {
protected:
    /**
     * \brief Cardinatlity
     *
     * Defines the cardinatlity of the FastFlow node.
     *
     * \param barrier defines the barrier
     *
     * \return 1 is always returned.
     */
    inline int   cardinality(BARRIER_T * const barrier)  {
        lb->set_barrier(barrier);
        return 1;
    }

    int  wait(/* timeout */) {
        if (lb->waitlb()<0) return -1;
        return 0;
    }

    // consumer
    virtual inline bool init_input_blocking(pthread_mutex_t   *&m,
                                            pthread_cond_t    *&c,
                                            std::atomic_ulong *&counter) {
        return lb->init_input_blocking(m,c,counter);
    }
    virtual inline void set_input_blocking(pthread_mutex_t   *&m,
                                           pthread_cond_t    *&c,
                                           std::atomic_ulong *&counter) {
        lb->set_input_blocking(m,c,counter);
        ff_node::set_input_blocking(m,c,counter);
    }

    // producer
    virtual inline bool init_output_blocking(pthread_mutex_t   *&m,
                                             pthread_cond_t    *&c,
                                             std::atomic_ulong *&counter) {
        return lb->init_output_blocking(m,c,counter);
    }
    virtual inline void set_output_blocking(pthread_mutex_t   *&m,
                                            pthread_cond_t    *&c,
                                            std::atomic_ulong *&counter) {
        ff_node::set_output_blocking(m,c,counter);
    }

    virtual inline pthread_mutex_t   &get_cons_m()        { return lb->get_cons_m();}
    virtual inline pthread_cond_t    &get_cons_c()        { return lb->get_cons_c();}
    virtual inline std::atomic_ulong &get_cons_counter()  { return lb->get_cons_counter();}

public:
    /**
     * \brief Constructor
     *
     * \param max_num_workers defines the maximum number of workers
     *
     */
    ff_monode(int max_num_workers=DEF_MAX_NUM_WORKERS):
        ff_node(), lb(new ff_loadbalancer(max_num_workers)) {}

    /**
     * \brief Destructor
     */
    virtual ~ff_monode() {
        if (lb) delete lb;
    }

    /**
     * \brief Assembly the output channels
     *
     * Attach output channelnames to ff_node channels
     */
    virtual inline int set_output(svector<ff_node *> & w) {
        for(size_t i=0;i<w.size();++i)
            outputNodes.push_back(w[i]);
        return 0;
    }

    /**
     * \brief Assembly an output channels
     *
     * Attach a output channelname to ff_node channel
     */
    virtual inline int set_output(ff_node *node) {
        outputNodes.push_back(node);
        return 0;
    }

    virtual bool isMultiOutput() const { return true;}

    virtual inline void get_out_nodes(svector<ff_node*>&w) {
        w = outputNodes;
    }

    /**
     * \brief Skips the first pop
     *
     * Set up spontaneous start
     */
    inline void skipfirstpop(bool sk)   {
        if (sk) lb->skipfirstpop();
    }

    /**
     * \brief Sends one task to a specific node id.
     *
     * \return true if successful, false otherwise
     */
    inline bool ff_send_out_to(void *task, int id) {
        return lb->ff_send_out_to(task,id);
    }

    inline void broadcast_task(void *task) {
        lb->broadcast_task(task);
    }

    /**
     * \brief run
     *
     * \param skip_init defines if the initilization should be skipped
     *
     * \return 0 if successful, otherwise -1 is returned.
     */
    int run(bool skip_init=false) {
        if (!lb) return -1;
        lb->set_filter(this);

        for(size_t i=0;i<outputNodes.size();++i)
            lb->register_worker(outputNodes[i]);
        if (ff_node::skipfirstpop()) lb->skipfirstpop();
        if (lb->runlb()<0) {
            error("ff_monode, running loadbalancer module\n");
            return -1;
        }
        return 0;
    }

    int freeze_and_run(bool=false) {
        lb->freeze();
        return run(true);
    }

    /**
     * \internal
     * \brief Gets the internal lb (Emitter)
     *
     * It gets the internal lb (Emitter)
     *
     * \return A pointer to the lb
     */
    inline ff_loadbalancer * getlb() const { return lb;}

#if defined(TRACE_FASTFLOW)
    /*
     * \brief Prints the FastFlow trace
     *
     * It prints the trace of FastFlow.
     */
    inline void ffStats(std::ostream & out) {
        lb->ffStats(out);
    }
#endif

protected:
    svector<ff_node*> outputNodes;
    ff_loadbalancer* lb;
};


/* ************************* Multi-Input and Multi-Output node ************************* */
/*                   (typed version based on ff_minode and ff_monode )                   */

/*!
 *  \class ff_minode_t
 *  \ingroup building_blocks
 *
 *  \brief Typed multiple input ff_node (the SPMC mediator).
 *
 *  Key method is: \p svc (pure virtual).
 *
 *  This class is defined in \ref node.hpp
 */

template<typename IN_t, typename OUT_t = IN_t>
struct ff_minode_t: ff_minode {
    typedef IN_t  in_type;
    typedef OUT_t out_type;
    ff_minode_t():
        GO_ON((OUT_t*)FF_GO_ON),
        EOS((OUT_t*)FF_EOS),EOSW((OUT_t*)FF_EOSW),
        GO_OUT((OUT_t*)FF_GO_OUT),
        EOS_NOFREEZE((OUT_t*) FF_EOS_NOFREEZE),
        BLK((OUT_t*)FF_BLK), NBLK((OUT_t*)FF_NBLK) {
	}
    OUT_t * const GO_ON,  *const EOS, *const EOSW, *const GO_OUT, *const EOS_NOFREEZE, *const BLK, *const NBLK;
    virtual ~ff_minode_t()  {}
    virtual OUT_t* svc(IN_t*)=0;
    inline  void *svc(void *task) { return svc(reinterpret_cast<IN_t*>(task)); };
};

/*!
 *  \class ff_monode_t
 *  \ingroup building_blocks
 *
 *  \brief Typed multiple output ff_node (the MPSC mediator).
 *
 *  Key method is: \p svc (pure virtual).
 *
 *  This class is defined in \ref node.hpp
 */

template<typename IN_t, typename OUT_t = IN_t>
struct ff_monode_t: ff_monode {
    typedef IN_t  in_type;
    typedef OUT_t out_type;
    ff_monode_t():
        GO_ON((OUT_t*)FF_GO_ON),
        EOS((OUT_t*)FF_EOS),EOSW((OUT_t*)FF_EOSW),
        GO_OUT((OUT_t*)FF_GO_OUT),
        EOS_NOFREEZE((OUT_t*) FF_EOS_NOFREEZE),
        BLK((OUT_t*)FF_BLK), NBLK((OUT_t*)FF_NBLK) {
	}
    OUT_t * const GO_ON,  *const EOS, *const EOSW, *const GO_OUT, *const EOS_NOFREEZE, *const BLK, *const NBLK;
    virtual ~ff_monode_t()  {}
    virtual OUT_t* svc(IN_t*)=0;
    inline  void *svc(void *task) { return svc(reinterpret_cast<IN_t*>(task)); };
};

    // TODO: implement ff_minode_F<> and ff_monode_F<>


} // namespace

#endif /* FF_MULTINODE_HPP */
