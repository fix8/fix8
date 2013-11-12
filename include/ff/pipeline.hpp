/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*! 
 * \link
 * \file pipeline.hpp
 * \ingroup high_level_patterns_shared_memory
 *
 * \brief This file describes the pipeline skeleton.
 *
 */

#ifndef _FF_PIPELINE_HPP_
#define _FF_PIPELINE_HPP_
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

#include <vector>
#include <ff/node.hpp>

#if defined( HAS_CXX11_VARIADIC_TEMPLATES )
#include <tuple>
#endif

namespace ff {

/* ---------------- basic high-level macros ----------------------- */

    /* FF_PIPEA creates an n-stage accelerator pipeline (max 16 stages) 
     * where channels have type 'type'. The functions called in each stage
     * must have the following signature:
     *   bool (*F)(type &)
     * If the function F returns false an EOS is produced in the 
     * output channel.
     *
     * Usage example:
     *
     *  struct task_t { .....};
     *
     *  bool F(task_t &task) { .....; return true;}
     *  bool G(task_t &task) { .....; return true;}
     *  bool H(task_t &task) { .....; return true;}
     * 
     *  FF_PIPEA(pipe1, task_t, F, G);     // 2-stage
     *  FF_PIPEA(pipe2, task_t, F, G, H);  // 3-stage
     *
     *  FF_PIPEARUN(pipe1); FF_PIPEARUN(pipe2);
     *  for(...) {
     *     if (even_task) FF_PIPEAOFFLOAD(pipe1, new task_t(...));
     *     else FF_PIPEAOFFLOAD(pipe2, new task_t(...));
     *  }
     *  FF_PIEAEND(pipe1);  FF_PIPEAEND(pipe2);
     *  FF_PIEAWAIT(pipe1); FF_PIPEAWAIT(pipe2);
     *
     */
#define FF_PIPEA(pipename, type, ...)                                   \
    ff_pipeline _FF_node_##pipename(true);                              \
    _FF_node_##pipename.cleanup_nodes();                                \
    {                                                                   \
      class _FF_pipe_stage: public ff_node {                            \
          typedef bool (*type_f)(type &);                               \
          type_f F;                                                     \
      public:                                                           \
       _FF_pipe_stage(type_f F):F(F) {}                                 \
       void* svc(void *task) {                                          \
          type* _ff_in = (type*)task;                                   \
          if (!F(*_ff_in)) return NULL;                                 \
          return task;                                                  \
       }                                                                \
      };                                                                \
      ff_pipeline *p = &_FF_node_##pipename;                            \
      FOREACHPIPE(p->add_stage, __VA_ARGS__);                           \
    }

#define FF_PIPEARUN(pipename)  _FF_node_##pipename.run()
#define FF_PIPEAWAIT(pipename) _FF_node_##pipename.wait()
#define FF_PIPEAOFFLOAD(pipename, task)  _FF_node_##pipename.offload(task);
#define FF_PIPEAEND(pipename)  _FF_node_##pipename.offload(EOS)
#define FF_PIPEAGETRESULTNB(pipename, result) _FF_node_##pipename.load_result_nb((void**)result)
#define FF_PIPEAGETRESULT(pipename, result) _FF_node_##pipename.load_result((void**)result)
/* ---------------------------------------------------------------- */


/*!
 * \ingroup high_level_patterns_shared_memory
 *
 *  @{
 */


/*!
 *  \class ff_pipeline
 * \ingroup high_level_patterns_shared_memory
 *
 *  \brief The Pipeline skeleton.
 *
 *  Pipelining is one of the simplest parallel patterns where data flows
 *  through a series of stages (or nodes) and each stage processes the input
 *  data in some ways, producing as output a modified version or new data. A
 *  pipeline's stage can operate sequentially or in parallel and may or may not
 *  have an internal state.
 *
 *  This class is defined in \ref pipeline.hpp
 */
 
class ff_pipeline: public ff_node {
protected:
    /**
     * It prepare the Pipeline skeleton for execution.
     *
     * \return TODO
     */
    inline int prepare() {
        // create input FFBUFFER
        int nstages=static_cast<int>(nodes_list.size());
        for(int i=1;i<nstages;++i) {
            if (nodes_list[i]->create_input_buffer(in_buffer_entries, fixedsize)<0) {
                error("PIPE, creating input buffer for node %d\n", i);
                return -1;
            }
        }
        
        // set output buffer
        for(int i=0;i<(nstages-1);++i) {
            if (nodes_list[i]->set_output_buffer(nodes_list[i+1]->get_in_buffer())<0) {
                error("PIPE, setting output buffer to node %d\n", i);
                return -1;
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
                    if (create_output_buffer(out_buffer_entries,fixedsize)<0) {
                        error("PIPE, creating output buffer for the accelerator\n");
                        ret = -1;
                    }
                }
            }
        }
        prepared=true; 
        return ret;
    }

    /**
     *  This function is required when no manager threads are present in the 
     *  pipeline, which would allow to freeze other threads before starting the 
     *  computation.
     *
     *  \return TODO
     */
    int freeze_and_run(bool=false) {
        freeze();
        int nstages=static_cast<int>(nodes_list.size());
        if (!prepared) if (prepare()<0) return -1;
        for(int i=0;i<nstages;++i) {
            nodes_list[i]->set_id(i);
            if (nodes_list[i]->freeze_and_run(true)<0) {
                error("ERROR: PIPE, (freezing and) running stage %d\n", i);
                return -1;
            }
        }
        return 0;
    } 

public:
    /**
     * TODO
     */
    enum { DEF_IN_BUFF_ENTRIES=512, DEF_OUT_BUFF_ENTRIES=(DEF_IN_BUFF_ENTRIES+128)};

    /**
     *  Constructor
     *
     *  \param input_ch = true to set accelerator mode
     *  \param in_buffer_entries = input queue length
     *  \param out_buffer_entries = output queue length
     *  \param fixedsize = true uses only fixed size queue
     */
    ff_pipeline(bool input_ch=false,
                int in_buffer_entries=DEF_IN_BUFF_ENTRIES,
                int out_buffer_entries=DEF_OUT_BUFF_ENTRIES, 
                bool fixedsize=true):
        has_input_channel(input_ch),prepared(false),
        node_cleanup(false),fixedsize(fixedsize),
        in_buffer_entries(in_buffer_entries),
        out_buffer_entries(out_buffer_entries) {               
    }
    
    /**
     * Destructor
     */
    ~ff_pipeline() {
        if (barrier) delete barrier;
        if (node_cleanup) {
            while(nodes_list.size()>0) {
                ff_node *n = nodes_list.back();
                nodes_list.pop_back();
                delete n;
            }
        }
    }

    /**
     *  It adds a stage to the Pipeline
     *
     *  \param s a ff_node that is the stage to be added to the skeleton. The
     *  stage contains the task that has to be executed.
     */
    int add_stage(ff_node * s) {        
        nodes_list.push_back(s);
        return 0;
    }

    /**
     * The last stage output queue will be connected 
     * to the first stage input queue (feedback channel).
     */
    int wrap_around() {
        if (nodes_list.size()<2) {
            error("PIPE, too few pipeline nodes\n");
            return -1;
        }

        fixedsize=false; // NOTE: force unbounded size for the queues!

        if (create_input_buffer(out_buffer_entries, fixedsize)<0)
            return -1;
        
        if (set_output_buffer(get_in_buffer())<0)
            return -1;

        nodes_list[0]->skip1pop = true;

        return 0;
    }

    /**
     * \brief Delete nodes when the destructor is called.
     *
     */
    void cleanup_nodes() { node_cleanup = true; }

    /**
     * It run the Pipeline skeleton.
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
        }
        if (!prepared) if (prepare()<0) return -1;

        if (has_input_channel) {
            /* freeze_and_run is required because in the pipeline 
             * where there are not any manager threads,
             * which allow to freeze other threads before starting the 
             * computation
             */
            for(int i=0;i<nstages;++i) {
                nodes_list[i]->set_id(i);
                if (nodes_list[i]->freeze_and_run(true)<0) {
                error("ERROR: PIPE, running stage %d\n", i);
                return -1;
                }
            }
        }  else {
            for(int i=0;i<nstages;++i) {
                nodes_list[i]->set_id(i);
                if (nodes_list[i]->run(true)<0) {
                    error("ERROR: PIPE, running stage %d\n", i);
                    return -1;
                }
            }
        }

        return 0;
    }

    /**
     * It run and wait all threads to finish.
     */
    int run_and_wait_end() {
        if (isfrozen()) return -1; // FIX !!!!
        stop();
        if (run()<0) return -1;           
        if (wait()<0) return -1;
        return 0;
    }
    
    /**
     * It run and then freeze.
     */
    int run_then_freeze() {
        if (isfrozen()) {
            // true means that next time threads are frozen again
            thaw(true);
            return 0;
        }
        if (!prepared) if (prepare()<0) return -1;
        freeze();
        return run();
    }
    
    /**
     * It waits for a stage to complete its task
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
     * It waits for freezing.
     */
    int wait_freezing(/* timeval */ ) {
        int ret=0;
        for(unsigned int i=0;i<nodes_list.size();++i)
            if (nodes_list[i]->wait_freezing()<0) {
                error("PIPE, waiting freezing of stage thread, id = %d\n",
                      nodes_list[i]->get_my_id());
                ret = -1;
            } 
        
        return ret;
    } 
    
    /**
     * It stops all stages.
     */
    void stop() {
        for(unsigned int i=0;i<nodes_list.size();++i) nodes_list[i]->stop();
    }

    /**
     * It freeze all stages.
     */
    void freeze() {
        for(unsigned int i=0;i<nodes_list.size();++i) nodes_list[i]->freeze();
    }
    /**
     * It Thaws all frozen stages.
     * if _freeze is true at next step all threads are frozen again
     */
    void thaw(bool _freeze=false) {
        for(unsigned int i=0;i<nodes_list.size();++i) nodes_list[i]->thaw(_freeze);
    }
    
    /** 
     * It checks if the pipeline is frozen 
     */
    bool isfrozen() { 
        int nstages=static_cast<int>(nodes_list.size());
        for(int i=0;i<nstages;++i) 
            if (!nodes_list[i]->isfrozen()) return false;
        return true;
    }

    /** 
     * offfload the given task to the pipeline 
     */
    inline bool offload(void * task,
                        unsigned int retry=((unsigned int)-1),
                        unsigned int ticks=ff_node::TICKS2WAIT) { 
        FFBUFFER * inbuffer = get_in_buffer();
        if (inbuffer) {
            for(unsigned int i=0;i<retry;++i) {
                if (inbuffer->push(task)) return true;
                ticks_wait(ticks);
            }     
            return false;
        }
        
        if (!has_input_channel) 
            error("PIPE: accelerator is not set, offload not available\n");
        else
            error("PIPE: input buffer creation failed\n");
        return false;
    }    
    
    /**
     *  It loads results. If \p false, EOS arrived or too many retries. If \p
     *  true, there is a new value
     */
    inline bool load_result(void ** task, 
                            unsigned int retry=((unsigned int)-1),
                            unsigned int ticks=ff_node::TICKS2WAIT) {
        FFBUFFER * outbuffer = get_out_buffer();
        if (outbuffer) {
            for(unsigned int i=0;i<retry;++i) {
                if (outbuffer->pop(task)) {
                    if ((*task != (void *)FF_EOS)) return true;
                    else return false;
                }
                ticks_wait(ticks);
            }     
            return false;
        }
        
        if (!has_input_channel) 
            error("PIPE: accelerator is not set, offload not available");
        else
            error("PIPE: output buffer not created");
        return false;
    }

    /**
     * TODO
     *
     * \return values:
     * false: no task present
     * true : there is a new value, you should check if the task is an FF_EOS
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
    
    /**
     * TODO
     *
     * \return TODO
     */
    int cardinality(BARRIER_T * const barrier)  { 
        int card=0;
        for(unsigned int i=0;i<nodes_list.size();++i) 
            card += nodes_list[i]->cardinality(barrier);
        
        return card;
    }
    
    /* 
     * The returned time comprise the time spent in svn_init and 
     * in svc_end methods
     *
     * \return TODO
     */
    double ffTime() {
        return diffmsec(nodes_list[nodes_list.size()-1]->getstoptime(),
                        nodes_list[0]->getstarttime());
    }
    
    /*  
     *  The returned time considers only the time spent in the svc
     *  methods
     *
     *  \return TODO
     */
    double ffwTime() {
        return diffmsec(nodes_list[nodes_list.size()-1]->getwstoptime(),
                        nodes_list[0]->getwstartime());
    }
    
#if defined(TRACE_FASTFLOW)
    /**
     * TODO
     */
    void ffStats(std::ostream & out) { 
        out << "--- pipeline:\n";
        for(unsigned int i=0;i<nodes_list.size();++i)
            nodes_list[i]->ffStats(out);
    }
#else
    /**
     * TODO
     */
    void ffStats(std::ostream & out) { 
        out << "FastFlow trace not enabled\n";
    }
#endif
    
protected:
    
    /**
     * TODO
     */
    void* svc(void * task) { return NULL; }
    
    /**
     * TODO
     */
    int   svc_init() { return -1; };
    
    /**
     * TODO
     */
    void  svc_end()  {}
    

    /**
     * TODO
     */
    void  setAffinity(int) { 
        error("PIPE, setAffinity: cannot set affinity for the pipeline\n");
    }
    
    /**
     * TODO
     */
    int   getCPUId() { return -1;}

    /**
     * TODO
     */
    int create_input_buffer(int nentries, bool fixedsize) { 
        if (in) return -1;
        if (nodes_list[0]->create_input_buffer(nentries, fixedsize)<0) {
            error("PIPE, creating input buffer for node 0\n");
            return -1;
        }
        ff_node::set_input_buffer(nodes_list[0]->get_in_buffer());
        return 0;
    }
    
    /**
     * TODO
     */
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

    /**
     * TODO
     */
    int set_output_buffer(FFBUFFER * const o) {
        int last = static_cast<int>(nodes_list.size())-1;
        if (!last) return -1;

        if (nodes_list[last]->set_output_buffer(o)<0) {
            error("PIPE, setting output buffer for node %d\n",last);
            return -1;
        }
        return 0;
    }

private:
    bool has_input_channel; // for accelerator
    bool prepared;
    bool node_cleanup;
    bool fixedsize;
    int in_buffer_entries;
    int out_buffer_entries;
    std::vector<ff_node *> nodes_list;
};


/* ------------------------ high-level (simpler) pipeline -------------------------------- */

// generic ff_node stage. It is built around the function F ( F: T* -> T* )
template<typename T>
class Fstage: public ff_node {
public:
    typedef T*(*F_t)(T*);
    Fstage(F_t F):F(F) {}
    inline void* svc(void *t) {	 return F((T*)t); }    
protected:
    F_t F;
};

#if defined( HAS_CXX11_VARIADIC_TEMPLATES )

template<typename TaskType>
class ff_pipe: public ff_pipeline {
private:
    typedef TaskType*(*F_t)(TaskType*);
    
    template<std::size_t I = 1, typename FuncT, typename... Tp>
    inline typename std::enable_if<I == (sizeof...(Tp)-1), void>::type
    for_each(std::tuple<Tp...> & t, FuncT f) { f(std::get<I>(t)); } // last one
    
    template<std::size_t I = 1, typename FuncT, typename... Tp>
    inline typename std::enable_if<I < (sizeof...(Tp)-1), void>::type
    for_each(std::tuple<Tp...>& t, FuncT f) {
        f(std::get<I>(t));
        for_each<I + 1, FuncT, Tp...>(t, f);  // all but the first
    }
        
    inline void add2pipe(ff_node *node) { ff_pipeline::add_stage(node); }
    inline void add2pipe(F_t F) { ff_pipeline::add_stage(new Fstage<TaskType>(F));  }

    struct add_to_pipe {
        ff_pipe *const P;
        add_to_pipe(ff_pipe *const P):P(P) {}
        template<typename T>
        void operator()(T t) const { P->add2pipe(t); }
    };
        
public:
    template<typename... Arguments>
    ff_pipe(Arguments...args) {
        std::tuple<Arguments...> t = std::make_tuple(args...);
        auto firstF = std::get<0>(t);
        add2pipe(firstF);
        for_each(t,add_to_pipe(this));
    }
    
    template<typename... Arguments>
    ff_pipe(bool input_ch, Arguments...args):ff_pipeline(input_ch) {
        std::tuple<Arguments...> t = std::make_tuple(args...);
        auto firstF = std::get<0>(t);
        add2pipe(firstF);
        for_each(t,add_to_pipe(this));
    }
    
    int add_feedback() {
        return ff_pipeline::wrap_around();
    }
    
    operator ff_node* () { return this;}
};
#endif /* HAS_CXX11_VARIADIC_TEMPLATES */
template<typename T>
ff_node* toffnode(T* p) { return p;}
template<typename T>
ff_node* toffnode(T& p) { return (ff_node*)p;}

/* --------------------------------------------------------------------------------------- */



/*!
 *  @}
 */

#define CONCAT(x, y)  x##y
#define FFPIPE_1(apply, x, ...)  apply(new _FF_pipe_stage(x))
#define FFPIPE_2(apply, x, ...)  apply(new _FF_pipe_stage(x)); FFPIPE_1(apply,  __VA_ARGS__)
#define FFPIPE_3(apply, x, ...)  apply(new _FF_pipe_stage(x)); FFPIPE_2(apply, __VA_ARGS__)
#define FFPIPE_4(apply, x, ...)  apply(new _FF_pipe_stage(x)); FFPIPE_3(apply,  __VA_ARGS__)
#define FFPIPE_5(apply, x, ...)  apply(new _FF_pipe_stage(x)); FFPIPE_4(apply,  __VA_ARGS__)
#define FFPIPE_6(apply, x, ...)  apply(new _FF_pipe_stage(x)); FFPIPE_5(apply,  __VA_ARGS__)
#define FFPIPE_7(apply, x, ...)  apply(new _FF_pipe_stage(x)); FFPIPE_6(apply,  __VA_ARGS__)
#define FFPIPE_8(apply, x, ...)  apply(new _FF_pipe_stage(x)); FFPIPE_7(apply,  __VA_ARGS__)
#define FFPIPE_9(apply, x, ...)  apply(new _FF_pipe_stage(x)); FFPIPE_8(apply,  __VA_ARGS__)
#define FFPIPE_10(apply, x, ...) apply(new _FF_pipe_stage(x)); FFPIPE_9(apply,  __VA_ARGS__)
#define FFPIPE_11(apply, x, ...) apply(new _FF_pipe_stage(x)); FFPIPE_10(apply,  __VA_ARGS__)
#define FFPIPE_12(apply, x, ...) apply(new _FF_pipe_stage(x)); FFPIPE_11(apply,  __VA_ARGS__)
#define FFPIPE_13(apply, x, ...) apply(new _FF_pipe_stage(x)); FFPIPE_12(apply,  __VA_ARGS__)
#define FFPIPE_14(apply, x, ...) apply(new _FF_pipe_stage(x)); FFPIPE_13(apply,  __VA_ARGS__)
#define FFPIPE_15(apply, x, ...) apply(new _FF_pipe_stage(x)); FFPIPE_14(apply,  __VA_ARGS__)
#define FFPIPE_16(apply, x, ...) apply(new _FF_pipe_stage(x)); FFPIPE_15(apply,  __VA_ARGS__)

#define FFPIPE_NARG(...) FFPIPE_NARG_(__VA_ARGS__, FFPIPE_RSEQ_N())
#define FFPIPE_NARG_(...) FFPIPE_ARG_N(__VA_ARGS__) 
#define FFPIPE_ARG_N(_1,_2,_3,_4,_5,_6,_7,_8, _9,_10,_11,_12,_13,_14,_15,_16, N, ...) N
#define FFPIPE_RSEQ_N() 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
#define FFPIPE_(N, apply, x, ...) CONCAT(FFPIPE_, N)(apply, x, __VA_ARGS__)
#define FFCOUNT_(_0,_1,_2,_3,_4,_5,_6,_7,_8, _9,_10,_11,_12,_13,_14,_15,_16, N, ...) N
#define FFCOUNT(...) COUNT_(X,##__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

#define FOREACHPIPE(apply, x, ...) FFPIPE_(FFPIPE_NARG(x, __VA_ARGS__), apply, x, __VA_ARGS__)

} // namespace ff

#endif /* _FF_PIPELINE_HPP_ */
