/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 * \link
 * \file clEnvironment.cpp
 * \ingroup streaming_network_simple_shared_memory
 *
 * \brief TODO
 *
 */

/* ***************************************************************************
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  As a special exception, you may use this file as part of a free software
 *  library without restriction.  Specifically, if other files instantiate
 *  templates or use macros or inline functions from this file, or you compile
 *  this file and link it with other files to produce an executable, this
 *  file does not by itself cause the resulting executable to be covered by
 *  the GNU General Public License.  This exception does not however
 *  invalidate any other reasons why the executable file might be covered by
 *  the GNU General Public License.
 *
 ****************************************************************************
 Mehdi Goli: m.goli@rgu.ac.uk */

#include <fix8/ff/ocl/clEnvironment.hpp>

/*!
 * \ingroup streaming_network_simple_shared_memory
 *
 * @{
 */

Environment* Environment::m_Environment = NULL;

/**
 * TODO
 */
Environment::Environment(){
    if (pthread_mutex_init(&mutex_set_policy, NULL)!=0) {
        ff::error("FATAL ERROR: Environment: pthread_mutex_init fails!\n");
        abort();
    }

    nodeId =0;

    //cl_int status;
    cl_platform_id *platforms = NULL;
    cl_uint numPlatforms;

    clGetPlatformIDs(0, NULL, &(numPlatforms));
    platforms =new cl_platform_id[numPlatforms];
    clGetPlatformIDs(numPlatforms, platforms,NULL);

    for (unsigned i = 0; i<numPlatforms; i++){
        cl_uint numDevices;
        clGetDeviceIDs(platforms[i],CL_DEVICE_TYPE_ALL,0,NULL,&(numDevices));
        cl_device_id* deviceIds =new cl_device_id[numDevices];
        // Fill in CLDevice with clGetDeviceIDs()
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL,numDevices,deviceIds,NULL);

        for(unsigned j=0; j<numDevices; j++){
            // estimating max number of thread per device
            cl_ulong memSize;
            cl_device_type d;
            cl_bool b;
            cl_context context;
            cl_int status;
            clGetDeviceInfo(deviceIds[j], CL_DEVICE_TYPE, sizeof(cl_device_type), &(d), NULL);
            clGetDeviceInfo(deviceIds[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &(memSize), NULL);
            clGetDeviceInfo(deviceIds[j], CL_DEVICE_AVAILABLE, sizeof(cl_bool), &(b), NULL);
            context = clCreateContext(NULL,1,&deviceIds[j],NULL,NULL,&status);

            if((b & CL_TRUE) && (status == CL_SUCCESS)){
                CLDevice *clDevice = new CLDevice;
                clDevice->deviceId = deviceIds[j];
                clDevice->d_type=d;
                // the policy need to consider the number of SM per device as well. it shuld be more heuristic
                std::cout<<"MEMSIZE "<< memSize <<std::endl;
                std::cout<<"THRESHOLD "<< THRESHOLD <<std::endl;
                if (memSize>THRESHOLD)
                    clDevice->maxNum = (memSize/THRESHOLD);
                else  clDevice->maxNum=1;
                std::cout<<"max application number for device id="<< clDevice->deviceId << " is " << clDevice->maxNum <<std::endl;
                clDevice->runningApp=0;
                clDevices.push_back(clDevice);
            }
            clReleaseContext(context);
        }
    }
}

 /**
  * TODO
  */
int Environment::staticSelectionPolicy(cl_device_type d, Ocl_Utilities* ocl_utilities){// need to be replace
    float proportion=1;
    int i=-1;
    //int size =Environment::instance()->clDevices.size();
    for(std::vector<CLDevice*>::iterator iter=Environment::instance()->clDevices.begin();
        iter< Environment::instance()->clDevices.end(); ++iter){

        if (((*iter)->d_type & d) && (((*iter)->runningApp)/((*iter)->maxNum) < proportion ) && ocl_utilities->device_rules((*iter)->deviceId)){
            proportion= ((*iter)->runningApp)/((*iter)->maxNum);
            i= distance(Environment::instance()->clDevices.begin(), iter); //becase iter is the current one.
        }
    }
    return i;
}

/**
 * TODO
 */
cl_device_id Environment::reallocation(int t_id) {
    return Environment::instance()->clDevices.at(Environment::instance()->clNodesDevice.at(t_id))->deviceId;// this should change to dynamic policy

}

/**
 * TODO
 */
Environment * Environment::instance(){
    if (!m_Environment){
        pthread_mutex_lock(&instanceMutex);
        if (!m_Environment)
            m_Environment = new Environment();
        pthread_mutex_unlock(&instanceMutex);
    }
    return m_Environment;
}

/**
 * TODO
 */
cl_device_id Environment::getDeviceId(int t_id){
    return Environment::instance()->clDevices.at(Environment::instance()->clNodesDevice.at(t_id))->deviceId;
}

/**
 * TODO
 */
void Environment::createEntry(int& t_id , Ocl_Utilities* ocl_utilities){

    pthread_mutex_lock(&(Environment::instance()->mutex_set_policy));
    int id = staticSelectionPolicy(CL_DEVICE_TYPE_GPU, ocl_utilities);
    if (id!=-1) {
        printf("selected GPU id =%d\n",id);
        t_id = Environment::instance()->nodeId++;
        Environment::instance()->clNodesDevice.push_back(id);
    }
    else {
        id = staticSelectionPolicy(CL_DEVICE_TYPE_CPU, ocl_utilities);
        if(id!=-1){
            t_id = Environment::instance()->nodeId++;
            printf("selected CPU id=%d\n",id);
            Environment::instance()->clNodesDevice.push_back(id);
        }
    }
    Environment::instance()->clDevices.at(id)->runningApp++;
    pthread_mutex_unlock(&(Environment::instance()->mutex_set_policy));
}

/*!
 *
 * @}
 * \endlink
 */
