/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*!
 *  \link
 *  \file mdf.hpp
 *  \ingroup high_level_patterns_shared_memory
 *
 *  \brief This file implements the macro dataflow pattern.
 */

#ifndef FF_MDF_HPP
#define FF_MDF_HPP
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
 * Author: Massimo Torquati (October 2013)
 *
 *
 * Acknowledgement:
 * This implementation is a refinement of the first implementation developed
 * at the Computer Science Department of University of Pisa in the early 2013
 * together with:
 *  - Daniele Buono       (d.buono@di.unipi.it)
 *  - Tiziano De Matteis  (dematteis@di.unipi.it)
 *  - Gabriele Mencagli   (mencagli@di.unipi.it)
 *
 */

#include <functional>
#include <tuple>
#include <vector>
#include <deque>
#include <queue>
#include <vector>
#include <fix8/ff/node.hpp>
#include <fix8/ff/pipeline.hpp>
#include <fix8/ff/farm.hpp>
#include <fix8/ff/allocator.hpp>
#include "./icl_hash.h"


namespace ff {

/* -------------- expanding tuple to func. arguments ------------------------- */
template<size_t N>
struct Apply {
    template<typename F, typename T, typename... A>
    static inline auto apply(F && f, T && t, A &&... a)
        -> decltype(Apply<N-1>::apply(
            ::std::forward<F>(f), ::std::forward<T>(t),
            ::std::get<N-1>(::std::forward<T>(t)), ::std::forward<A>(a)...))
    {
        return Apply<N-1>::apply(::std::forward<F>(f), ::std::forward<T>(t),
            ::std::get<N-1>(::std::forward<T>(t)), ::std::forward<A>(a)...
        );
    }
};

template<>
struct Apply<0> {
    template<typename F, typename T, typename... A>
    static inline auto apply(F && f, T &&, A &&... a)
        -> decltype(::std::forward<F>(f)(::std::forward<A>(a)...))
    {
        return ::std::forward<F>(f)(::std::forward<A>(a)...);
    }
};

template<typename F, typename T>
inline auto apply(F && f, T && t)
    -> decltype(Apply< ::std::tuple_size<typename ::std::decay<T>::type
					 >::value>::apply(::std::forward<F>(f), ::std::forward<T>(t)))
{
    return Apply< ::std::tuple_size<typename ::std::decay<T>::type>::value>::apply(::std::forward<F>(f), ::std::forward<T>(t));
}
/* --------------------------------------- */
/* ----------------------------------------------------------------------
 *Hashing funtions
 * Well known hash function: Fowler/Noll/Vo - 32 bit version
 */
static inline unsigned int fnv_hash_function( void *key, int len ) {
    unsigned char *p = (unsigned char*)key;
    unsigned int h = 2166136261u;
    int i;
    for ( i = 0; i < len; i++ )
        h = ( h * 16777619 ) ^ p[i];
    return h;
}
/**
 * Hash function to map addresses, cut into "long" size chunks, then
 * XOR. The result will be matched to hash table size using mod in the
 * hash table implementation
 */
static inline unsigned int address_hash_function(void *address) {
    int len = sizeof(void *);
    unsigned int hashval = fnv_hash_function( &address, len );
    return hashval;
}
/* Adress compare function for hash table */
static inline int address_key_compare(void *addr1, void *addr2) {
    return (addr1 == addr2);
}

static inline unsigned int ulong_hash_function( void *key ) {
    int len = sizeof(unsigned long);
    unsigned int hashval = fnv_hash_function( key, len );
    return hashval;
}
static inline int ulong_key_compare( void *key1, void *key2  ) {
    return ( *(unsigned long*)key1 == *(unsigned long*)key2 );
}

/* --------------------------------------- */

typedef enum {INPUT=0,OUTPUT=1,VALUE=2} data_direction_t;
struct param_info {
    uintptr_t        tag;  // unique tag for the parameter
    data_direction_t dir;
};


class ff_mdf:public ff_node {
public:
    enum {DEFAULT_OUTSTANDING_TASKS = 2048};
protected:
    typedef enum {NOT_READY, READY, DONE} status_t;
    struct base_f_t {
        virtual inline void call() {};
    };
    template<typename... Param>
    struct worker_task_t: public base_f_t {
        worker_task_t(void(*F)(Param...), Param&... a):F(F) {
            args = std::make_tuple(a...);
        }
        inline void call() { apply(F, args);  }
        void (*F)(Param...);
        std::tuple<Param...> args;
    };

    struct task_t {
        std::vector<param_info> P;  // svector could be used here
        base_f_t *wtask;
    };

    struct hash_task_t {
        union{
            struct {
                unsigned long id;
                base_f_t *wtask;
                status_t status;
                bool     is_dummy;
                //counter of dependencies that have to been yet satisfied
                long     remaining_dep;
                long     unblock_numb;
                long     num_out;
                //task list that have to be "unblocked"
                unsigned long *unblock_task_ids;
                long     unblock_act_numb;
            };
            char padding[64];
        };
    };

    /* --------------  graph descriptor ---------------------- */
    struct base_gd: public ff_node {
        virtual inline void setMaxTasks(size_t) {}
        virtual inline void activate(bool) {}
        virtual inline void alloc_and_send(std::vector<param_info> &, base_f_t *) {}
        virtual inline void thaw(bool freeze=false) {};
        virtual inline int  wait_freezing() { return 0; };
    };
    template<typename T>
    class GD: public base_gd {
    public:
        GD(void(*F)(T*const), T*const args):
            active(false),F(F),args(args),ntasks(0),maxMsgs(DEFAULT_OUTSTANDING_TASKS),TASKS(maxMsgs) {}

        void setMaxTasks(size_t maxtasks) {
            maxMsgs = maxtasks;
            TASKS.resize(maxMsgs);
        }
        void activate(bool a) { active=a;}
        void thaw(bool freeze=false) { ff_node::thaw(freeze); };
        int wait_freezing() { return ff_node::wait_freezing(); };

        inline void alloc_and_send(std::vector<param_info> &P, base_f_t *wtask) {
            task_t *task = &(TASKS[ntasks++ % maxMsgs]);
            task->P     = P;
            task->wtask = wtask;
            while(!ff_send_out(task, 1)) ff_relax(1);
        }
        void *svc(void *) {
            if (!active) return NULL;
            F(args);
            std::vector<param_info> useless;
            alloc_and_send(useless, NULL); // END task
            return NULL;
        }
    protected:
        bool active;
        void(*F)(T*const); // user's function
        T*const args;      // F's arguments
        unsigned long ntasks, maxMsgs;
        std::vector<task_t> TASKS;
    };

    /* --------------  generic workers ----------------------- */
    class Worker:public ff_node {
    public:
        inline void *svc(void *task) {
            hash_task_t *t = static_cast<hash_task_t*>(task);
            t->wtask->call();
            return task;
        }
    };

    /* --------------  scheduler ----------------------------- */
    class Scheduler: public ff_node {
    private:
        struct CompareTask {
            // Returns true if t1 is earlier than t2
            bool operator()(hash_task_t * &t1, hash_task_t* &t2) {
                if (t1->unblock_numb < t2->unblock_numb) return true;
                return false;
            }
        };
        typedef std::priority_queue<hash_task_t*, std::vector<hash_task_t*>, CompareTask> priority_queue_t;

    protected:
        enum { UNBLOCK_SIZE=16, TASK_PER_WORKER=128};
        enum { RELAX_MIN_BACKOFF=1, RELAX_MAX_BACKOFF=32};

#if !defined(DONT_USE_FFALLOC)
#define FFALLOC ffalloc->
#else
#define FFALLOC
#endif
#define MALLOC(size)          (FFALLOC malloc(size))
#define FREE(ptr)             (FFALLOC free(ptr))
#define REALLOC(ptr,newsize)  (FFALLOC realloc(ptr,newsize))

        inline hash_task_t* createTask(unsigned long id, status_t status, base_f_t *wtask) {
            hash_task_t *t=(hash_task_t*)MALLOC(sizeof(hash_task_t));

            t->id=id;  t->status=status;  t->remaining_dep=0;
            t->unblock_numb=0; t->wtask=wtask; t->is_dummy=false;
            t->unblock_task_ids=(unsigned long *)MALLOC(UNBLOCK_SIZE*sizeof(unsigned long));
            t->unblock_act_numb=UNBLOCK_SIZE;  t->num_out=0;
            return t;
        }
        inline void insertTask(task_t *const msg) {
            unsigned long act_id=task_id++;
            hash_task_t *act_task=createTask(act_id,NOT_READY,msg->wtask);
            icl_hash_insert(task_set, &act_task->id, act_task);

            for (auto p: msg->P) {
                auto d    = p.tag;
                auto dir  = p.dir;
                if(dir==INPUT) {
                    hash_task_t * t=(hash_task_t *)icl_hash_find(address_set,(void*)d);
                    if(t==NULL) { // no writier for this tag
                        hash_task_t *dummy=createTask(task_id,DONE,NULL);
                        dummy->is_dummy=true;
                        // the dummy task uses current data
                        icl_hash_insert(address_set,(void*)d,dummy);
                        // the dummy task unblocks the current data
                        dummy->unblock_task_ids[dummy->unblock_numb]=act_id;
                        dummy->unblock_numb++;
                        dummy->num_out++;
                        icl_hash_insert(task_set,&dummy->id,dummy);
                        task_id++;
                    } else {
                        if(t->unblock_numb == t->unblock_act_numb) {
                            t->unblock_act_numb+=UNBLOCK_SIZE;
                            t->unblock_task_ids=(unsigned long *)REALLOC(t->unblock_task_ids,t->unblock_act_numb*sizeof(unsigned long));
                        }
                        t->unblock_task_ids[t->unblock_numb]=act_id;
                        t->unblock_numb++;
                        if(t->status!=DONE) act_task->remaining_dep++;
                    }
                } else
                    if (dir==OUTPUT) {
                        hash_task_t * t=(hash_task_t *)icl_hash_find(address_set,(void*)d);
                        if(t != NULL) { // the task has been already written
                            if(t->unblock_numb>0) {
                                // for each unblocked task, checks if that task unblock also act_task (WAR dependency)
                                for(long ii=0;ii<t->unblock_numb;ii++) {
                                    hash_task_t* t2=(hash_task_t*)icl_hash_find(task_set,&t->unblock_task_ids[ii]);
                                    if(t2!=NULL && t2!=act_task && t2->status!=DONE) {
                                        if(t2->unblock_numb == t2->unblock_act_numb) {
                                            t2->unblock_act_numb+=UNBLOCK_SIZE;
                                            t2->unblock_task_ids=(unsigned long *)REALLOC(t2->unblock_task_ids,t2->unblock_act_numb*sizeof(unsigned long));
                                        }
                                        t2->unblock_task_ids[t2->unblock_numb]=act_id;
                                        t2->unblock_numb++;
                                        act_task->remaining_dep++;
                                    }
                                }
                            } else {
                                if(t->status!=DONE) {
                                    t->unblock_task_ids[0]=act_id;
                                    t->unblock_numb++;
                                    act_task->remaining_dep++;
                                }
                            }
                            t->num_out--;
                            if (t->status==DONE && t->num_out==0){
                                icl_hash_delete(task_set,&t->id,NULL,NULL);
                                FREE(t->unblock_task_ids); FREE(t);
                            }
                        }
                        icl_hash_update_insert(address_set, (void*)d, act_task);
                        act_task->num_out++;
                    }
            }
            if(act_task->remaining_dep==0) {
                act_task->status=READY;
                readytasks++;
                ready_queues[m].push(act_task);
                m = (m + 1) % runningworkers;
            }
        }

        // try to send at least one task to workers
        inline void schedule_task(const unsigned long th) {
            for(int i=0;(readytasks>0)&&(i<runningworkers);i++){
                if(nscheduled[i]<=th){
                    if(ready_queues[i].size()>0){
                        ++nscheduled[i];
                        lb->ff_send_out_to(ready_queues[i].top(),i);
                        ready_queues[i].pop();
                        --readytasks;
                    } else{
                        bool found = false;
                        for(int j=0; !found && (j<runningworkers);j++){
                            if(ready_queues[mmax].size()>0){
                                ++nscheduled[i];
                                lb->ff_send_out_to(ready_queues[mmax].top(),i);
                                ready_queues[mmax].pop();
                                found = true;
                                --readytasks;
                            }
                            mmax = (mmax + 1) % runningworkers;
                        }
                    }
                }
            }
        }

        inline void handleCompletedTask(hash_task_t *t, int workerid) {
            for(long i=0;i<t->unblock_numb;i++) {
                hash_task_t *tmp=(hash_task_t*)icl_hash_find(task_set,&t->unblock_task_ids[i]);
                tmp->remaining_dep--;
                if(tmp->remaining_dep==0) {
                    tmp->status=READY;
                    ++readytasks;
                    ready_queues[workerid].push(tmp);
                }
            }

            schedule_task(0);

            t->status=DONE;
            if(t->num_out==0) {
                icl_hash_delete(task_set,&t->id,NULL,NULL);
                FREE(t->unblock_task_ids); FREE(t);
            }
        }

        inline bool fromInput() { return (lb->get_channel_id() == -1);	}

    public:

        Scheduler(ff_loadbalancer* lb, const int maxnw, void (*schedRelaxF)(unsigned long)):
            lb(lb),ffalloc(NULL),runningworkers(0),address_set(NULL),task_set(NULL),
            task_id(1),task_numb(0),task_completed(0),bk_count(0),schedRelaxF(schedRelaxF),
            ready_queues(maxnw),nscheduled(maxnw),gd_ended(false) {
#if !defined(DONT_USE_FFALLOC)
            ffalloc=new ff_allocator;
            assert(ffalloc);
            int nslabs[N_SLABBUFFER]={0,2048,512,64,0,0,0,0,0 };
            if (ffalloc->init(nslabs)<0) {
                error("FATAL ERROR: allocator init failed\n");
                abort();
            }
#endif

            LOWER_TH = std::max(1024, TASK_PER_WORKER*maxnw); //FIX: potrebbe comunque stallare ..
            UPPER_TH = LOWER_TH+TASK_PER_WORKER;
        }
        ~Scheduler() {
#if !defined(DONT_USE_FFALLOC)
            if (ffalloc) delete ffalloc;
#endif
            if (task_set)    icl_hash_destroy(task_set,NULL,NULL);
            if (address_set) icl_hash_destroy(address_set,NULL,NULL);
        }

        int svc_init() {
            task_id = 1; task_numb = task_completed = 0;
            mmax = m = readytasks = 0;
            gd_ended = false;
            runningworkers = lb->getnworkers();
            bk_count = 0;

            ff_node::input_active(true);
            for(size_t i=0; i<nscheduled.size();++i) nscheduled[i] = 0;
            for(size_t i=0; i<ready_queues.size();++i) ready_queues[i] = priority_queue_t();

            if (task_set) icl_hash_destroy(task_set,NULL,NULL);
            if (task_set) icl_hash_destroy(address_set,NULL,NULL);
            task_set    = icl_hash_create( UPPER_TH*8, ulong_hash_function, ulong_key_compare );
            address_set = icl_hash_create( 0x01<<12, address_hash_function, address_key_compare);

            return 0;
        }
        void* svc(void* task) {
            if (!task) {
                if (!gd_ended && (task_numb-task_completed)<(unsigned long)LOWER_TH)
                    ff_node::input_active(true); // start receiveing from input channel again
                else  if (schedRelaxF) schedRelaxF(++bk_count);
                return GO_ON;
            }
            bk_count = 0;
            if (fromInput()) {
                task_t *const msg = static_cast<task_t*>(task);
                if (msg->wtask == NULL) {
                    gd_ended = true;
                    ff_node::input_active(false); // we don't want to read FF_EOS
                    return ((task_numb!=task_completed)?GO_ON:NULL);
                }
                ++task_numb;
                insertTask(msg);
                schedule_task(0);
                if ((task_numb-task_completed)>(unsigned long)LOWER_TH) {
                    ff_node::input_active(false); // stop receiving from input channel
                }
                return GO_ON;
            }
            hash_task_t * t = (hash_task_t *)task;
            ++task_completed;
            handleCompletedTask(t,lb->get_channel_id());

            --nscheduled[lb->get_channel_id()];
            schedule_task(1); // once more

            if(task_numb==task_completed && gd_ended)  return NULL;
            return GO_ON;
        }

        void eosnotify(int id) { lb->broadcast_task(EOS);}

        int wait_freezing() {
            return lb->wait_lb_freezing();
        }

    private:
        ff_loadbalancer  *lb;
        ff_allocator     *ffalloc;
        int               runningworkers;
        int               LOWER_TH, UPPER_TH;
        icl_hash_t       *address_set, *task_set;
        unsigned long     task_id, task_numb, task_completed, bk_count;
        void            (*schedRelaxF)(unsigned long);
        int mmax, readytasks,m;
        std::vector<priority_queue_t> ready_queues;
        std::vector<unsigned long> nscheduled;
        bool              gd_ended;
    };

    inline void reset() {
        gd->reset(); farm->reset(); sched->reset();
    }

public:
    /**
     *  \brief Constructor
     *
     *  \param F = is the user's function
     *  \param args = is the argument of the function F
     *  \param maxnw = is the maximum number of farm's workers that can be used
     *  \param schedRelaxF = is a function for managing busy-waiting in the farm scheduler
     */
    template<typename T1>
    ff_mdf(void (*F)(T1*const), T1*const args, size_t outstandingTasks=DEFAULT_OUTSTANDING_TASKS,
           int maxnw=ff_numCores(), void (*schedRelaxF)(unsigned long)=NULL):
        farmworkers(maxnw),pipe(false,outstandingTasks) { //NOTE: pipe has fixed size queue by default
        GD<T1> *_gd   = new GD<T1>(F,args);
        _gd->setMaxTasks(outstandingTasks+16); // NOTE: TASKS must be greater than pipe's queue!
        farm = new ff_farm<>(false,640*maxnw,1024*maxnw,true,maxnw,true);

        std::vector<fix8/ff_node *> w;
        // NOTE: Worker objects are going to be destroyed by the farm destructor
        for(int i=0;i<maxnw;++i) w.push_back(new Worker);
        farm->add_workers(w);
        farm->add_emitter(sched = new Scheduler(farm->getlb(), maxnw, schedRelaxF));
        farm->wrap_around(true);

        pipe.add_stage(_gd);
        pipe.add_stage(farm);
        if (pipe.run_then_freeze()<0) {
            error("ff_mdf: running pipeline\n");
        } else {
            pipe.wait_freezing();
            _gd->activate(true);
            gd = _gd;
            reset();
        }
    }
    ~ff_mdf() {
        if (gd)    delete gd;
        if (sched) delete sched;
        if (farm)  delete farm;
    }

    template<typename... Param>
    inline void AddTask(std::vector<param_info> &P, void(*F)(Param...), Param... args) {
        worker_task_t<Param...> *wtask = new worker_task_t<Param...>(F, args...);
        gd->alloc_and_send(P,wtask);
    }

    void setNumWorkers(int nw) {
        if (nw > ff_numCores())
            error("ff_mdf: setNumWorkers: too much workers, setting num worker to %d\n", ff_numCores());
        farmworkers=std::min(ff_numCores(),nw);
    }
    void setThreshold(size_t th=0) {} // <----------


    // FIX: TODO
    void *svc(void*) {
        // FIX: hashing tables on stream ?
        return NULL;
    }

    virtual inline int run_and_wait_end() {
        gd->thaw(true);
        farm->thaw(true,farmworkers);
        gd->wait_freezing();
        sched->wait_freezing();
        return 0;
    }

    double ffTime() { return pipe.ffTime(); }
    double ffwTime() { return pipe.ffwTime(); }

protected:
    int farmworkers;
    ff_pipeline pipe;
    base_gd   *gd;
    ff_farm<> *farm;
    Scheduler *sched;
};

} // namespace

#endif /* FF_MDF_HPP */
