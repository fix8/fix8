/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*! \file pipeline.hpp
 *  \brief This file describes the pipeline skeleton.
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

namespace ff {

/*!
 *  \ingroup high_level
 *
 *  @{
 */


/*!
 *  \class ff_pipeline
 *
 *  \brief The Pipeline skeleton.
 *
 *  Pipelining is one of the simplest parallel patterns where data flows through 
 *  a series of stages (or nodes) and each stage processes the input data in some 
 *  ways, producing as output a modified version or new data. A pipeline's stage 
 *  can operate sequentially or in parallel and may or may not have an internal 
 *  state.
 */
 
class ff_pipeline: public ff_node {
protected:
    /// Prepare the Pipeline skeleton for execution
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

    /*! 
     *  This function is required when no manager threads are present in the 
     *  pipeline, which would allow to freeze other threads before starting the 
     *  computation.
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
    enum { DEF_IN_BUFF_ENTRIES=512, DEF_OUT_BUFF_ENTRIES=(DEF_IN_BUFF_ENTRIES+128)};

    /*!
     *  Constructor
     *  \param input_ch = true to set accelerator mode
     *  \param in_buffer_entries = input queue length
     *  \param out_buffer_entries = output queue length
     *  \param fixedsize = true uses only fixed size queue
     */
    ff_pipeline(bool input_ch=false,
                int in_buffer_entries=DEF_IN_BUFF_ENTRIES,
                int out_buffer_entries=DEF_OUT_BUFF_ENTRIES, bool fixedsize=true):
        has_input_channel(input_ch),prepared(false),
        in_buffer_entries(in_buffer_entries),
        out_buffer_entries(out_buffer_entries),fixedsize(fixedsize) {               
    }
    
    ~ff_pipeline() {
        if (barrier) delete barrier;
    }

    /**
     *  Add a stage to the Pipeline
     *
     *  \param s a ff_node that is the stage to be added to the skeleton. The stage 
     *  contains the task that has to be executed.
     */
    int add_stage(ff_node * s) {        
        nodes_list.push_back(s);
        return 0;
    }

    /**
     * the last stage output queue will be connected 
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

    /// Run the Pipeline skeleton.
    int run(bool skip_init=false) {
        int nstages=static_cast<int>(nodes_list.size());

        if (!skip_init) {            
            // set the initial value for the barrier 
            if (!barrier)  barrier = new BARRIER_T;
            barrier->barrierSetup(cardinality(barrier));
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

    /// Run and wait all threads to finish.
    int run_and_wait_end() {
        if (isfrozen()) return -1; // FIX !!!!
        stop();
        if (run()<0) return -1;           
        if (wait()<0) return -1;
        return 0;
    }
    
    /// Run and then freeze.
    int run_then_freeze() {
        if (isfrozen()) {
            thaw();
            freeze();
            return 0;
        }
        if (!prepared) if (prepare()<0) return -1;
        freeze();
        return run();
    }
    
    /// Wait for a stage to complete its task
    int wait(/* timeval */ ) {
        int ret=0;
        for(unsigned int i=0;i<nodes_list.size();++i)
            if (nodes_list[i]->wait()<0) {
                error("PIPE, waiting stage thread, id = %d\n",nodes_list[i]->get_my_id());
                ret = -1;
            } 
        
        return ret;
    }
    
    /// Wait freezing.
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
    
    /// Stop all stages
    void stop() {
        for(unsigned int i=0;i<nodes_list.size();++i) nodes_list[i]->stop();
    }
    /// Freeze all stages    
    void freeze() {
        for(unsigned int i=0;i<nodes_list.size();++i) nodes_list[i]->freeze();
    }
    /// Thaw all frozen stages    
    void thaw() {
        for(unsigned int i=0;i<nodes_list.size();++i) nodes_list[i]->thaw();
    }
    
    /** check if the pipeline is frozen */
    bool isfrozen() { 
        int nstages=static_cast<int>(nodes_list.size());
        for(int i=0;i<nstages;++i) 
            if (!nodes_list[i]->isfrozen()) return false;
        return true;
    }

    /** Offload the given task to the pipeline */
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
    
    /*! 
     *  Load results. If \p false, EOS arrived or too many retries.
     *  If \p true, there is a new value
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

    // return values:
    //   false: no task present
    //   true : there is a new value, you should check if the task is an FF_EOS
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
    
    int   cardinality(BARRIER_T * const barrier)  { 
        int card=0;
        for(unsigned int i=0;i<nodes_list.size();++i) 
            card += nodes_list[i]->cardinality(barrier);
        
        return card;
    }
    
    /* the returned time comprise the time spent in svn_init and 
     * in svc_end methods
     */
    double ffTime() {
        return diffmsec(nodes_list[nodes_list.size()-1]->getstoptime(),
                        nodes_list[0]->getstarttime());
    }
    
    /*  the returned time considers only the time spent in the svc
     *  methods
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
    
    /// ff_node interface
    void* svc(void * task) { return NULL; }
    int   svc_init() { return -1; };
    void  svc_end()  {}
    int   get_my_id() const { return -1; };
    void  setAffinity(int) { 
        error("PIPE, setAffinity: cannot set affinity for the pipeline\n");
    }
    int   getCPUId() { return -1;}

    int create_input_buffer(int nentries, bool fixedsize) { 
        if (in) return -1;
        if (nodes_list[0]->create_input_buffer(nentries, fixedsize)<0) {
            error("PIPE, creating input buffer for node 0\n");
            return -1;
        }
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
    int in_buffer_entries;
    int out_buffer_entries;
    std::vector<ff_node *> nodes_list;
    bool fixedsize;
};


/*!
 *  @}
 */


} // namespace ff

#endif /* _FF_PIPELINE_HPP_ */
