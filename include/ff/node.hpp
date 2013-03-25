/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 *  \file node.hpp
 *  \brief This file contains the definition of the \p ff_node class, which acts as the basic 
 *  structure of each skeleton. Other supplementary classes are defined here.
 */

#ifndef _FF_NODE_HPP_
#define _FF_NODE_HPP_
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

/*
 *  \ingroup runtime
 *
 *  @{
 */

/*
 *  \class Barrier
 *
 *  \brief This class models a classical \a Mutex 
 *
 *  This class provides the methods necessary to implement a low-level \p pthread 
 *  mutex.
 */
class Barrier {
public:
    static inline Barrier * instance() {
        static Barrier b;
        return &b;
    }

    /*
     *  Default Constructor.
     *
     *  Checks whether the mutex variable(s) and the conditional variable(s) can 
     *  be propetly initialised
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
    
    inline void barrierSetup(int init) {        
        if (!_barrier && init>0) _barrier = init; 
        return;
    }

    /* Performs the barrier and waits on the condition variable */
    inline void doBarrier(int) {                
        pthread_mutex_lock(&bLock);
        if (!--_barrier) pthread_cond_broadcast(&bCond);
        else {
            pthread_cond_wait(&bCond, &bLock);
            assert(_barrier==0);
        }
        pthread_mutex_unlock(&bLock);
    }
   
    unsigned getCounter() const { return threadCounter;}
    void     incCounter()       { ++threadCounter;}
    void     decCounter()       { --threadCounter;}

private:
    int _barrier;           // num threads in the barrier
    // this is just a counter, it is used to set the ff_node::tid value.
    unsigned threadCounter;
    pthread_mutex_t bLock;  // Mutex variable
    pthread_cond_t  bCond;  // Condition variable
};

/*!
 *  @}
 */

/*
 *  \ingroup runtime
 *
 *  @{
 */
 
/*
 *  \class spinBarrier
 *
 *  \brief This class models a classical \a Mutex 
 *
 *  This class provides the methods necessary to implement a low-level \p pthread 
 *  mutex.
 */ 
class spinBarrier {
public:
    /* Get a static instance of the spinBarrier object */
    static inline spinBarrier * instance() {
        static spinBarrier b;
        return &b;
    }

    /*
     *  Default Constructor.
     *
     */
    spinBarrier(const int maxNThreads=MAX_NUM_THREADS):_barrier(0),threadCounter(0),maxNThreads(maxNThreads) {
        atomic_long_set(&B[0],0);
        atomic_long_set(&B[1],0);
        barArray=new bool[maxNThreads];
        for(int i=0;i<maxNThreads;++i) barArray[i]=false;
    }

    ~spinBarrier() {
        if (barArray != NULL) delete [] barArray;
        barArray=NULL;
    }
    
    inline void barrierSetup(int init) {        
        if (!_barrier && init>0) _barrier = init; 
        return;
    }

    /* Performs the barrier */
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

    unsigned long getCounter() const { return threadCounter;}
    void          incCounter()       { ++threadCounter;}
    void          decCounter()       { --threadCounter;}
    
private:
    long _barrier;          // num threads in the barrier    
    // this is just a counter, it is used to set the ff_node::tid value.
    unsigned long threadCounter;
    const long maxNThreads; // max number of threads
    // each thread has an entry in the barArray, it is used to 
    // point to the current barrier counter either B[0] or B[1]
    bool* barArray;          
    atomic_long_t B[2];   // barrier counter 
};

/*!
 *  @}
 */
    

// TODO: Should be rewritten in terms of mapping_utils.hpp 
#if defined(HAVE_PTHREAD_SETAFFINITY_NP) && !defined(NO_DEFAULT_MAPPING)
static inline void init_thread_affinity(pthread_attr_t*attr, int cpuId) {
    /*
    int ret;
    if (cpuId<0)
        ret = ff_mapThreadToCpu(threadMapper::instance()->getCoreId());
    else
        ret = ff_mapThreadToCpu(cpuId);
    if (ret==EINVAL) std::cerr << "ff_mapThreadToCpu failed\n";
    */
    
    cpu_set_t cpuset;    
    CPU_ZERO(&cpuset);

    if (cpuId<0)
        CPU_SET (threadMapper::instance()->getCoreId(), &cpuset);
    else 
        CPU_SET (cpuId, &cpuset);

    if (pthread_attr_setaffinity_np (attr, sizeof(cpuset), &cpuset)<0) {
        perror("pthread_attr_setaffinity_np");
    }
    
}
#else
static inline void init_thread_affinity(pthread_attr_t*,int) {}
#endif /* HAVE_PTHREAD_SETAFFINITY_NP */

/*
 * @}
 */



// forward decl
static void * proxy_thread_routine(void * arg);

/*!
 *  \ingroup runtime
 *
 *  @{
 */

/*!
 *  \class ff_thread
 *
 *  \brief FastFlow's  threads. 
 *
 *  This class manages all the low-level communications and 
 *  synchronisations needed among threads. It is responsible for threads creation and 
 *  destruction, suspension, freezing and thawing.
 */
class ff_thread {
    friend void * proxy_thread_routine(void *arg);

protected:
    /*! 
     *  Constructor 
     *
     *  \param barrier a Barrier object (i.e a custom Mutex) as input paramenter.
     */
    ff_thread(BARRIER_T * barrier=NULL):
        tid((unsigned)-1),barrier(barrier),
        stp(true), // only one shot by default
        spawned(false),
        freezing(false), frozen(false),thawed(false),
        init_error(false) {
        
        /* Attr is NULL, default mutex attributes are used. Upon successful initialization, 
         * the state of the mutex becomes initialized and unlocked. */
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

    /// Default destructor
    virtual ~ff_thread() {
        // MarcoA 27/04/12: Moved to wait
        /*
        if (pthread_attr_destroy(&attr)) {
            error("ERROR: ~ff_thread: pthread_attr_destroy fails!");
        }
        */
    }
    
    /** Each thread's life cycle  */
    void thread_routine() {
        if (barrier) barrier->doBarrier(tid);
        void * ret;
        do {
            init_error=false;
            if (svc_init()<0) {
                error("ff_thread, svc_init failed, thread exit!!!\n");
                init_error=true;
                break;
            } else  ret = svc(NULL);
            svc_end();
            
            if (disable_cancelability()) {
                error("ff_thread, thread_routine, could not change thread cancelability");
                return;
            }

            // acquire lock. While freezing is true,
            // freeze and wait. 
            pthread_mutex_lock(&mutex);
            if (ret != (void*)FF_EOS_NOFREEZE) {
                while(freezing) {
                    frozen=true; 
                    pthread_cond_signal(&cond_frozen);
                    pthread_cond_wait(&cond,&mutex);
                }
            }

            thawed=frozen;
            pthread_cond_signal(&cond);
            frozen=false;            
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
     * Make thread not cancelable. If a cancellation request is received,
     * it is blocked until cancelability is enabled.
     */
    int disable_cancelability()
    {
        if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancelstate)) {
            perror("pthread_setcanceltype");
            return -1;
        }
        return 0;
    }

    /// Make thread cancelable.
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
     * Pure virtual function.
     */
    virtual void* svc(void * task) = 0;
    /// Virtual (overridable) function.
    virtual int   svc_init() { return 0; };
    /// Virtual (overridable) function.
    virtual void  svc_end()  {}

    /**
     * Set a Barrier object for the ff_thread.
     *
     * \param b a barrier object
     */
    void set_barrier(BARRIER_T * const b) { barrier=b;}

    /// Create a new thread
    int spawn(int cpuId=-1) {
        if (spawned) return -1;

        if (pthread_attr_init(&attr)) {
                perror("pthread_attr_init");
                return -1;
        }

        init_thread_affinity(&attr, cpuId);
        if (barrier) tid=barrier->getCounter();
        if (pthread_create(&th_handle, &attr,
                           proxy_thread_routine, this) != 0) {
            perror("pthread_create");
            return -1;
        }
        if (barrier) barrier->incCounter();
        spawned = true;
        return 0;
    }
   
    /// Wait thread termination
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

    /// Wait for the thread to freeze (uses Mutex)
    int wait_freezing() {
        pthread_mutex_lock(&mutex);
        while(!frozen) pthread_cond_wait(&cond_frozen,&mutex);        
        pthread_mutex_unlock(&mutex);        
        return (init_error?-1:0);
    }

    /// Force the thread to stop at next EOS or next thawing
    void stop() { stp = true; };

    /// Force the thread to freeze himself
    void freeze() {  
        stp=false;
        freezing=true; 
    }
    
    /// If the thread is frozen, then thaw it
    void thaw() {
        pthread_mutex_lock(&mutex);
        freezing=false;
        assert(thawed==false);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        while(!thawed) pthread_cond_wait(&cond, &mutex);
        thawed=false;
        pthread_mutex_unlock(&mutex);
    }

    /// Check whether the thread is frozen
    bool isfrozen() { return freezing;} 

    pthread_t get_handle() const { return th_handle;}

protected:
    unsigned        tid;
private:
    BARRIER_T    *  barrier;            /// A \p Barrier object
    bool            stp;
    bool            spawned;
    bool            freezing;
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
 *  @}
 */

// -------------------------------------------------------------------------------------------------

/*!
 *  \ingroup low_level
 *
 *  @{
 */

/*!
 *  \class ff_node
 *
 *  \brief This class describes the \p ff_node, the basic building block of every 
 *  skeleton.
 *
 *  This class desribes the Node object, the basic unit of parallelism in a 
 *  streaming network.
 *  It is used to encapsulate sequential portions of code implementing functions, 
 *  as well as higher
 *  level parallel patterns such as pipelines and farms.
 *  \p ff_node defines 3 basic 
 *  methods, two optional - \p svc_init and \p svc_end - and one mandatory - 
 *  \p svc (pure virtual 
 *  method). The \p svc_init method is called once at node initialization, 
 *  while the \p svn_end method 
 *  is called once when the end-of-stream (EOS) is received in input or when the 
 *  \p svc method returns 
 *  \p NULL. The \p svc method is called each time an input task is ready to be 
 *  processed.
 */
class ff_node {
private:

    template <typename lb_t, typename gt_t> 
    friend class ff_farm;
    friend class ff_pipeline;
    friend class ff_loadbalancer;
    friend class ff_gatherer;

protected:
    /// Set node ID
    void set_id(int id) { myid = id;}
    /**
     *  Virtual method. Pushes data into the output buffer.
     *
     *  \param ptr pointer to the data to be pushed out.
     */
    virtual inline bool push(void * ptr) { return out->push(ptr); }
    /**
     *  Virtual method. Pop data from the input buffer.
     *
     *  \param ptr pointer to the location where to store the data.
     */
    virtual inline bool pop(void ** ptr) { return in->pop(ptr);   } 
    virtual inline void skipfirstpop(bool sk)   { skip1pop=sk;}
    bool skipfirstpop() const { return skip1pop; }
    
    /** 
     *  Create an input buffer for the ff_node. 
     *
     *  \param nentries the size of the buffer
     *  \param fixedsize flag to decide whether the buffer is resizable. 
     *  Default is \p true
     */
    virtual int create_input_buffer(int nentries, bool fixedsize=true) {
        if (in) return -1;
        in = new FFBUFFER(nentries,fixedsize);        
        if (!in) return -1;
        myinbuffer=true;
        return (in->init()?0:-1);
    }
    
    /** 
     *  Create an output buffer for the ff_node. 
     *
     *  \param nentries the size of the buffer
     *  \param fixedsize flag to decide whether the buffer is resizable. 
     *  Default is \p false
     */
    virtual int create_output_buffer(int nentries, bool fixedsize=false) {
        if (out) return -1;
        out = new FFBUFFER(nentries,fixedsize);        
        if (!out) return -1;
        myoutbuffer=true;
        return (out->init()?0:-1);
    }

    /** 
     *  Set the output buffer for the ff_node.
     *
     *  \param o a buffer object, which can be of type \p SWSR_Ptr_Buffer or 
     *  \p uSWSR_Ptr_Buffer
     */
    virtual int set_output_buffer(FFBUFFER * const o) {
        if (myoutbuffer) return -1;
        out = o;
        return 0;
    }

    /** 
     *  Set the input buffer for the ff_node.
     *
     *  \param i a buffer object, which can be of type \p SWSR_Ptr_Buffer or 
     *  \p uSWSR_Ptr_Buffer
     */
    virtual int set_input_buffer(FFBUFFER * const i) {
        if (myinbuffer) return -1;
        in = i;
        return 0;
    }

    /// Run the thread
    virtual int   run(bool=false) { 
        thread = new thWorker(this);
        if (!thread) return -1;
        return thread->run();
    }
    
    /// Freeze thread and then run.
    virtual int freeze_and_run(bool=false) {
        thread = new thWorker(this);
        if (!thread) return -1;
        freeze();
        return thread->run();
    }

    /// Wait thread termination
    virtual int  wait() { 
        if (!thread) return -1;
        return thread->wait(); 
    }
    
    /// Wait for thread to thaw
    virtual int  wait_freezing() { 
        if (!thread) return -1;
        return thread->wait_freezing(); 
    }
    
    /// Stop thread
    virtual void stop() {
        if (!thread) return; 
        thread->stop(); 
    }
    
    /// Freeze thread
    virtual void freeze() { 
        if (!thread) return; 
        thread->freeze(); 
    }
    
    /// If thread is frozem then thaw it
    virtual void thaw() { 
        if (!thread) return; 
        thread->thaw();
    }
    
    /// Check whether the thread is frozen
    virtual bool isfrozen() { 
        if (!thread) 
            return false;
        return thread->isfrozen();
    }
    

    virtual int  cardinality(BARRIER_T * const b) { 
        barrier = b;
        return 1;
    }
    
    /// Set the barrier.
    virtual void set_barrier(BARRIER_T * const b) {
        barrier = b;
    }

    virtual double ffTime() {
        return diffmsec(tstop,tstart);
    }

    virtual double wffTime() {
        return diffmsec(wtstop,wtstart);
    }

public:
	enum {TICKS2WAIT=1000};
    /**
     * Pure virtual function.\n
     * If \p svc returns a NULL value then End-Of-Stream (EOS) is produced
     * on the output channel.
     */
    virtual void* svc(void * task) = 0;
    
    /**
     * Virtual function.\n
     * This is called only once for each thread, right before the \p svc method
     */
    virtual int   svc_init() { return 0; }
    
    /**
     * Virtual function.\n
     * This is called only once for each thread, right after the \p svc method
     */
    virtual void  svc_end() {}

    /**
     * Virtual function.\n
     * Called once just after the EOS has arrived.
     *
     * \param id is the ID of the input channel, it makes sense only for
     * N-to-1 input channels.
     */
    virtual void  eosnotify(int id=-1) {}
    
    /// Virtual function.\n Get thread's ID
    virtual int   get_my_id() const { return myid; };
    
    /**
     * Virtual function.\n 
     * Map the working thread to the chosen CPU.
     *
     * \param cpuID the ID of the CPU to which the thread will be pinned.
     */
    virtual void  setAffinity(int cpuID) { 
        if (cpuID<0 || !threadMapper::instance()->checkCPUId(cpuID) ) {
            error("setAffinity, invalid cpuID\n");
        }
        CPUId=cpuID;
    }
    
    /// Get the ID of the CPU where the thread is running
    virtual int   getCPUId() const { return CPUId;}

#if defined(TEST_QUEUE_SPIN_LOCK)
    virtual bool  put(void * ptr) { 
        spin_lock(lock);
        bool r= in->push(ptr);
        spin_unlock(lock);
        return r;
    }
    virtual bool  get(void **ptr) { 
        spin_lock(lock);
        register bool r = out->pop(ptr);
        spin_unlock(lock);
        return r;
    }
#else
    /// See push.
    virtual bool  put(void * ptr) { return in->push(ptr);}
    /// See pop.
    virtual bool  get(void **ptr) { return out->pop(ptr);}
#endif
    /// Returns a pointer to the input buffer
    virtual FFBUFFER * const get_in_buffer() const { return in;}
    /// Returns a pointer to the output buffer    
    virtual FFBUFFER * const get_out_buffer() const { return out;}

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
#endif

protected:
    /// Protected constructor
    ff_node():in(0),out(0),myid(-1),CPUId(-1),
              myoutbuffer(false),myinbuffer(false),
              skip1pop(false), thread(NULL),callback(NULL),barrier(NULL) {
        time_setzero(tstart);time_setzero(tstop);
        time_setzero(wtstart);time_setzero(wtstop);
        wttime=0;
        FFTRACE(taskcnt=0;lostpushticks=0;pushwait=0;lostpopticks=0;popwait=0;ticksmin=(ticks)-1;ticksmax=0;tickstot=0);
#if defined(TEST_QUEUE_SPIN_LOCK)
        init_unlocked(lock);
#endif
    };
    
    /** 
     *  Destructor 
     *
     *  Deletes input and output buffers and the working threads
     */
    virtual  ~ff_node() {
        if (in && myinbuffer)  delete in;
        if (out && myoutbuffer) delete out;
        if (thread) delete thread;
    };
    
    /** Allows to queue tasks without returning from the \p svc method */
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

private:

    void registerCallback(bool (*cb)(void *,unsigned int,unsigned int,void *), void * arg) {
        callback=cb;
        callback_arg=arg;
    }
    
    /*!
     *  @}
     */

    /* Inner class that wraps ff_thread functions and adds \p push and \p pop methods */
    class thWorker: public ff_thread {
    public:
        thWorker(ff_node * const filter):
            ff_thread(filter->barrier),filter(filter) {}
        
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
        
        inline bool pop(void ** task) {
            //register int cnt = 0;       
            /* NOTE: filter->push and not buffer->push because of the filter can be a dnode
             */
            while (! filter->pop(task)) {
                //if (ch->thxcore>1) {
                //if (++cnt>PUSH_POP_CNT) { sched_yield(); cnt=0;}
                //else ticks_wait(TICKS2WAIT);
                //} else 

                FFTRACE(filter->lostpopticks+=ff_node::TICKS2WAIT; ++filter->popwait);
                ticks_wait(ff_node::TICKS2WAIT);
            } 
            return true;
        }
        
        inline bool put(void * ptr) { return filter->put(ptr);}
        inline bool get(void **ptr) { return filter->get(ptr);}
        
        void* svc(void * ) {
            void * task = NULL;
            void * ret = (void*)FF_EOS;
            bool inpresent  = (filter->get_in_buffer() != NULL);
            bool outpresent = (filter->get_out_buffer() != NULL);
            bool skipfirstpop = filter->skipfirstpop(); 
            bool exit=false;            

            gettimeofday(&filter->wtstart,NULL);
            do {
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
        
        int svc_init() {
#if !defined(HAVE_PTHREAD_SETAFFINITY_NP) && !defined(NO_DEFAULT_MAPPING)
            int cpuId = filter->getCPUId();            
            if (ff_mapThreadToCpu((cpuId<0) ? threadMapper::instance()->getCoreId(tid) : cpuId)!=0)
                error("Cannot map thread %d to CPU %d, going on...\n",tid,
                      (cpuId<0) ? threadMapper::instance()->getCoreId(tid) : cpuId);
#endif
            
            gettimeofday(&filter->tstart,NULL);
            return filter->svc_init(); 
        }
        
        virtual void svc_end() {
            filter->svc_end();
            gettimeofday(&filter->tstop,NULL);
        }
        
        int run() { return ff_thread::spawn(filter->getCPUId()); }
        int wait() { return ff_thread::wait();}
        int wait_freezing() { return ff_thread::wait_freezing();}
        void freeze() { ff_thread::freeze();}
        bool isfrozen() { return ff_thread::isfrozen();}

        int get_my_id() const { return filter->get_my_id(); };
        
    protected:    
        ff_node * const filter;
    };
    
    
/*!
 *  \ingroup low_level
 *
 *  @{
 */

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
 */


} // namespace ff

#endif /* _FF_NODE_HPP_ */
