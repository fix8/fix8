/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 *  \link
 *  \file node.hpp 
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 *  \brief This file contains the definition of the \p ff_node class, which
 *  acts as the basic structure of each skeleton on shared-memory architecture.
 *  Other supplementary classes are also defined here.
 *
 */

#ifndef _FF_NODE_HPP_
#define _FF_NODE_HPP_

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
#include <iostream>
#include "ff/platforms/platform.h"
#include <ff/cycle.h>
#include <ff/utils.hpp>
#include <ff/buffer.hpp>
#include <ff/ubuffer.hpp>
#include <ff/mapper.hpp>
#include <ff/config.hpp>

namespace ff {

/*!
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 *  @{
 */

/*!
 *  \class Barrier
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 *  \brief Models a classical mutex
 *
 *  This class models a classical \a Mutex. This class provides the
 *  methods necessary to implement a low-level \p pthread mutex.
 *
 *  This class is defined in \ref node.hpp
 */

class Barrier {
public:
    /**
     * \brief Creates instance of barrier
     *
     * It creates an instance of barrier.
     *
     * \return A pointer to the barrier.
     */
    static inline Barrier * instance() {
        static Barrier b;
        return &b;
    }

    /**
     *  \brief Constructor
     *
     *  It checks whether the mutex variable(s) and the conditional variable(s)
     *  can be properly initialised. In case initialization fails, the program
     *  is aborted.
     *
     */
    Barrier():_barrier(0),threadCounter(0) {
        if (pthread_mutex_init(&bLock,NULL)!=0) {
            error("ERROR: Barrier: pthread_mutex_init fails!\n");
            abort();
        }
        if (pthread_cond_init(&bCond,NULL)!=0) {
            error("ERROR: Barrier: pthread_cond_init fails!\n");
            abort();
        }
    }
    
    /**
     * \brief Setup barrier 
     *
     * It initialize barrier.
     *
     * \parm init determine the barrier
     *
     */
    inline void barrierSetup(int init) {
        if (!_barrier && init>0) _barrier = init;
        return;
    }

    /** 
     * \brief Performs barrier operation
     *
     * It performs the barrier operation and waits on the condition variable.
     * 
     */
    inline void doBarrier(int) {
        pthread_mutex_lock(&bLock);
        if (!--_barrier) pthread_cond_broadcast(&bCond);
        else {
            pthread_cond_wait(&bCond, &bLock);
            assert(_barrier==0);
        }
        pthread_mutex_unlock(&bLock);
    }
   
    /**
     * \brief Get the thread counter
     *
     * It gets the counter of the pthread.
     *
     * \return An integet value, showing the counter of the pthread.
     */
    unsigned getCounter() const { return threadCounter;}

    /**
     * \brief Increments the counter
     *
     * It increaments the counter of the pthread.
     */
    void     incCounter()       { ++threadCounter;}

    /**
     * \brief Decrements the counter
     *
     * It decremetns the counter of the pthread.
     */
    void     decCounter()       { --threadCounter;}

private:
    int _barrier;           
    // _barrier represent the number of threads in the barrier. This is just a
    // counter, and is used to set the ff_node::tid value.
    unsigned threadCounter;
    pthread_mutex_t bLock;  // Mutex variable
    pthread_cond_t  bCond;  // Condition variable
};
 
/*!
 *  \class spinBarrier
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 *  \brief Models a classical \a Mutex.
 *
 *  This class provides the methods necessary to implement a low-level
 *  pthread mutex.
 *
 *  This class is defined in file \ref node.hpp
 *
 */ 
class spinBarrier {
public:
    /**
     * \brief Gets an instance of the spinBarrier
     *
     * It gets a static instance of the spinBarrier object.
     *
     * \return A pointer to the spinBarrier.
     */
    static inline spinBarrier * instance() {
        static spinBarrier b;
        return &b;
    }

    /**
     *  \brief Constructor
     *
     *  It creates an instnce of the spin barrier.
     *
     *  \parm MAX_NUM_THREADS maximum number of threads
     *
     */
    spinBarrier(const int maxNThreads=MAX_NUM_THREADS):_barrier(0),threadCounter(0),maxNThreads(maxNThreads) {
        atomic_long_set(&B[0],0);
        atomic_long_set(&B[1],0);
        barArray=new bool[maxNThreads];
        for(int i=0;i<maxNThreads;++i) barArray[i]=false;
    }

    /**
     * \brief Destructor
     *
     * It deletes the elements in the barrier array.
     *
     */
    ~spinBarrier() {
        if (barArray != NULL) delete [] barArray;
        barArray=NULL;
    }
    
    /**
     *
     * \brief Setsup the barrier 
     *
     * It setup the barrier.
     *
     * \parm init initializes the barrier
     * 
     */
    inline void barrierSetup(int init) {
        if (!_barrier && init>0) _barrier = init; 
        return;
    }

    /**
     * \brief Performs the barrier
     *
     * It performs the barrier.
     *
     * \parm tid is the thread id.
     *
     */
    inline void doBarrier(int tid) {
        assert(tid<maxNThreads);
        const int whichBar = (barArray[tid] ^= true); // computes % 2
        long c = atomic_long_inc_return(&B[whichBar]);
        if (c == _barrier) 
            atomic_long_set(&B[whichBar], 0);
        else
            while(c) { 
                c= atomic_long_read(&B[whichBar]);
                PAUSE();
            }
    }

    /**
     * \brief Gets counter
     *
     * It gets the counter of the thread.
     *
     * \return An integer value showing the counter of the thread.
     */
    unsigned long getCounter() const { return threadCounter;}

    /**
     * \brief Increments counter
     * 
     * It increments the thread counter.
     */
    void          incCounter()       { ++threadCounter;}

    /**
     * \brief Decrements counter
     *
     * It decrements the thread counter.
     */
    void          decCounter()       { --threadCounter;}
    
private:
    long _barrier;
    /* 
     * _barrier represents the number of threads in the barrier. This is just a
     * counter, it is used to set the ff_node::tid value.
     */
    unsigned long threadCounter;
    /* 
     * maximum number of threads
     */
    const long maxNThreads;
    /* 
     * each thread has an entry in the barArray, it is used to 
     * point to the current barrier counter either B[0] or B[1]
     */
    bool* barArray;          
    atomic_long_t B[2];
};

// TODO: Should be rewritten in terms of mapping_utils.hpp 
#if defined(HAVE_PTHREAD_SETAFFINITY_NP) && !defined(NO_DEFAULT_MAPPING)

    /**
     *
     * \brief Initialize thread affinity 
     * It initializes thread affinity i.e. which cpu the thread should be
     * assigned.
     *
     * \parm attr is the pthread attribute
     * \parm cpuID is the identifier the core
     * \return -2  if error, the cpu identifier if successful
     */
static inline int init_thread_affinity(pthread_attr_t*attr, int cpuId) {
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

/**
 * \brief Initializes thread affinity
 *
 * It initializes thread affinity i.e. it defines to which core ths thread
 * should be assigned.
 *
 * \return always return -1 because no thread mapping is done
 */
static inline int init_thread_affinity(pthread_attr_t*,int) {
    // Just ensure that the threadMapper constructor is called.
    threadMapper::instance();
    return -1;
}
#else
/**
 * \brief Initializes thread affinity
 *
 * It initializes thread affinity i.e. it defines to which core ths thread
 * should be assigned.
 *
 * \return always return -1 because no thread mapping is done
 */
static inline int init_thread_affinity(pthread_attr_t*,int) {
    // do nothing
    return -1;
}
#endif /* HAVE_PTHREAD_SETAFFINITY_NP */


// forward decl
/**
 * \brief Proxy thread routine
 *
 */
static void * proxy_thread_routine(void * arg);

/*!
 *  \class ff_thread
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 *  \brief Defines FastFlow's threads
 *
 *  It defines FastFlow's threads. This class manages all the low-level
 *  communications and synchronisations needed among threads. It is responsible
 *  for threads creation and destruction, suspension, freezing and thawing.
 *
 *  This class is defined in \ref node.hpp
 *
 */
class ff_thread {
    /**
     * \brief Proxy thread routine
     *
     */

    friend void * proxy_thread_routine(void *arg);

protected:
    /*! 
     *  \brief Constructor
     *
     *  It checks initializations of mutex and conditional variable and then
     *  create the FastFlow thread.
     *
     *  \param barrier is a Barrier object (i.e a custom Mutex) as input
     *  paramenter.
     */
    ff_thread(BARRIER_T * barrier=NULL):
        tid((unsigned)-1),barrier(barrier),
        stp(true), // only one shot by default
        spawned(false),
        freezing(0), frozen(false),thawed(false),
        init_error(false) {
        
        /* Attr is NULL, default mutex attributes are used. Upon successful
         * initialization, the state of the mutex becomes initialized and
         * unlocked. 
         * */
        if (pthread_mutex_init(&mutex,NULL)!=0) {
            error("ERROR: ff_thread: pthread_mutex_init fails!\n");
            abort();
        }
        if (pthread_cond_init(&cond,NULL)!=0) {
            error("ERROR: ff_thread: pthread_cond_init fails!\n");
            abort();
        }
        if (pthread_cond_init(&cond_frozen,NULL)!=0) {
            error("ERROR: ff_thread: pthread_cond_init fails!\n");
            abort();
        }
    }

    /**
     * \brief Destructor
     *
     */
    virtual ~ff_thread() {}
    
    /**
     * \brief The life cycle of FastFlow thread
     *
     * It defines the life cycle of the FastFlow thread. 
     *
     */
    void thread_routine() {
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
                svc_releaseOCL();
            }
            svc_end();
            
            if (disable_cancelability()) {
                error("ff_thread, thread_routine, could not change thread cancelability");
                return;
            }

            // acquire lock. While freezing is true,
            // freeze and wait. 
            pthread_mutex_lock(&mutex);
            if (ret != (void*)FF_EOS_NOFREEZE && !stp) {  // <-------------- CONTROLLARE aggiunto !stp
                while(freezing==1) { // NOTE: freezing can change to 2
                    frozen=true; 
                    pthread_cond_signal(&cond_frozen);
                    pthread_cond_wait(&cond,&mutex);
                }
            }
            
            thawed=true;     // <----------------- CONTROLLARE !!!! era thawed = frozen;
            pthread_cond_signal(&cond);
            frozen=false; 
            if (freezing != 0) freezing = 1; // freeze again next time 
            pthread_mutex_unlock(&mutex);

            if (enable_cancelability()) {
                error("ff_thread, thread_routine, could not change thread cancelability");
                return;
            }
        } while(!stp);

        if (init_error && freezing) {
            pthread_mutex_lock(&mutex);
            frozen=true;
            pthread_cond_signal(&cond_frozen);
            pthread_mutex_unlock(&mutex);
        }
    }

    /**
     *
     * \brief Disable the cancelation of thread
     *
     * It makes thread to be not cancelable. If a cancellation request is received, it is
     * blocked until cancelability is enabled.
     *
     * \return 0 if successful, otherwise a negative value.
     */
    int disable_cancelability()
    {
        if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancelstate)) {
            perror("pthread_setcanceltype");
            return -1;
        }
        return 0;
    }

    /**
     * \brief Makes thread cancelable
     *
     * It makes thread cancelable.
     *
     * \return 0 if successful, otherwise a negative value.
     */
    int enable_cancelability()
    {
        if (pthread_setcancelstate(old_cancelstate, 0)) {
            perror("pthread_setcanceltype");
            return -1;
        }
        return 0;
    }
    
public:
    /** 
     *
     * \brief the \p svc method
     *
     * It defines the \p svc method as pure virtual function.
     */
    virtual void* svc(void * task) = 0;

    /**
     * \brief The \p svc_init method
     *
     * It defines the virtual (overridable) function.
     *
     * \return 0 is returned always.
     */
    virtual int   svc_init() { return 0; };
    
    /**
     *
     * \brief The svc_end method
     *
     * It defines the virtual (overridable) function.
     */
    virtual void  svc_end()  {}

    /**
     * \brief The \p svc_createOCL method
     *
     * It defines the virtual function.
     */
    virtual void  svc_createOCL()  {}

    /**
     * \brief The \p svc_releaseOCL
     *
     * It defines teh virtual function.
     */
    virtual void  svc_releaseOCL() {}

    /**
     * \brief Sets the barrier object
     *
     * It sets the Barrier object for the ff_thread.
     *
     * \param b is a barrier object.
     *
     */
    void set_barrier(BARRIER_T * const b) { barrier=b;}

    /**
     * \brief Creates a new thread
     *
     * It create a new FastFlow thread
     *
     * \parm cpuID is the identifier of the core.
     *
     * \return the -2 in case of error, 
     *             -1 in case the thread is not assigned to any CPU
     *             CPU-id if successful
     */
    int spawn(int cpuId=-1) {
        if (spawned) return -1;

        if (pthread_attr_init(&attr)) {
                perror("pthread_attr_init: pthread can not be created.");
                return -1;
        }

        int CPUId = init_thread_affinity(&attr, cpuId);
        if (CPUId==-2) return -2;
        if (barrier) tid=barrier->getCounter();
        if (pthread_create(&th_handle, &attr,
                           proxy_thread_routine, this) != 0) {
            perror("pthread_create: pthread creation failed.");
            return -2;
        }
        if (barrier) barrier->incCounter();
        spawned = true;
        return CPUId;
    }
   
    /**
     * \brief Waits for thread termination
     *
     * It defines the wait for thread termination.
     * 
     * \return 0 if successful.
     */
    int wait() {
        stp=true;
        if (isfrozen()) {
            wait_freezing();
            thaw();
        }
        if (spawned) {
            pthread_join(th_handle, NULL);
            if (barrier) barrier->decCounter();
        }
        if (pthread_attr_destroy(&attr)) {
            error("ERROR: ff_thread.wait: pthread_attr_destroy fails!");
        }
        spawned=false;
        return 0;
    }

    /**
     * \brief Waits for thread freezing
     *
     * It waits for the thread to freeze (uses Mutex).
     *
     * \return 0 if successful, otherwise a negative value.
     */
    int wait_freezing() {
        pthread_mutex_lock(&mutex);
        while(!frozen) pthread_cond_wait(&cond_frozen,&mutex);
        pthread_mutex_unlock(&mutex);
        return (init_error?-1:0);
    }

    /**
     * \brief Forces the thread to stop
     *
     * It forces the thread to stop at next EOS or next thawing.
     *
     */
    void stop() { stp = true; };

    /**
     * \brief Forces the thread to freeze
     *
     * It forces the thread to freeze himself.
     */
    void freeze() {  
        stp=false;
        freezing = 1;
    }
    
    /**
     * \brief Thaw the thread, if it is frozen
     *
     * If the thread is frozen, then thaw it. 
     */
    void thaw(bool _freeze=false) {
        pthread_mutex_lock(&mutex);
        if (_freeze) freezing=2; // next time freeze again the thread
        else freezing=0;
        assert(thawed==false);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        while(!thawed) pthread_cond_wait(&cond, &mutex);
        thawed=false;
        pthread_mutex_unlock(&mutex);
    }

    /**
     *
     * \brief Checks if the thread is frozen
     *
     * It checks whether the thread is frozen.
     *
     * \return A booleon value shoing the status of freezing.
     */
    bool isfrozen() { return freezing>0;} 

    /**
     *
     * \brief Gets the pthread handler
     *
     * It retrives the pthread handler.
     *
     * \return The handle for pthread.
     */
    pthread_t get_handle() const { return th_handle;}

    /**
     *  \brief Gets the internal unique thread identifier.
     *
     */
    inline int getTid() const { return tid; }

protected:
    unsigned        tid;
private:
    BARRIER_T    *  barrier;            /// A \p Barrier object
    bool            stp;
    bool            spawned;
    int             freezing;   // changed from bool to int
    bool            frozen;
    bool            thawed;
    bool            init_error;
    pthread_t       th_handle;
    pthread_attr_t  attr;
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
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 *  \brief Describes the FastFlow node
 *
 *  This class describes the \p ff_node, which is the basic building
 *  block of every skeleton in stream programming. It is used to encapsulate
 *  sequential portions of code implementing functions. \p ff_node defines 3
 *  methods; two optional and one mandatory. The optional methods are (1) \p
 *  svc_init and (2) \p svc_end. The mandatory method is \p svc (pure virtual
 *  method). The \p svc_init method is called once at node initialization,
 *  while the \p svn_end method is called once when the end-of-stream (EOS) is
 *  received in input or when the \p svc method returns \p NULL. the \p svc
 *  method is called each time an input task is ready to be procedded.
 *
 *  This class is defined in \ref node.hpp
 */

class ff_node {
private:

    template <typename lb_t, typename gt_t> 
    friend class ff_farm;
    friend class ff_pipeline;
    friend class ff_loadbalancer;
    friend class ff_gatherer;

protected:
    /**
     * \brief Sets the node identifer
     *
     * It sets node identifier.
     *
     * \parm id is the identifier of the node.
     */
    void set_id(int id) { myid = id;}

    /**
     *  \brief Pushes to output buffer
     *
     *  It is a virtual method. It pushes data into the output buffer.
     *
     *  \param ptr pointer to the data to be pushed out.
     *
     *  \return Boolean value showing the status of push operation.
     */
    virtual inline bool push(void * ptr) { return out->push(ptr); }
    
    /**
     *  \brief Pops the data from input buffer
     *
     *  It is a virtual method. It pops data from the input buffer.
     *
     *  \param ptr pointer to the location where the data is located
     *
     *  \return A boolean value showing the status of the pop operation.
     */
    virtual inline bool pop(void ** ptr) { 
        if (!in_active) return false; // it does not want to receive data
        return in->pop(ptr);
    }

    /**
     * \brief Skips the first element popped
     *
     * It skips the first element popped.
     *
     * \parm sk Boolean value showing if the first element should be skipped.
     */
    virtual inline void skipfirstpop(bool sk)   { skip1pop=sk;}

    /**
     * \brief Gets the status of \p skipfirstpop
     *
     * It returns the status of \p ski1pop 
     * 
     * \return A booleon value showing the status of \p ski1pop
     */
    bool skipfirstpop() const { return skip1pop; }
    
    /** 
     * \brief Creates the input buffer
     *
     *  It create an input buffer for the \p ff_node. 
     *
     *  \param nentries the size of the buffer
     *  \param fixedsize flag to decide whether the buffer is resizable.
     *  Default is \p true
     *
     *  \return 0 if successful, otherwise a negative value.
     */
    virtual int create_input_buffer(int nentries, bool fixedsize=true) {
        if (in) return -1;
        in = new FFBUFFER(nentries,fixedsize);        
        if (!in) return -1;
        myinbuffer=true;
        return (in->init()?0:-1);
    }
    
    /** 
     *  \brief Creates the output buffer
     *
     *  It creates an output buffer for the \p ff_node. 
     *
     *  \param nentries the size of the buffer 
     *  \param fixedsize flag to decide whether the buffer is resizable.
     *  Default is \p false .
     *
     *  \return 0 if successful, otherwise a negative value.
     */
    virtual int create_output_buffer(int nentries, bool fixedsize=false) {
        if (out) return -1;
        out = new FFBUFFER(nentries,fixedsize); 
        if (!out) return -1;
        myoutbuffer=true;
        return (out->init()?0:-1);
    }

    /** 
     *  \brief Sets the output buffer
     *
     *  It sets the output buffer for the ff_node.
     *
     *  \param o a buffer object of type \p FFBUFFER
     *
     *  \return 0 if successful, otherwise a negative value is returned.
     */
    virtual int set_output_buffer(FFBUFFER * const o) {
        if (myoutbuffer) return -1;
        out = o;
        return 0;
    }

    /** 
     *  \brief Sets the input buffer
     *
     *  It sets the input buffer for the ff_node.
     *
     *  \param i a buffer object of type \p FFBUFFER
     *
     *  \return 0 if successful otherwise a negative value is returned.
     */
    virtual int set_input_buffer(FFBUFFER * const i) {
        if (myinbuffer) return -1;
        in = i;
        return 0;
    }

    /**
     * \brief Executes the FastFlow thread
     *
     * It executes the FastFlow thread.
     *
     * \return The state of the run method if successful, otherwise a negative
     * value is returned.
     */
    virtual int run(bool=false) { 
        thread = new thWorker(this);
        if (!thread) return -1;
        return thread->run();
    }
    
    /**
     * \brief Freezes the thread and then run
     *
     * It freezes the thread and then run.
     *
     * \return It returns the state of the run method if successful, otherwise
     * a negative value is returned.
     */
    virtual int freeze_and_run(bool=false) {
        thread = new thWorker(this);
        if (!thread) return -1;
        freeze();
        return thread->run();
    }

    /**
     * \brief Waits for thread termination
     *
     * It lets the thread to wait until termination.
     *
     * \return If successful the state of the wait method otherwise a negative
     * value.
     */
    virtual int  wait() { 
        if (!thread) return -1;
        return thread->wait(); 
    }
    
    /**
     * \brief Waits for thread to thaw
     *
     * It wait for thread to thaw.
     *
     * \return If successful the state of the \p wait_freezing method, otherwise a
     * negative value.
     */
    virtual int  wait_freezing() { 
        if (!thread) return -1;
        return thread->wait_freezing(); 
    }
    
    /**
     * \brief Stops the thread
     *
     * It stops thread.
     *
     * \return If the thread does not exist, then the method is returned.
     */
    virtual void stop() {
        if (!thread) return; 
        thread->stop(); 
    }
    
    /**
     * \brief Freezes the thread
     *
     * It freezes the thread.
     *
     * \return If thread does not exist, then the method returns.
     */
    virtual void freeze() { 
        if (!thread) return; 
        thread->freeze(); 
    }
    
    /**
     * \brief Checks to thaw a thread
     *
     * It checks if the thread is created then thaw it.
     *
     * \return If the thread does not exist then the method is returned,
     * otherwise the thread is thawed.
     */
    virtual void thaw(bool _freeze=false) { 
        if (!thread) return; 
        thread->thaw(_freeze);
    }
    
    /**
     * \brief Checks if the thread is frozen
     *
     * It checks whether the thread is frozen.
     *
     * \return If the thread does not exists then false is returned, otherwise
     * the status of \p isfrozen() is returned.
     */
    virtual bool isfrozen() { 
        if (!thread) 
            return false;
        return thread->isfrozen();
    }
    
    /**
     * \brief Counts how many threads there are in this node.
     *
     * Counts the n. of threads that should block in the barrier.
     *
     * \parm b is the barrier.
     *
     * \return always 1
     */
    virtual int  cardinality(BARRIER_T * const b) { 
        barrier = b;
        return 1;
    }
    
    /**
     * \brief Sets the barrier
     *
     * It sets the barrier.
     *
     * \parmm b is the barrier.
     */
    virtual void set_barrier(BARRIER_T * const b) {
        barrier = b;
    }

    /**
     * \brief Gets the time
     *
     * It retrives the tim spent in FastFlow pattern.
     *
     * \return The difference in the time spent.
     */
    virtual double ffTime() {
        return diffmsec(tstop,tstart);
    }

    /**
     * \brief Gets the time
     *
     * It retrieves the time spend in FastFlow skeleton.
     *
     * \return The difference in the time spent.
     */
    virtual double wffTime() {
        return diffmsec(wtstop,wtstart);
    }

public:
    
    /**
     * Defines the number of ticks to wait.
     */
    enum {TICKS2WAIT=1000};

    /**
     * \brief The \p svc method
     *
     * It is a pure virtual function. If \p svc returns a NULL value then
     * End-Of-Stream (EOS) is produced on the output channel.
     *
     * \parm task is the input data stream.
     */
    virtual void* svc(void * task) = 0;
    
    /**
     * \brief The \p svc_init()
     *
     * It is a virtual function. This is called only once for each
     * thread, right before the \p svc method.
     *
     * \return 0 is always returned.
     */
    virtual int svc_init() { return 0; }
    
    /**
     * \brief The \p svc_end()
     *
     * It is a virtual function. This is called only once for each
     * thread, right after the \p svc method.
     */
    virtual void  svc_end() {}

    /**
     * \brief Notifies EOS
     *
     * It is a virtual function. It is called once just after the EOS
     * has arrived.
     *
     * \param id is the ID of the input channel, it makes sense only for N-to-1
     * input channels.
     */
    virtual void eosnotify(int id=-1) {}
    
    /**
     * \brief Gets the ID of the ff_node 
     *
     * \return The ID of the ff_node
     */
    virtual int get_my_id() const { return myid; };
    
    /**
     * \brief Sets threads affinity 
     *
     * It is a virtual function. It maps the working thread to the
     * chosen CPU.
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
     * \brief Gets the CPU id (if set) of this node is pinned
     *
     * It gets the ID of the CPU where the thread is running.
     *
     * \return The identifier of the CPU.
     */
    virtual int getCPUId() const { return CPUId; }

#if defined(TEST_QUEUE_SPIN_LOCK)
    /**
     * \brief Pushes the task
     *
     * It tries to acquire the lock, pushes the task to the input queue and
     * then releaes the lock
     *
     * \parm ptr is pointing to the task
     *
     * \return The status of \p push operation as Boolean value
     */
    virtual bool put(void * ptr) { 
        spin_lock(lock);
        bool r= in->push(ptr);
        spin_unlock(lock);
        return r;
    }

    /**
     * \brief Pops the task
     *
     * It tries to acquire the lock, pop the data from the output queue and
     * then releases the lock. 
     *
     * \parm ptr is pointing to the task
     *
     * \return The status of \p pop operation as Boolean value
     */
    virtual bool  get(void **ptr) { 
        spin_lock(lock);
        register bool r = out->pop(ptr);
        spin_unlock(lock);
        return r;
    }
#else
    /**
     * \brief Pushes the task
     *
     * It pushes task without using locks.
     *
     * \parm ptr is a pointer to the task
     *
     * \return The status of \p push as Boolean value
     */
    virtual inline bool  put(void * ptr) { 
        return in->push(ptr);
    }
    
    /**
     * \brief Pops the task
     *
     * It pops the task without acquiring the lock.
     *
     * \parm ptr is a pointer to the task
     *
     * \return The status of \p pop operation as Boolean value
     */
    virtual inline bool  get(void **ptr) { return out->pop(ptr);}
#endif

    /**
     * \brief Gets input buffer
     *
     * It returns a pointer to the input buffer.
     *
     * \return A pointer to the input buffer
     */
    virtual FFBUFFER * get_in_buffer() const { return in;}

    /**
     * \brief Gets pointer to the output buffer
     *
     * It returns a pointer to the output buffer.
     *
     * \return A pointer to the output buffer.
     */
    virtual FFBUFFER * get_out_buffer() const { return out;}

    /**
     * \brief Gets start time
     *
     * It resturns the starting time.
     *
     * \return Starting time.
     */
    virtual const struct timeval getstarttime() const { return tstart;}

    /**
     * \brief Gets stop time
     *
     * It returns the stopping time.
     *
     * \return Stopping time.
     */
    virtual const struct timeval getstoptime()  const { return tstop;}

    /**
     * \brief Gets start time
     *
     * It returns the starting time.
     *
     * \return Starting time.
     */
    virtual const struct timeval getwstartime() const { return wtstart;}

    /**
     * \brief Gets the stop time
     *
     * It returns the stopping time.
     *
     * \return Stopping time.
     */
    virtual const struct timeval getwstoptime() const { return wtstop;}    

    /**
     * \brief Create OCL
     *
     * It creates OCL. FIXME: How it creates?
     */
    virtual inline void svc_createOCL()  {} 

    /**
     * \brief Releases OCL
     *
     * It releases OCL. FIXME: How it releases?
     */
    virtual inline void svc_releaseOCL() {}

#if defined(TRACE_FASTFLOW)
    /**
     * \brief Prints the trace
     *
     * It prints the trace for debugging.
     */
    virtual void ffStats(std::ostream & out) {
        out << "ID: " << get_my_id()
            << "  work-time (ms): " << wttime    << "\n"
            << "  n. tasks      : " << taskcnt   << "\n"
            << "  svc ticks     : " << tickstot  << " (min= " << ticksmin << " max= " << ticksmax << ")\n"
            << "  n. push lost  : " << pushwait  << " (ticks=" << lostpushticks << ")" << "\n"
            << "  n. pop lost   : " << popwait   << " (ticks=" << lostpopticks  << ")" << "\n";
    }
#endif

    /** \brief Resets input/output queues.
     * 
     *  Warning resetting queues while the node is running may produce to unexpected results.
     */
    void reset() {
        if (in)  in->reset();
        if (out) out->reset();
    }

protected:
    /**
     * \brief Protected constructor
     *
     * It is the protected constructor. It initializes the FastFlow node.
     */
    ff_node():in(0),out(0),myid(-1),CPUId(-1),
              myoutbuffer(false),myinbuffer(false),
              skip1pop(false), in_active(true), thread(NULL),callback(NULL),barrier(NULL) {
        time_setzero(tstart);time_setzero(tstop);
        time_setzero(wtstart);time_setzero(wtstop);
        wttime=0;
        FFTRACE(taskcnt=0;lostpushticks=0;pushwait=0;lostpopticks=0;popwait=0;ticksmin=(ticks)-1;ticksmax=0;tickstot=0);
#if defined(TEST_QUEUE_SPIN_LOCK)
        init_unlocked(lock);
#endif
    };
    
    /** 
     *  \brief Destructor
     *
     *  It is a desctructor and it deletes input buffers and output buffers and
     *  the working thread.
     */
    virtual  ~ff_node() {
        if (in && myinbuffer) delete in;
        if (out && myoutbuffer) delete out;
        if (thread) delete thread;
    };
    
    /**
     * \brief Sends out the task
     *
     * It allows to queue tasks without returning from the \p svc method 
     *
     * \parm task a pointer to the task
     * \parm retry number of tries to push the task to the buffer
     * \parm ticks number of ticks to wait
     * 
     * \return If call back is defined, then returns the status of \p callback
     * function. Otherwise, if tries to push an element with the number of
     * \retry and if succesfful \p true is returned, and if after the number of
     * \retry the push is not successful \p false is returned.
     */
    virtual bool ff_send_out(void * task, 
                             unsigned int retry=((unsigned int)-1),
                             unsigned int ticks=(TICKS2WAIT)) { 
        if (callback) return  callback(task,retry,ticks,callback_arg);

        for(unsigned int i=0;i<retry;++i) {
            if (push(task)) return true;
            ticks_wait(ticks);
        }     
        return false;
    }
    
    virtual inline void input_active(const bool onoff) {
        if (in_active != onoff)
            in_active= onoff;
    }

private:
    
    /**
     * \brief Registers the call back
     *
     * It registers the call back method and arguments.
     *
     * \parm cb is the callback function
     * \parm arg is a pointer to arguments
     */
    void registerCallback(bool (*cb)(void *,unsigned int,unsigned int,void *), void * arg) {
        callback=cb;
        callback_arg=arg;
    }

    /**
     * Sets the CPU id for this node (used only in the run-time)  
     *
     */
    void  setCPUId(int id) { CPUId = id;}

    /**
     *  Returns the internal thread identifier.
     *
     */
    inline int   getTid() const { return thread->getTid();} 
    
    /**
     * \brief An inner class for FastFlow's thread
     *
     * It is an inner class that wraps ff_thread functions and adds \p push and \p pop methods. 
     */
    class thWorker: public ff_thread {
    public:
        /**
         * \brief Constructor
         *
         * It is contructor to create FastFlow thread.
         *
         * \parm filter is a pointer to FastFlow's node
         *
         */
        thWorker(ff_node * const filter):
            ff_thread(filter->barrier),filter(filter) {}
        
        /**
         * \brief Pushes the task
         *
         * It pushes the task to the \p filter
         *
         * \parm task is a pointer to the task.
         *
         * \return \p true is always returned.
         */
        inline bool push(void * task) {
            //register int cnt = 0;
            /* NOTE: filter->push and not buffer->push because of the filter can be a dnode
             */
            while (! filter->push(task)) { 
                // if (ch->thxcore>1) {
                // if (++cnt>PUSH_POP_CNT) { sched_yield(); cnt=0;}
                //    else ticks_wait(TICKS2WAIT);
                //} else 

                FFTRACE(filter->lostpushticks+=ff_node::TICKS2WAIT; ++filter->pushwait);
                ticks_wait(ff_node::TICKS2WAIT);
            }
            return true;
        }
        
        /**
         * \brief Pops the task
         *
         * It pops the task from the buffer of the filter.
         *
         * \task is a pointer to the task
         *
         * \return \p true is always returned.
         */
        inline bool pop(void ** task) {
            //register int cnt = 0;
            /* 
             * NOTE: filter->pop and not buffer->pop because of the filter can be a dnode
             */
            while (! filter->pop(task)) {
                if (!filter->in_active) { *task=NULL; return false;}

                //if (ch->thxcore>1) {
                //if (++cnt>PUSH_POP_CNT) { sched_yield(); cnt=0;}
                //else ticks_wait(TICKS2WAIT);
                //} else 

                FFTRACE(filter->lostpopticks+=ff_node::TICKS2WAIT; ++filter->popwait);
                ticks_wait(ff_node::TICKS2WAIT);
            } 
            return true;
        }
        
        /**
         * \brief Puts the task in buffer
         *
         * It pushes the task in the filter.
         *
         * \parm ptr is a pointer to the task.
         */
        inline bool put(void * ptr) { return filter->put(ptr);}

        /**
         * \brief Gets the task from the buffer.
         *
         * It gets the task from the buffer.
         *
         * \return The status of \p get function as Boolean value.
         */
        inline bool get(void **ptr) { return filter->get(ptr);}

        /**
         * \brief Creates OCL
         *
         * It creates OCL.
         */
        inline void svc_createOCL()  { filter-> svc_createOCL();}
        
        /**
         * \brief Releases OCL.
         *
         * It releases OCL.
         */
        inline void svc_releaseOCL() { filter-> svc_releaseOCL();}

        
        /**
         * \brief The \p svc method
         *
         * It defines the \p svc method of the FastFlow.
         */
        void* svc(void * ) {
            void * task = NULL;
            void * ret = (void*)FF_EOS;
            bool inpresent  = (filter->get_in_buffer() != NULL);
            bool outpresent = (filter->get_out_buffer() != NULL);
            bool skipfirstpop = filter->skipfirstpop(); 
            bool exit=false;            

            gettimeofday(&filter->wtstart,NULL);
            do {
                svc_createOCL();

                if (inpresent) {
                    if (!skipfirstpop) pop(&task); 
                    else skipfirstpop=false;
                    if ((task == (void*)FF_EOS) || 
                        (task == (void*)FF_EOS_NOFREEZE)) {
                        ret = task;
                        filter->eosnotify();
                        if (outpresent)  push(task);
                        break;
                    }
                }
                FFTRACE(++filter->taskcnt);
                FFTRACE(register ticks t0 = getticks());

                ret = filter->svc(task);

#if defined(TRACE_FASTFLOW)
                register ticks diff=(getticks()-t0);
                filter->tickstot +=diff;
                filter->ticksmin=(std::min)(filter->ticksmin,diff); // (std::min) for win portability)
                filter->ticksmax=(std::max)(filter->ticksmax,diff);
#endif                
                if (!ret || (ret == (void*)FF_EOS) || (ret == (void*)FF_EOS_NOFREEZE)) {
                    // NOTE: The EOS is gonna be produced in the output queue
                    // and the thread exits even if there might be some tasks
                    // in the input queue !!!
                    if (!ret) ret = (void *)FF_EOS;
                    exit=true;
                }
                if (outpresent && (ret != GO_ON)) push(ret);
            } while(!exit);
            
            gettimeofday(&filter->wtstop,NULL);
            filter->wttime+=diffmsec(filter->wtstop,filter->wtstart);
            
            return ret;
        }
        
        /**
         * \brief The \p svc_init method
         *
         * It defines the \p svc_init method of FastFlow.
         *
         * \return The status of \p svc_init() method.
         */
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
        
        /**
         * \brief The \p svc_end method
         *
         * It defines the \p svc_end method of the FastFlow.
         */
        virtual void svc_end() {
            filter->svc_end();
            gettimeofday(&filter->tstop,NULL);
        }
        
        /**
         * \brief Executes FastFlow thread
         *
         * It executs the FastFlow thread.
         *
         * \return The status of \p spawn method.
         */
        int run() { 
            int CPUId = ff_thread::spawn(filter->getCPUId());             
            filter->setCPUId(CPUId);
            return (CPUId==-2)?-1:0;
        }

        /**
         * \brief Waits the thread
         *
         * It waits for the thread.
         *
         * \return The staus of \p wait() method.
         */
        int wait() { return ff_thread::wait();}

        /**
         * \brief Waits for freezing
         *
         * It waits the thread to freeze.
         *
         * \return The status of \p wait_freezing() method.
         */
        int wait_freezing() { return ff_thread::wait_freezing();}

        /**
         * \brief Freezes the thread
         *
         * It freezes the thread.
         */
        void freeze() { ff_thread::freeze();}

        /**
         * \brief Checks if the thread is frozen
         * 
         * It checks if the thread is frozen.
         *
         * \return the status of the \p isfrozen() method.
         */
        bool isfrozen() { return ff_thread::isfrozen();}

        /**
         * \brief Gets the ID of the thread
         *
         * It returns the ID of the thread.
         *
         * \return An integer value showing the identifier of the thread.
         */
        int get_my_id() const { return filter->get_my_id(); };
        
    protected:    
        ff_node * const filter;
    };

private:
    FFBUFFER        * in;           ///< Input buffer, built upon SWSR lock-free (wait-free) 
                                    ///< (un)bounded FIFO queue                                 
    FFBUFFER        * out;          ///< Output buffer, built upon SWSR lock-free (wait-free) 
                                    ///< (un)bounded FIFO queue 
    int               myid;         ///< This is the node id, it is valid only for farm's workers
    int               CPUId;    
    bool              myoutbuffer;
    bool              myinbuffer;
    bool              skip1pop;
    bool              in_active;    // allows to disable/enable input tasks receiving   
    thWorker        * thread;       /// A \p thWorker object, which extends the \p ff_thread class 
    bool (*callback)(void *,unsigned int,unsigned int, void *);
    void            * callback_arg;
    BARRIER_T       * barrier;      /// A \p Barrier object
    struct timeval tstart;
    struct timeval tstop;
    struct timeval wtstart;
    struct timeval wtstop;
    double wttime;

#if defined(TEST_QUEUE_SPIN_LOCK)
    lock_t lock;
#endif

#if defined(TRACE_FASTFLOW)
    unsigned long taskcnt;
    ticks         lostpushticks;
    unsigned long pushwait;
    ticks         lostpopticks;
    unsigned long popwait;
    ticks         ticksmin;
    ticks         ticksmax;
    ticks         tickstot;
#endif
};

/*!
 *  @}
 *  \endlink
 */

} // namespace ff

#endif /* _FF_NODE_HPP_ */
