/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*! 
 *  \link
 *  \file parallel_for.hpp
 *  \ingroup high_level_patterns_shared_memory
 *
 *  \brief This file describes the parallel_for/parallel_reduce skeletons.
 */
 
#ifndef _FF_PARFOR_HPP_
#define _FF_PARFOR_HPP_
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


// #ifndef __INTEL_COMPILER
// // see http://www.stroustrup.com/C++11FAQ.html#11
// #if __cplusplus <= 199711L
// #error "parallel_for requires C++11 features"
// #endif
// #endif

#include <vector>
#include <cmath>
#include <functional>
#include <ff/lb.hpp>
#include <ff/node.hpp>
#include <ff/farm.hpp>


#if defined(__ICC)
#define PRAGMA_IVDEP _Pragma("ivdep")
#else
#define PRAGMA_IVDEP
#endif

namespace ff {

    /* -------------------- Parallel For/Reduce Macros -------------------- */
    /* Usage example:
     *                                 // loop parallelization using 3 workers
     *  for(int i=0;i<N;++i)           FF_PARFOR_BEGIN(for,i,0,N,1,1,3) {
     *    A[i]=f(i)            ---->      A[i]=f(i);
     *                                 } FF_PARFOR_END(for);
     * 
     *   parallel for + reduction:
     *     
     *  s=0;                           // loop parallelization using 3 workers
     *  for(int i=0;i<N;++i)           FF_PARFOR_BEGIN(for,s,i,0,N,1,1,3) {
     *    s*=f(i)              ---->      s*=f(i);
     *                                 } FF_PARFOR_END(for,s,*);
     *
     *
     *  NOTE: inside the body of the PARFOR it is possible to use the 
     *        'ff_thread_id' const integer variable to identify the thread id 
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
    ff_farm<>   forall_##name;                                                    \
    forall_##name.cleanup_workers();                                              \
    const int nw_##name = nw;                                                     \
    forall_##name.add_emitter(                                                    \
          new forall_Scheduler(forall_##name.getlb(),begin,end, step,chunk,nw));  \
    forall_##name.wrap_around();                                                  \
    auto F_##name = [&] (const long ff_start_##idx, const long ff_stop_##idx,     \
                         const int _ff_thread_id) -> int  {                       \
        PRAGMA_IVDEP                                                              \
        for(long idx=ff_start_##idx;idx<ff_stop_##idx;idx+=step) 

    /* This is equivalent to the above one except that the user has to define
     * the for loop in the range (ff_start_idx,ff_stop_idx(
     * This can be useful if you have to perform some actions before starting
     * the loop.
     */
#define FF_PARFOR2_BEGIN(name, idx, begin, end, step, chunk, nw)                  \
    ff_farm<>   forall_##name;                                                    \
    forall_##name.cleanup_workers();                                              \
    const int nw_##name = nw;                                                     \
    forall_##name.add_emitter(                                                    \
          new forall_Scheduler(forall_##name.getlb(),begin,end, step,chunk,nw));  \
    forall_##name.wrap_around();                                                  \
    auto F_##name = [&] (const long ff_start_##idx, const long ff_stop_##idx,     \
                         const int _ff_thread_id) -> int  {                       \
    /* here you have to define the for loop using ff_start/stop_idx  */


#define FF_PARFOR_END(name)                                                       \
    return 0;                                                                     \
    };                                                                            \
    {                                                                             \
        std::vector<ff_node *> forall_w_##name;                                   \
        for(int i=0;i<nw_##name;++i)                                              \
            forall_w_##name.push_back(new forallreduce_W<int>(F_##name)); \
        forall_##name.add_workers(forall_w_##name);                               \
        if (forall_##name.run_and_wait_end()<0) {                                 \
            error("running forall_##name\n");                                     \
        }                                                                         \
        /*forall_##name.~ff_farm<>();*/                                           \
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
    ff_farm<>   forall_##name;                                                    \
    forall_##name.cleanup_workers();                                              \
    const int nw_##name = nw;                                                     \
    forall_##name.add_emitter(                                                    \
          new forall_Scheduler(forall_##name.getlb(),begin,end, step,chunk,nw));  \
    forall_##name.wrap_around();                                                  \
    auto name_ovar = var; var=identity; /* icc does not support [var=identity]*/  \
    auto F_##name =[&,var](const long start, const long stop,                     \
                           const int _ff_thread_id) mutable ->                    \
        decltype(var) {                                                           \
        PRAGMA_IVDEP                                                              \
          for(long idx=start;idx<stop;idx+=step) 

#define FF_PARFORREDUCE_END(name, var, op)                                        \
          return var;                                                             \
        };                                                                        \
    {                                                                             \
        std::vector<ff_node *> forall_w_##name;                                   \
        for(int i=0;i<nw_##name;++i)                                              \
          forall_w_##name.push_back(new forallreduce_W<decltype(var)>(F_##name)); \
        forall_##name.add_workers(forall_w_##name);                               \
        if (forall_##name.run_and_wait_end()<0) {                                 \
            error("running forall_##name\n");                                     \
            return -1;                                                            \
        }                                                                         \
        var = ovar;                                                               \
        for(int i=0;i<nw_##name;++i)                                              \
            var op##= ((forallreduce_W<decltype(var)>*)                           \
                                 (forall_w_##name[i]))->getres();                 \
    }

    /* ---------------------------------------------- */

    /* FF_PARFOR_START and FF_PARFOR_STOP have the same meaning of 
     * FF_PARFOR_BEGIN and FF_PARFOR_END but they have to be used in 
     * conjunction with  FF_PARFOR_INIT FF_PARFOR_END.
     *
     * The same is for FF_PARFORREDUCE_START/END.
     */
#define FF_PARFOR_INIT(name, nw)                                                  \
    ff_forall_farm<int> *name = new ff_forall_farm<int>(nw);                      \
    WARMUP(name)

#define FF_PARFOR_DECL(name)      ff_forall_farm<int> * name
#define FF_PARFOR_ASSIGN(name,nw) name=new ff_forall_farm<int>(nw)

#define FF_PARFOR_DONE(name)                                                      \
    {                                                                             \
        auto donothing=[](const long,const long, const int) ->                    \
            int { return 0;};                                                     \
        name->setF(donothing);                                                    \
        name->run_and_wait_end();                                                 \
    }

#define FF_PARFORREDUCE_INIT(name, type, nw)                                      \
    ff_forall_farm<type> *name = new ff_forall_farm<type>(nw);                    \
    WARMUP(name)

#define FF_PARFORREDUCE_DECL(name,type)      ff_forall_farm<type> * name
#define FF_PARFORREDUCE_ASSIGN(name,type,nw) name=new ff_forall_farm<type>(nw)
    
#define FF_PARFORREDUCE_DONE(name)                                                \
    {                                                                             \
        auto donothing=[](const long,const long,const int) ->                     \
            decltype(name->t) { return 0;  };                                     \
        name->setF(donothing);                                                    \
        name->run_and_wait_end();                                                 \
    }

#define FF_PARFOR_START(name, idx, begin, end, step, chunk, nw)                   \
    name->setloop(begin,end,step,chunk,nw);                                       \
    auto F_##name = [&] (const long ff_start_##idx, const long ff_stop_##idx,     \
                         const int _ff_thread_id) -> int  {                       \
        PRAGMA_IVDEP                                                              \
        for(long idx=ff_start_##idx;idx<ff_stop_##idx;idx+=step) 

#define FF_PARFOR2_START(name, idx, begin, end, step, chunk, nw)                  \
    name->setloop(begin,end,step,chunk,nw);                                       \
    auto F_##name = [&] (const long ff_start_##idx, const long ff_stop_##idx,     \
                         const int _ff_thread_id) -> int  {                       \
    /* here you have to define the for loop using ff_start/stop_idx  */


#define FF_PARFOR_STOP(name)                                                      \
    return 0;                                                                     \
    };                                                                            \
    name->setF(F_##name);                                                         \
    if (name->run_then_freeze(name->getnw())<0)                                   \
        error("running ff_forall_farm (name)\n");                                 \
    name->wait_freezing()
    
#define FF_PARFORREDUCE_START(name, var,identity, idx,begin,end,step, chunk, nw)  \
    name->setloop(begin,end,step,chunk,nw);                                       \
    auto name_ovar = var; var=identity; /* icc does not support [var=identity]*/  \
    auto F_##name =[&,var](const long start, const long stop,                     \
                           const int _ff_thread_id) mutable ->                    \
        decltype(var) {                                                           \
        PRAGMA_IVDEP                                                              \
          for(long idx=start;idx<stop;idx+=step) 

#define FF_PARFORREDUCE_STOP(name, var, op)                                       \
          return var;                                                             \
        };                                                                        \
        name->setF(F_##name);                                                     \
        if (name->run_then_freeze(name->getnw())<0)                               \
            error("running ff_forall_farm (name)\n");                             \
        name->wait_freezing();                                                    \
        var = name_ovar;                                                          \
        for(int i=0;i<name->getnw();++i)  {                                       \
            var op##= name->getres(i);                                            \
        }


/* gets the working time of a parallel for/reduce */
#define FF_PARFOR_WTIME(name)   forall_##name.ffwTime()
/* gets the total time of a parallel for/reduce */
#define FF_PARFOR_TIME(name)    forall_##name.ffTime()


    /* ------------------------------------------------------------------- */

    /* helper define */
#define WARMUP(name)                                                              \
    {                                                                             \
        auto donothing=[](const long,const long, const int) ->                    \
            int { return 0;};                                                     \
        name->setF(donothing);                                                    \
        if (name->run_then_freeze(name->getnw())<0)                               \
            error("running ff_forall_farm (name)\n");                             \
        name->wait_freezing();                                                    \
    }



    // parallel for task, it represents a range (start,end( of indexes
struct forall_task_t {
    forall_task_t():start(0),end(0) {}
    forall_task_t(long start, long end):start(start),end(end) {}
    forall_task_t(const forall_task_t &t):start(t.start),end(t.end) {}
    forall_task_t & operator=(const forall_task_t &t) { start=t.start,end=t.end; return *this; }
    void set(long s, long e)  { start=s,end=e; }

    long start;
    long end;
};

// parallel for/reduce  worker node
template<typename Tres>
class forallreduce_W: public ff_node {
public:
    forallreduce_W(std::function<Tres(const long,const long,const int)> F):F(F) {}

    inline void* svc(void* t) {
        auto task = (forall_task_t*)t;
        res = F(task->start,task->end,get_my_id());
        return t;
    }
    void setF(std::function<Tres(const long,const long,const int)> _F) { 
        F=_F;
    }
    Tres getres() const { return res; }
protected:
    std::function<Tres(const long,const long,const int)> F;
    Tres res;
};
// parallel for/reduce task scheduler
class forall_Scheduler: public ff_node {
protected:
    std::vector<bool> active;
    std::vector<std::pair<long,forall_task_t> > data;
    std::vector<forall_task_t> taskv;
protected:
    // initialize the data vector
    virtual inline long init_data(long start, long stop) {
        const long numtasks  = std::ceil((stop-start)/(double)step);
        long totalnumtasks   = std::ceil(numtasks/(double)chunk);
        long ntxw = totalnumtasks / nw;
        long r    = totalnumtasks % nw;
        long tt   = totalnumtasks;

        if (ntxw == 0 && r>1) {
            ntxw = 1, r = 0;
        }
        data.resize(nw);
        taskv.resize(8*nw); // 8 is the maximum n. of jumps, see the heuristic below
        
        long end, t=0, e;
        for(int i=0;i<nw && totalnumtasks>0;++i, totalnumtasks-=t) {
            t       = ntxw + ((r>1 && (i<r))?1:0);
            e       = start + (t*chunk - 1)*step + 1;
            end     = (e<stop) ? e : stop;
            data[i] = std::make_pair(t, forall_task_t(start,end));
            start   = (end-1)+step;
        }

        if (totalnumtasks) {
            assert(totalnumtasks==1);
            data[nw-1].first += totalnumtasks;
            data[nw-1].second.end = stop;
        } 
        
        // for(int i=0;i<nw;++i) {
        //     printf("W=%d %ld <%ld,%ld>\n", i, data[i].first, data[i].second.start, data[i].second.end);
        // }
        // printf("totaltasks=%ld\n", tt);

        return tt;
    }    
public:
    forall_Scheduler(ff_loadbalancer* lb, long start, long stop, long step, long chunk, int nw):
        active(nw),lb(lb),step(step),chunk(chunk),totaltasks(0),nw(nw) {
        totaltasks = init_data(start,stop);
        assert(totaltasks>=1);
    }
    forall_Scheduler(ff_loadbalancer* lb, int nw):active(nw),lb(lb),step(1),chunk(1),totaltasks(0),nw(nw) {
        totaltasks = init_data(0,0);
        assert(totaltasks==0);
    }

    void* svc(void* t) {
        if (t==NULL) {
            bool skip1=false,skip2=false,skip3=false;
            if (totaltasks==0) return NULL;
            const long endchunk = (chunk-1)*step + 1;
            int jump = 0;
        moretask:
            for(int wid=0;wid<nw;++wid) {
                if (data[wid].first) {
                    long start = data[wid].second.start;
                    long end   = std::min(start+endchunk, data[wid].second.end);
                    taskv[wid+jump].set(start, end);
                    lb->ff_send_out_to(&taskv[wid+jump], wid);
                    
                    --data[wid].first;
                    (data[wid].second).start = (end-1)+step;  
                    active[wid]=true;
                } else {
                    // if we do not have task at the beginning the thread is terminated
                    lb->ff_send_out_to(EOS, wid);
                    active[wid]=false;
                    skip1=skip2=skip3=true;
                }
            }
            jump+=nw;
            assert((jump / nw) <= 8);
            // heuristic: try to assign more task at the very beginning
            if (!skip1 && totaltasks>=10*nw)  { skip1=true; goto moretask;}
            if (!skip2 && totaltasks>=100*nw) { skip1=false; skip2=true; goto moretask;}
            if (!skip3 && totaltasks>=1000*nw){ skip1=false; skip2=false; skip3=true; goto moretask;}

            return GO_ON;
        }
        if (--totaltasks <=0) return NULL;
        auto task = (forall_task_t*)t;
        const long endchunk = (chunk-1)*step + 1;
        const int wid = lb->get_channel_id();
        int id  = wid;
        for(int cnt=0;cnt<nw;++cnt) { 
            if (data[id].first) {
                long start = data[id].second.start;
                long end   = std::min(start+endchunk, data[id].second.end);
                task->set(start, end);
                lb->ff_send_out_to(task, wid);
                --data[id].first;
                (data[id].second).start = (end-1)+step;
                return GO_ON;
            }
            // no task available, trying to steal a task 
            id = (id+1) % nw;
        } 
        if (active[wid]) {
            lb->ff_send_out_to(EOS, wid); // the thread is terminated
            active[wid]=false;
        }
        return GO_ON;
    }

    // void  svc_end() {
    //     assert(totaltasks<=0);
    // }

    inline void setloop(long start, long stop, long _step, long _chunk, int _nw) {
        step=_step, chunk=_chunk, nw = _nw;
        totaltasks = init_data(start,stop);
        assert(totaltasks>=1);        
    }

protected:
    ff_loadbalancer* lb;
    long             step;        // for step
    long             chunk;       // a chunk of indexes
    long             totaltasks;  // total n. of tasks
    int              nw;          // num. of workers
};

template <typename Tres>
class ff_forall_farm: public ff_farm<> {
protected:
    // allows to remove possible EOS still in the input/output queues 
    // of workers
    inline void resetqueues(const int _nw) {
        ff_node **nodes = getWorkers();
        for(int i=0;i<_nw;++i) nodes[i]->reset();
    }
public:
    Tres t; // not used

    ff_forall_farm(int nw):ff_farm<>(false,100*nw,100*nw,true,nw,true),nw(nw) {
        std::vector<ff_node *> forall_w;
        auto donothing=[](const long,const long, const int) -> int {
            return 0;
        };
        for(int i=0;i<nw;++i)
            forall_w.push_back(new forallreduce_W<Tres>(donothing));
        ff_farm<>::add_emitter(new forall_Scheduler(getlb(),nw));
        ff_farm<>::add_workers(forall_w);
        ff_farm<>::wrap_around();
        if (ff_farm<>::run_then_freeze()<0) {
            error("running base forall farm\n");
        } else ff_farm<>::wait_freezing();
    }
    int run_then_freeze(int nw=-1) {        
        resetqueues((nw == -1)?getNWorkers():nw);
        // the scheduler skips the first pop
        getlb()->skipfirstpop();
        return ff_farm<>::run_then_freeze(nw);
    }
    int run_and_wait_end() {
        resetqueues((nw == -1)?getNWorkers():nw);
        return ff_farm<>::run_and_wait_end();
    }

    inline void setF(std::function<Tres(const long,const long,const int)>  _F) { 
        const int nw = getNWorkers();
        ff_node **nodes = getWorkers();
        for(int i=0;i<nw;++i) ((forallreduce_W<Tres>*)nodes[i])->setF(_F);
    }
    inline void setloop(long begin,long end,long step,long chunk,int _nw) {
        assert(nw<=getNWorkers());
        nw=_nw;
        forall_Scheduler *sched = (forall_Scheduler*)getEmitter();
        sched->setloop(begin,end,step,chunk,nw);
    }
    inline int getnw() const { return nw; }
    
    inline Tres getres(int i) {
        return  ((forallreduce_W<Tres>*)(getWorkers()[i]))->getres();
    }
protected:
    int nw;
};

} // namespace ff

#endif /* _FF_PARFOR_HPP_ */
