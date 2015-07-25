/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 *  \link
 *  \file map.hpp
 *  \ingroup high_level_patterns_shared_memory
 *
 *  \brief This file describes the map skeleton.
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


#ifndef FF_MAP_HPP
#define FF_MAP_HPP

#include <vector>
#include <fix8/ff/svector.hpp>
#include <fix8/ff/gt.hpp>
#include <fix8/ff/lb.hpp>
#include <fix8/ff/node.hpp>
#include <fix8/ff/farm.hpp>
#include <fix8/ff/partitioners.hpp>

// NOTE: A better check would be needed !
// both GNU g++ and Intel icpc define __GXX_EXPERIMENTAL_CXX0X__ if -std=c++0x or -std=c++11 is used
// (icpc -E -dM -std=c++11 -x c++ /dev/null | grep GXX_EX)
#if (__cplusplus >= 201103L) || (defined __GXX_EXPERIMENTAL_CXX0X__) || (defined(HAS_CXX11_AUTO) && defined(HAS_CXX11_LAMBDA))
#include <fix8/ff/parallel_for.hpp>
#else
#pragma message("C++ >= 201103L required, build will fail")
#endif

#if defined(FF_OCL)
#include <fix8/ff/oclnode.hpp>
#endif

#if defined(FF_CUDA)
#include <cuda.h>
#endif

namespace ff {

/*!
 *  \ingroup high_level_patterns_shared_memory
 *
 *  @{
 */

/* ---------------- basic high-level macros ----------------------- */

#define MAPDEF(mapname,func, basictype)                             \
    void* mapdef_##mapname(basePartitioner*const P,int tid) {		\
        LinearPartitioner<basictype>* const partitioner=			\
            (LinearPartitioner<basictype>* const)P;                 \
        LinearPartitioner<basictype>::partition_t Partition;		\
        partitioner->getPartition(tid, Partition);                  \
        basictype* p = (basictype*)(Partition.getData());           \
        size_t l = Partition.getLength();                           \
        func(p,l);                                                  \
        return p;                                                   \
    }
#define MAP(mapname, basictype, V,size,nworkers)		            \
    LinearPartitioner<basictype> P(size,nworkers);		            \
    ff_map _map_##mapname(mapdef_##mapname,&P,V)
#define RUNMAP(mapname)                                             \
    _map_##mapname.run_and_wait_end()
#define MAPTIME(mapname)                                            \
    _map_##mapname.ffTime()
#define MAPWTIME(mapname)                                           \
    _map_##mapname.ffwTime()



#if (__cplusplus >= 201103L) || (defined(HAS_CXX11_AUTO) && defined(HAS_CXX11_LAMBDA))
#define FF_MAP(mapname,V,size,func,nworkers)                        \
    parallel_for(0,size,[&V](const long i) { V[i]=func(i);},nworkers);

#else
#pragma message("C++ >= 201103L required, build will fail")
#endif


/* ---------------------------------------------------------------- */

/*!
 * \class map_lb
 *  \ingroup high_level_patterns_shared_memory
 *
 * \brief A loadbalancer for the \p map skeleton.
 *
 * The map loadbalancer extends the \p ff_loadbalancer and uses
 * ff_loadbalancer's method \p broadcast_task() to send task to all workers.
 *
 * This class is defined in \ref map.hpp
 *
 */
class map_lb: public ff_loadbalancer {
public:
    /**
     * Default constructor
     *
     * \param max_num_workers max number of workers
     */
    map_lb(int max_num_workers):ff_loadbalancer(max_num_workers) {}

    /// Broadcast the task to all workers
    void broadcast(void * task) {
        ff_loadbalancer::broadcast_task(task);
    }
};

/*!
 * \class map_gt
 *  \ingroup high_level_patterns_shared_memory
 *
 * \brief A gatherer for the \p map skeleton.
 *
 * The map gatherer extends the \p ff_gatherer and uses ff_gatherer's
 * method \p all_gather() to collect the result from all workers.
 *
 * This class is defined in \ref map.hpp
 */
class map_gt: public ff_gatherer {
public:
    /**
     * Default constructor
     *
     * \param max_num_workers max number of workers
     */
    map_gt(int max_num_workers):ff_gatherer(max_num_workers) {}

    /**
     * It collects results from all tasks.
     *
     * \return TODO
     */
    int all_gather(void * task, void **V) {
        return ff_gatherer::all_gather(task,V);
    }
};

/*!
 * \class ff_map
 *  \ingroup high_level_patterns_shared_memory
 *
 * \brief The map skeleton.
 *
 * The map skeleton, that extends the \p farm skeleton.
 *
 * This class is defined in \ref map.hpp
 *
 */
class ff_map: public ff_farm<map_lb,map_gt> {
public:
    // REW
    /**
     *  worker function type
     *  Function called by each worker thread as soon as an input task is received.
     *  The first parameter is the partitioner (that can be user-defined or one
     *  of those provided in the partitioners.hpp file) used the get a task
     *  partition for the worker.
     *  tid is the worker/thread id (from 0 to mapWorkers-1).
     */
    typedef void* (*map_worker_F_t) (basePartitioner*const, int tid);

    /**
     *  reduce function type
     *  It gets in input the array of tasks sent by each worker
     *  (one for each worker).
     *  vsize is the size of the V array.
     */
    typedef void* (*reduce_F_t) (void** V, int vsize);

private:
    // Emitter, Collector and Worker of the farm.
    /**
     * Emitter
     *
     * \return TODO
     */
    class mapE: public ff_node {
    public:
        mapE(map_lb * const lb, void* oneShotTask): lb(lb),ost(oneShotTask) {}
        void * svc(void * task) {
            if (task==NULL) {
                if (ost) lb->broadcast(ost);
                return NULL;
            }
            lb->broadcast(task);
            return GO_ON;
        }
    private:
        map_lb* lb;
        void*   ost;
    };

    /**
     * Collector
     *
     * \return TODO
     */
    class mapC: public ff_node {
    public:
        mapC(map_gt * const gt, reduce_F_t reduceF): gt(gt),reduceF(reduceF) {}

        void * svc(void *task) {
            int nw= gt->getnworkers();
            svector<void*> Task(nw);
            gt->all_gather(task, &Task[0]);
            if (reduceF) return reduceF(Task.begin(), nw);
            return Task[0];  // "default" reduceF
        }
    private:
        map_gt* const gt;
        reduce_F_t reduceF;
    };

    /**
     * Worker
     *
     * \return TODO
     */
    class mapW: public ff_node {
    public:
        mapW(map_worker_F_t mapF, basePartitioner *const P):mapF(mapF),P(P) {}
        void * svc(void * task) {
            P->setTask(task);
            return mapF(P,ff_node::get_my_id());
        }
    private:
        map_worker_F_t  mapF;
        basePartitioner * const P;
    };

public:

    /**
     *  Public Constructor (1).
     *
     *  This constructor allows to activate the map for working on a stream of
     *  tasks or as a software accelerator by setting \p input_ch \p = \p
     *  true.
     *
     *  \param mapF Specifies the \p Worker object that will execute the
     *  operations.
     *  \param mapP It is the partitioner that is responsible to partition the
     *  problem.
     *  \param reduceF The \p Reduce object. This parameter is optional and is
     *  to be specified when using a \a MapReduce skeleton. Defult is \p
     *  NULL.
     *  \param input_ch Specifies whether the map skeleton is used as an
     *  accelerator. Default is \p false.
     */
    ff_map ( map_worker_F_t mapF,
             basePartitioner* mapP,
             reduce_F_t reduceF=NULL,
             bool input_ch=false
           ) : ff_farm<map_lb,map_gt>(input_ch), mapP(mapP)
    {
        add_emitter(new mapE(getlb(),NULL));
        add_collector(new mapC(getgt(), reduceF));
        std::vector<fix8/ff_node *> w;
        for(size_t i=0;i<mapP->getParts();++i) w.push_back(new mapW(mapF,mapP));
        add_workers(w);
    }

    /**
     *  Public Constructor (2).
     *
     *  This constructor allows to activate the map for the computation of
     *  just one task
     *
     *  \param mapF Specifies the \p Worker object that will execute the operations.
     *  \param mapP It is the partitioner that is responsible to partition the
     *                 problem.
     *  \param task The task to be executed.
     *  \param reduceF The \p Reduce object. This parameter is optional and is
     * to be specified when using a \a MapReduce skeleton. Defult is \p NULL.
     */
    ff_map ( map_worker_F_t mapF,
             basePartitioner* mapP,
             void* task,
             reduce_F_t reduceF=NULL
           ) : ff_farm<map_lb,map_gt>(false), mapP(mapP)
    {
        add_emitter(new mapE(getlb(), task));
        if (reduceF)
            add_collector(new mapC(getgt(),reduceF));
        std::vector<fix8/ff_node *> w;
        for(size_t i=0;i<mapP->getParts();++i) w.push_back(new mapW(mapF,mapP));
        add_workers(w);
    }

    /**
     * Destructor
     *
     * \return TODO
     */
    ~ff_map() {
        if (end_callback) end_callback(end_callback_param);
        delete (mapE*)(getEmitter());
        mapC* C = (mapC*)(getCollector());
        if (C) delete C;
        const svector<fix8/ff_node*>& w= getWorkers();
        for(size_t i=0;i<w.size();++i) delete (mapW*)(w[i]);
    }

    int   get_my_id() const { return -1; };

    /**
     * This method sets the affinity for the emitter and collector threads,
     * both are pinned on the same core.
     *
     * \param cpuID the ID of the cpu to which the threads will be pinned
     */
    void  setAffinity(int cpuID) {
        if (cpuID<0 || !threadMapper::instance()->checkCPUId(cpuID) ) {
            error("MAP, setAffinity, invalid cpuID\n");
        }
        ((mapE*)getEmitter())->setAffinity(cpuID);
        if (getCollector())
            ((mapC*)getCollector())->setAffinity(cpuID);

        ff_node::setAffinity(cpuID);
    }

    /**
     * Retrivies the id of the cpu
     *
     * \return the core id
     */
    int   getCPUId() const { return ff_node::getCPUId();}

    /**
     * TODO
     */
    int wrap_around() {
        error("MAP, feedback channel between Emitter and Collector not supported\n");
        return -1;
    }

private:
    basePartitioner* mapP;
};


// map base task for OpenCL and CUDA implementation
class baseTask {
public:
    baseTask():task(NULL) {}
    baseTask(void*t):task(t) {}


    /* input size and bytesize */
    virtual size_t size() const =0;
    virtual size_t bytesize() const =0;

    virtual void   setTask(void* t) { if (t) task=t;}
    virtual void*  getInPtr()     { return task;}
    // by default the map works in-place
    virtual void*  newOutPtr()    { return task; }
    virtual void   deleteOutPtr() {}

protected:
    void* task;
};


#if defined(FF_OCL)

    /* The following OpenCL code macros have been derived from the SkePU OpenCL code
     * http://www.ida.liu.se/~chrke/skepu/
     *
     */
#define FFMAPFUNC(name, basictype, param, code)                         \
    static char name[] =                                                \
        "kern_" #name "|"                                               \
        #basictype "|"                                                  \
#basictype " f" #name "(" #basictype " " #param ") {" #code ";}\n"      \
"__kernel void kern_" #name "(__global " #basictype "* input,\n"        \
"                             __global " #basictype "* output,\n"       \
"                             const uint maxItems) {\n"                 \
"           int i = get_global_id(0);\n"                                \
"           uint gridSize = get_local_size(0)*get_num_groups(0);\n"     \
"           while(i < maxItems)  {\n"                                   \
"              output[i] = f" #name "(input[i]);\n"                     \
"              i += gridSize;\n"                                        \
"           }\n"                                                        \
"}"

#define FFREDUCEFUNC(name, basictype, param1, param2, code)             \
    static char name[] =                                                \
        "kern_" #name "|"                                               \
        #basictype "|"                                                  \
#basictype " f" #name "(" #basictype " " #param1 ", " #basictype " " #param2 ") {" #code ";}\n"    \
"__kernel void kern_" #name "(__global " #basictype "* input, __global " #basictype "* output, const uint n, __local " #basictype "* sdata) {\n" \
"        uint blockSize = get_local_size(0);\n"                         \
"        uint tid = get_local_id(0);\n"                                 \
"        uint i = get_group_id(0)*blockSize + get_local_id(0);\n"       \
"        uint gridSize = blockSize*get_num_groups(0);\n"                \
"        float result = 0;\n"                                           \
"        if(i < n) { result = input[i]; i += gridSize; }\n"             \
"        while(i < n) {\n"                                              \
"          result = f" #name "(result, input[i]);\n"                    \
"          i += gridSize;\n"                                            \
"        }\n"                                                           \
"        sdata[tid] = result;\n"                                        \
"        barrier(CLK_LOCAL_MEM_FENCE);\n"                               \
"        if(blockSize >= 512) { if (tid < 256 && tid + 256 < n) { sdata[tid] = f" #name "(sdata[tid], sdata[tid + 256]); } barrier(CLK_LOCAL_MEM_FENCE); }\n" \
"        if(blockSize >= 256) { if (tid < 128 && tid + 128 < n) { sdata[tid] = f" #name "(sdata[tid], sdata[tid + 128]); } barrier(CLK_LOCAL_MEM_FENCE); }\n" \
"        if(blockSize >= 128) { if (tid <  64 && tid +  64 < n) { sdata[tid] = f" #name "(sdata[tid], sdata[tid +  64]); } barrier(CLK_LOCAL_MEM_FENCE); }\n" \
"        if(blockSize >=  64) { if (tid <  32 && tid +  32 < n) { sdata[tid] = f" #name "(sdata[tid], sdata[tid +  32]); } barrier(CLK_LOCAL_MEM_FENCE); }\n" \
"        if(blockSize >=  32) { if (tid <  16 && tid +  16 < n) { sdata[tid] = f" #name "(sdata[tid], sdata[tid +  16]); } barrier(CLK_LOCAL_MEM_FENCE); }\n" \
"        if(blockSize >=  16) { if (tid <   8 && tid +   8 < n) { sdata[tid] = f" #name "(sdata[tid], sdata[tid +   8]); } barrier(CLK_LOCAL_MEM_FENCE); }\n" \
"        if(blockSize >=   8) { if (tid <   4 && tid +   4 < n) { sdata[tid] = f" #name "(sdata[tid], sdata[tid +   4]); } barrier(CLK_LOCAL_MEM_FENCE); }\n" \
"        if(blockSize >=   4) { if (tid <   2 && tid +   2 < n) { sdata[tid] = f" #name "(sdata[tid], sdata[tid +   2]); } barrier(CLK_LOCAL_MEM_FENCE); }\n" \
"        if(blockSize >=   2) { if (tid <   1 && tid +   1 < n) { sdata[tid] = f" #name "(sdata[tid], sdata[tid +   1]); } barrier(CLK_LOCAL_MEM_FENCE); }\n" \
"        if(tid == 0) output[get_group_id(0)] = sdata[tid];\n"          \
"}\n"


#define NEWMAP(name, task_t, f, input, sz)               \
    ff_mapOCL<task_t > *name =                           \
        new ff_mapOCL<task_t>(f, input,sz)
#define NEWMAPONSTREAM(task_t, f)                        \
    new ff_mapOCL<task_t>(f)
#define DELETEMAP(name)                                  \
    delete name
#define NEWREDUCE(name, task_t, f, input, sz)            \
    ff_reduceOCL<task_t> *name =                         \
        new ff_reduceOCL<task_t>(f, input,sz)
#define NEWREDUCEONSTREAM(task_t, f)                     \
    new ff_reduceOCL<task_t>(f)
#define DELETEREDUCE(name)                               \
    delete name

/*!
 * \class ff_mapOCL
 *  \ingroup high_level_patterns_shared_memory
 *
 * \brief The map skeleton.
 *
 * The map skeleton using OpenCL
 *
 * This class is defined in \ref map.hpp
 *
 */
template<typename T>
class ff_ocl: public ff_oclNode {
private:
    void setcode(const std::string &codestr) {
        int n = codestr.find("|");
        assert(n>0);
        ff_ocl<T>::kernel_name = codestr.substr(0,n);
        const std::string &tmpstr = codestr.substr(n+1);
        n = tmpstr.find("|");
        assert(n>0);

        // check double type
        if (tmpstr.substr(0,n) == "double") {
            kernel_code = "#pragma OPENCL EXTENSION cl_khr_fp64: enable\n" +
                tmpstr.substr(n+1);
        } else
            kernel_code  = tmpstr.substr(n+1);
    }

public:
    ff_ocl(const std::string &codestr):oneshot(false) {
        setcode(codestr);
        oldSize = 0;
        oldOutPtr = false;
    }
    ff_ocl(const std::string &codestr,
           typename T::base_type* task, size_t s):oneshot(true), Task(task,s) {
        setcode(codestr);
        oldSize = 0;
        oldOutPtr = false;
    }

    const T* getTask() const { return &Task; }
protected:
    inline void checkResult(cl_int s, const char* msg) {
        if(s != CL_SUCCESS) {
            std::cerr << msg << ":";
            printOCLErrorString(s,std::cerr);
        }
    }

    void svc_SetUpOclObjects(cl_device_id dId) {
        cl_int status;
        context = clCreateContext(NULL,1,&dId,NULL,NULL,&status);
        checkResult(status, "creating context");

        cmd_queue = clCreateCommandQueue (context, dId, 0, &status);
        checkResult(status, "creating command-queue");

        size_t sourceSize = kernel_code.length();

        const char* code = kernel_code.c_str();

        //printf("code=\n%s\n", code);
        program = clCreateProgramWithSource(context,1, &code, &sourceSize,&status);
        checkResult(status, "creating program with source");

        status = clBuildProgram(program,1,&dId,NULL,NULL,NULL);
        checkResult(status, "building program");

        kernel = clCreateKernel(program, kernel_name.c_str(), &status);
        checkResult(status, "CreateKernel");

        status = clGetKernelWorkGroupInfo(kernel, dId,
                                          CL_KERNEL_WORK_GROUP_SIZE,sizeof(size_t), &workgroup_size,0);
        checkResult(status, "GetKernelWorkGroupInfo");

        // allocate memory on device having the initial size
        if (Task.bytesize()>0) {
            inputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                         Task.bytesize(), NULL, &status);
            checkResult(status, "CreateBuffer input (1)");
            oldSize = Task.bytesize();
        }
    }

    void svc_releaseOclObjects(){
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseCommandQueue(cmd_queue);
        clReleaseMemObject(inputBuffer);
        if (oldOutPtr)
            clReleaseMemObject(outputBuffer);
        clReleaseContext(context);
    }

    cl_mem* getInputBuffer()  const { return (cl_mem*)&inputBuffer;}
    cl_mem* getOutputBuffer() const { return (cl_mem*)&outputBuffer;}

protected:
    const bool oneshot;
    T Task;
    std::string kernel_code;
    std::string kernel_name;
    size_t workgroup_size;
    size_t oldSize;
    bool   oldOutPtr;
    cl_context context;
    cl_program program;
    cl_command_queue cmd_queue;
    cl_mem inputBuffer;
    cl_mem outputBuffer;
    cl_kernel kernel;
    cl_int status;
};



/*!
 * \class ff_mapOCL
 *  \ingroup high_level_patterns_shared_memory
 *
 * \brief The map skeleton.
 *
 * The map skeleton using OpenCL
 *
 * This class is defined in \ref map.hpp
 *
 */
template<typename T>
class ff_mapOCL: public ff_ocl<T> {
public:

    ff_mapOCL(std::string codestr):ff_ocl<T>(codestr) {
    }


    ff_mapOCL(std::string codestr, void* task, size_t s):
        ff_ocl<T>(codestr, (typename T::base_type*)task,s) {
        assert(task);
        ff_node::skipfirstpop(true);
    }

    int  run(bool=false) { return  ff_node::run(); }
    int  wait() { return ff_node::wait(); }

    int run_and_wait_end() {
        if (run()<0) return -1;
        if (wait()<0) return -1;
        return 0;
    }

    double ffTime()  { return ff_node::ffTime();  }
    double ffwTime() { return ff_node::wffTime(); }

    const T* getTask() const { return ff_ocl<T>::getTask(); }

protected:

    void * svc(void* task) {
        cl_int   status;
        cl_event events[2];
        size_t globalThreads[1];
        size_t localThreads[1];

        ff_ocl<T>::Task.setTask(task);
        size_t size = ff_ocl<T>::Task.size();
        void* inPtr  = ff_ocl<T>::Task.getInPtr();
        void* outPtr = ff_ocl<T>::Task.newOutPtr();

        if (size  < ff_ocl<T>::workgroup_size) {
            localThreads[0]  = size;
            globalThreads[0] = size;
        } else {
            localThreads[0]  = ff_ocl<T>::workgroup_size;
            globalThreads[0] = nextMultipleOfIf(size,ff_ocl<T>::workgroup_size);
        }

        if ( ff_ocl<T>::oldSize < ff_ocl<T>::Task.bytesize() ) {
            if (ff_ocl<T>::oldSize != 0) clReleaseMemObject(ff_ocl<T>::inputBuffer);
            ff_ocl<T>::inputBuffer = clCreateBuffer(ff_ocl<T>::context, CL_MEM_READ_WRITE,
                                                    ff_ocl<T>::Task.bytesize(), NULL, &status);
            ff_ocl<T>::checkResult(status, "CreateBuffer input (2)");
            ff_ocl<T>::oldSize = ff_ocl<T>::Task.bytesize();
        }

        if (inPtr == outPtr) {
            ff_ocl<T>::outputBuffer = ff_ocl<T>::inputBuffer;
        } else {
            if (ff_ocl<T>::oldOutPtr) clReleaseMemObject(ff_ocl<T>::outputBuffer);
            ff_ocl<T>::outputBuffer = clCreateBuffer(ff_ocl<T>::context, CL_MEM_READ_WRITE,
                                                     ff_ocl<T>::Task.bytesize(), NULL, &status);
            ff_ocl<T>::checkResult(status, "CreateBuffer output");
            ff_ocl<T>::oldOutPtr = true;
        }

        status = clSetKernelArg(ff_ocl<T>::kernel, 0, sizeof(cl_mem), ff_ocl<T>::getInputBuffer());
        ff_ocl<T>::checkResult(status, "setKernelArg input");
        status = clSetKernelArg(ff_ocl<T>::kernel, 1, sizeof(cl_mem), ff_ocl<T>::getOutputBuffer());
        ff_ocl<T>::checkResult(status, "setKernelArg output");
        status = clSetKernelArg(ff_ocl<T>::kernel, 2, sizeof(cl_uint), (void *)&size);
        ff_ocl<T>::checkResult(status, "setKernelArg size");

        status = clEnqueueWriteBuffer(ff_ocl<T>::cmd_queue,ff_ocl<T>::inputBuffer,CL_FALSE,0,
                                      ff_ocl<T>::Task.bytesize(), ff_ocl<T>::Task.getInPtr(),
                                      0,NULL,NULL);
        ff_ocl<T>::checkResult(status, "copying Task to device input-buffer");

        status = clEnqueueNDRangeKernel(ff_ocl<T>::cmd_queue,ff_ocl<T>::kernel,1,NULL,
                                        globalThreads, localThreads,0,NULL,&events[0]);

        status |= clWaitForEvents(1, &events[0]);
        status |= clEnqueueReadBuffer(ff_ocl<T>::cmd_queue,ff_ocl<T>::outputBuffer,CL_TRUE, 0,
                                      ff_ocl<T>::Task.bytesize(), outPtr,0,NULL,&events[1]);
        status |= clWaitForEvents(1, &events[1]);

        clReleaseEvent(events[0]);
        clReleaseEvent(events[1]);

        //return (ff_ocl<T>::oneshot?NULL:task);
        return (ff_ocl<T>::oneshot?NULL:outPtr);
    }
};



/*!
 * \class ff_reduceOCL
 *  \ingroup high_level_patterns_shared_memory
 *
 * \brief The reduce skeleton using OpenCL
 *
 *
 */
template<typename T>
class ff_reduceOCL: public ff_ocl<T> {
public:

    ff_reduceOCL(std::string codestr):ff_ocl<T>(codestr) {}


    ff_reduceOCL(std::string codestr, void* task, size_t s):
        ff_ocl<T>(codestr, (typename T::base_type*)task,s) {

        ff_node::skipfirstpop(true);
    }

    int  run(bool=false) { return  ff_node::run(); }
    int  wait() { return ff_node::wait(); }

    int run_and_wait_end() {
        if (run()<0) return -1;
        if (wait()<0) return -1;
        return 0;
    }

    double ffTime()  { return ff_node::ffTime();  }
    double ffwTime() { return ff_node::wffTime(); }

    const T* getTask() const { return ff_ocl<T>::getTask(); }

protected:
    inline void checkResult(cl_int s, const char* msg) {
        if(s != CL_SUCCESS) {
            std::cerr << msg << ":";
            printOCLErrorString(s,std::cerr);
        }
    }

    /*!
     * Computes the number of threads and blocks to use for the reduction kernel.
     */
    inline void getBlocksAndThreads(const size_t size,
                                    const size_t maxBlocks, const size_t maxThreads,
                                    size_t & blocks, size_t &threads) {

        threads = (size < maxThreads*2) ? nextPowerOf2((size + 1)/ 2) : maxThreads;
        blocks  = (size + (threads * 2 - 1)) / (threads * 2);
        blocks  = std::min(maxBlocks, blocks);
    }

    void * svc(void* task) {
        cl_int   status = CL_SUCCESS;
        cl_event events[2];
        size_t globalThreads[1];
        size_t localThreads[1];

        ff_ocl<T>::Task.setTask(task);
        size_t size = ff_ocl<T>::Task.size();
        size_t elemSize = ff_ocl<T>::Task.bytesize()/size;
        size_t numBlocks = 0;
        size_t numThreads = 0;

        /* 64 and 256 are the max number of blocks and threads we want to use */
        getBlocksAndThreads(size, 64, 256, numBlocks, numThreads);

        size_t outMemSize =
            (numThreads <= 32) ? (2 * numThreads * elemSize) : (numThreads * elemSize);

        localThreads[0]  = numThreads;
        globalThreads[0] = numBlocks * numThreads;

        ff_ocl<T>::inputBuffer = clCreateBuffer(ff_ocl<T>::context, CL_MEM_READ_ONLY,ff_ocl<T>::Task.bytesize(), NULL, &status);
        ff_ocl<T>::checkResult(status, "CreateBuffer input (3)");

        ff_ocl<T>::outputBuffer = clCreateBuffer(ff_ocl<T>::context, CL_MEM_READ_WRITE,numBlocks*elemSize, NULL, &status);
        ff_ocl<T>::checkResult(status, "CreateBuffer output");

        status |= clSetKernelArg(ff_ocl<T>::kernel, 0, sizeof(cl_mem), ff_ocl<T>::getInputBuffer());
        status |= clSetKernelArg(ff_ocl<T>::kernel, 1, sizeof(cl_mem), ff_ocl<T>::getOutputBuffer());
        status |= clSetKernelArg(ff_ocl<T>::kernel, 2, sizeof(cl_uint), (void *)&size);
        status != clSetKernelArg(ff_ocl<T>::kernel, 3, outMemSize, NULL);
        checkResult(status, "setKernelArg ");

        status = clEnqueueWriteBuffer(ff_ocl<T>::cmd_queue,ff_ocl<T>::inputBuffer,CL_FALSE,0,
                                      ff_ocl<T>::Task.bytesize(), ff_ocl<T>::Task.getInPtr(),
                                      0,NULL,NULL);
        ff_ocl<T>::checkResult(status, "copying Task to device input-buffer");

        status = clEnqueueNDRangeKernel(ff_ocl<T>::cmd_queue,ff_ocl<T>::kernel,1,NULL,
                                        globalThreads,localThreads,0,NULL,&events[0]);
        status = clWaitForEvents(1, &events[0]);

        // Sets the kernel arguments for second reduction
        size = numBlocks;
        status |= clSetKernelArg(ff_ocl<T>::kernel, 0, sizeof(cl_mem),ff_ocl<T>::getOutputBuffer());
        status |= clSetKernelArg(ff_ocl<T>::kernel, 1, sizeof(cl_mem),ff_ocl<T>::getOutputBuffer());
        status |= clSetKernelArg(ff_ocl<T>::kernel, 2, sizeof(cl_uint),(void*)&size);
        status |= clSetKernelArg(ff_ocl<T>::kernel, 3, outMemSize, NULL);
        ff_ocl<T>::checkResult(status, "setKernelArg ");

        localThreads[0]  = numThreads;
        globalThreads[0] = numThreads;

        status = clEnqueueNDRangeKernel(ff_ocl<T>::cmd_queue,ff_ocl<T>::kernel,1,NULL,
                                        globalThreads,localThreads,0,NULL,&events[0]);
        void* outPtr = ff_ocl<T>::Task.newOutPtr();
        status |= clWaitForEvents(1, &events[0]);
        status |= clEnqueueReadBuffer(ff_ocl<T>::cmd_queue,ff_ocl<T>::outputBuffer,CL_TRUE, 0,
                                      elemSize, outPtr ,0,NULL,&events[1]);
        status |= clWaitForEvents(1, &events[1]);
        ff_ocl<T>::checkResult(status, "ERROR during OpenCL computation");

        clReleaseEvent(events[0]);
        clReleaseEvent(events[1]);

        //return (ff_ocl<T>::oneshot?NULL:task);
        return (ff_ocl<T>::oneshot?NULL:outPtr);
    }
};


#endif /* FF_OCL  */

#if defined(FF_CUDA)

#define FFMAPFUNC(name, basictype, param, code )        \
struct name {                                           \
 __device__ basictype K(basictype param) {              \
     code ;                                             \
 }                                                      \
}

#define FFREDUCEFUNC(name, basictype, param1, param2, code )     \
struct name {                                                    \
 __device__ basictype K(basictype param1, basictype param2) {    \
     code ;                                                      \
 }                                                               \
}



#define NEWMAP(name, task_t, f, input, sz)               \
    ff_mapCUDA<task_t,f> *name =                         \
        new ff_mapCUDA<task_t, f>(new f, input, sz)
#define NEWMAPONSTREAM(task_t, f)                        \
    new ff_mapCUDA<task_t, f>(new f)
#define DELETEMAP(name)                                  \
    { name->cleanup(); delete name; }


/* The following code (mapCUDAKernerl, SharedMemory and reduceCUDAKernel)
 * has been taken from the SkePU CUDA code
 * http://www.ida.liu.se/~chrke/skepu/
 *
 */

template <typename T, typename kernelF>
__global__ void mapCUDAKernel(kernelF K, T* input, T* output, size_t size) {
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int gridSize = blockDim.x*gridDim.x;

    while(i < size) {
        output[i] = K.K(input[i]);
        i += gridSize;
    }
}

#if 0

// Utility class used to avoid linker errors with extern
// unsized shared memory arrays with templated type
template<class T>
struct SharedMemory
{
    __device__ inline operator       T*()
    {
        extern __shared__ int __smem[];
        return (T*)__smem;
    }

    __device__ inline operator const T*() const
    {
        extern __shared__ int __smem[];
        return (T*)__smem;
    }
};

// specialize for double to avoid unaligned memory
// access compile errors
template<>
struct SharedMemory<double>
{
    __device__ inline operator       double*()
    {
        extern __shared__ double __smem_d[];
        return (double*)__smem_d;
    }

    __device__ inline operator const double*() const
    {
        extern __shared__ double __smem_d[];
        return (double*)__smem_d;
    }
};


template<typename T, typename kernelF, unsigned int blockSize, bool nIsPow2>
__global__ void reduceCUDAKernel(kernelF K, T *input, T *output, size_t sizen) {

    T *sdata = SharedMemory<T>();

    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockSize*2 + threadIdx.x;
    unsigned int gridSize = blockSize*2*gridDim.x;

    T result = 0;

    if(i < size) {
        result = input[i];
        // ensure we don't read out of bounds -- this is optimized away for powerOf2 sized arrays

        // There we pass it always false
        if (nIsPow2 || i + blockSize < n)
            result = K.K(result, input[i+blockSize]);
        i += gridSize;
    }

	// we reduce multiple elements per thread.  The number is determined by the
    // number of active thread blocks (via gridDim).  More blocks will result
    // in a larger gridSize and therefore fewer elements per thread
    while(i < size) {
        result = reduceFunc.CU(result, input[i]);
        // ensure we don't read out of bounds -- this is optimized away for powerOf2 sized arrays
        if (nIsPow2 || i + blockSize < n)
            result = K.K(result, input[i+blockSize]);
        i += gridSize;
    }

    // each thread puts its local sum into shared memory
    sdata[tid] = result;

    __syncthreads();

    // do reduction in shared mem
    if (blockSize >= 512) { if (tid < 256) { sdata[tid] = result = K.K(result, sdata[tid + 256]); } __syncthreads(); }
    if (blockSize >= 256) { if (tid < 128) { sdata[tid] = result = K.K(result, sdata[tid + 128]); } __syncthreads(); }
    if (blockSize >= 128) { if (tid <  64) { sdata[tid] = result = K.K(result, sdata[tid +  64]); } __syncthreads(); }

    if (tid < 32)  {
        // now that we are using warp-synchronous programming (below)
        // we need to declare our shared memory volatile so that the compiler
        // doesn't reorder stores to it and induce incorrect behavior.
        volatile T* smem = sdata;
        if (blockSize >=  64) { smem[tid] = result = K.K(result, smem[tid + 32]); }
        if (blockSize >=  32) { smem[tid] = result = K.K(result, smem[tid + 16]); }
        if (blockSize >=  16) { smem[tid] = result = K.K(result, smem[tid +  8]); }
        if (blockSize >=   8) { smem[tid] = result = K.K(result, smem[tid +  4]); }
        if (blockSize >=   4) { smem[tid] = result = K.K(result, smem[tid +  2]); }
        if (blockSize >=   2) { smem[tid] = result = K.K(result, smem[tid +  1]); }
    }

    // write result for this block to global mem
    if (tid == 0)
        output[blockIdx.x] = sdata[0];
}
#endif


/*!
 * \class ff_mapCUDA
 *  \ingroup high_level_patterns_shared_memory
 *
 * \brief The map skeleton.
 *
 * The map skeleton using OpenCL
 *
 * This class is defined in \ref map.hpp
 *
 */
template<typename T, typename kernelF>
class ff_mapCUDA: public ff_node {
public:

    ff_mapCUDA(kernelF *mapF): oneshot(false), kernel(mapF) {
        maxThreads=maxBlocks=0;
        oldSize=0;
        in_buffer = out_buffer = NULL;
    }

    ff_mapCUDA(kernelF *mapF, void* task, size_t s):
        oneshot(true),Task((typename T::base_type*)task,s), kernel(mapF) {
        assert(task);
        ff_node::skipfirstpop(true);
        maxThreads=maxBlocks=0;
        oldSize=0;
        in_buffer = out_buffer = NULL;
    }

    int  run(bool=false) { return  ff_node::run(); }
    int  wait() { return ff_node::wait(); }

    int run_and_wait_end() {
        if (run()<0) return -1;
        if (wait()<0) return -1;
        return 0;
    }

    double ffTime()  { return ff_node::ffTime();  }
    double ffwTime() { return ff_node::wffTime(); }

    const T* getTask() const { return &Task; }

    void cleanup() { if (kernel) delete kernel; }

protected:

    int svc_init() {
        int deviceID = 0;         // FIX:  we have to manage multiple devices
        cudaDeviceProp deviceProp;

        cudaSetDevice(deviceID);
        if (cudaGetDeviceProperties(&deviceProp, deviceID) != cudaSuccess)
            error("mapCUDA, error getting device properties\n");

        if(deviceProp.major == 1 && deviceProp.minor < 2)
            maxThreads = 256;
        else
            maxThreads = deviceProp.maxThreadsPerBlock;
        maxBlocks = deviceProp.maxGridSize[0];

        if(cudaStreamCreate(&stream) != cudaSuccess)
            error("mapCUDA, error creating stream\n");

        // allocate memory on device having the initial size
        if(cudaMalloc(&in_buffer, Task.bytesize()) != cudaSuccess)
            error("mapCUDA error while allocating mmemory on device\n");
        oldSize = Task.bytesize();

        return 0;
    }

    void * svc(void* task) {
        Task.setTask(task);
        size_t size = Task.size();

        void* inPtr  = Task.getInPtr();
        void* outPtr = Task.newOutPtr();

        if (oldSize < Task.bytesize()) {
            cudaFree(in_buffer);
            if(cudaMalloc(&in_buffer, Task.bytesize()) != cudaSuccess)
                error("mapCUDA error while allocating mmemory on device\n");
        }

        // async transfer data to GPU
        cudaMemcpyAsync(in_buffer, inPtr, Task.bytesize(), cudaMemcpyHostToDevice, stream);

        if (inPtr == outPtr) {
            out_buffer = in_buffer;
        } else {
            if (oldSize < Task.bytesize()) {
                if (out_buffer) cudaFree(out_buffer);
                if(cudaMalloc(&out_buffer, Task.bytesize()) != cudaSuccess)
                    error("mapCUDA error while allocating mmemory on device (output buffer)\n");
            }
        }
        oldSize = Task.bytesize();

        size_t thxblock = std::min(maxThreads, size);
        size_t blockcnt = std::min(size/thxblock + (size%thxblock == 0 ?0:1), maxBlocks);

        mapCUDAKernel<<<blockcnt,thxblock,0,stream>>>(*kernel, in_buffer, out_buffer, size);
        cudaMemcpyAsync(outPtr, out_buffer, Task.bytesize(), cudaMemcpyDeviceToHost, stream);
        cudaStreamSynchronize(stream);

        //return (oneshot?NULL:task);
        return (oneshot?NULL:outPtr);
    }

    void svc_end() {
        if (in_buffer != out_buffer)
            if (out_buffer) cudaFree(out_buffer);
        if (in_buffer) cudaFree(in_buffer);
        if(cudaStreamDestroy(stream) != cudaSuccess)
            error("mapCUDA, error destroying stream\n");
    }
private:
    const bool   oneshot;
    T            Task;
    kernelF     *kernel;     // user function
    cudaStream_t stream;
    size_t       maxThreads;
    size_t       maxBlocks;
    size_t       oldSize;
    typename T::base_type* in_buffer;
    typename T::base_type* out_buffer;
};

#endif /* FF_CUDA */

/*!
*  @}
*  \endlink
*/

} // namespace ff

#endif /* FF_MAP_HPP */
