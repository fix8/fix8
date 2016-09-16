/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 * \link
 * \file node.hpp
 * \ingroup building_blocks
 *
 * \brief FastFlow ff_node
 *
 * @detail FastFlow basic contanier for a shared-memory parallel activity
 *
 */

#ifndef FF_NODE_HPP
#define FF_NODE_HPP

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

#include <stdlib.h>
#include <iosfwd>
#include <functional>
#include <fix8/ff/platforms/platform.h>
#include <fix8/ff/cycle.h>
#include <fix8/ff/utils.hpp>
#include <fix8/ff/buffer.hpp>
#include <fix8/ff/ubuffer.hpp>
#include <fix8/ff/mapper.hpp>
#include <fix8/ff/config.hpp>
#include <fix8/ff/svector.hpp>
#include <fix8/ff/barrier.hpp>
#include <atomic>

static void *GO_ON        = (void*)ff::FF_GO_ON;
static void *GO_OUT       = (void*)ff::FF_GO_OUT;
static void *EOS_NOFREEZE = (void*)ff::FF_EOS_NOFREEZE;
static void *EOS          = (void*)ff::FF_EOS;
static void *EOSW         = (void*)ff::FF_EOSW;
static void *BLK          = (void*)ff::FF_BLK;
static void *NBLK         = (void*)ff::FF_NBLK;

namespace ff {

// fftree stuff
struct fftree;   // forward declaration
enum fftype {
	FARM, PIPE, EMITTER, WORKER, OCL_WORKER, TPC_WORKER, COLLECTOR
};


// TODO: Should be rewritten in terms of mapping_utils.hpp
#if defined(HAVE_PTHREAD_SETAFFINITY_NP) && !defined(NO_DEFAULT_MAPPING)

    /*
     *
     * \brief Initialize thread affinity
     * It initializes thread affinity i.e. which cpu the thread should be
     * assigned.
     *
     * \note Linux-specific code
     *
     * \param attr is the pthread attribute
     * \param cpuID is the identifier the core
     * \return -2  if error, the cpu identifier if successful
     */
static inline int init_thread_affinity(pthread_attr_t*attr, int cpuId) {
    // This is linux-specific code
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    int id;
    if (cpuId<0) {
        id = threadMapper::instance()->getCoreId();
        CPU_SET (id, &cpuset);
    } else  {
        id = cpuId;
        CPU_SET (cpuId, &cpuset);
    }

    if (pthread_attr_setaffinity_np (attr, sizeof(cpuset), &cpuset)<0) {
        perror("pthread_attr_setaffinity_np");
        return -2;
    }
    return id;
}
#elif !defined(HAVE_PTHREAD_SETAFFINITY_NP) && !defined(NO_DEFAULT_MAPPING)

/*
 * \brief Initializes thread affinity
 *
 * It initializes thread affinity i.e. it defines to which core ths thread
 * should be assigned.
 *
 * \return always return -1 because no thread mapping is done
 */
static inline int init_thread_affinity(pthread_attr_t*,int) {
    // Ensure that the threadMapper constructor is called
    threadMapper::instance();
    return -1;
}
#else
/*
 * \brief Initializes thread affinity
 *
 * It initializes thread affinity i.e. it defines to which core ths thread
 * should be assigned.
 *
 * \return always return -1 because no thread mapping is done
 */
static inline int init_thread_affinity(pthread_attr_t*,int) {
    // Do nothing
    return -1;
}
#endif /* HAVE_PTHREAD_SETAFFINITY_NP */


// forward decl
/*
 * \brief Proxy thread routine
 *
 */
static void * proxy_thread_routine(void * arg);

/*!
 *  \class ff_thread
 *  \ingroup buiding_blocks
 *
 *  \brief thread container for (leaves) ff_node
 *
 * It defines FastFlow's threading abstraction to run ff_node in parallel
 * in the shared-memory runtime
 *
 * \note Should not be used directly, it is called by ff_node
 */
class ff_thread {

    friend void * proxy_thread_routine(void *arg);

protected:
    ff_thread(BARRIER_T * barrier=NULL):
    	fftree_ptr(NULL),
        tid((size_t)-1),threadid(0), barrier(barrier),
        stp(true), // only one shot by default
        spawned(false),
        freezing(0), frozen(false),isdone(false),
        init_error(false), attr(NULL) {

        /* Attr is NULL, default mutex attributes are used. Upon successful
         * initialization, the state of the mutex becomes initialized and
         * unlocked.
         * */
        if (pthread_mutex_init(&mutex,NULL)!=0) {
            error("FATAL ERROR: ff_thread: pthread_mutex_init fails!\n");
            abort();
        }
        if (pthread_cond_init(&cond,NULL)!=0) {
            error("FATAL ERROR: ff_thread: pthread_cond_init fails!\n");
            abort();
        }
        if (pthread_cond_init(&cond_frozen,NULL)!=0) {
            error("FATAL ERROR: ff_thread: pthread_cond_init fails!\n");
            abort();
        }
    }

    virtual ~ff_thread() {}

    void thread_routine() {
        threadid = ff_getThreadID();
        if (barrier) barrier->doBarrier(tid);
        void * ret;
        do {
            init_error=false;
            if (svc_init()<0) {
                error("ff_thread, svc_init failed, thread exit!!!\n");
                init_error=true;
                break;
            } else  {
                ret = svc(NULL);
            }
            svc_end();

            if (disable_cancelability()) {
                error("ff_thread, thread_routine, could not change thread cancelability");
                return;
            }

            // acquire lock. While freezing is true,
            // freeze and wait.
            pthread_mutex_lock(&mutex);
            if (ret != EOS_NOFREEZE && !stp) {
                if ((freezing == 0) && (ret == EOS)) stp = true;
                while(freezing==1) { // NOTE: freezing can change to 2
                    frozen=true;
                    pthread_cond_signal(&cond_frozen);
                    pthread_cond_wait(&cond,&mutex);
                }
            }

            //thawed=true;
            //pthread_cond_signal(&cond);
            //frozen=false;
            if (freezing != 0) freezing = 1; // freeze again next time
            pthread_mutex_unlock(&mutex);

            if (enable_cancelability()) {
                error("ff_thread, thread_routine, could not change thread cancelability");
                return;
            }
        } while(!stp);

        if (freezing) {
            pthread_mutex_lock(&mutex);
            frozen=true;
            pthread_cond_signal(&cond_frozen);
            pthread_mutex_unlock(&mutex);
        }
        isdone = true;
    }

    int disable_cancelability() {
        if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancelstate)) {
            perror("pthread_setcanceltype");
            return -1;
        }
        return 0;
    }

    int enable_cancelability() {
        if (pthread_setcancelstate(old_cancelstate, 0)) {
            perror("pthread_setcanceltype");
            return -1;
        }
        return 0;
    }

    fftree *getfftree() const   { return fftree_ptr;}
    void setfftree(const fftree *ptr) {
        fftree_ptr=const_cast<fftree*>(ptr);
    }

#if defined(FF_TASK_CALLBACK)
    virtual void callbackIn(void  *t=NULL) {  }
    virtual void callbackOut(void *t=NULL) {  }
#endif

public:

    virtual void* svc(void * task) = 0;
    virtual int   svc_init() { return 0; };
    virtual void  svc_end()  {}

    virtual void set_barrier(BARRIER_T * const b) { barrier=b;}

    virtual int run(bool=false) { return spawn(); }

    virtual int spawn(int cpuId=-1) {
        if (spawned) return -1;

        if ((attr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t))) == NULL) {
            printf("spawn: pthread can not be created, malloc failed\n");
            return -1;
        }
        if (pthread_attr_init(attr)) {
                perror("pthread_attr_init: pthread can not be created.");
                return -1;
        }

        int CPUId = init_thread_affinity(attr, cpuId);
        if (CPUId==-2) return -2;
        if (barrier) tid= barrier->getCounter();
        int r=0;
        if ((r=pthread_create(&th_handle, attr,
                              proxy_thread_routine, this)) != 0) {
            errno=r;
            perror("pthread_create: pthread creation failed.");
            return -2;
        }
        if (barrier) barrier->incCounter();
        spawned = true;
        return CPUId;
    }

    virtual int wait() {
        int r=0;
        stp=true;
        if (isfrozen()) {
            wait_freezing();
            thaw();
        }
        if (spawned) {
            pthread_join(th_handle, NULL);
            if (barrier) barrier->decCounter();
        }
        if (attr) {
            if (pthread_attr_destroy(attr)) {
                error("ERROR: ff_thread.wait: pthread_attr_destroy fails!");
                r=-1;
            }
            free(attr);
            attr = NULL;
        }
        spawned=false;
        return r;
    }

    virtual int wait_freezing() {
        pthread_mutex_lock(&mutex);
        while(!frozen) pthread_cond_wait(&cond_frozen,&mutex);
        pthread_mutex_unlock(&mutex);
        return (init_error?-1:0);
    }

    virtual void stop() { stp = true; };

    virtual void freeze() {
        stp=false;
        freezing = 1;
    }

    virtual void thaw(bool _freeze=false, ssize_t=-1) {
        pthread_mutex_lock(&mutex);
        // if this function is called even if the thread is not
        // in frozen state, then freezing has to be set to 1 and not 2
        //if (_freeze) freezing= (frozen?2:1); // next time freeze again the thread
        // October 2014, changed the above policy.
        // If thaw is called and the thread is not in the frozen stage,
        // then the thread won't fall to sleep at the next freezing point

        if (_freeze) freezing = 2; // next time freeze again the thread
        else freezing=0;
        //assert(thawed==false);
        frozen=false;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        //pthread_mutex_lock(&mutex);
        //while(!thawed) pthread_cond_wait(&cond, &mutex);
        //thawed=false;
        //pthread_mutex_unlock(&mutex);
    }
    virtual bool isfrozen() const { return freezing>0;}
    virtual bool done()     const { return isdone || (frozen && !stp);}

    pthread_t get_handle() const { return th_handle;}

    inline size_t getTid() const { return tid; }
    inline size_t getOSThreadId() const { return threadid; }

protected:
    fftree       *  fftree_ptr;         /// fftree stuff
    size_t          tid;                /// unique logical id of the thread
    size_t          threadid;           /// OS specific thread ID
private:
    BARRIER_T    *  barrier;            /// A \p Barrier object
    bool            stp;
    bool            spawned;
    int             freezing;   // changed from bool to int
    bool            frozen,isdone;
    bool            init_error;
    pthread_t       th_handle;
    pthread_attr_t *attr;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    pthread_cond_t  cond_frozen;
    int             old_cancelstate;
};

static void * proxy_thread_routine(void * arg) {
    ff_thread & obj = *(ff_thread *)arg;
    obj.thread_routine();
    pthread_exit(NULL);
    return NULL;
}


/*!
 *  \class ff_node
 *  \ingroup building_blocks
 *
 *  \brief The FastFlow abstract contanier for a parallel activity (actor).
 *
 * Implements \p ff_node, i.e. the general container for a parallel
 * activity. From the orchestration viewpoint, the process model to
 * be employed is a CSP/Actor hybrid model where activities (\p
 * ff_nodes) are named and the data paths between processes are
 * clearly identified. \p ff_nodes synchronise each another via
 * abstract units of SPSC communications and synchronisation (namely
 * 1:1 channels), which models data dependency between two
 * \p ff_nodes.  It is used to encapsulate
 * sequential portions of code implementing functions.
 *
 * \p In a multicore, a ff_node is implemented as non-blocking thread.
 * It is not and should
 * not be confused with a task. Typically a \p ff_node uses the 100% of one CPU
 * context (i.e. one core, either physical or HT, if any). Overall, the number of
 * ff_nodes running should not exceed the number of logical cores of the platform.
 *
 * \p A ff_node behaves as a loop that gets an input (i.e. the parameter of \p svc
 * method) and produces one or more outputs (i.e. return parameter of \p svc method
 * or parameter of the \p ff_send_out method that can be called in the \p svc method).
 * The loop complete on the output of the special value "end-of_stream" (EOS).
 * The EOS is propagated across channels to the next \p ff_node.
 *
 * Key methods are: \p svc_init, \p svc_end (optional), and \p svc (pure virtual,
 * mandatory). The \p svc_init method is called once at node initialization,
 * while the \p svn_end method is called after a EOS task has been returned.
 *
 *  This class is defined in \ref node.hpp
 */

class ff_node {
private:

    template <typename lb_t, typename gt_t>
    friend class ff_farm;
    friend class ff_pipeline;
    friend class ff_map;
    template <typename IN,typename OUT>
    friend class ff_nodeSelector;
    friend class ff_loadbalancer;
    friend class ff_gatherer;
    friend class ff_ofarm;

private:
    FFBUFFER        * in;           ///< Input buffer, built upon SWSR lock-free (wait-free)
                                    ///< (un)bounded FIFO queue
    FFBUFFER        * out;          ///< Output buffer, built upon SWSR lock-free (wait-free)
                                    ///< (un)bounded FIFO queue
    ssize_t           myid;         ///< This is the node id, it is valid only for farm's workers
    ssize_t           CPUId;
    bool              myoutbuffer;
    bool              myinbuffer;
    bool              skip1pop;
    bool              in_active;    // allows to disable/enable input tasks receiving
    bool              multiInput;   // if the node is a multi input node this is true
    bool              multiOutput;  // if the node is a multi output node this is true
    bool              my_own_thread;

    ff_thread       * thread;       /// A \p thWorker object, which extends the \p ff_thread class
    bool (*callback)(void *,unsigned long,unsigned long, void *);
    void            * callback_arg;
    BARRIER_T       * barrier;      /// A \p Barrier object
    struct timeval tstart;
    struct timeval tstop;
    struct timeval wtstart;
    struct timeval wtstop;
    double wttime;

protected:
    bool               blocking_in;
    bool               blocking_out;
protected:

    void set_id(ssize_t id) { myid = id;}

    virtual inline bool push(void * ptr) { return out->push(ptr); }
    virtual inline bool pop(void ** ptr) {
        if (!in_active) return false; // it does not want to receive data
        return in->pop(ptr);
    }
    virtual inline bool Push(void *ptr, unsigned long retry=((unsigned long)-1), unsigned long ticks=(TICKS2WAIT)) {
        if (blocking_out) {
        retry:
            bool r = push(ptr);
            if (r) { // OK
                pthread_mutex_lock(p_cons_m);
                if ((*p_cons_counter).load() == 0) {
                    pthread_cond_signal(p_cons_c);
                }
                ++(*p_cons_counter);
                pthread_mutex_unlock(p_cons_m);
                ++prod_counter;
            } else { // FULL
                //assert(fixedsize);
                pthread_mutex_lock(&prod_m);
                while(prod_counter.load() >= out->buffersize()) {
                    pthread_cond_wait(&prod_c,&prod_m);
                }
                pthread_mutex_unlock(&prod_m);
                goto retry;
            }
            return true;
        }
        for(unsigned long i=0;i<retry;++i) {
            if (push(ptr)) return true;
            losetime_out(ticks);
        }
        return false;
    }

    virtual inline bool Pop(void **ptr, unsigned long retry=((unsigned long)-1), unsigned long ticks=(TICKS2WAIT)) {
        if (blocking_in) {
            if (!in_active) { *ptr=NULL; return false; }
        retry:
            bool r = in->pop(ptr);
            if (r) { // OK
                // TODO: possible optimization, p_prod_m is NULL if the queue is unbounded
                //if (p_prod_m) { // this is true only if fixedsize==true
                pthread_mutex_lock(p_prod_m);
                if ((*p_prod_counter).load() >= in->buffersize()) {
                    pthread_cond_signal(p_prod_c);
                }
                --(*p_prod_counter);
                pthread_mutex_unlock(p_prod_m);
                //}
                --cons_counter;
            } else { // EMPTY
                pthread_mutex_lock(&cons_m);
                while (cons_counter.load() == 0) {
                    pthread_cond_wait(&cons_c, &cons_m);
                }
                pthread_mutex_unlock(&cons_m);
                goto retry;
            }
            return true;
        }
        for(unsigned long i=0;i<retry;++i) {
            if (!in_active) { *ptr=NULL; return false; }
            if (pop(ptr)) return true;
            losetime_in(ticks);
        }
        return true;
    }


    // consumer
    virtual inline bool init_input_blocking(pthread_mutex_t   *&m,
                                            pthread_cond_t    *&c,
                                            std::atomic_ulong *&counter) {
        if (cons_counter.load() == (unsigned long)-1) {
            if (pthread_mutex_init(&cons_m, NULL) != 0) return false;
            if (pthread_cond_init(&cons_c, NULL) != 0) {
                pthread_mutex_destroy(&cons_m);
                return false;
            }
            cons_counter.store(0);
        }
        m = &cons_m,  c = &cons_c, counter = &cons_counter;
        return true;
    }
    virtual inline void set_input_blocking(pthread_mutex_t   *&m,
                                           pthread_cond_t    *&c,
                                           std::atomic_ulong *&counter) {
        p_prod_m = m,  p_prod_c = c,  p_prod_counter = counter;
    }

    // producer
    virtual inline bool init_output_blocking(pthread_mutex_t   *&m,
                                             pthread_cond_t    *&c,
                                             std::atomic_ulong *&counter) {
        if (prod_counter.load() == (unsigned long)-1) {
            if (pthread_mutex_init(&prod_m, NULL) != 0) return false;
            if (pthread_cond_init(&prod_c, NULL) != 0) {
                pthread_mutex_destroy(&prod_m);
                return false;
            }
            prod_counter.store(0);
        }
        m = &prod_m, c = &prod_c, counter = &prod_counter;
        return true;
    }
    virtual inline void set_output_blocking(pthread_mutex_t   *&m,
                                            pthread_cond_t    *&c,
                                            std::atomic_ulong *&counter) {
        p_cons_m = m, p_cons_c = c, p_cons_counter = counter;
    }

    virtual inline pthread_mutex_t   &get_cons_m()       { return cons_m;}
    virtual inline pthread_cond_t    &get_cons_c()       { return cons_c;}
    virtual inline std::atomic_ulong &get_cons_counter() { return cons_counter;}

    virtual inline pthread_mutex_t   &get_prod_m()       { return prod_m;}
    virtual inline pthread_cond_t    &get_prod_c()       { return prod_c;}
    virtual inline std::atomic_ulong &get_prod_counter() { return prod_counter;}

    /**
     * \brief Set the ff_node to start with no input task
     *
     * Setting it to true let the \p ff_node execute the \p svc method spontaneusly
     * before receiving a task on the input channel. \p skipfirstpop makes it possible
     * to define a "producer" node that starts the network.
     *
     * \param sk \p true start spontaneously (*task will be NULL)
     *
     */
    virtual inline void skipfirstpop(bool sk)   { skip1pop=sk;}

    /**
     * \brief Gets the status of spontaneous start
     *
     * If \p true the \p ff_node execute the \p svc method spontaneusly
     * before receiving a task on the input channel. \p skipfirstpop makes it possible
     * to define a "producer" node that produce the stream.
     *
     * \return \p true if skip-the-first-element mode is set, \p false otherwise
     *
     * Example: \ref l1_ff_nodes_graph.cpp
     */
    bool skipfirstpop() const { return skip1pop; }

    /**
     * \brief Creates the input channel
     *
     *  \param nentries: the number of elements of the buffer
     *  \param fixedsize flag to decide whether the buffer is bound or unbound.
     *  Default is \p true.
     *
     *  \return 0 if successful, -1 otherwise
     */
    virtual int create_input_buffer(int nentries, bool fixedsize=true) {
        if (in) return -1;
        // MA: here probably better to use p = posix_memalign to 64 bits; new (p) FFBUFFER
		in = new FFBUFFER(nentries,fixedsize);
        if (!in) return -1;
        myinbuffer=true;
        if (!in->init()) return -1;
        return 0;
    }

    /**
     *  \brief Creates the output channel
     *
     *  \param nentries: the number of elements of the buffer
     *  \param fixedsize flag to decide whether the buffer is bound or unbound.
     *  Default is \p true.
     *
     *  \return 0 if successful, -1 otherwise
     */
    virtual int create_output_buffer(int nentries, bool fixedsize=false) {
        if (out) return -1;
        out = new FFBUFFER(nentries,fixedsize);
        if (!out) return -1;
        myoutbuffer=true;
        if (!out->init()) return -1;
        return 0;
    }

    /**
     *  \brief Assign the output channelname to a channel
     *
     * Attach the output of a \p ff_node to an existing channel, typically the input
     * channel of another \p ff_node
     *
     *  \param o reference to a channel of type \p FFBUFFER
     *
     *  \return 0 if successful, -1 otherwise
     */
    virtual int set_output_buffer(FFBUFFER * const o) {
        if (myoutbuffer) return -1;
        out = o;
        return 0;
    }


    /**
     *  \brief Assign the input channelname to a channel
     *
     * Attach the input of a \p ff_node to an existing channel, typically the output
     * channel of another \p ff_node
     *
     *  \param i a buffer object of type \p FFBUFFER
     *
     *  \return 0 if successful, -1 otherwise
     */
    virtual int set_input_buffer(FFBUFFER * const i) {
        if (myinbuffer) return -1;
        in = i;
        return 0;
    }

    virtual inline int set_input(svector<ff_node *> & w) { return -1;}
    virtual inline int set_input(ff_node *) { return -1;}
    virtual inline bool isMultiInput() const { return multiInput;}
    virtual inline void setMultiInput()      { multiInput = true; }
    virtual inline int set_output(svector<ff_node *> & w) { return -1;}
    virtual inline int set_output(ff_node *) { return -1;}
    virtual inline bool isMultiOutput() const { return multiOutput;}
    virtual inline void setMultiOutput()      { multiOutput = true; }

    virtual inline void get_out_nodes(svector<ff_node*>&w) {}


    /**
     * \brief Run the ff_node
     *
     * \return 0 success, -1 otherwise
     */
    virtual int run(bool=false) {
        if (thread) delete reinterpret_cast<thWorker*>(thread);
        thread = new thWorker(this);
        if (!thread) return -1;
        return thread->run();
    }

    /**
     * \brief Suspend (freeze) the ff_node and run it
     *
     * Only initialisation will be performed
     *
     * \return 0 success, -1 otherwise
     */
    virtual int freeze_and_run(bool=false) {
        if (thread) delete reinterpret_cast<thWorker*>(thread);
        thread = new thWorker(this);
        if (!thread) return 0;
        freeze();
        return thread->run();
    }

    /**
     * \brief Wait ff_node termination
     *
     * \return 0 success, -1 otherwise
     */
    virtual int  wait() {
        if (!thread) return 0;
        return thread->wait();
    }

    /**
     * \brief Wait the freezing state
     *
     * It will happen on EOS arrival on the input channel
     *
     * \return 0 success, -1 otherwise
     */
    virtual int  wait_freezing() {
        if (!thread) return 0;
        return thread->wait_freezing();
    }

    virtual void stop() {
        if (!thread) return;
        thread->stop();
    }

    /**
     * \brief Freeze (suspend) a ff_node
     */
    virtual void freeze() {
        if (!thread) return;
        thread->freeze();
    }

    /**
     * \brief Thaw (resume) a ff_node
     */
    virtual void thaw(bool _freeze=false, ssize_t=-1) {
        if (!thread) return;
        thread->thaw(_freeze);
    }

    /**
     * \brief Checks if a ff_node is frozen
     * \return \p true is it frozen
     */
    virtual bool isfrozen() const {
        if (!thread) return false;
        return thread->isfrozen();
    }


    virtual bool done() const {
        if (!thread) return true;
        return thread->done();
    }


    virtual bool isoutbuffermine() const { return myoutbuffer;}

    virtual int  cardinality(BARRIER_T * const b) {
        barrier = b;
        return 1;
    }

    virtual void set_barrier(BARRIER_T * const b) {
        barrier = b;
    }

    /**
     * \brief Misure \ref ff::ff_node execution time
     *
     * \return time (ms)
     */
    virtual double ffTime() {
        return diffmsec(tstop,tstart);
    }

    /**
     * \brief Misure \ref ff_node::svc execution time
     *
     * \return time (ms)
     */
    virtual double wffTime() {
        return diffmsec(wtstop,wtstart);
    }

public:
    /*
     * \brief Default retry delay in nonblocking get/put on channels
     */
    enum {TICKS2WAIT=1000};

    /**
     *  \brief Destructor, polymorphic deletion through base pointer is allowed.
     *
     *
     */
    virtual  ~ff_node() {
        if (in && myinbuffer) delete in;
        if (out && myoutbuffer) delete out;
        if (thread && my_own_thread) delete reinterpret_cast<thWorker*>(thread);
    };

    /**
     * \brief The service callback (should be filled by user with parallel activity business code)
     *
     * \param task is a the input data stream item pointer (task)
     * \return output data stream item pointer
     */
    virtual void* svc(void * task) = 0;

    /**
     * \brief Service initialisation
     *
     * Called after run-time initialisation (e.g. thread spawning) but before
     * to start to get items from input stream (can be useful for initialisation
     * of parallel activities, e.g. manual thread pinning that cannot be done in
     * the costructor because threads stil do not exist).
     *
     * \return 0
     */
    virtual int svc_init() { return 0; }

    /**
     *
     * \brief Service finalisation
     *
     * Called after EOS arrived (logical termination) but before shutdding down
     * runtime support (can be useful for housekeeping)
     */
    virtual void  svc_end() {}


    /**
     * \brief Node initialisation
     *
     * This is a different initialization method with respect to svc_init (the default method).
     * This can be used to explicitly initialize the object when the node is not running as a thread.
     *
     * \return 0
     */
    virtual int   nodeInit() { return 0; }

    /**
     * \brief Node finalisation.
     *
     * This is a different finalisation method with respect to svc_end (the default method).
     * This can be used to explicitly finalise the object when the node is not running as a thread.
     */
    virtual void  nodeEnd()  { }

    virtual void eosnotify(ssize_t id=-1) {}

    virtual ssize_t get_my_id() const { return myid; };

    /**
     * \brief Returns the OS specific thread id of the node.
     *
     * The returned id is valid (>0) only if the node is an active node (i.e. the thread has been created).
     *
     */
    inline size_t getOSThreadId() const { if (thread) return thread->getOSThreadId(); return 0; }


#if defined(FF_TASK_CALLBACK)
    virtual void callbackIn(void *t=NULL)  { }
    virtual void callbackOut(void *t=NULL) { }
#endif

    // returns the kind of node
    virtual inline fftype getFFType() const   { return WORKER; }


    /**
     * \brief Force ff_node-to-core pinning
     *
     * \param cpuID is the ID of the CPU to which the thread will be pinned.
     */
    virtual void  setAffinity(int cpuID) {
        if (cpuID<0 || !threadMapper::instance()->checkCPUId(cpuID) ) {
            error("setAffinity, invalid cpuID\n");
        }
        CPUId=cpuID;
    }

    /**
     * \internal
     * \brief Gets the CPU id (if set) of this node is pinned
     *
     * It gets the ID of the CPU where the ff_node is running.
     *
     * \return The identifier of the CPU.
     */
    virtual int getCPUId() const { return CPUId; }

    /**
     * \brief Nonblocking put onto output channel
     *
     * Wait-free and fence-free (under TSO)
     *
     * \param ptr is a pointer to the task
     *
     */
    virtual inline bool  put(void * ptr) {
        return in->push(ptr);
    }

    /**
     * \brief Noblocking pop from input channel
     *
     * Wait-free and fence-free (under TSO)
     *
     * \param ptr is a pointer to the task
     *
     */
    virtual inline bool  get(void **ptr) { return out->pop(ptr);}

    virtual inline void losetime_out(unsigned long ticks=ff_node::TICKS2WAIT) {
        FFTRACE(lostpushticks+=ticks; ++pushwait);
#if defined(SPIN_USE_PAUSE)
        const long n = (long)ticks/2000;
        for(int i=0;i<=n;++i) PAUSE();
#else
        ticks_wait(ticks);
#endif /* SPIN_USE_PAUSE */
    }

    virtual inline void losetime_in(unsigned long ticks=ff_node::TICKS2WAIT) {
        FFTRACE(lostpopticks+=ticks; ++popwait);
#if defined(SPIN_USE_PAUSE)
        const long n = (long)ticks/2000;
        for(int i=0;i<=n;++i) PAUSE();
#else
        ticks_wait(ticks);
#endif /* SPIN_USE_PAUSE */
    }

    /**
     * \brief Gets input channel
     *
     * It returns a pointer to the input buffer.
     *
     * \return A pointer to the input buffer
     */
    virtual FFBUFFER * get_in_buffer() const { return in;}

    /**
     * \brief Gets pointer to the output channel
     *
     * It returns a pointer to the output buffer.
     *
     * \return A pointer to the output buffer.
     */
    virtual FFBUFFER * get_out_buffer() const { return out;}

    virtual const struct timeval getstarttime() const { return tstart;}

    virtual const struct timeval getstoptime()  const { return tstop;}

    virtual const struct timeval getwstartime() const { return wtstart;}

    virtual const struct timeval getwstoptime() const { return wtstop;}

#if defined(TRACE_FASTFLOW)
    virtual void ffStats(std::ostream & out) {
        out << "ID: " << get_my_id()
            << "  work-time (ms): " << wttime    << "\n"
            << "  n. tasks      : " << taskcnt   << "\n"
            << "  svc ticks     : " << tickstot  << " (min= " << ticksmin << " max= " << ticksmax << ")\n"
            << "  n. push lost  : " << pushwait  << " (ticks=" << lostpushticks << ")" << "\n"
            << "  n. pop lost   : " << popwait   << " (ticks=" << lostpopticks  << ")" << "\n";
    }

    virtual double getworktime() const { return wttime; }
    virtual size_t getnumtask()  const { return taskcnt; }
    virtual ticks  getsvcticks() const { return tickstot; }
    virtual size_t getpushlost() const { return pushwait;}
    virtual size_t getpoplost()  const { return popwait; }
#endif

    /**
     * \brief Sends out the task
     *
     * It allows to emit tasks on output stream without returning from the \p svc method.
     * Make the ff_node to emit zero or more tasks per input task
     *
     * \param task a pointer to the task
     * \param retry number of tries to put (nonbloking partial) the task to output channel
     * \param ticks delay between successive retries
     *
     */
    virtual bool ff_send_out(void * task,
                             unsigned long retry=((unsigned long)-1),
                             unsigned long ticks=(TICKS2WAIT)) {
        if (callback) return  callback(task,retry,ticks,callback_arg);
        bool r =Push(task,retry,ticks);
        if (task == BLK || task == NBLK) {
            blocking_out = (task == BLK);
        }
#if defined(FF_TASK_CALLBACK)
        if (r) callbackOut();
#endif
        return r;
    }

    // Warning resetting queues while the node is running may produce unexpected results.
    virtual void reset() {
        if (in)  in->reset();
        if (out) out->reset();
    }


#if defined(FF_REPARA)
    struct rpr_measure_t {
        size_t time_before, time_after;
        size_t bytesIn, bytesOut;
        size_t vmSize, vmPeak;
        double energy;
    };

    using RPR_devices_measure = std::vector<std::pair<int, std::vector<rpr_measure_t> > >;
    using RPR_measures_vector = std::vector<std::vector<RPR_devices_measure> >;

    /**
     *  Returns input data size
     */
    virtual size_t rpr_get_sizeIn()  const { return rpr_sizeIn; }

    /**
     *  Returns output data size
     */
    virtual size_t rpr_get_sizeOut() const { return rpr_sizeOut; }



    virtual bool rpr_get_measure_energy() const { return measureEnergy; }

    virtual void rpr_set_measure_energy(bool v) { measureEnergy = v; }


    /**
     *  Returns all measures collected by the node.
     *  The structure is:
     *    - the outermost vector is greater than 1 if the node is a pipeline or a farm
     *    - each stage of a pipeline or a worker of a farm can be a pipeline or a farm as well
     *      therefore the second level vector is grater than 1 only if the stage is a pipeline or a farm
     *    - each entry of a stage is a vector containing info for each device associated to the stage.
     *      The device is identified by the first entry of the std::pair, the second element of the pair
     *      is a vector containing the measurments for the period considered.
     */
    virtual RPR_measures_vector rpr_get_measures() { return RPR_measures_vector(); }

protected:
    bool   measureEnergy = false;
    size_t rpr_sizeIn  = {0};
    size_t rpr_sizeOut = {0};
#endif  /* FF_REPARA */

protected:

    ff_node():in(0),out(0),myid(-1),CPUId(-1),
              myoutbuffer(false),myinbuffer(false),
              skip1pop(false), in_active(true),
              multiInput(false), multiOutput(false), my_own_thread(true),
              thread(NULL),callback(NULL),barrier(NULL) {
        time_setzero(tstart);time_setzero(tstop);
        time_setzero(wtstart);time_setzero(wtstop);
        wttime=0;
        FFTRACE(taskcnt=0;lostpushticks=0;pushwait=0;lostpopticks=0;popwait=0;ticksmin=(ticks)-1;ticksmax=0;tickstot=0);

        fftree_ptr = NULL;

        cons_counter.store(-1);
        prod_counter.store(-1);
        p_prod_m = NULL, p_prod_c = NULL, p_prod_counter = NULL;
        p_cons_m = NULL, p_cons_c = NULL, p_cons_counter = NULL;

        blocking_in = blocking_out = RUNTIME_MODE;
    };

    virtual inline void input_active(const bool onoff) {
        if (in_active != onoff)
            in_active= onoff;
    }

    fftree *getfftree() const   { return fftree_ptr;}
    void setfftree(const fftree *ptr) {
        fftree_ptr=const_cast<fftree*>(ptr);
    }

    void registerCallback(bool (*cb)(void *,unsigned long,unsigned long,void *), void * arg) {
        callback=cb;
        callback_arg=arg;
    }

private:
    /* ------------------------------------------------------------------------------------- */
    class thWorker: public ff_thread {
    public:
        thWorker(ff_node * const filter):
            ff_thread(filter->barrier),filter(filter) {}

        inline bool push(void * task) {
            /* NOTE: filter->push and not buffer->push because of the filter can be a dnode
             */
            return filter->Push(task);
        }

        inline bool pop(void ** task) {
            /*
             * NOTE: filter->pop and not buffer->pop because of the filter can be a dnode
             */
            return filter->Pop(task);
        }

        inline bool put(void * ptr) { return filter->put(ptr);}

        inline bool get(void **ptr) { return filter->get(ptr);}

        inline void* svc(void * ) {
            void * task = NULL;
            void * ret  = EOS;
            bool inpresent  = (filter->get_in_buffer() != NULL);
            bool outpresent = (filter->get_out_buffer() != NULL);
            bool skipfirstpop = filter->skipfirstpop();
            bool exit=false;

            gettimeofday(&filter->wtstart,NULL);
            do {
                if (inpresent) {
                    if (!skipfirstpop) pop(&task);
                    else skipfirstpop=false;
                    if ((task == EOS) || (task == EOSW) ||
                        (task == EOS_NOFREEZE)) {
                        ret = task;
                        filter->eosnotify();
                        // only EOS and EOSW are propagated
                        if (outpresent && ( (task == EOS) || (task == EOSW)) )  {
                            push(task);
                        }
                        break;
                    }
                    if (task == GO_OUT) break;
                }
                if (task == BLK || task == NBLK) {
                    if (outpresent) push(task);
                    filter->blocking_in = (task == BLK);
                    filter->blocking_out = filter->blocking_in;
                    continue;
                }
                FFTRACE(++filter->taskcnt);
                FFTRACE(ticks t0 = getticks());

#if defined(FF_TASK_CALLBACK)
                if (filter) callbackIn();
#endif

                ret = filter->svc(task);

#if defined(TRACE_FASTFLOW)
                ticks diff=(getticks()-t0);
                filter->tickstot +=diff;
                filter->ticksmin=(std::min)(filter->ticksmin,diff); // (std::min) for win portability)
                filter->ticksmax=(std::max)(filter->ticksmax,diff);
#endif

                if (ret == GO_OUT) break;
                if (outpresent && (ret == BLK || ret == NBLK)) {
                    push(ret);
                    filter->blocking_out = (ret == BLK);
                    continue;
                }
                if (!ret || (ret >= EOSW)) { // EOS or EOS_NOFREEZE or EOSW
                    // NOTE: The EOS is gonna be produced in the output queue
                    // and the thread exits even if there might be some tasks
                    // in the input queue !!!
                    if (!ret) ret = EOS;
                    exit=true;
                }
                if ( outpresent && ((ret != GO_ON) && (ret != EOS_NOFREEZE)) ) {
                    push(ret);
#if defined(FF_TASK_CALLBACK)
                    if (filter) callbackOut();
#endif
                }
            } while(!exit);

            gettimeofday(&filter->wtstop,NULL);
            filter->wttime+=diffmsec(filter->wtstop,filter->wtstart);

            return ret;
        }

        int svc_init() {
#if !defined(HAVE_PTHREAD_SETAFFINITY_NP) && !defined(NO_DEFAULT_MAPPING)
            int cpuId = filter->getCPUId();
            if (ff_mapThreadToCpu((cpuId<0) ? (cpuId=threadMapper::instance()->getCoreId(tid)) : cpuId)!=0)
                error("Cannot map thread %d to CPU %d, mask is %u,  size is %u,  going on...\n",tid, (cpuId<0) ? threadMapper::instance()->getCoreId(tid) : cpuId, threadMapper::instance()->getMask(), threadMapper::instance()->getCListSize());
            filter->setCPUId(cpuId);
#endif
            gettimeofday(&filter->tstart,NULL);
            return filter->svc_init();
        }

        void svc_end() {
            filter->svc_end();
            gettimeofday(&filter->tstop,NULL);
        }

        int run(bool=false) {
            int CPUId = ff_thread::spawn(filter->getCPUId());
            filter->setCPUId(CPUId);
            return (CPUId==-2)?-1:0;
        }

        inline int  wait() { return ff_thread::wait();}
        inline int  wait_freezing() { return ff_thread::wait_freezing();}
        inline void freeze() { ff_thread::freeze();}
        inline bool isfrozen() const { return ff_thread::isfrozen();}
        inline bool done()     const { return ff_thread::done();}
        inline int  get_my_id() const { return filter->get_my_id(); };

    protected:
#if defined(FF_TASK_CALLBACK)
        void callbackIn(void  *t=NULL) { filter->callbackIn(t);  }
        void callbackOut(void *t=NULL) { filter->callbackOut(t); }
#endif
    protected:
        ff_node * const filter;
    };
    /* ------------------------------------------------------------------------------------- */

    inline void   setCPUId(int id) { CPUId = id;}
    inline void   setThread(ff_thread *const th) { my_own_thread = false; thread = th; }
    inline size_t getTid() const { return thread->getTid();}


protected:

#if defined(TRACE_FASTFLOW)
    size_t        taskcnt;
    ticks         lostpushticks;
    size_t        pushwait;
    ticks         lostpopticks;
    size_t        popwait;
    ticks         ticksmin;
    ticks         ticksmax;
    ticks         tickstot;
#endif

    fftree *fftree_ptr;       //fftree stuff

    // for the input queue
    pthread_mutex_t     cons_m;
    pthread_cond_t      cons_c;
    std::atomic_ulong   cons_counter;

    // for synchronizing with the previous multi-output stage
    pthread_mutex_t    *p_prod_m;
    pthread_cond_t     *p_prod_c;
    std::atomic_ulong  *p_prod_counter;


    // for the output queue
    pthread_mutex_t     prod_m;
    pthread_cond_t      prod_c;
    std::atomic_ulong   prod_counter;

    // for synchronizing with the next multi-input stage
    pthread_mutex_t    *p_cons_m;
    pthread_cond_t     *p_cons_c;
    std::atomic_ulong  *p_cons_counter;
};

/* just a node interface for the input and output buffers */
struct ff_buffernode: ff_node {
    ff_buffernode() {}
    ff_buffernode(int nentries, bool fixedsize=true, int id=-1) {
        set(nentries,fixedsize,id);
    }
    ff_buffernode(int id, FFBUFFER *in, FFBUFFER *out) {
        set_id(id);
        set_input_buffer(in);
        set_output_buffer(out);
    }
    void* svc(void*){return NULL;}
    void set(int nentries, bool fixedsize=true, int id=-1) {
        set_id(id);
        if (create_input_buffer(nentries,fixedsize) < 0) {
            error("FATAL ERROR: ff_buffernode::set: create_input_buffer fails!\n");
            abort();
        }
        set_output_buffer(ff_node::get_in_buffer());
    }

    template<typename T>
    inline bool  put(T *ptr) {
        if (blocking_out) {
            if (ff_node::get_in_buffer()->isFixedSize()) {
                do {
                    if (ff_node::put(ptr)) {
                        pthread_mutex_lock(p_cons_m);
                        if ((*p_cons_counter).load() == 0) {
                            pthread_cond_signal(p_cons_c);
                        }
                        ++(*p_cons_counter);
                        pthread_mutex_unlock(p_cons_m);
                        ++prod_counter;
                        return true;
                    }
                    pthread_mutex_lock(&prod_m);
                    while(prod_counter.load() >= (ff_node::get_in_buffer()->buffersize())) {
                        pthread_cond_wait(&prod_c, &prod_m);
                    }
                    pthread_mutex_unlock(&prod_m);

                } while(1);
            } else {
                ff_node::put(ptr);
                pthread_mutex_lock(p_cons_m);
                if ((*p_cons_counter).load() == 0) {
                    pthread_cond_signal(p_cons_c);
                }
                ++(*p_cons_counter);
                pthread_mutex_unlock(p_cons_m);
                ++prod_counter;
            }
            return true;
        }
        return ff_node::put(ptr);
    }

    virtual inline pthread_mutex_t   &get_prod_m()       { return prod_m;}
    virtual inline pthread_cond_t    &get_prod_c()       { return prod_c;}
    virtual inline std::atomic_ulong &get_prod_counter() { return prod_counter;}
};

/* *************************** Typed node ************************* */

//#ifndef WIN32 //VS12
/*!
 *  \class ff_node_base_t
 *  \ingroup building_blocks
 *
 *  \brief The FastFlow typed abstract contanier for a parallel activity (actor).
 *
 *  Key method is: \p svc (pure virtual).
 *
 *  This class is defined in \ref node.hpp
 */

template<typename IN_t, typename OUT_t = IN_t>
struct ff_node_t: ff_node {
    typedef IN_t  in_type;
    typedef OUT_t out_type;
    ff_node_t():
        GO_ON((OUT_t*)FF_GO_ON),
        EOS((OUT_t*)FF_EOS),
        EOSW((OUT_t*)FF_EOSW),
        GO_OUT((OUT_t*)FF_GO_OUT),
        EOS_NOFREEZE((OUT_t*) FF_EOS_NOFREEZE),
        BLK((OUT_t*)FF_BLK), NBLK((OUT_t*)FF_NBLK) {
	}
    OUT_t * const GO_ON,  *const EOS, *const EOSW, *const GO_OUT, *const EOS_NOFREEZE, *const BLK, *const NBLK;
    virtual ~ff_node_t()  {}
    virtual OUT_t* svc(IN_t*)=0;
    inline  void *svc(void *task) { return svc(reinterpret_cast<IN_t*>(task)); };
};

#if (__cplusplus >= 201103L) || (defined __GXX_EXPERIMENTAL_CXX0X__) || (defined(HAS_CXX11_VARIADIC_TEMPLATES))

/*!
 *  \class ff_node_F
 *  \ingroup building_blocks
 *
 *  \brief The FastFlow typed abstract contanier for a parallel activity (actor).
 *
 *  Creates an ff_node_t from a lambdas, function pointer, etc
 *
 *  This class is defined in \ref node.hpp
 */
template<typename TIN, typename TOUT=TIN,
         typename FUNC=std::function<TOUT*(TIN*,ff_node*const)> >
struct ff_node_F: public ff_node_t<TIN,TOUT> {
   ff_node_F(FUNC f):F(f) {}
   TOUT* svc(TIN* task) { return F(task, this); }
   FUNC F;
};

#endif
//#endif

} // namespace ff

#endif /* FF_NODE_HPP */
