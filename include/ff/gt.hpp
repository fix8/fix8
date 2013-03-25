/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*! \file gt.hpp
 *  \brief Contains the \p ff_gatherer class and methods used to model the \a Collector node, 
 *  which is optionally used to gather tasks coming from workers.
 */

#ifndef _FF_GT_HPP_
#define _FF_GT_HPP_
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
#include <deque>
#include <ff/svector.hpp>
#include <ff/utils.hpp>
#include <ff/node.hpp>

namespace ff {

/*!
 *  \ingroup low_level
 *
 *  @{
 */

/*!
 *  \class ff_gatherer
 *
 *  \brief A class representing the \a Collector node in a \a Farm skeleton.
 *
 *  This class models the \p gatherer, which wraps all the methods and structures used by the \a 
 *  Collector node in a \p Farm skeleton. The \p farm can be seen as a three-stages \p pipeline, the 
 *  stages being a \p ff_node called \a emitter, a pool of \p ff_node called \a workers and - 
 *  optionally - a \p ff_node called \a collector. The \a Collector node can be used to gather the 
 *  results outcoming from the computations executed by the pool of \a workers. The \a collector can 
 *  also be connected to the \a emitter node via a feedback channel, in order to create a 
 *  \p farm-with-feedback skeleton.
 *
 */

class ff_gatherer: public ff_thread {

    template <typename T1, typename T2>  friend class ff_farm;
    friend class ff_pipeline;
public:
    enum {TICKS2WAIT=5000};

protected:

    /**
     * Virtual function. \n Select a worker
     * 
     * Gets the next one using the RR policy.
     * The one selected has to be alive (and kicking).
     */
    virtual inline int selectworker() { 
        do nextr = (nextr+1) % nworkers;
        while(offline[nextr]);
        return nextr;
    }

    virtual inline void notifyeos(int id) {}

    /* number of tentative before wasting some times and than retry */
    virtual inline unsigned int ntentative() { return nworkers;}

    virtual inline void losetime_out() { 
        FFTRACE(lostpushticks+=TICKS2WAIT;++pushwait);
        ticks_wait(TICKS2WAIT); 
#if 0
        FFTRACE(register ticks t0 = getticks());
        usleep(TICKS2WAIT);
        FFTRACE(register ticks diff=(getticks()-t0));
        FFTRACE(lostpushticks+=diff;++pushwait);
#endif
    }
    
    virtual inline void losetime_in() { 
        FFTRACE(lostpopticks+=TICKS2WAIT;++popwait);
        ticks_wait(TICKS2WAIT);
#if 0        
        FFTRACE(register ticks t0 = getticks());
        usleep(TICKS2WAIT);
        FFTRACE(register ticks diff=(getticks()-t0));
        FFTRACE(lostpopticks+=diff;++popwait);
#endif
    }    

    virtual int gather_task(void ** task) {
        register unsigned int cnt;
        do {
            cnt=0;
            do {
                nextr = selectworker();
                assert(offline[nextr]==false);
                if (workers[nextr]->get(task)) return nextr;
                else if (++cnt == ntentative()) break;
            } while(1);
            losetime_in();
        } while(1);

        // not reached
        return -1;
    }


    /// Virtual function. \n Gather tasks' results from all workers
    virtual int all_gather(void *task, void **V) {
        V[channelid]=task;
        int nw=getnworkers();
        svector<ff_node*> _workers(nw);
        for(int i=0;i<nworkers;++i) 
            if (!offline[i]) _workers.push_back(workers[i]);
        svector<int> retry(nw);

        for(register int i=0;i<nw;++i) {
            if(i!=channelid && !_workers[i]->get(&V[i]))
                retry.push_back(i);
        }
        while(retry.size()) {
            channelid = retry.back();
            if(_workers[channelid]->get(&V[channelid]))
                retry.pop_back();
            else losetime_in();
        }
        for(register int i=0;i<nw;++i)
            if (V[i] == (void *)FF_EOS || V[i] == (void*)FF_EOS_NOFREEZE)
                return -1;
        FFTRACE(taskcnt+=nw-1);
        return 0;
    }

    /// Push the task in the tasks queue.
    void push(void * task) {
        //register int cnt = 0;
        if (!filter) {
            while (! buffer->push(task)) {
                // if (ch->thxcore>1) {
                // if (++cnt>PUSH_POP_CNT) { sched_yield(); cnt=0;}
                //    else ticks_wait(TICKS2WAIT);
                //} else 
                losetime_out();
            }     
            return;
        }
        while (! filter->push(task)) {
            // if (ch->thxcore>1) {
            // if (++cnt>PUSH_POP_CNT) { sched_yield(); cnt=0;}
            //    else ticks_wait(TICKS2WAIT);
            //} else 
            losetime_out();
        }     
    }

    /// Pop a task out of the queue.
    bool pop(void ** task) {
        //register int cnt = 0;       
        if (!get_out_buffer()) return false;
        while (! buffer->pop(task)) {
            losetime_in();
        } 
        return true;
    }

    bool pop_nb(void ** task) {
        if (!get_out_buffer()) return false;
        return buffer->pop(task);
    }  



public:

    /**
     *  Constructor
     *
     *  Creates \a max_num_workers \p NULL pointers to worker objects
     */
    ff_gatherer(int max_num_workers):
        max_nworkers(max_num_workers),nworkers(0),nextr(0),
        neos(0),neosnofreeze(0),channelid(-1),
        filter(NULL), workers(max_nworkers), offline(max_nworkers), buffer(NULL) {
        time_setzero(tstart);time_setzero(tstop);
        time_setzero(wtstart);time_setzero(wtstop);
        wttime=0;        
        FFTRACE(taskcnt=0;lostpushticks=0;pushwait=0;lostpopticks=0;popwait=0;ticksmin=(ticks)-1;ticksmax=0;tickstot=0);        
    }


    int set_filter(ff_node * f) { 
        if (filter) {
            error("GT, setting collector filter\n");
            return -1;
        }
        filter = f;
        return 0;
    }

    /// Set output buffer
    void set_out_buffer(FFBUFFER * const buff) { buffer=buff;}

    /**
     * Get the channel id of the last pop.  
     */
    const int get_channel_id() const { return channelid;}

    /// Gets the number of worker threads currently running.
    inline const int getnworkers() const { return workers.size()-neos-neosnofreeze; }
    
    /// Get the ouput buffer
    FFBUFFER * const get_out_buffer() const { return buffer;}

    /// Register the given worker to the list of workers
    int  register_worker(ff_node * w) {
        if (nworkers>=max_nworkers) {
            error("GT, max number of workers reached (max=%d)\n",max_nworkers);
            return -1;
        }
        workers.push_back(w);
        ++nworkers;
        return 0;
    }

    /// Virtual function: initialise the gatherer task.
    virtual int svc_init() { 
        gettimeofday(&tstart,NULL);
        for(unsigned i=0;i<workers.size();++i)  offline[i]=false;
        if (filter) return filter->svc_init(); 
        return 0;
    }

    /// Virtual function: the gatherer task.
    virtual void * svc(void *) {
        void * ret  = (void*)FF_EOS;
        void * task = NULL;
        bool outpresent  = (get_out_buffer() != NULL);

        // the following case is possible when the collector is a dnode
        if (!outpresent && filter && (filter->get_out_buffer()!=NULL)) {
            outpresent=true;
            set_out_buffer(filter->get_in_buffer());
        }
       
        gettimeofday(&wtstart,NULL);
        do {
            task = NULL;
            nextr = gather_task(&task); 
            if (task == (void *)FF_EOS) {
                if (filter) filter->eosnotify(workers[nextr]->get_my_id());
                offline[nextr]=true;
                ++neos;
                ret=task;
            } else if (task == (void *)FF_EOS_NOFREEZE) {
                if (filter) filter->eosnotify(workers[nextr]->get_my_id());
                offline[nextr]=true;
                ++neosnofreeze;
                ret = task;
            } else {
                FFTRACE(++taskcnt);
                if (filter)  {
                    channelid = workers[nextr]->get_my_id();
                    FFTRACE(register ticks t0 = getticks());
                    task = filter->svc(task);

#if defined(TRACE_FASTFLOW)
                    register ticks diff=(getticks()-t0);
                    tickstot +=diff;
                    ticksmin=(std::min)(ticksmin,diff);
                    ticksmax=(std::max)(ticksmax,diff);
#endif    
                }

                // if the filter returns NULL we exit immediatly
                if (task == GO_ON) continue;
                
                // if the filter returns NULL we exit immediatly
                if (task ==(void*)FF_EOS_NOFREEZE) { 
                    ret = task;
                    break; 
                }
                if (!task || task==(void*)FF_EOS) {
                    ret = (void*)FF_EOS;
                    break;
                }                
                if (outpresent) {
                    //if (filter) filter->push(task);
                    //else 
                    push(task);
                }
            }
        } while((neos<nworkers) && (neosnofreeze<nworkers));

        if (outpresent) {
            // push EOS
            task = ret;
            push(task);
        }

        gettimeofday(&wtstop,NULL);
        wttime+=diffmsec(wtstop,wtstart);
        if (neos>=nworkers) neos=0;
        if (neosnofreeze>=nworkers) neosnofreeze=0;

        return ret;
    }

    /// Virtual function: finalise the gatherer task.
    virtual void svc_end() {
        if (filter) filter->svc_end();
        gettimeofday(&tstop,NULL);
    }

    /// Execute the gatherer task.
    int run(bool=false) {  
        if (this->spawn(filter?filter->getCPUId():-1)<0) {
            error("GT, spawning GT thread\n");
            return -1; 
        }
        return 0;
    }

    virtual double ffTime() {
        return diffmsec(tstop,tstart);
    }

    virtual double wffTime() {
        return diffmsec(wtstop,wtstart);
    }

    virtual const struct timeval & getstarttime() const { return tstart;}
    virtual const struct timeval & getstoptime()  const { return tstop;}
    virtual const struct timeval & getwstartime() const { return wtstart;}
    virtual const struct timeval & getwstoptime() const { return wtstop;}

#if defined(TRACE_FASTFLOW)    
    virtual void ffStats(std::ostream & out) { 
        out << "Collector: "
            << "  work-time (ms): " << wttime    << "\n"
            << "  n. tasks      : " << taskcnt   << "\n"
            << "  svc ticks     : " << tickstot  << " (min= " << (filter?ticksmin:0) << " max= " << ticksmax << ")\n"
            << "  n. push lost  : " << pushwait  << " (ticks=" << lostpushticks << ")" << "\n"
            << "  n. pop lost   : " << popwait   << " (ticks=" << lostpopticks  << ")" << "\n";
    }
#endif

private:
    int               max_nworkers;
    int               nworkers; // this is the # of workers initially registered
    int               nextr;

    int               neos;
    int               neosnofreeze;
    int               channelid;

    ff_node         * filter;
    svector<ff_node*> workers;
    svector<bool>     offline;
    FFBUFFER        * buffer;

    struct timeval tstart;
    struct timeval tstop;
    struct timeval wtstart;
    struct timeval wtstop;
    double wttime;

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

#endif /*  _FF_GT_HPP_ */
