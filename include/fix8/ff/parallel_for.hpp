/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 *  \link
 *  \file parallel_for.hpp
 *  \ingroup high_level_patterns_shared_memory
 *
 *  \brief This file describes the parallel_for/parallel_reduce skeletons.
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
/*
 *  - Author:
 *     Massimo Torquati <torquati@di.unipi.it>
 *
 *    History:
 *      - started in May 2013
 *      - January 2014: code optimized
 *      - February 2014:
 *          - avoided to start the Scheduler thread if it is not needed
 *            A new (non lock-free) decentralized scheduler has been implemented
 *            for the case when adding an extra thread is not useful.
 *          - introduced the parallel_for functions
 *          - added the ParallelFor and ParallelForReduce classes
 */

#ifndef FF_PARFOR_HPP
#define FF_PARFOR_HPP

// #ifndef __INTEL_COMPILER
// // see http://www.stroustrup.com/C++11FAQ.html#11
// #if __cplusplus <= 199711L
// #error "parallel_for requires C++11 features"
// #endif
// #endif

#include <atomic>
#include <algorithm>
#include <deque>
#include <vector>
#include <cmath>
#include <functional>
#include <fix8/ff/lb.hpp>
#include <fix8/ff/node.hpp>
#include <fix8/ff/farm.hpp>
#include <fix8/ff/spin-lock.hpp>

#if defined(__ICC)
#define PRAGMA_IVDEP _Pragma("ivdep")
#else
#define PRAGMA_IVDEP
#endif

namespace ff {

    /* -------------------- Parallel For/Reduce Macros -------------------- */
    /* Usage example:
     *                              // loop parallelization using 3 workers
     *                              // and a minimum task grain of 2
     *                              wthread = 3;
     *                              grain = 2;
     *  for(int i=0;i<N;++i)        FF_PARFOR_BEGIN(for,i,0,N,1,grain,wthread) {
     *    A[i]=f(i)          ---->    A[i]=f(i);
     *                              } FF_PARFOR_END(for);
     *
     *   parallel for + reduction:
     *
     *  s=4;
     *  for(int i=0;i<N;++i)        FF_PARFORREDUCE_BEGIN(for,s,0,i,0,N,1,grain,wthread) {
     *    s*=f(i)            ---->    s*=f(i);
     *                              } FF_PARFORREDUCE_END(for,s,*);
     *
     *
     *                              FF_PARFOR_INIT(pf,maxwthread);
     *                              ....
     *  while(k<nTime) {            while(k<nTime) {
     *    for(int i=0;i<N;++i)        FF_PARFORREDUCE_START(pf,s,0,i,0,N,1,grain,wthread) {
     *      s*=f(i,k);       ---->       s*=f(i,k);
     *  }                             } FF_PARFORREDUCE_STOP(pf,s,*);
     *                             }
     *                             ....
     *
     *                             FF_PARFOR_DONE(pf);
     *
     *
     *  NOTE: inside the body of the PARFOR/PARFORREDUCE, it is possible to use the
     *        '_ff_thread_id' const integer variable to identify the thread id
     *        running the sequential portion of the loop.
     */

    /**
     *  name : of the parallel for
     *  idx  : iteration index
     *  begin: for starting point
     *  end  : for ending point
     *  step : for step
     *  chunk: chunk size
     *  nw   : n. of worker threads
     */
#define FF_PARFOR_BEGIN(name, idx, begin, end, step, chunk, nw)                   \
    ff_forall_farm<int> name(nw,false,true);                                      \
    name.setloop(begin,end,step,chunk,nw);                                        \
    auto F_##name = [&] (const long ff_start_##idx, const long ff_stop_##idx,     \
                         const int _ff_thread_id, const int) {                    \
        PRAGMA_IVDEP;                                                             \
        for(long idx=ff_start_##idx;idx<fix8/ff_stop_##idx;idx+=step)

    /* This is equivalent to the above one except that the user has to define
     * the for loop in the range (ff_start_idx,ff_stop_idx(
     * This can be useful if you have to perform some actions before starting
     * the loop.
     */
#define FF_PARFOR2_BEGIN(name, idx, begin, end, step, chunk, nw)                  \
    ff_forall_farm<int> name(nw,false,true);                                      \
    name.setloop(begin,end,step,chunk, nw);                                       \
    auto F_##name = [&] (const long ff_start_##idx, const long ff_stop_##idx,     \
                         const int _ff_thread_id, const int) {                    \
    /* here you have to define the for loop using ff_start/stop_idx  */


#define FF_PARFOR_END(name)                                                       \
    };                                                                            \
    {                                                                             \
      if (name.getnw()>1) {                                                       \
        name.setF(F_##name);                                                      \
        if (name.run_and_wait_end()<0) {                                          \
            error("running parallel for\n");                                      \
        }                                                                         \
      } else F_##name(name.startIdx(),name.stopIdx(),0,0);                        \
    }

    /* ---------------------------------------------- */

    /**
     *  name    : of the parallel for
     *  var     : variable on which the reduce operator is applied
     *  identity: the value such that var == var op identity
     *  idx     : iteration index
     *  begin   : for starting point
     *  end     : for ending point
     *  step    : for step
     *  chunk   : chunk size
     *  nw      : n. of worker threads
     *
     *  op      : reduce operation (+ * ....)
     */
#define FF_PARFORREDUCE_BEGIN(name, var,identity, idx,begin,end,step, chunk, nw)  \
    ff_forall_farm<decltype(var)> name(nw,false,true);                            \
    name.setloop(begin,end,step,chunk,nw);                                        \
    auto idtt_##name =identity;                                                   \
    auto F_##name =[&](const long start,const long stop,const int _ff_thread_id,  \
                       decltype(var) &var) {                                      \
        PRAGMA_IVDEP;                                                             \
        for(long idx=start;idx<stop;idx+=step)

#define FF_PARFORREDUCE_END(name, var, op)                                        \
        };                                                                        \
        if (name.getnw()>1) {                                                     \
          auto ovar_##name = var;                                                 \
          name.setF(F_##name,idtt_##name);                                        \
          if (name.run_and_wait_end()<0) {                                        \
            error("running forall_##name\n");                                     \
          }                                                                       \
          var = ovar_##name;                                                      \
          for(size_t i=0;i<name.getnw();++i)  {                                   \
              var op##= name.getres(i);                                           \
          }                                                                       \
        } else {                                                                  \
          var = ovar_##name;                                                      \
          F_##name(name.startIdx(),name.stopIdx(),0,var);                         \
        }


#define FF_PARFORREDUCE_F_END(name, var, F)                                       \
        };                                                                        \
        if (name.getnw()>1) {                                                     \
          auto ovar_##name = var;                                                 \
          name.setF(F_##name,idtt_##name);                                        \
          if (name.run_and_wait_end()<0)                                          \
              error("running ff_forall_farm (reduce F end)\n");                   \
          var = ovar_##name;                                                      \
          for(size_t i=0;i<name.getnw();++i)  {                                   \
             F(var,name.getres(i));                                               \
          }                                                                       \
        } else {                                                                  \
            F_##name(name.startIdx(),name.stopIdx(),0,var);                       \
        }


    /* ---------------------------------------------- */

    /* FF_PARFOR_START and FF_PARFOR_STOP have the same meaning of
     * FF_PARFOR_BEGIN and FF_PARFOR_END but they have to be used in
     * conjunction with  FF_PARFOR_INIT FF_PARFOR_END.
     *
     * The same is for FF_PARFORREDUCE_START/STOP.
     */
#define FF_PARFOR_INIT(name, nw)                                                  \
    ff_forall_farm<int> *name = new ff_forall_farm<int>(nw);

#define FF_PARFOR_DECL(name)         ff_forall_farm<int> * name
#define FF_PARFOR_ASSIGN(name,nw)    name=new ff_forall_farm<int>(nw)
#define FF_PARFOR_DONE(name)         name->stop(); name->wait(); delete name

#define FF_PARFORREDUCE_INIT(name, type, nw)                                      \
    ff_forall_farm<type> *name = new ff_forall_farm<type>(nw)

#define FF_PARFORREDUCE_DECL(name,type)      ff_forall_farm<type> * name
#define FF_PARFORREDUCE_ASSIGN(name,type,nw) name=new ff_forall_farm<type>(nw)
#define FF_PARFORREDUCE_DONE(name)           name->stop();name->wait();delete name

#define FF_PARFOR_START(name, idx, begin, end, step, chunk, nw)                   \
    name->setloop(begin,end,step,chunk,nw);                                       \
    auto F_##name = [&] (const long ff_start_##idx, const long ff_stop_##idx,     \
                         const int _ff_thread_id, const int) {                    \
        PRAGMA_IVDEP;                                                             \
        for(long idx=ff_start_##idx;idx<fix8/ff_stop_##idx;idx+=step)

#define FF_PARFOR2_START(name, idx, begin, end, step, chunk, nw)                  \
    name->setloop(begin,end,step,chunk,nw);                                       \
    auto F_##name = [&] (const long ff_start_##idx, const long ff_stop_##idx,     \
                         const int _ff_thread_id, const int) {                    \
    /* here you have to define the for loop using ff_start/stop_idx  */

// just another variat that may be used together with FF_PARFORREDUCE_INIT
#define FF_PARFOR_T_START(name, type, idx, begin, end, step, chunk, nw)           \
    name->setloop(begin,end,step,chunk,nw);                                       \
    auto F_##name = [&] (const long ff_start_##idx, const long ff_stop_##idx,     \
                         const int _ff_thread_id, const type&) {                  \
        PRAGMA_IVDEP;                                                             \
        for(long idx=ff_start_##idx;idx<fix8/ff_stop_##idx;idx+=step)


#define FF_PARFOR_STOP(name)                                                      \
    };                                                                            \
    if (name->getnw()>1) {                                                        \
      name->setF(F_##name);                                                       \
      if (name->run_then_freeze(name->getnw())<0)                                 \
          error("running ff_forall_farm (name)\n");                               \
      name->wait_freezing();                                                      \
    } else F_##name(name->startIdx(),name->stopIdx(),0,0)

#define FF_PARFORREDUCE_START(name, var,identity, idx,begin,end,step, chunk, nw)  \
    name->setloop(begin,end,step,chunk,nw);                                       \
    auto idtt_##name =identity;                                                   \
    auto F_##name =[&](const long ff_start_##idx, const long ff_stop_##idx,       \
                       const int _ff_thread_id, decltype(var) &var) {             \
        PRAGMA_IVDEP                                                              \
        for(long idx=ff_start_##idx;idx<fix8/ff_stop_##idx;idx+=step)

#define FF_PARFORREDUCE_STOP(name, var, op)                                       \
        };                                                                        \
        if (name->getnw()>1) {                                                    \
          auto ovar_##name = var;                                                 \
          name->setF(F_##name,idtt_##name);                                       \
          if (name->run_then_freeze(name->getnw())<0)                             \
              error("running ff_forall_farm (name)\n");                           \
          name->wait_freezing();                                                  \
          var = ovar_##name;                                                      \
          for(size_t i=0;i<name->getnw();++i)  {                                  \
              var op##= name->getres(i);                                          \
          }                                                                       \
        } else {                                                                  \
          F_##name(name->startIdx(),name->stopIdx(),0,var);                       \
        }


#define FF_PARFORREDUCE_F_STOP(name, var, F)                                      \
        };                                                                        \
        if (name->getnw()>1) {                                                    \
          auto ovar_##name = var;                                                 \
          name->setF(F_##name,idtt_##name);                                       \
          if (name->run_then_freeze(name->getnw())<0)                             \
              error("running ff_forall_farm (name)\n");                           \
          name->wait_freezing();                                                  \
          var = ovar_##name;                                                      \
          for(size_t i=0;i<name->getnw();++i)  {                                  \
             F(var,name->getres(i));                                              \
          }                                                                       \
        } else {                                                                  \
            F_##name(name->startIdx(),name->stopIdx(),0,var);                     \
        }

    /* ------------------------------------------------------------------- */



// parallel for task, it represents a range (start,end( of indexes
struct forall_task_t {
    forall_task_t():start(0),end(0) {}
    forall_task_t(const forall_task_t &t):start(t.start.load(std::memory_order_relaxed)),end(t.end) {}
    forall_task_t & operator=(const forall_task_t &t) {
        start=t.start.load(std::memory_order_relaxed), end=t.end;
        return *this;
    }
    void set(long s, long e)  { start=s,end=e; }

    std::atomic_long start;
    long             end;
};
struct dataPair {
    std::atomic_long ntask;
    union {
        forall_task_t task;
        char padding[CACHE_LINE_SIZE];
    };
    dataPair():ntask(0),task() {};
    dataPair(const dataPair &d):ntask(d.ntask.load(std::memory_order_relaxed)),task(d.task) {}
    dataPair& operator=(const dataPair &d) { ntask=d.ntask.load(std::memory_order_relaxed), task=d.task; return *this; }
};


//  used just to redefine losetime_in
class foralllb_t: public ff_loadbalancer {
protected:
    virtual inline void losetime_in() {
        if ((int)(getnworkers())>=ncores) {
            //FFTRACE(lostpopticks+=(100*TICKS2WAIT);++popwait); // FIX: adjust tracing
            ff_relax(0);
            return;
        }
        //FFTRACE(lostpushticks+=TICKS2WAIT;++pushwait);
        PAUSE();
    }
public:
    foralllb_t(size_t n):ff_loadbalancer(n),ncores(ff_numCores()) {}
    inline int getNCores() const { return ncores;}
private:
    const int ncores;
};

// compare functiong
static inline bool data_cmp(const dataPair &a,const dataPair &b) {
    return a.ntask < b.ntask;
}
// delay function for worker threads
static inline void workerlosetime_in(const bool aggressive) {
    if (aggressive) PAUSE();
    else ff_relax(0);
}


// parallel for/reduce task scheduler
class forall_Scheduler: public ff_node {
protected:
    std::vector<bool>      eossent;
    std::vector<dataPair>  data;
    std::atomic_long       maxid;
protected:
    // initialize the data vector
    virtual inline size_t init_data(long start, long stop) {
        const long numtasks  = std::ceil((stop-start)/(double)_step);
        long totalnumtasks   = std::ceil(numtasks/(double)_chunk);
        long tt     = totalnumtasks;
        size_t ntxw = totalnumtasks / _nw;
        size_t r    = totalnumtasks % _nw;

        // try to keep the n. of tasks per worker as smaller as possible
        if (ntxw == 0 && r>=1) {  ntxw = 1, r = 0; }

        data.resize(_nw); eossent.resize(_nw);
        taskv.resize(8*_nw); // 8 is the maximum n. of jumps, see the heuristic below
        skip1=false,jump=0,maxid=-1;

        long end, t=0, e;
        for(size_t i=0;i<_nw && totalnumtasks>0;++i, totalnumtasks-=t) {
            t       = ntxw + ( (r>1 && (i<r)) ? 1 : 0 );
            e       = start + (t*_chunk - 1)*_step + 1;
            end     = (e<stop) ? e : stop;
            data[i].ntask=t;
            data[i].task.set(start,end);
            start   = (end-1)+_step;
        }

        if (totalnumtasks) {
            assert(totalnumtasks==1);
            // try to keep the n. of tasks per worker as smaller as possible
            if (ntxw > 1) data[_nw-1].ntask += totalnumtasks;
            else { --tt, _chunk*=2; }
            data[_nw-1].task.end = stop;
        }
         // printf("init_data\n");
         // for(size_t i=0;i<_nw;++i) {
         //      printf("W=%d %ld <%ld,%ld>\n", i, data[i].ntask, data[i].task.start, data[i].task.end);
         // }
         // printf("totaltasks=%ld\n", tt);
        return tt;
    }
    // initialize the data vector
    virtual inline size_t init_data_static(long start, long stop) {
        const long numtasks  = (stop-start)/_step;
        long totalnumtasks   = (long)_nw;
        size_t r             = numtasks % _nw;
        _chunk               = numtasks / _nw;

        data.resize(_nw); taskv.resize(_nw);eossent.resize(_nw);
        skip1=false,jump=0,maxid=-1;

        long end, e;
        for(size_t i=0; totalnumtasks>0; ++i,--totalnumtasks) {
            e       = start + (_chunk - 1)*_step + 1 + ((i<r) ? 1 : 0 );
            end     = (e<stop) ? e : stop;
            data[i].ntask=1;
            data[i].task.set(start,end);
            start   = (end-1)+_step;
        }
        if (r) ++_chunk;
        return _nw;
    }
public:
    forall_Scheduler(ff_loadbalancer* lb, long start, long stop, long step, long chunk, size_t nw):
        maxid(-1),lb(lb),_start(start),_stop(stop),_step(step),_chunk(chunk),totaltasks(0),_nw(nw),
        jump(0),skip1(false),workersspinwait(false) {
        if (_chunk<=0) totaltasks = init_data_static(start,stop);
        else           totaltasks = init_data(start,stop);
        assert(totaltasks>=1);
    }
    forall_Scheduler(ff_loadbalancer* lb, size_t nw):
        maxid(-1),lb(lb),_start(0),_stop(0),_step(1),_chunk(1),totaltasks(0),_nw(nw),
        jump(0),skip1(false) {
        totaltasks = init_data(0,0);
        assert(totaltasks==0);
    }

    inline bool sendTask(const bool skipmore=false) {
        size_t remaining    = totaltasks;
        const long endchunk = (_chunk-1)*_step + 1;

    more:
        for(size_t wid=0;wid<_nw;++wid) {
            if (data[wid].ntask >0) {
                long start = data[wid].task.start;
                long end   = std::min(start+endchunk, data[wid].task.end);
                taskv[wid+jump].set(start, end);
                lb->ff_send_out_to(&taskv[wid+jump], wid);
                --remaining, --data[wid].ntask;
                (data[wid].task).start = (end-1)+_step;
                eossent[wid]=false;
            } else  skip1=true; //skip2=skip3=true;
        }
        // January 2014 (massimo): this heuristic maight not be the best option in presence
        // of very high load imbalance between iterations.
        // Update: removed skip2 and skip3 so that it is less aggressive !

        jump+=_nw;
        assert((jump / _nw) <= 8);
        // heuristic: try to assign more task at the very beginning
        if (!skipmore && !skip1 && totaltasks>=4*_nw)   { skip1=true; goto more;}
        //if (!skip2 && totaltasks>=64*_nw)  { skip1=false; skip2=true; goto moretask;}
        //if (!skip3 && totaltasks>=1024*_nw){ skip1=false; skip2=false; skip3=true; goto moretask;}
        return (remaining>0);
    }

    inline void sendWakeUp() {
        for(size_t id=0;id<_nw;++id) {
            taskv[id].set(0,0);
            lb->ff_send_out_to(&taskv[id], id);
        }
    }

    // this method is accessed concurrently by all worker threads
    inline bool nextTaskConcurrent(forall_task_t *task, const int wid) {
        const long endchunk = (_chunk-1)*_step + 1; // next end-point
        auto id  = wid;
    L1:
        if (data[id].ntask.load(std::memory_order_acquire)>0) {
            auto oldstart = data[id].task.start.load(std::memory_order_relaxed);
            auto end      = std::min(oldstart+endchunk, data[id].task.end);
            auto newstart = (end-1)+_step;

            if (!data[id].task.start.compare_exchange_weak(oldstart, newstart,
                                                           std::memory_order_release,
                                                           std::memory_order_relaxed)) {
                workerlosetime_in(_nw <= lb->getnworkers());
                goto L1; // restart the sequence from the beginning
            }

            // after fetch_sub ntask may be less than 0
            data[id].ntask.fetch_sub(1,std::memory_order_release);
            if (oldstart<end) { // it might be possible that oldstart == end
                task->set(oldstart, end);
                return true;
            }
        }

        // no available task for the current thread
#if !defined(PARFOR_MULTIPLE_TASKS_STEALING)
        // the following scheduling policy for the tasks focuses mostly to load-balancing
        long _maxid = 0, ntask = 0;
        if (maxid.load(std::memory_order_acquire)<0)
            _maxid = (std::max_element(data.begin(),data.end(),data_cmp) - data.begin());
        else _maxid = maxid;
        ntask  = data[_maxid].ntask.load(std::memory_order_relaxed);
        if (ntask>0) {
            if (_maxid != maxid) maxid.store(_maxid, std::memory_order_release);
            id = _maxid;
            goto L1;
        }
        // no more tasks, exit

#else
        // the following scheduling policy for the tasks is a little bit more
        // complex and costly. It tries to find a trade-off between
        // task-to-thread localy and load-balancing by moving a bunch of tasks
        // from one thread to another one
        long _maxid = 0, ntask = 0;
        if (maxid.load(std::memory_order_acquire)<0)
            _maxid = (std::max_element(data.begin(),data.end(),data_cmp) - data.begin());
        else _maxid = maxid;
    L2:
        ntask  = data[_maxid].ntask.load(std::memory_order_relaxed);
        if (ntask>0) {
            if (_maxid != maxid) maxid.store(_maxid, std::memory_order_release);
            if (ntask<=3) { id = _maxid; goto L1; }

            // try to steal half of the tasks remaining to _maxid

            auto oldstart = data[_maxid].task.start.load(std::memory_order_relaxed);
            auto q = ((data[_maxid].task.end-oldstart)/_chunk) >> 1;
            if (q<=3) { id = _maxid; goto L1; }
            auto newstart = oldstart + (q*_chunk-1)*_step +1;
            if (!data[_maxid].task.start.compare_exchange_weak(oldstart, newstart,
                                                               std::memory_order_release,
                                                               std::memory_order_relaxed)) {
                workerlosetime_in(_nw <= lb->getnworkers());
                goto L2; // restart the sequence from the beginning
            }
            assert(newstart <= data[_maxid].task.end);

            data[_maxid].ntask.fetch_sub(q, std::memory_order_release);
            data[wid].task.start.store(oldstart, std::memory_order_relaxed);
            data[wid].task.end = newstart;
            data[wid].ntask.store(q, std::memory_order_release);
            id = wid;
            goto L1;
        }
#endif
        return false;
    }

    inline bool nextTask(forall_task_t *task, const int wid) {
        const long endchunk = (_chunk-1)*_step + 1;
        int id  = wid;
        if (data[id].ntask) {
        L1:
            long start = data[id].task.start;
            long end = std::min(start+endchunk, data[id].task.end);
            --data[id].ntask, (data[id].task).start = (end-1)+_step;
            task->set(start, end);
            return true;
        }
        // no available task for the current thread
#if !defined(PARFOR_MULTIPLE_TASKS_STEALING)
        // the following scheduling policy for the tasks focuses mostly to load-balancing
        if (maxid<0)  { //check if maxid has been set
        L2:
            maxid = (std::max_element(data.begin(),data.end(),data_cmp) - data.begin());
            if (data[maxid].ntask > 0) {
                id=maxid;
                goto L1;
            }
            // no more tasks, exit
        } else {
            if (data[maxid].ntask > 0) {
                id=maxid;
                goto L1;
            }
            goto L2;
        }
#else
        auto flag=false;
        if (maxid<0)  {
        L2:
            maxid = (std::max_element(data.begin(),data.end(),data_cmp) - data.begin());
            flag=true;
        }
        id = maxid;
        if (data[id].ntask>0) {
            if (data[id].ntask<=3) goto L1;

            // steal half of the tasks
            auto q = data[id].ntask >> 1, r = data[id].ntask & 0x1;
            data[id].ntask  = q;
            data[wid].ntask = q+r;
            data[wid].task.end   = data[id].task.end;
            data[id].task.end    = data[id].task.start + (q*_chunk-1)*_step +1;
            data[wid].task.start = data[id].task.end;
            id = wid;
            goto L1;
        } else if (!flag) goto L2;
#endif
        return false;
    }

    inline void* svc(void* t) {
        if (t==NULL) {
            if (totaltasks==0) { lb->broadcast_task(GO_OUT); return GO_OUT;}
            sendTask();
            return GO_ON;
        }
        auto wid =  lb->get_channel_id();
        if (--totaltasks <=0) {
            if (!eossent[wid]) {
                lb->ff_send_out_to(workersspinwait?EOS_NOFREEZE:GO_OUT, wid);
                eossent[wid]=true;
                //printf("SCHED EXIT, GO_OUT to %d\n",wid);
            } //else printf("SCHED_EXIT\n");
            //fflush(stdout);
            return GO_OUT;
        }
        if (nextTask((forall_task_t*)t, wid)) lb->ff_send_out_to(t, wid);
        else  {
            if (!eossent[wid]) {

                //auto task = (forall_task_t*)t;
                //printf("SCHED GO_OUT to %d (%ld) [%ld,%ld[\n", wid, totaltasks, task->start.load(), task->end);
                //fflush(stdout);


                lb->ff_send_out_to((workersspinwait?EOS_NOFREEZE:GO_OUT), wid);
                eossent[wid]=true;
            }
        }
        return GO_ON;
    }

    inline void setloop(long start, long stop, long step, long chunk, size_t nw) {
        _start=start, _stop=stop, _step=step, _chunk=chunk, _nw=nw;
        if (_chunk<=0) totaltasks = init_data_static(start,stop);
        else           totaltasks = init_data(start,stop);

        assert(totaltasks>=1);
        // adjust the number of workers that have to be started
        if ( (totaltasks/(double)_nw) <= 1.0 || (totaltasks==1) )
           _nw = totaltasks;
    }

    inline long startIdx() const { return _start;}
    inline long stopIdx()  const { return _stop;}
    inline long stepIdx()  const { return _step;}
    inline size_t running() const { return _nw; }
    inline void workersSpinWait() { workersspinwait=true;}
protected:
    // the following fields are used only by the scheduler thread
    ff_loadbalancer *lb;
    long             _start,_stop,_step;  // for step
    long             _chunk;              // a chunk of indexes
    size_t           totaltasks;          // total n. of tasks
    size_t           _nw;                 // num. of workers
    long             jump;
    bool             skip1;
    bool             workersspinwait;
    std::vector<forall_task_t> taskv;
};

// parallel for/reduce  worker node
template<typename Tres>
class forallreduce_W: public ff_node {
protected:
    typedef std::function<void(const long,const long, const int, Tres&)> F_t;
protected:
    virtual inline void losetime_in(void) {
        //FFTRACE(lostpopticks+=ff_node::TICKS2WAIT; ++popwait); // FIX
        workerlosetime_in(aggressive);
    }
public:
    forallreduce_W(forall_Scheduler *const sched, Barrier *const loopbar, F_t F):
        sched(sched),loopbar(loopbar), schedRunning(true),
        spinwait(false), aggressive(true),F(F) {}

    inline void setSchedRunning(bool r) { schedRunning = r; }

    inline void* svc(void* t) {
        auto task = (forall_task_t*)t;
        auto myid = get_my_id();

        //printf("Worker%d (%ld,%ld(\n", myid, task->start.load(),task->end);
        //fflush(stdout);

        F(task->start,task->end,myid,res);
        if (schedRunning) return t;

        // the code below is executed only if the scheduler thread is not running
        while(sched->nextTaskConcurrent(task,myid))
            F(task->start,task->end,myid,res);

        if (spinwait) {
            loopbar->doBarrier(myid);
            return GO_ON;
        }
        return GO_OUT;
    }

    inline void enableSpinWait() {  spinwait=true; }

    inline void setF(F_t _F, const Tres idtt, bool a=true) {
        F=_F, res=idtt, aggressive=a;
    }
    inline const Tres& getres() const { return res; }

private:
    forall_Scheduler *const sched;
    Barrier *const loopbar;
    bool schedRunning;
protected:
    bool spinwait,aggressive;
    F_t  F;
    Tres res;
};

template <typename Tres>
class ff_forall_farm: public ff_farm<foralllb_t> {
    typedef std::function<void(const long,const long, const int, Tres&)> F_t;
protected:
    // removes possible EOS still in the input queues of the workers
    inline void resetqueues(const int _nw) {
        const svector<fix8/ff_node*> nodes = getWorkers();
        for(int i=0;i<_nw;++i) nodes[i]->reset();
    }

private:
    Tres t; // not used
    size_t numCores;
    Barrier loopbar;
public:

    ff_forall_farm(ssize_t maxnw, const bool spinwait=false, const bool skipwarmup=false):
        ff_farm<foralllb_t>(false,8*ff_farm<>::DEF_MAX_NUM_WORKERS,8*ff_farm<>::DEF_MAX_NUM_WORKERS,
                            true, ff_farm<>::DEF_MAX_NUM_WORKERS,true),
        skipwarmup(skipwarmup),spinwait(spinwait) {
        numCores = ((foralllb_t*const)getlb())->getNCores();
        if (maxnw<0) maxnw=numCores;
        std::vector<fix8/ff_node *> forall_w;
        auto donothing=[](const long,const long,const int,const Tres) -> int {
            return 0;
        };
        forall_Scheduler *sched = new forall_Scheduler(getlb(),maxnw);
        ff_farm<foralllb_t>::add_emitter(sched);
        for(size_t i=0;i<(size_t)maxnw;++i)
            forall_w.push_back(new forallreduce_W<Tres>(sched, &loopbar, donothing));
        ff_farm<foralllb_t>::add_workers(forall_w);
        ff_farm<foralllb_t>::wrap_around();

        // needed to avoid the initial barrier (see (**) below)
        if (ff_farm<foralllb_t>::prepare() < 0)
            error("running base forall farm(2)\n");

        if (!skipwarmup) {
            auto r=-1;
            getlb()->freeze();
            if (getlb()->run() != -1)
                r = getlb()->wait_freezing();
            if (r<0) error("running base forall farm(1)\n");
        }

        if (spinwait) {
            sched->workersSpinWait();
            for(size_t i=0;i<(size_t)maxnw;++i) {
                auto w = (forallreduce_W<Tres>*)forall_w[i];
                w->enableSpinWait();
            }
            //resetqueues(maxnw);
        }
    }

    // It returns true if the scheduler has to be started, false otherwise
    //
    // Unless the removeSched flag is set, the scheduler thread will be started
    // only if there are less threads than cores AND if the number of tasks per thread
    // is greather than 1.
    //
    // By defining at compile time NO_PARFOR_SCHEDULER_THREAD the
    // scheduler won't be started.
    // To always start the scheduler thread, the PARFOR_SCHEDULER_THREAD
    // may be defined at compile time.
    inline bool startScheduler(const size_t nwtostart) const {
#if   defined(NO_PARFOR_SCHEDULER_THREAD)
        return false;
#elif defined(PARFOR_SCHEDULER_THREAD)
        return true;
#else
        if (removeSched) return false;
        return (nwtostart < numCores);
#endif
    }
    // set/reset removeSched flag
    // By calling this method with 'true' the scheduler will be disabled.
    //
    // NOTE:
    // Sometimes may be usefull (in terms of performance) to explicitly disable
    // the scheduler thread when #numworkers > ff_realNumCores() on systems where
    // ff_numCores() > ff_realNumCores() (i.e. HT or HMT is enabled)
    inline void disableScheduler(bool onoff=true) { removeSched=onoff; }

    inline int run_then_freeze(ssize_t nw_=-1) {
        assert(skipwarmup == false); // TODO
        const ssize_t nwtostart = (nw_ == -1)?getNWorkers():nw_;
        auto r = -1;
        if (startScheduler(nwtostart)) {
            getlb()->skipfirstpop();
            if (spinwait) {
                // NOTE: here we have to be sure to send one task to each worker!
                ((forall_Scheduler*)getEmitter())->sendTask(true);
            }
            r=ff_farm<foralllb_t>::run_then_freeze(nwtostart);
        } else {
            if (spinwait) {
                // all worker threads have already crossed the barrier so it is safe to restart it
                loopbar.barrierSetup(nwtostart+1);
                ((forall_Scheduler*)getEmitter())->sendWakeUp();
            } else
                ((forall_Scheduler*)getEmitter())->sendTask(true);

            r = getlb()->thawWorkers(true, nwtostart);
        }
        return r;
    }
    int run_and_wait_end() {
        assert(spinwait == false);
        const size_t nwtostart = getnw();
        auto r= -1;
        if (startScheduler(nwtostart)) {
            //resetqueues(nwtostart);
            getlb()->skipfirstpop();
            // (**) this way we avoid the initial barrier
            if (getlb()->runlb()!= -1) {
                if (getlb()->runWorkers(nwtostart)!=-1)
                    r = getlb()->wait();
            }
        } else {
            ((forall_Scheduler*)getEmitter())->sendTask(true);
            if (getlb()->runWorkers(nwtostart) != -1)
                r = getlb()->waitWorkers();
        }
        return r;
    }

    inline int wait_freezing() {
        if (startScheduler(getnw())) return getlb()->wait_lb_freezing();
        if (spinwait) {
            loopbar.doBarrier(getnw()+1);
            return 0;
        }
        return getlb()->wait_freezingWorkers();
    }

    inline int wait() {
        if (spinwait){
            const svector<fix8/ff_node*> &nodes = getWorkers();
            for(size_t i=0;i<nodes.size();++i)
                getlb()->ff_send_out_to(EOS,i);
        }
        return ff_farm<foralllb_t>::wait();
    }

    inline void setF(F_t  _F, const Tres idtt=(Tres)0) {
        const size_t nw                = getnw();
        const svector<fix8/ff_node*> &nodes = getWorkers();
        // aggressive mode enabled if the number of threads is less than
        // or equal to the number of cores
        const bool mode = (nw <= numCores)?true:false;
        if (startScheduler(nw))  {
            for(size_t i=0;i<nw;++i) {
                auto w = (forallreduce_W<Tres>*)nodes[i];
                w->setF(_F, idtt, mode);
                w->setSchedRunning(true);
            }
        } else {
            for(size_t i=0;i<nw;++i) {
                auto w = (forallreduce_W<Tres>*)nodes[i];
                w->setF(_F, idtt, mode);
                w->setSchedRunning(false);
            }
        }
    }
    inline void setloop(long begin,long end,long step,long chunk,long nw) {
        assert(nw<=(ssize_t)getNWorkers());
        forall_Scheduler *sched = (forall_Scheduler*)getEmitter();
        sched->setloop(begin,end,step,chunk,(nw<=0)?getNWorkers():(size_t)nw);
    }
    // return the number of workers running or supposed to run
    inline size_t getnw() { return ((const forall_Scheduler*)getEmitter())->running(); }

    inline const Tres& getres(int i) {
        return  ((forallreduce_W<Tres>*)(getWorkers()[i]))->getres();
    }
    inline long startIdx(){ return ((const forall_Scheduler*)getEmitter())->startIdx(); }
    inline long stopIdx() { return ((const forall_Scheduler*)getEmitter())->stopIdx(); }
    inline long stepIdx() { return ((const forall_Scheduler*)getEmitter())->stepIdx(); }
protected:
    bool   removeSched = false;
    bool   skipwarmup  = false;
    bool   spinwait    = false;
};

/* --------------------- Function interface -------------------- */
//! Parallel iteration over a range of indexes (step=1)
template <typename Function>
static void parallel_for(long first, long last, const Function& body,
                         const long nw=-1) {
    FF_PARFOR_BEGIN(pfor, parforidx,first,last,1,-1,nw) {
        body(parforidx);
    } FF_PARFOR_END(pfor);
}
//! Parallel iteration over a range of indexes using a given step
template <typename Function>
static void parallel_for(long first, long last, long step, const Function& body,
                         const long nw=-1) {
    FF_PARFOR_BEGIN(pfor, parforidx,first,last,step,-1,nw) {
        body(parforidx);
    } FF_PARFOR_END(pfor);
}
//! Parallel iteration over a range of indexes using a given step and granularity
template <typename Function>
static void parallel_for(long first, long last, long step, long grain,
                         const Function& body, const long nw=-1) {
    FF_PARFOR_BEGIN(pfor, parforidx,first,last,step,grain,nw) {
        body(parforidx);
    } FF_PARFOR_END(pfor);
}

template <typename Function, typename Value, typename FReduction>
void parallel_reduce(Value& var, const Value& identity,
                     long first, long last,
                     const Function& body, const FReduction& finalreduce,
                     const long nw=-1) {
    Value _var = var;
    FF_PARFORREDUCE_BEGIN(pfr, _var, identity, parforidx, first, last, 1, -1, nw) {
        body(parforidx, _var);
    } FF_PARFORREDUCE_F_END(pfr, _var, finalreduce);
    var=_var;
}



//! ParallelFor class
class ParallelFor {
protected:
    ff_forall_farm<int> * pf;
public:
    ParallelFor(const long maxnw=-1,bool spinwait=false, bool skipwarmup=false):
        pf(new ff_forall_farm<int>(maxnw,spinwait,skipwarmup)) {}

    ~ParallelFor()                { FF_PARFOR_DONE(pf); }

    // By calling this method with 'true' the scheduler will be disabled,
    // to restore the usage of the scheduler thread just pass 'false' as
    // parameter
    inline void disableScheduler(bool onoff=true) {
        pf->disableScheduler(onoff);
    }

    template <typename Function>
    inline void parallel_for(long first, long last, const Function& f,
                             const long nw=-1) {
        FF_PARFOR_START(pf, parforidx,first,last,1,-1,nw) {
            f(parforidx);
        } FF_PARFOR_STOP(pf);
    }
    template <typename Function>
    inline void parallel_for(long first, long last, long step, const Function& f,
                             const long nw=-1) {
        FF_PARFOR_START(pf, parforidx,first,last,step,-1,nw) {
            f(parforidx);
        } FF_PARFOR_STOP(pf);
    }
    template <typename Function>
    inline void parallel_for(long first, long last, long step, long grain,
                             const Function& f, const long nw=-1) {
        FF_PARFOR_START(pf, parforidx,first,last,step,grain,nw) {
            f(parforidx);
        } FF_PARFOR_STOP(pf);
    }
};

//! ParallelForReduce class
template<typename T>
class ParallelForReduce {
protected:
    ff_forall_farm<T> * pfr;
public:
    ParallelForReduce(const long maxnw=-1, bool spinwait=false, bool skipwarmup=false):
        pfr(new ff_forall_farm<T>(maxnw,spinwait,skipwarmup)) {}

    ~ParallelForReduce()                { FF_PARFORREDUCE_DONE(pfr); }

    // By calling this method with 'true' the scheduler will be disabled,
    // to restore the usage of the scheduler thread just pass 'false' as
    // parameter
    inline void disableScheduler(bool onoff=true) {
        pfr->removeSched(onoff);
    }

    /* -------------------- parallel_for -------------------- */
    template <typename Function>
    inline void parallel_for(long first, long last, const Function& f,
                             const long nw=-1) {
        FF_PARFOR_T_START(pfr, T, parforidx,first,last,1,-1,nw) {
            f(parforidx);
        } FF_PARFOR_STOP(pfr);
    }
    template <typename Function>
    inline void parallel_for(long first, long last, long step, const Function& f,
                             const long nw=-1) {
        FF_PARFOR_T_START(pfr, T, parforidx,first,last,step,-1,nw) {
            f(parforidx);
        } FF_PARFOR_STOP(pfr);
    }
    template <typename Function>
    inline void parallel_for(long first, long last, long step, long grain,
                             const Function& f, const long nw=-1) {
        FF_PARFOR_T_START(pfr, T, parforidx,first,last,step,grain,nw) {
            f(parforidx);
        } FF_PARFOR_STOP(pfr);
    }

    /* ------------------ parallel_reduce ------------------- */

    template <typename Function, typename Value, typename FReduction>
    inline void parallel_reduce(Value& var, const Value& identity,
                                long first, long last,
                                const Function& body, const FReduction& finalreduce,
                                const long nw=-1) {
        FF_PARFORREDUCE_START(pfr, var, identity, parforidx, first, last, 1, -1, nw) {
            body(parforidx, var);
        } FF_PARFORREDUCE_F_STOP(pfr, var, finalreduce);
    }
    template <typename Function, typename Value, typename FReduction>
    inline void parallel_reduce(Value& var, const Value& identity,
                                long first, long last, long step,
                                const Function& body, const FReduction& finalreduce,
                                const long nw=-1) {
        FF_PARFORREDUCE_START(pfr, var, identity, parforidx,first,last,step,-1,nw) {
            body(parforidx, var);
        } FF_PARFORREDUCE_F_STOP(pfr, var, finalreduce);
    }
    template <typename Function, typename Value, typename FReduction>
    inline void parallel_reduce(Value& var, const Value& identity,
                                long first, long last, long step, long grain,
                                const Function& body, const FReduction& finalreduce,
                                const long nw=-1) {
        FF_PARFORREDUCE_START(pfr, var, identity, parforidx,first,last,step,grain,nw) {
            body(parforidx, var);
        } FF_PARFORREDUCE_F_STOP(pfr, var, finalreduce);
    }
};


} // namespace ff

#endif /* FF_PARFOR_HPP */

