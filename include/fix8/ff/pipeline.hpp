/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 * \file pipeline.hpp
 * \ingroup core_patterns high_level_patterns
 *
 * \brief This file implements the pipeline skeleton, both in the high-level pattern
 * syntax (\ref ff::ff_pipe) and low-level syntax (\ref ff::ff_pipeline)
 *
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

#ifndef FF_PIPELINE_HPP
#define FF_PIPELINE_HPP

#include <cassert>
#include <memory>
#include <functional>
#include <fix8/ff/svector.hpp>
#include <fix8/ff/fftree.hpp>
#include <fix8/ff/node.hpp>
#include <fix8/ff/ocl/clEnvironment.hpp>

namespace ff {

/**
 * \class ff_pipeline
 * \ingroup core_patterns
 *
 *  \brief The Pipeline skeleton (low-level syntax)
 *
 */
class ff_pipeline: public ff_node {
protected:
    inline int prepare() {
        // create input FFBUFFER
        const int nstages=static_cast<int>(nodes_list.size());
        for(int i=1;i<nstages;++i) {
            pthread_mutex_t   *m        = NULL;
            pthread_cond_t    *c        = NULL;
            std::atomic_ulong *counter  = NULL;
            if (!nodes_list[i]->init_input_blocking(m,c,counter)) {
                error("PIPE, init input blocking mode for node %d\n", i);
                return -1;
            }
            nodes_list[i-1]->set_output_blocking(m,c,counter);
            if (nodes_list[i]->isMultiInput()) {
                if (nodes_list[i-1]->create_output_buffer(out_buffer_entries,
                                                          fixedsize)<0)
                    return -1;
                svector<ff_node*> w(MAX_NUM_THREADS);
                nodes_list[i-1]->get_out_nodes(w);
                if (w.size() == 0) {
                    nodes_list[i]->set_input(nodes_list[i-1]);
                    if (!nodes_list[i-1]->init_output_blocking(m,c,counter)) {
                        error("PIPE, buffernode condition, init output blocking mode for node %d\n", i);
                        return -1;
                    }
                }
                else {
                    nodes_list[i]->set_input(w);
                    for(size_t i=0;i<w.size();++i) {
                        w[i]->set_output_blocking(m,c,counter);

                        // the following is needed because w[i] can be a buffernode and not a real node
                        pthread_mutex_t   *mo        = NULL;
                        pthread_cond_t    *co        = NULL;
                        std::atomic_ulong *countero  = NULL;
                        if (!w[i]->init_output_blocking(mo,co,countero)) {
                            error("PIPE, buffernode condition, init output blocking mode for node %d\n", i);
                            return -1;
                        }
                    }
                }
            } else {
                if (nodes_list[i]->create_input_buffer(in_buffer_entries, fixedsize)<0) {
                    error("PIPE, creating input buffer for node %d\n", i);
                    return -1;
                }
            }
        }

        // set output buffer
        for(int i=0;i<(nstages-1);++i) {
            if (nodes_list[i+1]->isMultiInput()) continue;
            pthread_mutex_t   *m        = NULL;
            pthread_cond_t    *c        = NULL;
            std::atomic_ulong *counter  = NULL;
            if (!nodes_list[i]->init_output_blocking(m,c,counter)) {
                error("PIPE, init output blocking mode for node %d\n", i);
                return -1;
            }
            nodes_list[i+1]->set_input_blocking(m,c,counter);
            if (nodes_list[i]->isMultiOutput()) {
                nodes_list[i]->set_output(nodes_list[i+1]);
            } else {
                if (nodes_list[i]->set_output_buffer(nodes_list[i+1]->get_in_buffer())<0) {
                    error("PIPE, setting output buffer to node %d\n", i);
                    return -1;
                }
            }
        }

        // Preparation of buffers for the accelerator
        int ret = 0;
        if (has_input_channel) {
            if (create_input_buffer(in_buffer_entries, fixedsize)<0) {
                error("PIPE, creating input buffer for the accelerator\n");
                ret=-1;
            } else {
                if (get_out_buffer()) {
                    error("PIPE, output buffer already present for the accelerator\n");
                    ret=-1;
                } else {
                    // NOTE: the last buffer is forced to be unbounded |
                    if (create_output_buffer(out_buffer_entries, false)<0) {
                        error("PIPE, creating output buffer for the accelerator\n");
                        ret = -1;
                    }
                }
            }

            pthread_mutex_t   *m        = NULL;
            pthread_cond_t    *c        = NULL;
            std::atomic_ulong *counter  = NULL;

            // set blocking input for the first stage (cons_m,...)
            if (!init_input_blocking(m,c,counter)) {
                error("PIPE, init input blocking mode for accelerator\n");
            }
            // set my pointers to the first stage input blocking staff
            ff_node::set_output_blocking(m,c,counter);

            m=NULL,c=NULL,counter=NULL;

            // set my blocking output (prod_m, ....)
            if (!ff_node::init_output_blocking(m,c,counter)) {
                error("PIPE, init output blocking mode for accelerator\n");
            }
            // give pointers to pipeline first stage (p_prod_m, ...)
            set_input_blocking(m,c,counter);

            m=NULL,c=NULL,counter=NULL;

            // pipeline's first stage blocking output staff (prod_m, ....)
            if (!init_output_blocking(m,c,counter)) {
                error("FARM, add_collector, init input blocking mode for accelerator\n");
            }
            // set my pointers (p_prod_m,....)
            ff_node::set_input_blocking(m,c,counter);

            m=NULL,c=NULL,counter=NULL;

            // set my blocking input (cons_m, ....)
            if (!ff_node::init_input_blocking(m,c,counter)) {
                error("PIPE, init input blocking mode for accelerator\n");
            }
            // give pointers to my blocking input to the last pipeline stage (p_cons_m,...)
            set_output_blocking(m,c,counter);
        }

        prepared=true;
        return ret;
    }


    int freeze_and_run(bool skip_init=false) {
        int nstages=static_cast<int>(nodes_list.size());
        if (!skip_init) {
            // set the initial value for the barrier
            if (!barrier)  barrier = new BARRIER_T;
            const int nthreads = cardinality(barrier);
            if (nthreads > MAX_NUM_THREADS) {
                error("PIPE, too much threads, increase MAX_NUM_THREADS !\n");
                return -1;
            }
            barrier->barrierSetup(nthreads);
        }
        if (!prepared) if (prepare()<0) return -1;
        ssize_t startid = (get_my_id()>0)?get_my_id():0;
        for(ssize_t i=0;i<nstages;++i) {
            nodes_list[i]->set_id(i+startid);
            if (nodes_list[i]->freeze_and_run(true)<0) {
                error("ERROR: PIPE, (freezing and) running stage %d\n", i);
                return -1;
            }
        }
        return 0;
    }

public:

    enum { DEF_IN_BUFF_ENTRIES=512, DEF_OUT_BUFF_ENTRIES=(DEF_IN_BUFF_ENTRIES+128)};

    /**
     *  \brief Constructor
     *
     *  \param input_ch \p true set accelerator mode
     *  \param in_buffer_entries input queue length
     *  \param out_buffer_entries output queue length
     *  \param fixedsize \p true uses bound channels (SPSC queue)
     */
    explicit ff_pipeline(bool input_ch=false,
                                   int in_buffer_entries=DEF_IN_BUFF_ENTRIES,
                                   int out_buffer_entries=DEF_OUT_BUFF_ENTRIES,
                                   bool fixedsize=true):
        has_input_channel(input_ch),prepared(false),
        node_cleanup(false),fixedsize(fixedsize),
        in_buffer_entries(in_buffer_entries),
        out_buffer_entries(out_buffer_entries) {
        //fftree stuff
        fftree_ptr = new fftree(this, PIPE);
        assert(fftree_ptr);
    }

    /**
     * \brief Destructor
     */
    virtual ~ff_pipeline() {
        if (barrier) delete barrier;
        if (node_cleanup) {
            while(nodes_list.size()>0) {
                ff_node *n = nodes_list.back();
                nodes_list.pop_back();
                delete n;
            }
        }
        for(size_t i=0;i<internalSupportNodes.size();++i) {
            delete internalSupportNodes.back();
            internalSupportNodes.pop_back();
        }

        //fftree stuff
        if (fftree_ptr) { delete fftree_ptr; fftree_ptr=NULL; }
    }



    /*  WARNING: if these methods are called after prepare (i.e. after having called
     *  run_and_wait_end/run_then_freeze/run/....) they have no effect.
     *
     */
    void setFixedSize(bool fs)             { fixedsize = fs;         }
    void setXNodeInputQueueLength(int sz)  { in_buffer_entries = sz; }
    void setXNodeOutputQueueLength(int sz) { out_buffer_entries = sz;}


    /**
     *  \brief It adds a stage to the pipeline
     *
     *  \param s a ff_node (or derived, e.g. farm) object that is the stage to be added
     *  to the pipeline
     */
    int add_stage(ff_node * s) {
        if (nodes_list.size()==0 && s->isMultiInput())
            ff_node::setMultiInput();
        nodes_list.push_back(s);
        //fftree stuff
        fftree *treeptr = s->getfftree();
        if (treeptr==NULL) {
            treeptr = new fftree(s, s->getFFType());
            assert(treeptr);
        	s->setfftree(treeptr);
        }
        fftree_ptr->add_child(treeptr);
        return 0;
    }

    inline void setMultiOutput() {
        ff_node::setMultiOutput();
    }

    /**
     * \brief Feedback channel (pattern modifier)
     *
     * The last stage output stream will be connected to the first stage
     * input stream in a cycle (feedback channel)
     */
    int wrap_around(bool multi_input=false) {
        if (nodes_list.size()<2) {
            error("PIPE, too few pipeline nodes\n");
            return -1;
        }
        const int last = static_cast<int>(nodes_list.size())-1;

        pthread_mutex_t   *mi        = NULL;
        pthread_cond_t    *ci        = NULL;
        std::atomic_ulong *counteri  = NULL;
        if (!init_input_blocking(mi,ci,counteri)) {
            error("PIPE, init input blocking mode for node %d\n", 0);
            return -1;
        }
        set_output_blocking(mi,ci,counteri);
        pthread_mutex_t   *mo        = NULL;
        pthread_cond_t    *co        = NULL;
        std::atomic_ulong *countero  = NULL;
        if (!init_output_blocking(mo,co,countero)) {
            error("PIPE, init output blocking mode for node %d\n", last);
            return -1;
        }
        set_input_blocking(mo,co,countero);

        if (nodes_list[0]->isMultiInput()) {
            if (nodes_list[last]->isMultiOutput()) {
                // NOTE: forces unbounded size for the feedback channel queue!
                ff_node *t = new ff_buffernode(out_buffer_entries,false);
                if (!t) return -1;
                t->set_id(last); // NOTE: that's not the real node id !
                t->set_input_blocking(mo,co,countero);
                t->set_output_blocking(mi,ci,counteri);
                internalSupportNodes.push_back(t);
                nodes_list[0]->set_input(t);
                nodes_list[last]->set_output(t);
            } else {
                // NOTE: forces unbounded size for the feedback channel queue!
                if (create_output_buffer(out_buffer_entries, false)<0)
                    return -1;
                svector<ff_node*> w(MAX_NUM_THREADS);
                this->get_out_nodes(w);
                nodes_list[0]->set_input(w);
            }
            if (!multi_input) nodes_list[0]->skipfirstpop(true);
        } else {
            // NOTE: forces unbounded size for the feedback channel queue!
            if (create_input_buffer(out_buffer_entries, false)<0)
                return -1;

            if (nodes_list[last]->isMultiOutput())
                nodes_list[last]->set_output(nodes_list[0]);
            else
                if (set_output_buffer(get_in_buffer())<0)
                    return -1;

            nodes_list[0]->skipfirstpop(true);
        }
        return 0;
    }

    inline void cleanup_nodes() { node_cleanup = true; }

    inline void get_out_nodes(svector<ff_node*>&w) {
        assert(nodes_list.size()>0);
        int last = static_cast<int>(nodes_list.size())-1;
        const size_t sizebefore = w.size();
        nodes_list[last]->get_out_nodes(w);
        if (w.size()==sizebefore)
            w.push_back(nodes_list[last]);
    }

    /**
     * returns the stages added to the pipeline
     */
    const svector<ff_node*>& getStages() const { return nodes_list; }

    /**
     * \brief Run the pipeline skeleton asynchronously
     *
     * Run the pipeline, the method call return immediately. To be coupled with
     * \ref ff_pipeline::wait()
     */
    int run(bool skip_init=false) {
        int nstages=static_cast<int>(nodes_list.size());

        if (!skip_init) {
            // set the initial value for the barrier
            if (!barrier)  barrier = new BARRIER_T;
            const int nthreads = cardinality(barrier);
            if (nthreads > MAX_NUM_THREADS) {
                error("PIPE, too much threads, increase MAX_NUM_THREADS !\n");
                return -1;
            }
            barrier->barrierSetup(nthreads);

            // REMOVE THIS ?
            // check if we have to setup the OpenCL environment !
            if (fftree_ptr->hasOpenCLNode()) {
                // setup openCL environment
                clEnvironment::instance();
            }

        }
        if (!prepared) if (prepare()<0) return -1;

        ssize_t startid = (get_my_id()>0)?get_my_id():0;
        for(int i=0;i<nstages;++i) {
            nodes_list[i]->set_id(i+startid);
            if (nodes_list[i]->run(true)<0) {
                error("ERROR: PIPE, running stage %d\n", i);
                return -1;
            }
        }
        return 0;
    }

    int dryrun() {
        if (!prepared)
            if (prepare()<0) return -1;
        return 0;
    }

    /**
     * \relates ff_pipe
     * \brief run the pipeline, waits that all stages received the End-Of-Stream (EOS),
     * and destroy the pipeline run-time
     *
     * Blocking behaviour w.r.t. main thread to be clarified
     */
    int run_and_wait_end() {
        if (isfrozen()) {  // TODO
            error("PIPE: Error: feature not yet supported\n");
            return -1;
        }
        stop();
        if (run()<0) return -1;
        if (wait()<0) return -1;
        return 0;
    }

    /**
     * \related ff_pipe
     * \brief run the pipeline, waits that all stages received the End-Of-Stream (EOS),
     * and suspend the pipeline run-time
     *
     * Run-time threads are suspended by way of a distrubuted protocol.
     * The same pipeline can be re-started by calling again run_then_freeze
     */
    int run_then_freeze(ssize_t nw=-1) {
        if (isfrozen()) {
            // true means that next time threads are frozen again
            thaw(true, nw);
            return 0;
        }
        if (!prepared) if (prepare()<0) return -1;
        //freeze();
        //return run();
        /* freeze_and_run is required because in the pipeline
         * there isn't no manager thread, which allows to freeze other
         * threads before starting the computation
         */
        return freeze_and_run(false);
    }

    /**
     * \brief wait for pipeline termination (all stages received EOS)
     */
    int wait(/* timeval */ ) {
        int ret=0;
        for(unsigned int i=0;i<nodes_list.size();++i)
            if (nodes_list[i]->wait()<0) {
                error("PIPE, waiting stage thread, id = %d\n",nodes_list[i]->get_my_id());
                ret = -1;
            }

        return ret;
    }

    /**
     * \brief wait for pipeline to complete and suspend (all stages received EOS)
     *
     *
     */
    inline int wait_freezing(/* timeval */ ) {
        int ret=0;
        for(unsigned int i=0;i<nodes_list.size();++i)
            if (nodes_list[i]->wait_freezing()<0) {
                error("PIPE, waiting freezing of stage thread, id = %d\n",
                      nodes_list[i]->get_my_id());
                ret = -1;
            }

        return ret;
    }


    inline void stop() {
        for(unsigned int i=0;i<nodes_list.size();++i) nodes_list[i]->stop();
    }


    inline void freeze() {
        for(unsigned int i=0;i<nodes_list.size();++i) nodes_list[i]->freeze();
    }

    inline bool done() const {
        int nstages=static_cast<int>(nodes_list.size());
        for(int i=0;i<nstages;++i)
            if (!nodes_list[i]->done()) return false;
        return true;
    }

    /**
     * \brief offload a task to the pipeline from the offloading thread (accelerator mode)
     *
     * Offload a task onto a pipeline accelerator, tipically the offloading
     * entity is the main thread (even if it can be used from any
     * \ref ff_node::svc method)
     *
     * \note to be used in accelerator mode only
     */
    inline bool offload(void * task,
                        unsigned long retry=((unsigned long)-1),
                        unsigned long ticks=ff_node::TICKS2WAIT) {
         FFBUFFER * inbuffer = get_in_buffer();
         assert(inbuffer != NULL);

         if (ff_node::blocking_out) {
         _retry:
             if (inbuffer->push(task)) {
                 pthread_mutex_lock(p_cons_m);
                 if ((*p_cons_counter).load() == 0)
                     pthread_cond_signal(p_cons_c);
                 ++(*p_cons_counter);
                 pthread_mutex_unlock(p_cons_m);
                 ++prod_counter;
                 return true;
             }
             pthread_mutex_lock(&prod_m);
             while(prod_counter.load() >= (inbuffer->buffersize())) {
                 pthread_cond_wait(&prod_c, &prod_m);
             }
             pthread_mutex_unlock(&prod_m);
             goto _retry;
         }
         for(unsigned long i=0;i<retry;++i) {
            if (inbuffer->push(task)) return true;
            losetime_out(ticks);
        }
        return false;
    }

    /**
     * \brief gets a result from a task to the pipeline from the main thread
     * (accelator mode)
     *
     * Total call: return when a result is available. To be used in accelerator mode only
     *
     * \param[out] task
     * \param retry number of attempts to get a result before failing
     * (related to nonblocking get from channel - expert use only)
     * \param ticks number of clock cycles between successive attempts
     * (related to nonblocking get from channel - expert use only)
     * \return \p true is a task is returned, \p false if End-Of-Stream (EOS)
     */
    inline bool load_result(void ** task,
                            unsigned long retry=((unsigned long)-1),
                            unsigned long ticks=ff_node::TICKS2WAIT) {
        FFBUFFER * outbuffer = get_out_buffer();

        if (!outbuffer) {
            if (!has_input_channel)
                error("PIPE: accelerator is not set, offload not available");
            else
                error("PIPE: output buffer not created");
            return false;
        }

        if (ff_node::blocking_in) {
        _retry:
            if (outbuffer->pop(task)) {
                pthread_mutex_lock(p_prod_m);
                if ((*p_prod_counter).load() >= outbuffer->buffersize()) {
                    pthread_cond_signal(p_prod_c);
                }
                --(*p_prod_counter);
                pthread_mutex_unlock(p_prod_m);
                --cons_counter;

                if ((*task != (void *)FF_EOS)) return true;
                else return false;
            }
            pthread_mutex_lock(&cons_m);
            while(cons_counter.load() == 0) {
                pthread_cond_wait(&cons_c, &cons_m);
            }
            pthread_mutex_unlock(&cons_m);
            goto _retry;
        }
        for(unsigned long i=0;i<retry;++i) {
            if (outbuffer->pop(task)) {
                if ((*task != (void *)FF_EOS)) return true;
                else return false;
            }
            losetime_in(ticks);
        }
        return false;
    }
    /**
     * \brief try to get a result from a task to the pipeline from the main thread
     * (accelator mode)
     *
     * Partial call: can return no result. To be used in accelerator mode only
     *
     * \param[out] task
     * \return \p true is a task is returned (including EOS),
     * \p false if no task is returned
     */
    inline bool load_result_nb(void ** task) {
        FFBUFFER * outbuffer = get_out_buffer();
        if (outbuffer) {
            if (outbuffer->pop(task)) return true;
            else return false;
        }

        if (!has_input_channel)
            error("PIPE: accelerator is not set, offload not available");
        else
            error("PIPE: output buffer not created");
        return false;
    }


    int cardinality(BARRIER_T * const barrier)  {
        int card=0;
        for(unsigned int i=0;i<nodes_list.size();++i)
            card += nodes_list[i]->cardinality(barrier);

        return card;
    }

    /*
     * \brief Misure execution time (including init and finalise)
     *
     * \return pipeline execution time (including init and finalise)
     */
    double ffTime() {
        return diffmsec(nodes_list[nodes_list.size()-1]->getstoptime(),
                        nodes_list[0]->getstarttime());
    }

    /*
     * \brief Misure execution time (excluding init and finalise)
     *
     * \return pipeline execution time (excluding runtime setup)
     */
    double ffwTime() {
        return diffmsec(nodes_list[nodes_list.size()-1]->getwstoptime(),
                        nodes_list[0]->getwstartime());
    }

#if defined(TRACE_FASTFLOW)
    void ffStats(std::ostream & out) {
        out << "--- pipeline:\n";
        for(unsigned int i=0;i<nodes_list.size();++i)
            nodes_list[i]->ffStats(out);
    }
#else
    void ffStats(std::ostream & out) {
        out << "FastFlow trace not enabled\n";
    }
#endif

protected:

    void* svc(void * task) { return NULL; }
    int   svc_init() { return -1; };
    void  svc_end()  {}

    void  setAffinity(int) {
        error("PIPE, setAffinity: cannot set affinity for the pipeline\n");
    }

    int   getCPUId() const { return -1;}

    inline void thaw(bool _freeze=false, ssize_t nw=-1) {
        for(unsigned int i=0;i<nodes_list.size();++i) nodes_list[i]->thaw(_freeze, nw);
    }

    inline bool isfrozen() const {
        int nstages=static_cast<int>(nodes_list.size());
        for(int i=0;i<nstages;++i)
            if (!nodes_list[i]->isfrozen()) return false;
        return true;
    }

    // consumer
    virtual inline bool init_input_blocking(pthread_mutex_t   *&m,
                                            pthread_cond_t    *&c,
                                            std::atomic_ulong *&counter) {
        return nodes_list[0]->init_input_blocking(m,c,counter);
    }
    virtual inline void set_input_blocking(pthread_mutex_t   *&m,
                                           pthread_cond_t    *&c,
                                           std::atomic_ulong *&counter) {
        nodes_list[0]->set_input_blocking(m,c,counter);
    }

    // producer
    virtual inline bool init_output_blocking(pthread_mutex_t   *&m,
                                             pthread_cond_t    *&c,
                                             std::atomic_ulong *&counter) {
        const int last = static_cast<int>(nodes_list.size())-1;
        if (last<0) return false;
        return nodes_list[last]->init_output_blocking(m,c,counter);
    }
    virtual inline void set_output_blocking(pthread_mutex_t   *&m,
                                            pthread_cond_t    *&c,
                                            std::atomic_ulong *&counter) {
        const int last = static_cast<int>(nodes_list.size())-1;
        if (last<0) return;
        nodes_list[last]->set_output_blocking(m,c,counter);
    }

    virtual inline pthread_mutex_t   &get_cons_m()        { return nodes_list[0]->get_cons_m();}
    virtual inline pthread_cond_t    &get_cons_c()        { return nodes_list[0]->get_cons_c();}
    virtual inline std::atomic_ulong &get_cons_counter()  { return nodes_list[0]->get_cons_counter();}

    virtual inline pthread_mutex_t &get_prod_m()        {
        const int last = static_cast<int>(nodes_list.size())-1;
        return nodes_list[last]->get_prod_m();
    }
    virtual inline pthread_cond_t  &get_prod_c()        {
        const int last = static_cast<int>(nodes_list.size())-1;
        return nodes_list[last]->get_prod_c();
    }
    virtual inline std::atomic_ulong  &get_prod_counter()  {
        const int last = static_cast<int>(nodes_list.size())-1;
        return nodes_list[last]->get_prod_counter();
    }

    int create_input_buffer(int nentries, bool fixedsize) {
        if (in) return -1;

        if (nodes_list[0]->create_input_buffer(nentries, fixedsize)<0) {
            error("PIPE, creating input buffer for node 0\n");
            return -1;
        }
        if (!nodes_list[0]->isMultiInput())
            ff_node::set_input_buffer(nodes_list[0]->get_in_buffer());
        return 0;
    }

    int create_output_buffer(int nentries, bool fixedsize=false) {
        int last = static_cast<int>(nodes_list.size())-1;
        if (last<0) return -1;

        if (nodes_list[last]->create_output_buffer(nentries, fixedsize)<0) {
            error("PIPE, creating output buffer for node %d\n",last);
            return -1;
        }
        ff_node::set_output_buffer(nodes_list[last]->get_out_buffer());
        return 0;
    }

    int set_output_buffer(FFBUFFER * const o) {
        int last = static_cast<int>(nodes_list.size())-1;
        if (last<0) return -1;

        if (nodes_list[last]->set_output_buffer(o)<0) {
            error("PIPE, setting output buffer for node %d\n",last);
            return -1;
        }
        return 0;
    }

    inline bool isMultiInput() const {
        if (nodes_list.size()==0) return false;
        return nodes_list[0]->isMultiInput();
    }
    inline bool isMultiOutput() const {
        if (nodes_list.size()==0) return false;
        int last = static_cast<int>(nodes_list.size())-1;
        return nodes_list[last]->isMultiOutput();
    }
    inline int set_input(ff_node *node) {
        return nodes_list[0]->set_input(node);
    }
    inline int set_input(svector<ff_node *> & w) {
             return nodes_list[0]->set_input(w);
    }
    inline int set_output(ff_node *node) {
        int last = static_cast<int>(nodes_list.size())-1;
        return nodes_list[last]->set_output(node);
    }

private:
    bool has_input_channel; // for accelerator
    bool prepared;
    bool node_cleanup;
    bool fixedsize;
    int in_buffer_entries;
    int out_buffer_entries;
    svector<ff_node *> nodes_list;
    svector<ff_node*>  internalSupportNodes;
};


//#ifndef WIN32 //VS12
    // ------------------------ high-level (simpler) pipeline ------------------
#if ((__cplusplus >= 201103L) || (defined __GXX_EXPERIMENTAL_CXX0X__)) || (defined(HAS_CXX11_VARIADIC_TEMPLATES))

#include <ff/make_unique.hpp>

    /*!
     * \class ff_Pipe
     * \ingroup high_level_patterns
     *
     * \brief Pipeline pattern (high-level pattern syntax)
     *
     * Set up a parallel for pipeline pattern run-time support object.
     * Run with \p run_and_wait_end or \p run_the_freeze. See related functions.
     *
     * \note Don't use to model a workflow of tasks, stages are nonblocking threads
     * and
     * require one core per stage. If you need to model a workflow use \ref ff::ff_mdf
     *
     * \example pipe_basic.cpp
     */

    template<typename IN_t=char,typename OUT_t=IN_t>
    class ff_Pipe: public ff_pipeline {
    private:
#if (!defined(__CUDACC__) && !defined(WIN32) && !defined(__ICC))
        //
        // Thanks to Suter Toni (HSR) for suggesting the following code for checking
        // correct input-output types ordering.
        //

        template<class A, class...>
        struct valid_stage_types : std::true_type {};

        template<class A, class B, class... Bs>
        struct valid_stage_types<A&&, B&&, Bs &&...> : std::integral_constant<bool, std::is_same<typename A::out_type, typename B::in_type>{} && valid_stage_types<B, Bs...>{}> {};

        template<class A, class B, class... Bs>
        struct valid_stage_types<std::unique_ptr<A>&&, std::unique_ptr<B>&&, Bs &&...> : std::integral_constant<bool, std::is_same<typename A::out_type, typename B::in_type>{} && valid_stage_types<std::unique_ptr<B>, Bs...>{}> {};

        template<class A, class B, class... Bs>
        struct valid_stage_types<std::unique_ptr<A>&&, B&&, Bs &&...> : std::integral_constant<bool, std::is_same<typename A::out_type, typename B::in_type>{} && valid_stage_types<B, Bs...>{}> {};
        template<class A, class B, class... Bs>
        struct valid_stage_types<A&&, std::unique_ptr<B>&&, Bs &&...> : std::integral_constant<bool, std::is_same<typename A::out_type, typename B::in_type>{} && valid_stage_types<std::unique_ptr<B>, Bs...>{}> {};

        //struct valid_stage_types<A, B, Bs ...> : std::integral_constant<bool, std::is_same<typename A::out_type, typename B::in_type>{} && valid_stage_types<B, Bs...>{}> {};

#endif

        //
        // Thanks to Peter Sommerlad for suggesting the following simpler code
        //
        void add2pipeall(){} // base case
        // need to see this before add2pipeall variadic template function
        inline void add2pipe(ff_node &node) { ff_pipeline::add_stage(&node); }
        // need to see this before add2pipeall variadic template function
        inline void add2pipe(ff_node *node) {
            cleanup_stages.push_back(node);
            ff_pipeline::add_stage(node);
        }
        template<typename FIRST,typename ...ARGS>
        void add2pipeall(FIRST &stage,ARGS&...args){
        	add2pipe(stage);
        	add2pipeall(args...); // recurse
        }
        template<typename FIRST,typename ...ARGS>
        void add2pipeall(std::unique_ptr<FIRST> & stage,ARGS&...args){
        	add2pipe(stage.release());
        	add2pipeall(args...); // recurse
        }

    protected:
        std::vector<ff_node*> cleanup_stages;

    public:

        // NOTE: The ff_Pipe accepts as stages either l-value references or std::unique_ptr l-value references.
        //       The ownership of the (unique) pointer stage is transferred to the pipeline !!!!

        typedef IN_t  in_type;
        typedef OUT_t out_type;

        /**
         * \brief Create a stand-alone pipeline (no input/output streams). Run with \p run_and_wait_end or \p run_the_freeze.
         *
         * Identifies an stream parallel construct in which stages are executed
         * in parallel.
         * It does require a stream of tasks, either external of created by the
         * first stage.
         * \param stages pipeline stages
         *
         * Example: \ref pipe_basic.cpp
         */
        template<typename... STAGES>
        ff_Pipe(STAGES &&...stages) {    // forwarding reference (aka universal reference)
#if ( !defined(__CUDACC__) && !defined(WIN32) && !defined(__ICC) )
        	static_assert(valid_stage_types<STAGES...>{}, "Input & output types of the pipe's stages don't match");
#endif
        	this->add2pipeall(stages...); //this->add2pipeall(std::forward<STAGES>(stages)...);
        }
        /**
         * \brief Create a pipeline (with input stream). Run with \p run_and_wait_end or \p run_the_freeze.
         *
         * Identifies an stream parallel construct in which stages are executed
         * in parallel.
         * It does require a stream of tasks, either external of created by the
         * first stage.
         * \param input_ch \p true to enable first stage input stream
         * \param stages pipeline stages
         *
         * Example: \ref pipe_basic.cpp
         */
        template<typename... STAGES>
        explicit ff_Pipe(bool input_ch, STAGES &&...stages):ff_pipeline(input_ch) {
#if (!defined(__CUDACC__) && !defined(WIN32))
        	static_assert(valid_stage_types<STAGES...>{},
                          "Input & output types of the pipe's stages don't match");
#endif
        	this->add2pipeall(stages...);
        }

        ~ff_Pipe() {
            for (auto s: cleanup_stages) delete s;
        }

        operator ff_node* () { return this;}

        bool load_result(OUT_t *&task,
                         unsigned long retry=((unsigned long)-1),
                         unsigned long ticks=ff_node::TICKS2WAIT) {
            return ff_pipeline::load_result((void**)&task, retry,ticks);
        }

        // deleted members
        bool load_result(void ** task,
                         unsigned long retry=((unsigned long)-1),
                         unsigned long ticks=ff_node::TICKS2WAIT) = delete;

        /*
         *  using the following two add_stage method, no static check on the input/output
         *  types is executed
         */
        int add_stage(ff_node &s) {
            add2pipe(s);
            return 0;
        }
        int add_stage(std::unique_ptr<ff_node> &&s) {
            add2pipe(s.release());
            return 0;
        }

        // deleted functions
        int add_stage(ff_node * s) = delete;
        void cleanup_nodes() = delete;
    };
#endif /* HAS_CXX11_VARIADIC_TEMPLATES */
//#endif //VS12

} // namespace ff

#endif /* FF_PIPELINE_HPP */
