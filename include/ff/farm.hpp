/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*! 
 *  \link
 *  \file farm.hpp
 *  \ingroup high_level_patterns_shared_memory
 *  
 *
 *  \brief This file describes the farm skeleton.
 */
 
#ifndef _FF_FARM_HPP_
#define _FF_FARM_HPP_
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

#include <iostream>
#include <vector>
#include <algorithm>
#include <ff/platforms/platform.h>
#include <ff/lb.hpp>
#include <ff/gt.hpp>
#include <ff/node.hpp>


namespace ff {

/* ---------------- basic high-level macros ----------------------- */

    /* FF_FARMA creates an accelerator farm with 'nworkers' workers. 
     * The functions called in each worker must have the following 
     * signature:
     *   bool (*F)(type &)
     * If the function F returns false an EOS is produced in the 
     * output channel.
     */

#define FF_FARMA(farmname, F, type, nworkers, qlen)                     \
    class _FF_node_##farmname_##F: public ff_node {                     \
    public:                                                             \
     void* svc(void* task) {                                            \
        type* _ff_in = (type*)task;                                     \
        if (!F(*_ff_in)) return NULL;                                   \
        return task;                                                    \
     }                                                                  \
    };                                                                  \
    ff_farm<> _FF_node_##farmname(true,qlen,qlen,true,nworkers,true);   \
    _FF_node_##farmname.cleanup_workers();                              \
    _FF_node_##farmname.set_scheduling_ondemand();                      \
    std::vector<ff_node*> w_##farmname;                                 \
    for(int i=0;i<nworkers;++i)                                         \
        w_##farmname.push_back(new _FF_node_##farmname_##F);            \
    _FF_node_##farmname.add_workers(w_##farmname);                      \
    _FF_node_##farmname.add_collector(NULL);

#define FF_FARMARUN(farmname)  _FF_node_##farmname.run()
#define FF_FARMAOFFLOAD(farmname, task)  _FF_node_##farmname.offload(task);
#define FF_FARMAGETRESULTNB(farmname, result) _FF_node_##farmname.load_result_nb((void**)result)
#define FF_FARMAGETRESULT(farmname, result) _FF_node_##farmname.load_result((void**)result)
#define FF_FARMAWAIT(farmname) _FF_node_##farmname.wait()
#define FF_FARMAEND(farmname)  _FF_node_##farmname.offload(EOS)

/* ---------------------------------------------------------------- */


/*!
 * \ingroup high_level_patterns_shared_memory
 *
 *  @{
 */

/*!
 *  \class ff_farm
 * \ingroup high_level_patterns_shared_memory
 *
 *  \brief The Farm skeleton, with Emitter (\p lb_t) and Collector (\p gt_t).
 *
 *  The Farm skeleton can be seen as a 3-stages pipeline. The first stage is
 *  the \a Emitter (\ref ff_loadbalancer "lb_t") that act as a load-balancer;
 *  the last (optional) stage would be the \a Collector (\ref ff_gatherer
 *  "gt_t") that gathers the results computed by the \a Workers, which are
 *  ff_nodes.
 *
 *  This class is defined in \ref farm.hpp
 */
template<typename lb_t=ff_loadbalancer, typename gt_t=ff_gatherer>
class ff_farm: public ff_node {
protected:
    /**
     * \brief Cardinality of the Farm
     *
     * It defines the cardinality of the farm.
     * 
     * \parm barrier is a parameter of type \p BARRIER_T
     *
     * \return An integer value showing the cardinality.
     */
    inline int cardinality(BARRIER_T * const barrier)  { 
        int card=0;
        for(int i=0;i<nworkers;++i) 
            card += workers[i]->cardinality(barrier);
        
        lb->set_barrier(barrier);
        if (gt) gt->set_barrier(barrier);

        return (card + 1 + ((collector && !collector_removed)?1:0));
    }

    /**
     * \brief Prepares the Farm skeleton
     *
     * It prepares the Farm skeleton for execution.
     *
     * \return 0 if successful, otherwise a negative value is returned.
     */
    inline int prepare() {
        for(int i=0;i<nworkers;++i) {
            if (workers[i]->create_input_buffer((ondemand ? ondemand: (in_buffer_entries/nworkers + 1)), 
                                                (ondemand ? true: fixedsize))<0) return -1;
            if (collector || lb->masterworker() || collector_removed) 
                // NOTE: force unbounded queue if masterworker
                if (workers[i]->create_output_buffer(out_buffer_entries/nworkers + DEF_IN_OUT_DIFF, 
                                                     (lb->masterworker()?false:fixedsize))<0)
                    return -1;
            lb->register_worker(workers[i]);
            if (collector && !collector_removed) gt->register_worker(workers[i]);
        }
        prepared=true;
        return 0;
    }

    /**
     * \brief Frezes the thread and then execute
     *
     * It first freezes the thread for one cycle, and then start executing it.
     *
     * \return The status of the run function
     */
    int freeze_and_run(bool=false) {
        freeze();
        return run(true);
    } 

    template<typename T>
    struct ff_node_F: public ff_node {
        typedef T*(*F_t)(T*);
        ff_node_F(F_t F):F(F) {};
        void *svc(void *t) { return F((T*)t); }
        F_t F;
    };

public:
    /*
     *TODO
     */
    enum { DEF_MAX_NUM_WORKERS=(MAX_NUM_THREADS-2), DEF_IN_BUFF_ENTRIES=2048, DEF_IN_OUT_DIFF=128, 
           DEF_OUT_BUFF_ENTRIES=(DEF_IN_BUFF_ENTRIES+DEF_IN_OUT_DIFF)};

    /**
     * A typedef of loadBalancer_t
     */
    typedef lb_t LoadBalancer_t;

    /**
     * A typedef of Gatherer_t
     */
    typedef gt_t Gatherer_t;

    /**                                                                                                                                 
     *  \brief Constructor  
     * 
     *  This is the basic (the simplest) farm that can be built.
     *  It has a default emitter and a default collector.
     */
    template<typename T>
    ff_farm(T*(*F)(T*), int nw, bool input_ch=false):
        has_input_channel(input_ch),prepared(false),collector_removed(false),ondemand(0),
        nworkers(0), in_buffer_entries(DEF_IN_BUFF_ENTRIES),
        out_buffer_entries(DEF_OUT_BUFF_ENTRIES),
        worker_cleanup(true),
        max_nworkers(nw),
        emitter(NULL),collector(NULL),
        lb(new lb_t(nw)),gt(new gt_t(nw)),
        workers(new ff_node*[nw]),fixedsize(false) {

        std::vector<ff_node*> w(nw);        
        for(int i=0;i<nw;++i) w[i]=new ff_node_F<T>(F);
        add_workers(w);
        add_collector(NULL);
        if (has_input_channel) { 
            if (create_input_buffer(in_buffer_entries, fixedsize)<0) {
                error("FARM, creating input buffer\n");
            }
        }
    }

    ff_farm(std::vector<ff_node*>& W, bool input_ch=false):
        has_input_channel(input_ch),prepared(false),collector_removed(false),ondemand(0),
        nworkers(0), in_buffer_entries(DEF_IN_BUFF_ENTRIES),
        out_buffer_entries(DEF_OUT_BUFF_ENTRIES),
        worker_cleanup(false),
        max_nworkers(W.size()),
        emitter(NULL),collector(NULL),
        lb(new lb_t(W.size())),gt(new gt_t(W.size())),
        workers(new ff_node*[W.size()]),fixedsize(false) {

        add_workers(W);
        add_collector(NULL);
        if (has_input_channel) { 
            if (create_input_buffer(in_buffer_entries, fixedsize)<0) {
                error("FARM, creating input buffer\n");
            }
        }
    }


    /**
     *  \brief Constructor
     *
     *  This is the default constructor.
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
        emitter(NULL),collector(NULL),
        lb(new lb_t(max_num_workers)),gt(new gt_t(max_num_workers)),
        workers(new ff_node*[max_num_workers]),fixedsize(fixedsize) {
        for(int i=0;i<max_num_workers;++i) workers[i]=NULL;

        if (has_input_channel) { 
            if (create_input_buffer(in_buffer_entries, fixedsize)<0) {
                error("FARM, creating input buffer\n");
            }
        }
    }
    
    /** 
     * \brief Destructor
     *
     * This is the default destructor, which delete the load balancer, the
     * gatherer, all the workers, and the barriers.
     *
     */
    ~ff_farm() { 
        if (lb) { delete lb; lb=NULL;}
        if (gt) { delete gt; gt=NULL;}
        if (workers) {
            if (worker_cleanup) {
                for(int i=0;i<max_nworkers; ++i) 
                    if (workers[i]) delete workers[i];
            }
            delete [] workers;
            workers = NULL;
        }
        if (barrier) {delete barrier; barrier=NULL;}
    }

    /** 
     *
     *  \brief Adds the emitter
     *
     *  It adds an Emitter to the Farm. The Emitter is of type \p ff_node and
     *  there can be only one Emitter in a Farm skeleton. 
     *  
     *  \param e the \p ff_node acting as an Emitter 
     *
     *  \return Returns 0 if successful -1 otherwise
     *
     */
    int add_emitter(ff_node * e) { 
        assert(e!=NULL);
        if (emitter) return -1; 

        emitter = e;
        if (in) {
            assert(has_input_channel);
            emitter->set_input_buffer(in);
        }
        if (lb->set_filter(emitter)) return -1;
        return 0;
    }

    /**
     *
     * \brief Set scheduling with on demand polity
     *
     * The default scheduling policy is round-robin, When there is a great
     * computational difference among tasks the round-robin scheduling policy
     * could lead to load imbalance in worker's workload (expecially with short
     * stream length). The on-demand scheduling policy can guarantee a near
     * optimal load balancing in lots of cases. Alternatively it is always
     * possible to define a complete application-level scheduling by redefining
     * the ff_loadbalancer class.
     *
     * \parm inbufferentries sets the number of queue slot for one worker
     * threads.
     *
     */
    void set_scheduling_ondemand(const int inbufferentries=1) { 
        if (inbufferentries<=0) ondemand=1;
        else ondemand=inbufferentries;
    }

    /**
     *  \brief Adds workers to the form
     *
     *  Add workers to the Farm. There is a limit to the number of workers that
     *  can be added to a Farm. This limit is set by default to 64. This limit
     *  can be augmented by passing the desired limit as the fifth parameter of
     *  the \p ff_farm constructor.
     *
     *  \param w a vector of \p ff_nodes which are Workers to be attached to
     *  the Farm.
     *
     *  \return 0 if successsful, otherwise -1 is returned.
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
     *  \brief Adds the collector
     *
     *  It adds the Collector filter to the farm skeleton. If no object is
     *  passed as a colelctor, than a default collector will be added (i.e.
     *  \link ff_gatherer \endlink). Note that it is not possible to add more
     *  than one collector. 
     *
     *  \param c the \p ff_node acting as Collector node.
     *  \parm outpresent TODO
     *
     *  \return The status of \p set_filter(x) if successful, otherwise -1 is
     *  returned.
     */
    int add_collector(ff_node * c, bool outpresent=false) { 
        if (collector) {
            error("add_collector: collector already defined!\n");
            return -1; 
        }
        if (!gt) return -1; 

        collector = ((c!=NULL)?c:(ff_node*)gt);
        
        if (has_input_channel) { /* it's an accelerator */
            if (create_output_buffer(out_buffer_entries, false)<0) return -1;
        }
        
        return gt->set_filter(c);
    }
    
    /**
     *
     * \brief Sets the feedback channel from the collector to the emitter
     *
     * This method allows to estabilish a feedback channel from the Collector
     * to the Emitter. If the collector is present, than the collector output
     * queue will be connected to the emitter input queue (feedback channel),
     * otherwise the emitter acts as collector filter (pure master-worker
     * skeleton).
     *
     * \return 0 if successful, otherwise -1 is returned.
     *
     */
    int wrap_around() {
        if (!collector || collector_removed) {
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
     *
     * \brief Removes the collector
     *
     * It allows not to start the collector thread, whereas all worker's output
     * buffer will be created as if it were present.
     *
     * \return 0 is always returned.
     */
    int remove_collector() { 
        collector_removed = true;
        return 0;
    }

    /**
     * \brief Sets multiple input
     *
     * It sets multiple inputs to the node.
     *
     * \parm mi is a pointer of \p ff_node
     * \parm misize is the size of the input queue
     *
     * \return The status of \p set_multi_input(x) otherwise -1 is returned.
     */
    int set_multi_input(ff_node **mi, int misize) {
        // if (lb->masterworker()) {
        //     error("FARM, master-worker paradigm and multi-input farm used together\n");
        //     return -1;
        // }
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


    /**
     * \brief Delete workers when the destructor is called.
     *
     */
    void cleanup_workers() {
        worker_cleanup = true;
    }

    /**
     * \brief Execute the Farm 
     *
     * It executes the form.
     *
     * \parm skip_init A booleon value showing if the initialization should be
     * skipped
     *
     * \return If successful 0, otherwise a negative is returned.
     *
     */
    int run(bool skip_init=false) {
        if (!skip_init) {
            // set the initial value for the barrier 

            if (!barrier)  barrier = new BARRIER_T;
            const int nthreads = cardinality(barrier);
            if (nthreads > MAX_NUM_THREADS) {
                error("FARM, too much threads, increase MAX_NUM_THREADS !\n");
                return -1;
            }

            barrier->barrierSetup(nthreads);            
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

    /** 
     * \brief Executs the farm and wait for workers to complete
     *
     * It executes the farm and waits for all workers to complete their
     * tasks.
     *
     * \return If successful 0, otherwise a negative value is returned.
     */
    virtual int run_and_wait_end() {
        if (isfrozen()) {
            stop();
            thaw();
            if (wait()<0) return -1;
            return 0;
        }
        stop();
        if (run()<0) return -1;
        if (wait()<0) return -1;
        return 0;
    }

    /** 
     * \brief Executes the farm and then freeze.
     *
     * It executs the form and then freezes the form.
     * If workers are frozen, it is possible to wake up just a subset of them.
     *
     * \return If successful 0, otherwise a negative value
     */
    virtual int run_then_freeze(int nw=-1) {
        if (isfrozen()) {
            // true means that next time threads are frozen again
            thaw(true, nw); 
            return 0;
        }
        if (!prepared) if (prepare()<0) return -1;
        freeze();
        return run();
    }
    
    /** 
     * \brief Puts the thread in waiting state
     *
     * It puts the thread in waiting state.
     *
     * \return 0 if successful, otherwise -1 is returned.
     */
    int wait(/* timeval */ ) {
        int ret=0;
        if (lb->wait()<0) ret=-1;
        if (collector) if (gt->wait()<0) ret=-1;
        return ret;
    }

    /** 
     * \brief Waits for freezing
     *
     * It waits for thread to freeze.
     *
     * \return 0 if successful otherwise -1 is returned.
     */
    int wait_freezing(/* timeval */ ) {
        int ret=0;
        if (lb->wait_freezing()<0) ret=-1;
        if (collector) if (gt->wait_freezing()<0) ret=-1;
        return ret; 
    } 

    /** 
     * \brief Forces a thread to Stop
     *
     * It forces the thread to stop at the next EOS signal. 
     */
    void stop() {
        lb->stop();
        if (collector) gt->stop();
    }

    /** 
     * \brief Forces the thread to freeze at next FF_EOS.
     *
     * It forces a thread to Freeze itself.
     */
    void freeze() {
        lb->freeze();
        if (collector) gt->freeze();
    }

    /**
     * \brief Thaws the thread
     *
     * If the thread is frozen, then thaw it. 
     */
    void thaw(bool _freeze=false, int nw=-1) {

        // TODO: 
        if ((nw != -1) && (nw < nworkers) && collector) {
            error("running less thread then total NOT YET SUPPORTED in case of collector present\n"); 
            abort();
        }

        lb->thaw(_freeze, nw);
        if (collector) gt->thaw(_freeze);
    }

    /**
     * \brief Checks if the Farm is frozen
     *
     * It checks if the farm is frozen.
     *
     * \return The status of \p isfrozen().
     */
    bool isfrozen() { return lb->isfrozen(); }

    /**
     * \breif Offloads teh task to farm
     *
     * It offloads the given task to the farm.
     *
     * \parm task is a void pointer
     * \parm retry showing the number of tries to offload
     * \parm ticks is the number of ticks to wait
     *
     * \return \p true if successful, otherwise \p false
     */
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
     * \brief Loads results into gatherer
     *
     * It loads the results from the gatherer (if any).
     *
     * \parm task is a void pointer
     * \parm retry is the number of tries to load the results
     * \parm ticks is the number of ticks to wait
     *
     * \return \p false if EOS arrived or too many retries, \p true if  there is a new value
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

    /**
     * \brief Loads result with non-blocking
     *
     * It loads the result with non-blocking situation.
     *
     * \parm task is a void pointer
     *
     * \return \false if no task is present, otherwise \true if there is a new
     * value. It should be checked if the task has a \p FF_EOS
     *
     */
    inline bool load_result_nb(void ** task) {
        if (!collector) return false;
        return gt->pop_nb(task);
    }
    
    /**
     * \brief Gets Emitter node
     *
     * It gets the emitter node.
     *
     * \return A pointer to the load balancer is returned.
     *
     */
    inline lb_t * getlb() const { return lb;}

    /**
     * \breif Gets Collector node
     *
     * It gets the collector node.
     *
     * \return A pointer to the gatherer is returned.
     */
    inline gt_t * getgt() const { return gt;}

    /**
     * \brief Gets workers list
     *
     * It gets the list of the workers
     *
     * \return A list of workers is returned.
     */
    ff_node** getWorkers() const { return workers; }

    /**
     * \brief Gets the number of workers
     *
     * The number of workers is returned.
     *
     * \return An integet value showing the number of workers.
     */
    int getNWorkers() const { return nworkers;}

    /**
     * \brief Gets the starting time
     *
     * It returns the starting time.
     *
     * \return A structure of \p timeval showing the starting time.
     *
     */
    const struct timeval getstarttime() const { return lb->getstarttime();}

    /**
     * \brief Gets the stoping time
     *
     * It returns the structure showing the finishing time. It
     * is the collector then return the finishing time of the farm. otherwise,
     * collects the finishing time in all workers and add them in a vector and
     * then return the vector, showing the collective finishing time of the
     * farm with no collector.
     *
     * \return A \timeval showing the finishing time of the farm.
     */
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

    /**
     * \brief Gets the starting time
     *
     * It returnes the starting time.
     *
     * TODO: Do not know what is the difference between wstartime and startime.
     *
     * \return A struct of type timeval showing the starting time.
     */
    const struct timeval  getwstartime() const { return lb->getwstartime(); }    

    /**
     * \brief Gets the finishing time
     *
     * It returns the finishing time if there exists a collector in the farm.
     * If there is no collector, then the finishing time of individual workers
     * is collected in the form of a vector and return that vector.
     *
     * \return The vector showing the finishing time.
     */
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
    
    /**
     * \brief Gets the time spent in \p svc_init
     *
     * The returned time comprises the time spent in \p svc_init and in \p
     * svc_end methods.
     *
     * \return A double value showing the time taken in \p svc_init
     */
    double ffTime() {
        if (collector)
            return diffmsec(gt->getstoptime(), lb->getstarttime());
        return diffmsec(getstoptime(),lb->getstarttime());
    }

    /**
     * \brief Gets the time spent in \p svc
     *
     * The returned time considers only the time spent in the svc methods.
     *
     * \return A double value showing the time taken in \p svc.
     */
    double ffwTime() {
        if (collector)
            return diffmsec(gt->getwstoptime(), lb->getwstartime());

        return diffmsec(getwstoptime(),lb->getwstartime());
    }

#if defined(TRACE_FASTFLOW)
    /**
     * \brief Prints FastFlow trace
     *
     * It prints the trace of FastFlow
     *
     * \parm out determines the output terminal.
     */
    void ffStats(std::ostream & out) { 
        out << "--- farm:\n";
        lb->ffStats(out);
        for(int i=0;i<nworkers;++i) workers[i]->ffStats(out);
        if (collector) gt->ffStats(out);
    }
#else
    /**
     * \brief Prints FastFlow message
     *
     * It prints the FastFlow trace message indicateing that the define to
     * print trace should be defined.
     *
     * \parm out determines the output terminal.
     */
    void ffStats(std::ostream & out) { 
        out << "FastFlow trace not enabled\n";
    }

#endif
    
protected:

    /**
     * \brief svc method
     *
     * It gives the definition of the \p svc method
     *
     * \parm task is a void pointer
     *
     * \return always return \p NULL
     */
    void* svc(void * task) { return NULL; }

    /**
     * \brief The svc_init method
     *
     * It gives the definition of the \p svc_init
     *
     * \return -1 is always returned.
     */
    int svc_init()       { return -1; };

    /**
     * \brief The svc_end method
     *
     * It defines the \p svc_end method.
     *
     */
    void svc_end()        {}

    /**
     * \brief Gets the thread id
     *
     * It returns the thread is 
     *
     * \return An integer value showing the id of the thread.
     *
     * FIXME: Why -1 is returned? I think it should return the id??
     */
    int get_my_id() const { return -1; };

    /**
     * \brief Sets the affinity
     *
     * It sets the affinity of the farm
     *
     * FIXME: Why the message is displayed? It should set the affinity. No?
     */
    void setAffinity(int) { 
        error("FARM, setAffinity: cannot set affinity for the farm\n");
    }

    /**
     * \brief Gets the id of the core
     *
     * It gets the identifier of the core.
     *
     * \return The identifier of the core
     *
     * FIXME: why -1? it should return the id of the core.
     */
    int getCPUId() const { return -1;}

    /** 
     *  \brief Creates the input buffer for the emitter node
     *
     *  This function redefines the ff_node's virtual method of the same name.
     *  It creates an input buffer for the Emitter node. 
     *
     *  \param nentries the size of the buffer
     *  \param fixedsize flag to decide whether the buffer is resizable. 
     *
     *  \return If successful 0, otherwsie a negative value.
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
     *  \brief Creates the output buffer for the collector
     *
     *  This function redefines the ff_node's virtual method of the same name.
     *  It create an output buffer for the Collector
     *
     *  \param nentries the size of the buffer
     *  \param fixedsize flag to decide whether the buffer is resizable. 
     *  Default is \p false
     *
     *  \return If successful 0, otherwise a negative value.
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
     *
     *  \brief Sets the output buffer of the collector 
     *
     *  This function redefines the ff_node's virtual method of the same name.
     *  Set the output buffer for the Collector.
     *
     *  \param o a buffer object, which can be of type \p SWSR_Ptr_Buffer or 
     *  \p uSWSR_Ptr_Buffer
     *
     *  \return 0 if successful, otherwise -1 is returned.
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

    /**
     * \brief Gets emitter
     *
     * It returns a pointer to the emitter.
     *
     * \return A pointer of the FastFlow node which is actually the emitter.
     */
    ff_node* getEmitter()   { return emitter;}

    /**
     * \brief Gets collector
     * 
     * It returns a pointer to the collector.
     *
     * \return A pointer to collector node if exists, otherwise a \p NULL
     */
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

    lb_t             * lb;
    gt_t             * gt;
    ff_node         ** workers;
    bool               fixedsize;
};

/*!
 * \ingroup high_level_patterns_shared_memory
 *  
 *  \brief Ordered farm
 *
 *  This class defines the ordered form.
 *
 *  This class is defined in \ref farm.hpp
 */
class ofarm_lb: public ff_loadbalancer {
protected:
    /**
     * \brief Selects workers
     *
     * It selects the worker.
     *
     * \return An integer value showing the worker.
     */
    inline int selectworker() { return victim; }
public:
    /**
     * \brief Constructos
     *
     * It defines the constructos which formst the load balancer with the give
     * number of workers.
     *
     * \parm max_num_workers defines the maximum number of workers.
     */
    ofarm_lb(int max_num_workers):ff_loadbalancer(max_num_workers) {}

    /**
     * \brief Sets the workers
     *
     * It sets the worker.
     *
     * \parm v is the number of the worker.
     */
    void set_victim(int v) { victim=v;}
private:
    int victim;
};

/*!
 * \ingroup high_level_patterns_shared_memory
 *
 * \brief Ordered farm with gatherer
 *
 * It defines an ordered farm with gatherer.
 *
 * This class is defined in \ref farm.hpp
 */
class ofarm_gt: public ff_gatherer {
protected:
    /**
     * \brief Selects worker
     *
     * It selects a worker
     *
     * \return An integet value showing he id of the worker
     */
    inline int selectworker() { return victim; }
public:
    /**
     * \brief Constructor
     *
     * It build the ordered farm.
     *
     * \parm max_num_workers defines the maximum number of workers.
     *
     */
    ofarm_gt(int max_num_workers):
        ff_gatherer(max_num_workers),dead(max_num_workers) {
        dead.resize(max_num_workers);
        reset();
    }

    /**
     * \brief Sets the victim
     *
     * It sets the victim with the given worker
     *
     * \return \p true if successful, otherwise \p false.
     */
    inline bool set_victim(int v) { 
        if (dead[v]) return false; 
        victim=v; 
        return true;
    }

    /**
     * \brief Sets the dead
     *
     * It sets an element in the dead vector to true, to make it dead.
     *
     * \parm v is number in the dead vector.
     */
    inline void set_dead(int v)   { dead[v]=true;}

    /**
     * \brief Resets
     *
     * It makes the element in the dead vector as false i.e. makes it alive.
     *
     */
    inline void reset() {
        for(size_t i=0;i<dead.size();++i) dead[i]=false;
    }
private:
    int victim;
    svector<bool> dead;
};

/*!
 *  \class ff_ofarm
 *  \ingroup high_level_patterns_shared_memory
 *
 *  \brief The ordered Farm skeleton.
 *
 *  This class is defined in \ref farm.hpp
 */

class ff_ofarm: public ff_farm<ofarm_lb, ofarm_gt> {
private:
    /**
     * \brief inline class emitter of ordered farm
     */
    class ofarmE: public ff_node {
    public:
        /**
         * \brief Emitter of ordered form
         *
         * It creates an ordered form
         *
         * \parm lb is ordered farm load balancer
         *
         */
        ofarmE(ofarm_lb * const lb):
            nworkers(0),nextone(0), lb(lb),E_f(NULL) {}

        /**
         * \brief Sets number of workers
         *
         * It sets the number of workers.
         *
         * \parm nw is the number of workers
         */
        void setnworkers(int nw) { nworkers=nw;}

        /**
         * \brief Sets filter
         *
         * It sets the filter
         *
         * \parm f is the FastFlow node
         *
         */
        void setfilter(ff_node* f) { E_f = f;}

        /**
         * \brief \p svc_init method
         *
         * It defines the svc initialization method.
         *
         * \return 0 if not successful, otherwise the status of \p svc_init
         */
        int svc_init() {
            assert(nworkers>0);
            int ret = 0;
            if (E_f) ret = E_f->svc_init();
            nextone = 0;
            lb->set_victim(nextone);
            return ret;
        }  

        /**
         * \brief \p svc method
         *
         * It defines the \p svc method.
         *
         * \parm task is a void pointer
         *
         * \return If it is \p FF_EOS then the status of svc method is returned
         * otherwise \p GO_ON
         *
         */
        void * svc(void * task) {
            if (E_f) task = E_f->svc(task);
            if (task == (void*)FF_EOS) return task;
            ff_send_out(task);
            nextone = (nextone+1) % nworkers;
            lb->set_victim(nextone);
            return GO_ON;
        }

        /**
         * \brief \p svc_end method
         *
         * It defines the \p svc_end method.
         */
        void svc_end() {
            if (E_f) E_f->svc_end();
        }
    private:
        int nworkers;
        int nextone;
        ofarm_lb * lb;
        ff_node* E_f;
    };

    /**
     * inline class collector for oredred farm
     */
    class ofarmC: public ff_node {
    public:
        /**
         * \brief Constructor
         *
         * it defines the collectors for the oredered farm.
         *
         */
        ofarmC(ofarm_gt * const gt):
            nworkers(0),nextone(0), gt(gt),C_f(NULL) {}

        /**
         * \brief Sets worker
         *
         * It sets the number of workers
         *
         * \parm nw is the number of workers
         */
        void setnworkers(int nw) { nworkers=nw;}

        /**
         * \brief Sets filter
         *
         * It sets the filter.
         *
         */
        void setfilter(ff_node* f) { C_f = f;}

        /**
         * \brief \p svc_init method
         *
         * It defines the \p svc_init method.
         *
         * \return 0 if successful, otherwise the status of \p set_victim
         */
        int svc_init() {
            assert(nworkers>0);
            int ret = 0;
            if (C_f) ret = C_f->svc_init();
            nextone=0;
            gt->reset();
            gt->set_victim(nextone);
            return ret;
        }

        /**
         * \brief \p svc method
         *
         * It defines the \p svc method.
         *
         * \return \p GO_ON is always returned.
         */
        void * svc(void * task) {
            if (C_f) task = C_f->svc(task);
            ff_send_out(task);
            do nextone = (nextone+1) % nworkers;
            while(!gt->set_victim(nextone));
            return GO_ON;
        }

        /**
         * \brief Notifies EOS
         *
         * \It notifies the EOS.
         *
         */
        void eosnotify(int id) { 
            gt->set_dead(id);
            if (nextone == id) {
                nextone= (nextone+1) % nworkers;
                gt->set_victim(nextone);
            }
        }

        /**
         * \brief \p svc_end method
         *
         * It defines the \p svc_end method.
         */
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
    /**
     * \brief Constructor
     *
     * It creates an orderred farm.
     *
     * \parm input_ch states the presence of input channel
     * \parm in_buffer_entries defines the number of input entries
     * \parm worker_cleanup states the cleaning of workers
     * \parm max_num_workers defines the maximum number of workers in the
     * ordered farm
     * \parm fixedsize states the status of fixed size buffer
     *
     */
    ff_ofarm(bool input_ch=false,
            int in_buffer_entries=DEF_IN_BUFF_ENTRIES, 
            int out_buffer_entries=DEF_OUT_BUFF_ENTRIES,
            bool worker_cleanup=false,
            int max_num_workers=DEF_MAX_NUM_WORKERS,
            bool fixedsize=false):  // NOTE: by default all the internal farm queues are unbounded !
        ff_farm<ofarm_lb,ofarm_gt>(input_ch,in_buffer_entries,out_buffer_entries,worker_cleanup,max_num_workers,fixedsize),E(NULL),C(NULL),E_f(NULL),C_f(NULL) {
        E = new ofarmE(this->getlb());
        C = new ofarmC(this->getgt());
        this->add_emitter(E);
        this->add_collector(C);
    }

    /**
     * \brief Destructor 
     *
     * It clean emitter and collector.
     */
    ~ff_ofarm() {
        if (E) delete E;
        if (C) delete C;
    }

    /**
     * \brief Sets emitter
     *
     * It sets the emitter.
     *
     * \parm f is the FastFlow node.
     */
    void setEmitterF  (ff_node* f) { E_f = f; }

    /**
     * \brief Sets collector
     *
     * It sets the collecto
     *
     * \parm f is the FastFlow node.
     */
    void setCollectorF(ff_node* f) { C_f = f; }
    
    /**
     * \brief Runs the farm
     *
     * It executes the farm.
     *
     * \parm skip_init shows the status of weather the init phase to be
     * cancelled or not
     *
     * \return The status of run method
     */
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
 * \ingroup streaming_network_arbitrary_shared_memory
 *
 * \brief FastFlow node with multiple inputs
 *
 * This class defines the FastFlow node with multiple entries.
 *
 * This class is defined in \ref farm.hpp
 */

class ff_minode: public ff_node {
protected:

    /**
     * \brief Defines the cardinality of the FastFlow node
     *
     * It defines the cardinatlity of the FastFlow node by setting the barrier
     * to the give number of barriers.
     *
     * \parm barrier shows the number of barriers
     *
     * \return 1 is alway returned.
     */
    inline int cardinality(BARRIER_T * const barrier)  { 
        gt->set_barrier(barrier);
        return 1;
    }

    /**
     * \brief Creates the input buffer
     *
     * It creates the input buffer to the node.
     *
     * \return 0 if successful, otherwise -1 is returned.
     */
    int create_input_buffer(int, bool =true) {
        std::vector<ff_node*> w;
        if (set_input(w)<0) return -1;
        if (w.size()==0) return -1;
        for(size_t i=0;i<w.size();++i)
            gt->register_worker(w[i]);

        if (ff_node::skipfirstpop()) gt->skipfirstpop();

        return 0;
    }

    /**
     * \brief Waits 
     *
     * It wait for TODO?
     *
     * \return 0 if successful, otherwise -1 is returned.
     */
    int  wait(/* timeout */) { 
        if (gt->wait()<0) return -1;
        return 0;
    }

public:
    /**
     * \brief Constructor
     *
     * It defines the multiple input FastFlow node.
     *
     * \parm max_num_workers defines the maximum number of workers
     *
     */
    ff_minode(int max_num_workers=ff_farm<>::DEF_MAX_NUM_WORKERS):
        ff_node(), gt(new ff_gatherer(max_num_workers)) {}

    /**
     * \brief Destructor 
     *
     * It deletes the gatherer.
     */
    ~ff_minode() {
        if (gt) delete gt;
    }
    
    /**
     * \brief Sets input buffer
     *
     * It is a virtual function to set the input buffer.
     */
    virtual int set_input(std::vector<ff_node *> & w)=0;

    /**
     * \brief Skip first pop
     *
     * It calls the \p skipfirstpop method to skip the first pop
     */
    inline void skipfirstpop(bool sk)   { ff_node::skipfirstpop(sk);}

    /**
     * \brief Runs the multiple input farm
     *
     * It executes the multiple input farm.
     *
     * \return 0 if successful, otherwise -1 is returned.
     *
     */
    int run(bool skip_init=false) {
        if (!gt) return -1;
        gt->set_filter(this);
        if (gt->run()<0) {
            error("ff_minode, running gather module\n");
            return -1;
        }
        return 0;
    }

    /**
     * \brief Gets the gatherer
     *
     * It gets the internal gatherer.
     *
     * \return A pointer to the FastFlow gatherer.
     */
    inline ff_gatherer * getgt() const { return gt;}

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
    ff_gatherer* gt;
};

/*!
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 * \brief Multiple output FastFlow node
 *
 * This class defines the FastFlow node with multiple output buffers.
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
     * \parm barrier defines the barrier
     *
     * \return 1 is always returned.
     */
    inline int   cardinality(BARRIER_T * const barrier)  { 
        lb->set_barrier(barrier);
        return 1;
    }

    /**
     * \brief Wait
     *
     * It waits for TODO. Wait for what?
     *
     * \return 0 if successful, otherwise -1 is returned.
     */
    int  wait(/* timeout */) { 
        if (lb->waitlb()<0) return -1;
        return 0;
    }


public:
    /**
     * \brief Constructor
     *
     * It creates a FastFlow node with multiple output buffers.
     *
     * \parm max_num_workers defines the maximum number of workers
     *
     */
    ff_monode(int max_num_workers=ff_farm<>::DEF_MAX_NUM_WORKERS):
        ff_node(), lb(new ff_loadbalancer(max_num_workers)) {}

    /**
     * \brief Destructor 
     *
     * It deletes the load balancer.
     */
    ~ff_monode() {
        if (lb) delete lb;
    }
    
    /**
     * \brief Sets the output buffer
     *
     * It is a virtual function which sets the output buffer.
     */
    virtual int set_output(std::vector<ff_node *> & w)=0;

    /**
     * \brief Skips the first pop
     *
     * It skips the first pop.
     */
    inline void skipfirstpop(bool sk)   {
        if (sk)
            lb->skipfirstpop();
    }

    /**
     * \brief Executes the farm
     *
     * It executes the farm.
     *
     * \parm skip_init defines if the initilization should be skipped
     *
     * \return 0 if successful, otherwsie -1 is returned.
     */
    int run(bool skip_init=false) {
        if (!lb) return -1;
        lb->set_filter(this);

        std::vector<ff_node*> w;
        if (set_output(w)<0) {
            error("ff_monode, run, set_output failed\n");
            return -1;
        }
        if (w.size()==0) {
            error("ff_monode, run, set_output returned 0 workers\n");
            return -1;
        }
        for(size_t i=0;i<w.size();++i)
            lb->register_worker(w[i]);
        
        if (lb->runlb()<0) {
            error("ff_monode, running loadbalancer module\n");
            return -1;
        }
        return 0;
    }

    /**
     * \brief Gets the internal gatherer
     *
     * It gets the internal gatherer.
     *
     * \return A pointer to the FastFlow load balancer
     */
    inline ff_loadbalancer * getlb() const { return lb;}

#if defined(TRACE_FASTFLOW) 
    /**
     * \brief Prints the FastFlow trace
     *
     * It prints the trace of FastFlow.
     */
    inline void ffStats(std::ostream & out) { 
        lb->ffStats(out);
    }
#endif

private:
    ff_loadbalancer* lb;
};

/*!
 *  @}
 *  \endlink
 */

} // namespace ff

#endif /* _FF_FARM_HPP_ */
