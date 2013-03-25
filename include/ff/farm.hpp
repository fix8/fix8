/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*! 
 *  \file farm.hpp
 *  \brief This file describes the farm skeleton.
 */
 
#ifndef _FF_FARM_HPP_
#define _FF_FARM_HPP_
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

#include <iostream>
#include <vector>
#include <algorithm>
#include <ff/platforms/platform.h>
#include <ff/lb.hpp>
#include <ff/gt.hpp>
#include <ff/node.hpp>


namespace ff {

/*!
 *  \ingroup high_level
 *
 *  @{
 */

/*!
 *  \class ff_farm
 *
 *  \brief The Farm skeleton, with Emitter (\p lb_t) and Collector (\p gt_t).
 *
 *  The Farm skeleton can be seen as a 3-stages pipeline. The first stage is the \a Emitter (\ref 
 *  ff_loadbalancer "lb_t") that act as a load-balancer; the last (optional) stage would be the \a 
 *  Collector (\ref ff_gatherer "gt_t") that gathers the results computed by the \a Workers, which 
 *  are ff_nodes.
 */
template<typename lb_t=ff_loadbalancer, typename gt_t=ff_gatherer>
class ff_farm: public ff_node {
protected:
    inline int   cardinality(BARRIER_T * const barrier)  { 
        int card=0;
        for(int i=0;i<nworkers;++i) 
            card += workers[i]->cardinality(barrier);
        
        lb->set_barrier(barrier);
        if (gt) gt->set_barrier(barrier);

        return (card + 1 + (collector?1:0));
    }

    /// Prepare the Farm skeleton for execution
    inline int prepare() {
        for(int i=0;i<nworkers;++i) {
            if (workers[i]->create_input_buffer((ondemand ? ondemand: (in_buffer_entries/nworkers + 1)), 
                                                (ondemand ? true: fixedsize))<0) return -1;
            if (collector || lb->masterworker() || collector_removed) 
                if (workers[i]->create_output_buffer(out_buffer_entries/nworkers + DEF_IN_OUT_DIFF, fixedsize)<0) 
                    return -1;
            lb->register_worker(workers[i]);
            if (collector && !collector_removed) gt->register_worker(workers[i]);
        }
        prepared=true;
        return 0;
    }

    int freeze_and_run(bool=false) {
        freeze();
        return run(true);
    } 

public:
    enum { DEF_MAX_NUM_WORKERS=64, DEF_IN_BUFF_ENTRIES=2048, DEF_IN_OUT_DIFF=128, 
           DEF_OUT_BUFF_ENTRIES=(DEF_IN_BUFF_ENTRIES+DEF_IN_OUT_DIFF)};

    typedef lb_t LoadBalancer_t;
    typedef gt_t Gatherer_t;

    /*!
     *  Constructor
     *
     *  \param input_ch = true to set accelerator mode
     *  \param in_buffer_entries = input queue length
     *  \param out_buffer_entries = output queue length
     *  \param max_num_workers = highest number of farm's worker
     *  \param worker_cleanup = true deallocate worker object at exit
     *  \param fixedsize = true uses only fixed size queue
     */
    ff_farm(bool input_ch=false,
            int in_buffer_entries=DEF_IN_BUFF_ENTRIES, 
            int out_buffer_entries=DEF_OUT_BUFF_ENTRIES,
            bool worker_cleanup=false,
            int max_num_workers=DEF_MAX_NUM_WORKERS,
            bool fixedsize=false):  // NOTE: by default all the internal farm queues are unbounded !
        has_input_channel(input_ch),prepared(false),collector_removed(false),ondemand(0),
        nworkers(0),
        in_buffer_entries(in_buffer_entries),
        out_buffer_entries(out_buffer_entries),
        worker_cleanup(worker_cleanup),
        max_nworkers(max_num_workers),
        emitter(NULL),collector(NULL),fallback(NULL),
        lb(new lb_t(max_num_workers)),gt(new gt_t(max_num_workers)),
        workers(new ff_node*[max_num_workers]),fixedsize(fixedsize) {
        for(int i=0;i<max_num_workers;++i) workers[i]=NULL;

        if (has_input_channel) { 
            if (create_input_buffer(in_buffer_entries, fixedsize)<0) {
                error("FARM, creating input buffer\n");
            }
        }
    }
    
    /** Destructor */
    ~ff_farm() { 
        if (lb) delete lb; 
        if (gt) delete(gt); 
        if (workers) {
            if (worker_cleanup) {
                for(int i=0;i<max_nworkers; ++i) 
                    if (workers[i]) delete workers[i];
            }
            delete [] workers;
        }
        if (barrier) delete barrier;
    }

    /** 
     *  Add an Emitter to the Farm.\n
     *  The Emitter is of type \p ff_node and there can be only one Emitter in a 
     *  Farm skeleton. 
     *  
     *  \param e the ff_node acting as an Emitter
     *  \param fb an ff_node acting as a fallback (Note that it is not possible to 
     *  add a fallback funtion if the collector is present or a master-worker 
     *  configuration has been set).
     *
     */
    int add_emitter(ff_node * e, ff_node * fb=NULL) { 
        if (emitter) return -1; 

        /* NOTE: if there is a collector filter then no 
         * fallback execution is possible 
         */
        if ((collector || lb->masterworker()) && fb) {
            error("FARM, cannot add fallback function if the collector is present or master-worker configuration has been set\n");
            return -1;
        }
        emitter = e;
        if (in) {
            assert(has_input_channel);
            emitter->set_input_buffer(in);
        }
        fallback=fb;
        if (lb->set_filter(emitter)) return -1;
        return lb->set_fallback(fallback);
    }

    /*!
     * The default scheduling policy is round-robin,
     * When there is a great computational difference among tasks
     * the round-robin scheduling policy could lead to load imbalance
     * in worker's workload (expecially with short stream length).
     * The on-demand scheduling policy can guarantee a near optimal
     * load balancing in lots of cases.
     *
     * Alternatively it is always possible to define a complete 
     * application-level scheduling by redefining the ff_loadbalancer class.
     *
     * The function parameter, sets the number of queue slot for 
     * one worker threads.
     *
     */
    void set_scheduling_ondemand(const int inbufferentries=1) { 
        if (in_buffer_entries<=0) ondemand=1;
        else ondemand=inbufferentries;
    }

    /**
     *  Add workers to the Farm.\n
     *  There is a limit to the number of workers that can be added to a Farm. This 
     *  limit is set by default to 64. This limit can be augmented by passing the 
     *  desired limit as the fifth parameter of the ff_farm constructor.
     *
     *  \param w a vector of \p ff_nodes which are Workers to be attached to the 
     *  Farm.
     */
    int add_workers(std::vector<ff_node *> & w) { 
        if ((nworkers+w.size())> (unsigned int)max_nworkers) {
            error("FARM, try to add too many workers, please increase max_nworkers\n");
            return -1; 
        }
        if ((nworkers+w.size())==0) {
            error("FARM, try to add zero workers!\n");
            return -1; 
        }        
        for(unsigned int i=nworkers;i<(nworkers+w.size());++i) {
            workers[i] = w[i];
            workers[i]->set_id(i);
        }
        nworkers+= (unsigned int) w.size();
        return 0;
    }

    /**
     *  Add the Collector filter to the farm skeleton.\n 
     *  If no object is passed as a colelctor, than a default collector will be 
     *  added (i.e. \link ff_gatherer \endlink). Note that it is not possible to 
     *  add more than one collector. And it is not possible to add a collector 
     *  when a fallback funciton is defined.
     *
     *  \param c the ff_node acting as Collector node.
     */
    int add_collector(ff_node * c, bool outpresent=false) { 
        if (collector) {
            error("add_collector: collector already defined!\n");
            return -1; 
        }
        if (!gt)       return -1; 

        if (fallback) {
            error("FARM, cannot add collector filter with fallback function\n");
            return -1;
        }        
        collector = ((c!=NULL)?c:(ff_node*)gt);
        
        if (has_input_channel) { /* it's an accelerator */
            if (create_output_buffer(out_buffer_entries, fixedsize)<0) return -1;
        }
        
        return gt->set_filter(c);
    }
    
    /**
     * This method allows to estabilish a feedback channel from the Collector to 
     * the Emitter.
     * If the collector is present, than the collector output queue 
     * will be connected to the emitter input queue (feedback channel),
     * otherwise the emitter acts as collector filter (pure master-worker
     * skeleton).
     */
    int wrap_around() {
        if (fallback) {
            error("FARM, cannot add feedback channels if the fallback function has been set in the Emitter\n");
            return -1;
        }
        if (!collector) {
            if (lb->set_masterworker()<0) return -1;
            if (!has_input_channel) lb->skipfirstpop();
            return 0;
        }

        if (create_input_buffer(in_buffer_entries, false)<0) {
            error("FARM, creating input buffer\n");
            return -1;
        }
        
        if (set_output_buffer(get_in_buffer())<0) {
            error("FARM, setting output buffer\n");
            return -1;
        }

        lb->skipfirstpop();
        return 0;
    }

    /**
     * Allows not to start the collector thread, whereas all worker's 
     * output buffer will be created as if it were present.
     */
    int remove_collector() { 
        collector_removed = true;
        return 0;
    }

    
    int set_multi_input(ff_node **mi, int misize) {
        if (lb->masterworker()) {
            error("FARM, master-worker paradigm and multi-input farm used together\n");
            return -1;
        }
        if (mi == NULL) {
            error("FARM, invalid multi-input vector\n");
            return -1;
        }
        if (misize <= 0) {
            error("FARM, invalid multi-input vector size\n");
            return -1;
        }
        return lb->set_multi_input(mi, misize);
    }

    /** Execute the Farm */
    int run(bool skip_init=false) {
        if (!skip_init) {
            // set the initial value for the barrier 

            if (!barrier)  barrier = new BARRIER_T;
            barrier->barrierSetup(cardinality(barrier));
        }
        
        if (!prepared) if (prepare()<0) return -1;

        if (lb->run()<0) {
            error("FARM, running load-balancer module\n");
            return -1;        
        }
        if (!collector_removed)
            if (collector && gt->run()<0) {
                error("FARM, running gather module\n");
                return -1;
            }

        return 0;
    }

    /** Execute the farm and wait for all workers to complete their tasks */
    virtual int run_and_wait_end() {
        if (isfrozen()) return -1; // FIX !!!!

        stop();
        if (run()<0) return -1;           
        if (wait()<0) return -1;
        return 0;
    }

    /** Execute the farm and then freeze. */
    virtual int run_then_freeze() {
        if (isfrozen()) {
            thaw();
            freeze();
            return 0;
        }
        if (!prepared) if (prepare()<0) return -1;
        freeze();
        return run();
    }
    
    /** Wait */
    int wait(/* timeval */ ) {
        int ret=0;
        if (lb->wait()<0) ret=-1;
        if (collector) if (gt->wait()<0) ret=-1;
        return ret;
    }

    /** Wait freezing */
    int wait_freezing(/* timeval */ ) {
        int ret=0;
        if (lb->wait_freezing()<0) ret=-1;
        if (collector) if (gt->wait_freezing()<0) ret=-1;
        return ret; 
    } 

    /** Force a thread to Stop at the next EOS signal. */
    void stop() {
        lb->stop();
        if (collector) gt->stop();
    }

    /** Force a thread to Freeze itself */
    void freeze() {
        lb->freeze();
        if (collector) gt->freeze();
    }

    /** If the thread is frozen, then thaw it. */
    void thaw() {
        lb->thaw();
        if (collector) gt->thaw();
    }

    /** Check if the farm is frozen */
    bool isfrozen() { return lb->isfrozen(); }

    /** Offload the given task to the farm */
    inline bool offload(void * task,
                        unsigned int retry=((unsigned int)-1),
                        unsigned int ticks=ff_loadbalancer::TICKS2WAIT) { 
        FFBUFFER * inbuffer = get_in_buffer();

        if (inbuffer) {
            for(unsigned int i=0;i<retry;++i) {
                if (inbuffer->push(task)) return true;
                ticks_wait(ticks);
            }     
            return false;
        }
        
        if (!has_input_channel) 
            error("FARM: accelerator is not set, offload not available");
        else
            error("FARM: input buffer creation failed");
        return false;

    }    

    /**
     * Load results into the gatherer (if any).
     *
     * \return \p false if EOS arrived or too many retries
     * \return \p true if  there is a new value
     */
    inline bool load_result(void ** task,
                            unsigned int retry=((unsigned int)-1),
                            unsigned int ticks=ff_gatherer::TICKS2WAIT) {
        if (!collector) return false;
        for(unsigned int i=0;i<retry;++i) {
            if (gt->pop_nb(task)) {
                if ((*task != (void *)FF_EOS)) return true;
                else return false;
            }
            ticks_wait(ticks);
        }
        return false;
    }

    // return values:
    //   false: no task present
    //   true : there is a new value, you should check if the task is an FF_EOS
    inline bool load_result_nb(void ** task) {
        if (!collector) return false;
        return gt->pop_nb(task);
    }
    
    /// Get Emitter node
    inline lb_t * const getlb() const { return lb;}

    /// Get Collector node
    inline gt_t * const getgt() const { return gt;}

    /// Get workers list
    ff_node** const getWorkers() const { return workers; }
    int getNWorkers() const { return nworkers;}

    const struct timeval  getstarttime() const { return lb->getstarttime();}
    const struct timeval  getstoptime()  const {
        if (collector) return gt->getstoptime();
        const struct timeval zero={0,0};
        std::vector<struct timeval > workertime(nworkers+1,zero);
        for(int i=0;i<nworkers;++i)
            workertime[i]=workers[i]->getstoptime();
        workertime[nworkers]=lb->getstoptime();
        std::vector<struct timeval >::iterator it=
            std::max_element(workertime.begin(),workertime.end(),time_compare);
        return (*it);
    }

    const struct timeval  getwstartime() const { return lb->getwstartime(); }    
    const struct timeval  getwstoptime() const {
        if (collector) return gt->getwstoptime();
        const struct timeval zero={0,0};
        std::vector<struct timeval > workertime(nworkers+1,zero);
        for(int i=0;i<nworkers;++i) {
            workertime[i]=workers[i]->getwstoptime();
            }
        workertime[nworkers]=lb->getwstoptime();
        std::vector<struct timeval >::iterator it=
            std::max_element(workertime.begin(),workertime.end(),time_compare);
        return (*it);
    }
    
    /* the returned time comprises the time spent in svn_init and 
     * in svc_end methods
     */
    double ffTime() {
        if (collector)
            return diffmsec(gt->getstoptime(),
                            lb->getstarttime());
        return diffmsec(getstoptime(),lb->getstarttime());
    }

    /*  the returned time considers only the time spent in the svc
     *  methods
     */
    double ffwTime() {
        if (collector)
            return diffmsec(gt->getwstoptime(),
                            lb->getwstartime());

        return diffmsec(getwstoptime(),lb->getwstartime());
    }

#if defined(TRACE_FASTFLOW)
    void ffStats(std::ostream & out) { 
        out << "--- farm:\n";
        lb->ffStats(out);
        for(int i=0;i<nworkers;++i) workers[i]->ffStats(out);
        if (collector) gt->ffStats(out);
    }
#else
    void ffStats(std::ostream & out) { 
        out << "FastFlow trace not enabled\n";
    }

#endif
    
protected:

    void* svc(void * task) { return NULL; }
    int   svc_init()       { return -1; };
    void  svc_end()        {}
    int   get_my_id() const { return -1; };
    void  setAffinity(int) { 
        error("FARM, setAffinity: cannot set affinity for the farm\n");
    }
    int   getCPUId() { return -1;}

    /** 
     *  This function redefines the ff_node's virtual method of the same name.
     *  It creates an input buffer for the Emitter node. 
     *
     *  \param nentries the size of the buffer
     *  \param fixedsize flag to decide whether the buffer is resizable. 
     */
    int create_input_buffer(int nentries, bool fixedsize) {
        if (in) {
            error("FARM create_input_buffer, buffer already present\n");
            return -1;
        }
        if (emitter) {
            if (emitter->create_input_buffer(nentries,fixedsize)<0) return -1;
            in = emitter->get_in_buffer();
        } else {
            if (ff_node::create_input_buffer(nentries, fixedsize)<0) return -1;
        }
        lb->set_in_buffer(in);

        // old code
        //if (ff_node::create_input_buffer(nentries, fixedsize)<0) return -1;
        //lb->set_in_buffer(in);

        return 0;
    }
    
    /** 
     *  This function redefines the ff_node's virtual method of the same name. 
     *  It create an output buffer for the Collector
     *
     *  \param nentries the size of the buffer
     *  \param fixedsize flag to decide whether the buffer is resizable. 
     *  Default is \p false
     */
    int create_output_buffer(int nentries, bool fixedsize=false) {
        if (!collector) {
            error("FARM with no collector, cannot create output buffer\n");
            return -1;
        }        
        if (out) {
            error("FARM create_output_buffer, buffer already present\n");
            return -1;
        }
        if (ff_node::create_output_buffer(nentries, fixedsize)<0) return -1;
        gt->set_out_buffer(out);
        if ((ff_node*)gt != collector) collector->set_output_buffer(out);
        return 0;
    }

    /** 
     *  This function redefines the ff_node's virtual method of the same name. 
     *  Set the output buffer for the Collector.
     *
     *  \param o a buffer object, which can be of type \p SWSR_Ptr_Buffer or 
     *  \p uSWSR_Ptr_Buffer
     */
    int set_output_buffer(FFBUFFER * const o) {
        if (!collector && !collector_removed) {
            error("FARM with no collector, cannot set output buffer\n");
            return -1;
        }
        gt->set_out_buffer(o);
        if (collector && ((ff_node*)gt != collector)) collector->set_output_buffer(o);
        return 0;
    }

    ff_node* getEmitter()   { return emitter;}
    ff_node* getCollector() { 
        if (collector == (ff_node*)gt) return NULL;
        return collector;
    }

protected:
    bool has_input_channel; // for accelerator
    bool prepared;
    bool collector_removed;
    int ondemand;          // if >0, emulates on-demand scheduling
    int nworkers;
    int in_buffer_entries;
    int out_buffer_entries;
    bool worker_cleanup;
    int max_nworkers;

    ff_node          *  emitter;
    ff_node          *  collector;
    ff_node          *  fallback;

    lb_t             * lb;
    gt_t             * gt;
    ff_node         ** workers;
    bool               fixedsize;
};

/*!
 *  @}
 */
 
/*!
 *  \ingroup runtime
 *
 *  @{
 */
class ofarm_lb: public ff_loadbalancer {
protected:
    inline int selectworker() { return victim; }
public:
    ofarm_lb(int max_num_workers):ff_loadbalancer(max_num_workers) {}
    void set_victim(int v) { victim=v;}
private:
    int victim;
};

class ofarm_gt: public ff_gatherer {
protected:
    inline int selectworker() { return victim; }
public:
    ofarm_gt(int max_num_workers):
        ff_gatherer(max_num_workers),dead(max_num_workers) {
        dead.resize(max_num_workers);
        reset();
    }
    inline bool set_victim(int v) { 
        if (dead[v]) return false; victim=v; return true;
    }
    inline void set_dead(int v)   { dead[v]=true;}
    inline void reset() {
        for(size_t i=0;i<dead.size();++i) dead[i]=false;
    }
private:
    int victim;
    svector<bool> dead;
};

/*!
 *  @}
 */


/*!
 *  \ingroup high_level
 *
 *  @{
 */

/*!
 *  \class ff_ofarm
 *
 *  \brief The ordered Farm skeleton.
 *
 *
 */

class ff_ofarm: public ff_farm<ofarm_lb, ofarm_gt> {
private:
    // emitter
    class ofarmE: public ff_node {
    public:
        ofarmE(ofarm_lb * const lb):
            nworkers(0),nextone(0), lb(lb),E_f(NULL) {}
        void setnworkers(int nw) { nworkers=nw;}
        void setfilter(ff_node* f) { E_f = f;}
        int svc_init() {
            assert(nworkers>0);
            int ret = 0;
            if (E_f) ret = E_f->svc_init();
            nextone = 0;
            lb->set_victim(nextone);
            return ret;
        }        
        void * svc(void * task) {
            if (E_f) task = E_f->svc(task);
            ff_send_out(task);
            nextone=(nextone+1) % nworkers;
            lb->set_victim(nextone);
            return GO_ON;
        }        
        void svc_end() {
            if (E_f) E_f->svc_end();
        }
    private:
        int nworkers;
        int nextone;
        ofarm_lb * lb;
        ff_node* E_f;
    };

    // collector
    class ofarmC: public ff_node {
    public:
        ofarmC(ofarm_gt * const gt):
            nworkers(0),nextone(0), gt(gt),C_f(NULL) {}

        void setnworkers(int nw) { nworkers=nw;}
        void setfilter(ff_node* f) { C_f = f;}
        int svc_init() {
            assert(nworkers>0);
            int ret = 0;
            if (C_f) ret = C_f->svc_init();
            nextone=0;
            gt->reset();
            gt->set_victim(nextone);
            return ret;
        }        
        void * svc(void * task) {
            if (C_f) task = C_f->svc(task);
            ff_send_out(task);
            do nextone = (nextone+1) % nworkers;
            while(!gt->set_victim(nextone));
            return GO_ON;
        }        
        void eosnotify(int id) { 
            gt->set_dead(id);
            if (nextone == id) {
                nextone= (nextone+1) % nworkers;
                gt->set_victim(nextone);
            }
        }  
        void svc_end() {
            if (C_f) C_f->svc_end();
        }
    private:
        int nworkers;
        int nextone;
        ofarm_gt * gt;
        ff_node* C_f;
    };
    
public:


    ff_ofarm(bool input_ch=false,
            int in_buffer_entries=DEF_IN_BUFF_ENTRIES, 
            int out_buffer_entries=DEF_OUT_BUFF_ENTRIES,
            bool worker_cleanup=false,
            int max_num_workers=DEF_MAX_NUM_WORKERS):
        ff_farm<ofarm_lb,ofarm_gt>(input_ch,in_buffer_entries,out_buffer_entries,worker_cleanup,max_num_workers),E(NULL),C(NULL),E_f(NULL),C_f(NULL) {
        E = new ofarmE(this->getlb());
        C = new ofarmC(this->getgt());
        this->add_emitter(E);
        this->add_collector(C);
    }

    ~ff_ofarm() {
        if (E) delete E;
        if (C) delete C;
    }

    void setEmitterF  (ff_node* f) { E_f = f; }
    void setCollectorF(ff_node* f) { C_f = f; }
    
    int run(bool skip_init=false) {
        E->setnworkers(this->getNWorkers());
        C->setnworkers(this->getNWorkers());
        E->setfilter(E_f);
        C->setfilter(C_f);
        return ff_farm<ofarm_lb,ofarm_gt>::run(skip_init);
    }
protected:
    ofarmE* E;
    ofarmC* C;
    ff_node* E_f;
    ff_node* C_f;
};


/*!
 *  @}
 */
 

} // namespace ff

#endif /* _FF_FARM_HPP_ */
