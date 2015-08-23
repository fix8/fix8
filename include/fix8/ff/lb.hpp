/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 *  \link
 *  \file lb.hpp
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 *  \brief Contains the \p ff_loadbalancer class and methods used to model the \a Emitter node,
 *  which is used to distribute tasks among workers.
 */

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


#ifndef FF_LB_HPP
#define FF_LB_HPP

#include <iostream>
#include <deque>

#include <fix8/ff/utils.hpp>
#include <fix8/ff/node.hpp>

namespace ff {

/*!
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 *  @{
 */

/*!
 *  \class ff_loadbalancer
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 *  \brief A class representing the \a Emitter node in a typical \a Farm
 *  skeleton.
 *
 *  This class models the \p loadbalancer, which wraps all the methods and
 *  structures used by the \a Emitter node in a \p Farm skeleton. The \a
 *  emitter node is used to generate the stream of tasks for the pool of \a
 *  workers. The \a emitter can also be used as sequential preprocessor if the
 *  stream is coming from outside the farm, as is the case when the stream is
 *  coming from a previous node of a pipeline chain or from an external
 *  device.\n The \p Farm skeleton must have the \a emitter node defined: if
 *  the user does not add it to the farm, the run-time support adds a default
 *  \a emitter, which acts as a stream filter and schedules tasks in a
 *  round-robin fashion towards the \a workers.
 *
 *  This class is defined in \ref lb.hpp
 *
 */

class ff_loadbalancer: public ff_thread {
    template <typename T1, typename T2>  friend class ff_farm;
public:

    // NOTE:
    //  - TICKS2WAIT should be a valued profiled for the application
    //    Consider to redifine losetime_in and losetime_out for your app.
    //
    enum {TICKS2WAIT=1000};
protected:
    /**
     * \brief Pushes EOS to the worker
     *
     * It pushes the EOS to the queue of the worker.
     *
     * \parm nofreeze is a booleon value to determine if the EOS should be freezed or not
     */
    inline void push_eos(bool nofreeze=false) {
        //register int cnt=0;
        void * eos = nofreeze?EOS_NOFREEZE:EOS;
        for(register ssize_t i=0;i<running;++i) {
            while(!workers[i]->put(eos)) {
                //if (sched_emitter && (++cnt>PUSH_CNT_EMIT)) {
                // cnt=0;sched_yield();}
                //else
                losetime_out();
            }
        }
    }
    inline void push_goon() {
        void * goon = (void *)(GO_ON);
        for(register int i=0;i<running;++i) {
            while(!workers[i]->put(goon)) {
                losetime_out();
            }
        }
    }


    /**
     * \brief Virtual function that can be redefined to implement a new scheduling
     * policy.
     *
     * It is a virtual function that can be redefined to implement a new
     * scheduling polity.
     *
     * \return The number of worker to be selected.
     */
    virtual inline int selectworker() { return (++nextw % running); }

#if defined(LB_CALLBACK)

    /**
     * \brief Defines callback
     *
     * It defines the call back
     *
     * \parm n TODO
     */
    virtual inline void callback(int n) { }

    /**
     * \brief Defines callback
     *
     * It defines the callback and returns a pointer.
     *
     * \parm n TODO
     * \parm task is a void pointer
     *
     * \return \p NULL pointer is returned.
     */
    virtual inline void * callback(int n, void *task) { return NULL;}
#endif

    /**
     * \brief Gets the number of tentatives before wasting some times
     *
     * The number of tentative before wasting some times and than retry.
     *
     * \return The number of workers.
     */
    virtual inline unsigned int ntentative() { return running;}

    /**
     * \brief Loses some time before sending the message to output buffer
     *
     * It loses some time before the message is sent to the output buffer.
     *
     */
    virtual inline void losetime_out() {
        FFTRACE(lostpushticks+=TICKS2WAIT;++pushwait);
        ticks_wait(TICKS2WAIT);
    }

    /**
     * \brief Loses time before retrying to get a message from the input buffer
     *
     * It loses time before retrying to get a message from the input buffer.
     */
    virtual inline void losetime_in() {
        FFTRACE(lostpopticks+=TICKS2WAIT;++popwait);
        ticks_wait(TICKS2WAIT);
    }

    /**
     * \brief Scheduling of tasks
     *
     * It is the main scheduling function. This is a virtual function and can
     * be redefined to implement a custom scheduling policy.
     *
     * \parm task is a void pointer
     * \parm retry is the number of tries to schedule a task
     * \parm ticks are the number of ticks to be lost
     *
     * \return \p true, if successful, or \p false if not successful.
     *
     */
    virtual bool schedule_task(void * task, unsigned int retry=(unsigned)-1, unsigned int ticks=0) {
        register unsigned int cnt; //,cnt2;
        do {
            cnt=0;//,cnt2=0;
            do {
                nextw = selectworker();
                if (nextw<0) return false;
#if defined(LB_CALLBACK)
                task = callback(nextw, task);
#endif
                if(workers[nextw]->put(task)) {
                    FFTRACE(++taskcnt);
                    return true;
                }
                else {
                    //std::cerr << ".";
                    ++cnt;
                    if (cnt>=retry) { nextw=-1; return false; }
                    if (cnt == ntentative()) break;
                }
            } while(1);
            losetime_out();
            //std::cerr << "-";
        } while(1);

        return false;
    }

    /**
     * \brief Collects tasks
     *
     * It collects tasks from the worker and returns in the form of deque.
     *
     * \parm task is a void pointer
     * \parm availworkers is a queue of available workers
     * \parm start is a queue of TODO
     *
     * \return The deque of the tasks.
     */
    virtual std::deque<fix8/ff_node *>::iterator  collect_task(void ** task,
                                                           std::deque<fix8/ff_node *> & availworkers,
                                                           std::deque<fix8/ff_node *>::iterator & start) {
        register int cnt, nw= (int)(availworkers.end()-availworkers.begin());
        const std::deque<fix8/ff_node *>::iterator & ite(availworkers.end());
        do {
            cnt=0;
            do {
                if (++start == ite) start=availworkers.begin();
                if((*start)->get(task)) {
                    channelid =
                        ((start-availworkers.begin()) >= (ssize_t)multi_input_start) ? -1:(*start)->get_my_id();
                    return start;
                }
                else if (++cnt == nw) {
                    if (filter && !filter->in_active) { *task=NULL; channelid=-2; return ite;}
                    if (buffer && buffer->pop(task)) {
                        channelid = -1;
                        return ite;
                    }
                    break;
                }
            } while(1);
            losetime_in();
        } while(1);
        return ite;
    }

    /**
     * \brief Pop a task from buffer
     *
     * It pops the task from buffer.
     *
     * \parm task is a void pointer
     *
     * \return \p true if successful
     */
    bool pop(void ** task) {
        //register int cnt = 0;
        if (!filter) {
            while (! buffer->pop(task)) {
                //    if (ch->thxcore>1) {
                //if (++cnt>PUSH_POP_CNT) { sched_yield(); cnt=0;}
                //else ticks_wait(TICKS2WAIT);
                //} else
                losetime_in();
            }
            return true;
        }
        while (! filter->pop(task)) {
            //    if (ch->thxcore>1) {
            //if (++cnt>PUSH_POP_CNT) { sched_yield(); cnt=0;}
            //else ticks_wait(TICKS2WAIT);
            //} else
            losetime_in();
        }
        return true;
    }

    /**
     *
     * \brief Task scheduler
     *
     * It defines the static version of the task scheduler.
     *
     * \return The status of scheduled task, which can be either \p true or \p
     * false.
     */
    static bool ff_send_out_emitter(void * task,unsigned int retry,unsigned int ticks, void *obj) {
        return ((ff_loadbalancer *)obj)->schedule_task(task, retry, ticks);
    }

    int set_internal_multi_input(svector<fix8/ff_node*> &mi) {
        if (mi.size() == 0) {
            error("LB, invalid internal multi-input vector size\n");
            return -1;
        }
        int_multi_input = mi;
        return 0;
    }

    void absorb_eos(svector<fix8/ff_node*>& W) {
        void *task;
        for(size_t i=0;i<W.size();++i) {
            do ; while(!W[i]->get(&task));
            assert((task == (void*)FF_EOS) || (task == EOS_NOFREEZE));
        }
    }


public:
    /**
     *  \brief Default constructor
     *
     *  It is the defauls constructor
     *
     *  \param max_num_workers The max number of workers allowed
     */
    ff_loadbalancer(size_t max_num_workers):
        running(-1),max_nworkers(max_num_workers),nextw(-1),channelid(-2),
        filter(NULL),workers(max_num_workers),
        buffer(NULL),skip1pop(false),master_worker(false),
        multi_input(256),multi_input_start(max_num_workers+1),
        int_multi_input(256) {
        time_setzero(tstart);time_setzero(tstop);
        time_setzero(wtstart);time_setzero(wtstop);
        wttime=0;
        FFTRACE(taskcnt=0;lostpushticks=0;pushwait=0;lostpopticks=0;popwait=0;ticksmin=(ticks)-1;ticksmax=0;tickstot=0);
    }

    /**
     *  \brief Destructor
     *
     *  It deallocates dynamic memory spaces previoulsy allocated for workers.
     */
    virtual ~ff_loadbalancer() {}

    /**
     * \brief Sets filter node
     *
     * It sets the filter with the FastFlow node.
     *
     * \parm f is FastFlow node
     *
     * \return 0 if successful, otherwise -1
     */
    int set_filter(ff_node * f) {
        if (filter) {
            error("LB, setting emitter filter\n");
            return -1;
        }
        filter = f;
        filter->registerCallback(ff_send_out_emitter, this);

        return 0;
    }

    /**
     * \brief Sets input buffer
     *
     * It sets the input buffer with the instance of FFBUFFER
     *
     * \parm buff is a pointer of FFBUFFER
     */
    void set_in_buffer(FFBUFFER * const buff) {
        buffer=buff;
        skip1pop=false;
    }

    /**
     * \brief Gets input buffer
     *
     * It gets the input buffer
     *
     * \return The buffer
     */
    FFBUFFER * get_in_buffer() const { return buffer;}

    /**
     *
     * \brief Gets channel id
     *
     * It returns the identifier of the channel.
     *
     * \return the channel id
     */
    int get_channel_id() const { return channelid;}

    /**
     * \brief Resets the channel id
     *
     * It reset the channel id to -2
     *
     */
    void reset_channel_id() { channelid=-2;}

    /**
     *  \brief Resets input buffer
     *
     *   Warning resetting the buffer while the node is running may produce unexpected results.
     */
    void reset() { if (buffer) buffer->reset();}

    /**
     * \brief Get the number of workers
     *
     * It returns the number of workers running
     *
     * \return Number of worker
     */
    inline size_t getnworkers() const { return (size_t)running;}

    /**
     * \brief Get the number of workers
     *
     * It returns the number of total workers registered
     *
     * \return Number of worker
     */
    inline size_t getNWorkers() const { return workers.size();}

    // AGGIUNTO
    const svector<fix8/ff_node*>& getWorkers() const { return workers; }

    /**
     * \brief Skips first pop
     *
     * It sets \p skip1pop to \p true
     *
     */
    void skipfirstpop() { skip1pop=true;}

    /**
     * \brief Decides master-worker schema
     *
     * It desides the master-worker schema.
     *
     * \return 0 if successful, or -1 if unsuccessful.
     *
     */
    int set_masterworker() {
        if (master_worker) {
            error("LB, master_worker flag already set\n");
            return -1;
        }
        master_worker=true;
        return 0;
    }

    inline int getTid(ff_node *node) const {
        return node->getTid();
    }

    /**
     * \brief Sets multiple input buffers
     *
     * It sets the multiple input buffers.
     *
     * \return 0 if successful, otherwise -1.
     */
    int set_input(svector<fix8/ff_node*> &mi) {
        if (mi.size() == 0) {
            error("LB, set_input, invalid multi-input vector size\n");
            return -1;
        }
        multi_input += mi;
        return 0;
    }

    int set_input(ff_node * node) {
        multi_input.push_back(node);
        return 0;
    }

    inline bool ff_send_out_to(void *task, int id) {
        return workers[id]->put(task);
    }

    /**
     * \brief Send the same task to all workers
     *
     * \parm task is a void pointer
     *
     * It sends the same task to all workers.
     *
     */
    inline void broadcast_task(void * task) {
        std::vector<int> retry;

        for(register ssize_t i=0;i<running;++i) {
            if(!workers[i]->put(task))
                retry.push_back(i);
        }
        while(retry.size()) {
            if(workers[retry.back()]->put(task))
                retry.pop_back();
            else losetime_out();
        }
    }

    /**
     * \brief Gets master worker
     *
     * It returns master worker
     *
     * \return The master worker
     *
     */
    bool masterworker() const { return master_worker;}

    /**
     * \brief Registers the given node into the workers' list
     *
     * It registers the given node in the worker list.
     *
     * \parm w is the worker
     *
     * \return 0 if successful, or -1 if not successful
     */
    int  register_worker(ff_node * w) {
        if (workers.size()>=max_nworkers) {
            error("LB, max number of workers reached (max=%ld)\n",max_nworkers);
            return -1;
        }
        workers.push_back(w);
        return 0;
    }

    /**
     * \brief Deregister worker
     *
     * It deregister worker.
     *
     * \return -1 ia always returned.
     */
    int deregister_worker() {
        return -1;
    }

    /**
     *
     * \brief Load balances task
     *
     * It is a virtual function which loadbalances the task.
     */
    virtual void * svc(void *) {
        void * task = NULL;
        void * ret  = (void *)FF_EOS;
        bool inpresent  = (get_in_buffer() != NULL);
        bool skipfirstpop = skip1pop;

        // the following case is possible when the emitter is a dnode
        if (!inpresent && filter && (filter->get_in_buffer()!=NULL)) {
            inpresent = true;
            set_in_buffer(filter->get_in_buffer());
        }

        gettimeofday(&wtstart,NULL);
        if (!master_worker && (multi_input.size()==0) && (int_multi_input.size()==0)) {
            do {
                if (inpresent) {
                    if (!skipfirstpop) pop(&task);
                    else skipfirstpop=false;

                    if (task == EOS) {
                        if (filter) filter->eosnotify();
                        push_eos();
                        break;
                    } else if (task == EOS_NOFREEZE) {
                        if (filter) filter->eosnotify();
                        ret = task;
                        break;
                    }
                }

                if (filter) {
                    FFTRACE(register ticks t0 = getticks());

                    task = filter->svc(task);

#if defined(TRACE_FASTFLOW)
                    register ticks diff=(getticks()-t0);
                    tickstot +=diff;
                    ticksmin=(std::min)(ticksmin,diff);
                    ticksmax=(std::max)(ticksmax,diff);
#endif
                    if (task == GO_ON) continue; // going to get another task
                    if ((task == GO_OUT) || (task == EOS_NOFREEZE)) {
                        ret = task;
                        break; // exiting from the loop without sending out the task
                    }
                    // if the filter returns NULL we exit immediatly
                    if (!task || task==EOS) {
                        push_eos();
                        ret = EOS;
                        break;
                    }
                } else
                    if (!inpresent) { push_goon(); push_eos(); ret=(void*)FF_EOS; break;}

                const bool r = schedule_task(task);
                assert(r); (void)r;
            } while(true);
        } else {
            size_t nw=0;
            // contains current worker
            std::deque<fix8/ff_node *> availworkers;

            //assert( master_worker ^ ( multi_input != NULL) );

            if (master_worker) {
                for(int i=0;i<running;++i)
                    availworkers.push_back(workers[i]);
                nw = running;
            }
            if (int_multi_input.size()>0) {
                assert(!master_worker);
                for(size_t i=0;i<int_multi_input.size();++i)
                    availworkers.push_back(int_multi_input[i]);
                nw += int_multi_input.size();
            }
            if (multi_input.size()>0) {
                multi_input_start = availworkers.size();
                for(size_t i=0;i<multi_input.size();++i)
                    availworkers.push_back(multi_input[i]);
                nw += multi_input.size();
            }
            if ((master_worker || int_multi_input.size()>0) && inpresent) {
                assert(multi_input.size() == 0);
                nw += 1;
            }
            std::deque<fix8/ff_node *>::iterator start(availworkers.begin());
            std::deque<fix8/ff_node *>::iterator victim(availworkers.begin());
            do {
                if (!skipfirstpop) {
                    victim=collect_task(&task, availworkers, start);
                } else skipfirstpop=false;

                if ((task == EOS) ||
                    (task == EOS_NOFREEZE)) {
                    if (filter) filter->eosnotify(channelid);
                    if ((victim != availworkers.end())) {
                        if (channelid>0 && (task != EOS_NOFREEZE) &&
                            (!workers[channelid]->isfrozen())) {
                            availworkers.erase(victim);
                            start=availworkers.begin(); // restart iterator
                        }
                    }

                    //if (master_worker ||
                    //    (channelid==-1 && multi_input.size()>0) ||
                    //    (channelid>=0 && int_multi_input.size()>0)) {
                    if (!--nw) {
                        // this conditions means there is a loop
                        // so in this case I don't want to send an additional
                        // EOS since I have already received all of them
                        if (!master_worker && (task==EOS) && (int_multi_input.size()==0)) {
                            push_eos();
                        }
                        ret = task;
                        break; // received all EOS, exit
                    }
                    //}
                } else {
                    if (filter) {
                        FFTRACE(register ticks t0 = getticks());

                        task = filter->svc(task);

#if defined(TRACE_FASTFLOW)
                        register ticks diff=(getticks()-t0);
                        tickstot +=diff;
                        ticksmin=(std::min)(ticksmin,diff);
                        ticksmax=(std::max)(ticksmax,diff);
#endif

                        if (task == GO_ON) continue;
                        if ((task == GO_OUT) || (task == EOS_NOFREEZE)){
                            ret = task;
                            break; // exiting from the loop without sending out the task
                        }
                        // if the filter returns NULL we exit immediatly
                        if (!task || (task==(void*)FF_EOS)) {
                            push_eos();
                            // try to remove the additional EOS due to
                            // the feedback channel
                            if (inpresent || multi_input.size()>0) {
                                if (master_worker) absorb_eos(workers);
                                if (int_multi_input.size()>0) absorb_eos(int_multi_input);
                            }
                            ret = (void*)FF_EOS;
                            break;
                        }
                    }
                    schedule_task(task);
                }
            } while(1);
        }
        gettimeofday(&wtstop,NULL);
        wttime+=diffmsec(wtstop,wtstart);

        return ret;
    }

    /**
     * \brief Initializes the load balancer task
     *
     * It is a virtual function which initialises the loadbalancer task.
     *
     * \return 0 if successful, otherwise -1 is returned.
     *
     */
    virtual int svc_init() {
        gettimeofday(&tstart,NULL);

        if (filter && filter->svc_init() <0) return -1;

        return 0;
    }

    /**
     * \brief Finalizes the loadbalancer task
     *
     * It is a virtual function which finalises the loadbalancer task.
     *
     */
    virtual void svc_end() {
        if (filter) filter->svc_end();
        gettimeofday(&tstop,NULL);
    }

    /**
     * \brief Runs the loadbalancer
     *
     * It runs the load balancer.
     *
     * \return 0 if successful, otherwise -1 is returned.
     */
    int runlb(bool=false, ssize_t nw=-1) {
        if (this->spawn(filter?filter->getCPUId():-1) == -2) {
            error("LB, spawning LB thread\n");
            return -1;
        }
        running = (nw<=0)?workers.size():nw;
        return 0;
    }

    // AGGIUNTI
    int runWorkers(ssize_t nw=-1) {
        running = (nw<=0)?workers.size():nw;
        if (isfrozen()) {
            for(size_t i=0;i<(size_t)running;++i) {
                if (workers[i]->freeze_and_run()<0) {
                    error("LB, spawning worker thread\n");
                    return -1;
                }
            }
        } else {
            for(size_t i=0;i<(size_t)running;++i) {
                if (workers[i]->run()<0) {
                    error("LB, spawning worker thread\n");
                    return -1;
                }
            }
        }
        return 0;
    }
    int thawWorkers(bool _freeze=false, ssize_t nw=-1) {
        if (nw == -1 || (size_t)nw > workers.size()) running = workers.size();
        else running = nw;
        for(ssize_t i=0;i<running;++i) workers[i]->thaw(_freeze);
        return 0;
    }
    inline int wait_freezingWorkers() {
        int ret = 0;
        for(ssize_t i=0;i<running;++i)
            if (workers[i]->wait_freezing()<0) {
                error("LB, waiting freezing of worker thread, id = %d\n",workers[i]->get_my_id());
                ret = -1;
            }
        running = -1;
        return ret;
    }
    inline int waitWorkers() {
        int ret=0;
        for(size_t i=0;i<workers.size();++i)
            if (workers[i]->wait()<0) {
                error("LB, waiting worker thread, id = %d\n",workers[i]->get_my_id());
                ret = -1;
            }
        running = -1;
        return ret;
    }

    /**
     * \brief Spawns workers threads
     *
     * It spawns workers threads.
     *
     * \return 0 if successful, otherwise -1 is returned
     */
    virtual int run(bool=false) {
        running = workers.size();
        if (this->spawn(filter?filter->getCPUId():-1) == -2) {
            error("LB, spawning LB thread\n");
            return -1;
        }
        if (isfrozen()) {
            for(size_t i=0;i<workers.size();++i) {
                if (workers[i]->freeze_and_run(true)<0) {
                    error("LB, spawning worker thread\n");
                    return -1;
                }
            }
        } else {
            for(size_t i=0;i<workers.size();++i) {
                if (workers[i]->run(true)<0) {
                    error("LB, spawning worker thread\n");
                    return -1;
                }
            }
        }
        return 0;
    }

    /**
     * \brief Waits for load balancer
     *
     * It waits for the load balancer.
     *
     * \return 0 if successful, otherwise -1 is returned.
     */
    int waitlb() {
        if (ff_thread::wait()<0) {
            error("LB, waiting LB thread\n");
            return -1;
        }
        return 0;
    }


    /**
     * \brief Waits for workers to finish their task
     *
     * It waits for all workers to finish their tasks.
     *
     * \return 0 if successful, otherwise -1 is returned.
     */
    virtual int wait() {
        int ret=0;
        for(size_t i=0;i<workers.size();++i)
            if (workers[i]->wait()<0) {
                error("LB, waiting worker thread, id = %d\n",workers[i]->get_my_id());
                ret = -1;
            }
        running = -1;
        if (ff_thread::wait()<0) {
            error("LB, waiting LB thread\n");
            ret = -1;
        }
        return ret;
    }


    inline int wait_lb_freezing() {
        if (ff_thread::wait_freezing()<0) {
            error("LB, waiting LB thread freezing\n");
            return -1;
        }
        running = -1;
        return 0;
    }

    /**
     * \brief Waits for freezing
     *
     * It waits for the freezing of all threads.
     *
     * \return 0 if successful, otherwise -1 is returned.
     *
     */
    virtual inline int wait_freezing() {
        int ret = 0;
        for(ssize_t i=0;i<running;++i)
            if (workers[i]->wait_freezing()<0) {
                error("LB, waiting freezing of worker thread, id = %d\n",workers[i]->get_my_id());
                ret = -1;
            }
        if (ff_thread::wait_freezing()<0) {
            error("LB, waiting LB thread freezing\n");
            ret = -1;
        }
        running = -1;
        return ret;
    }

    /**
     * \brief Waits for freezing for one single worker thread
     *
     */
    inline int wait_freezing(const size_t n) {
        assert(n<(size_t)running);
        if (workers[n]->wait_freezing()<0) {
            error("LB, waiting freezing of worker thread, id = %d\n",workers[n]->get_my_id());
            return -1;
        }
        return 0;
    }


    /**
     * \brief Stops the thread
     *
     * It stops all workers and the emitter.
     */
    inline void stop() {
        for(size_t i=0;i<workers.size();++i) workers[i]->stop();
        ff_thread::stop();
    }

    /**
     * \brief Freezes all threads registered with the lb and the lb itself
     *
     * It freezes all workers and the emitter.
     */
    inline void freeze() {
        for(ssize_t i=0;i<running;++i) workers[i]->freeze();
        ff_thread::freeze();
    }

    /**
     * \brief Freezes one worker thread
     */
    inline void freeze(const size_t n) {
        assert(n<workers.size());
        workers[n]->freeze();
    }

    /**
     * \brief Thaws all threads register with the lb and the lb itself
     *
     *
     */
    virtual inline void thaw(bool _freeze=false, ssize_t nw=-1) {
        if (nw == -1 || (size_t)nw > workers.size()) running = workers.size();
        else running = nw;
        ff_thread::thaw(_freeze); // NOTE:start scheduler first
        for(ssize_t i=0;i<running;++i) workers[i]->thaw(_freeze);
    }

    /**
     * \brief Thaws one single worker thread
     *
     */
    inline void thaw(const size_t n, bool _freeze=false) {
        assert(n<workers.size());
        workers[n]->thaw(_freeze);
    }

    /**
     * \brief FastFlow start timing
     *
     * It returns the starting of FastFlow timing.
     *
     * \return The difference in FastFlow timing.
     */
    virtual double ffTime() {
        return diffmsec(tstop,tstart);
    }

    /**
     * \brief FastFlow finish timing
     *
     * It returns the finishing of FastFlow timing.
     *
     * \return The difference in FastFlow timing.
     */
    virtual double wffTime() {
        return diffmsec(wtstop,wtstart);
    }

    virtual const struct timeval & getstarttime() const { return tstart;}
    virtual const struct timeval & getstoptime()  const { return tstop;}
    virtual const struct timeval & getwstartime() const { return wtstart;}
    virtual const struct timeval & getwstoptime() const { return wtstop;}

#if defined(TRACE_FASTFLOW)
    /**
     * \brief Prints the FastFlow trace
     *
     * It prints the trace of FastFlow.
     */
    virtual void ffStats(std::ostream & out) {
        out << "Emitter: "
            << "  work-time (ms): " << wttime    << "\n"
            << "  n. tasks      : " << taskcnt   << "\n"
            << "  svc ticks     : " << tickstot  << " (min= " << (filter?ticksmin:0) << " max= " << ticksmax << ")\n"
            << "  n. push lost  : " << pushwait  << " (ticks=" << lostpushticks << ")" << "\n"
            << "  n. pop lost   : " << popwait   << " (ticks=" << lostpopticks  << ")" << "\n";
    }
#endif

private:
    ssize_t            running;             /// Number of workers running
    size_t             max_nworkers;        /// Max number of workers allowed
    ssize_t            nextw;               // out index
    ssize_t            channelid;
    ff_node         *  filter;
    svector<fix8/ff_node*>  workers;
    FFBUFFER        *  buffer;
    bool               skip1pop;
    bool               master_worker;
    svector<fix8/ff_node*>  multi_input;
    size_t             multi_input_start;   // position in the availworkers array
    svector<fix8/ff_node*>  int_multi_input;

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
 *  \endlink
 */

} // namespace ff

#endif  /* FF_LB_HPP */
