/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 * \link
 * \file ocl_utilities.hpp
 * \ingroup opencl_fastflow
 *
 * \brief TODO
 *
 */

/* ***************************************************************************
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc., 59
 *  Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  As a special exception, you may use this file as part of a free software
 *  library without restriction.  Specifically, if other files instantiate
 *  templates or use macros or inline functions from this file, or you compile
 *  this file and link it with other files to produce an executable, this file
 *  does not by itself cause the resulting executable to be covered by the GNU
 *  General Public License.  This exception does not however invalidate any
 *  other reasons why the executable file might be covered by the GNU General
 *  Public License.
 *
 ****************************************************************************
  Mehdi Goli: m.goli@rgu.ac.uk
  aldinuc: minor changes */
//OpenCL

#ifndef FF_OCL_UTILITIES_HPP
#define FF_OCL_UTILITIES_HPP


#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#include <iostream>

/*!
 * \ingroup opencl_fastflow
 *
 * @{
 */



static inline void printOCLErrorString(cl_int error, std::ostream & out) {
    switch (error){
    case CL_SUCCESS: 
        out << "CL_SUCCESS" << std::endl;
        break;
    case CL_DEVICE_NOT_FOUND: 
        out << "CL_DEVICE_NOT_FOUND" << std::endl;
        break;
    case CL_DEVICE_NOT_AVAILABLE: 
        out << "CL_DEVICE_NOT_AVAILABLE" << std::endl;
        break;
    case CL_COMPILER_NOT_AVAILABLE: 
        out << "CL_COMPILER_NOT_AVAILABLE" << std::endl;
        break;
    case CL_MEM_OBJECT_ALLOCATION_FAILURE: 
        out << "CL_MEM_OBJECT_ALLOCATION_FAILURE" << std::endl;
        break;
    case CL_OUT_OF_RESOURCES: 
        out << "CL_OUT_OF_RESOURCES" << std::endl;
        break;
    case CL_OUT_OF_HOST_MEMORY: 
        out << "CL_OUT_OF_HOST_MEMORY" << std::endl;
        break;
    case CL_PROFILING_INFO_NOT_AVAILABLE: 
        out << "CL_PROFILING_INFO_NOT_AVAILABLE" << std::endl;
        break;
    case CL_MEM_COPY_OVERLAP: 
        out << "CL_MEM_COPY_OVERLAP" << std::endl;
        break;
    case CL_IMAGE_FORMAT_MISMATCH: 
        out << "CL_IMAGE_FORMAT_MISMATCH" << std::endl;
        break;
    case CL_IMAGE_FORMAT_NOT_SUPPORTED: 
        out << "CL_IMAGE_FORMAT_NOT_SUPPORTED" << std::endl;
        break;
    case CL_BUILD_PROGRAM_FAILURE: 
        out << "CL_BUILD_PROGRAM_FAILURE" << std::endl;
        break;
    case CL_MAP_FAILURE: 
        out << "CL_MAP_FAILURE" << std::endl;
        break;
    case CL_MISALIGNED_SUB_BUFFER_OFFSET: 
        out << "CL_MISALIGNED_SUB_BUFFER_OFFSET" << std::endl;
        break;
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: 
        out << "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST" << std::endl;
        break;
    case CL_INVALID_VALUE: 
        out << "CL_INVALID_VALUE" << std::endl;
        break;
    case CL_INVALID_DEVICE_TYPE: 
        out << "CL_INVALID_DEVICE_TYPE" << std::endl;
        break;
    case CL_INVALID_PLATFORM: 
        out << "CL_INVALID_PLATFORM" << std::endl;
        break;
    case CL_INVALID_DEVICE: 
        out << "CL_INVALID_DEVICE" << std::endl;
        break;
    case CL_INVALID_CONTEXT: 
        out << "CL_INVALID_CONTEXT" << std::endl;
        break;
    case CL_INVALID_QUEUE_PROPERTIES: 
        out << "CL_INVALID_QUEUE_PROPERTIES" << std::endl;
        break;
    case CL_INVALID_COMMAND_QUEUE: 
        out << "CL_INVALID_COMMAND_QUEUE" << std::endl;
        break;
    case CL_INVALID_HOST_PTR: 
        out << "CL_INVALID_HOST_PTR" << std::endl;
        break;
    case CL_INVALID_MEM_OBJECT: 
        out << "CL_INVALID_MEM_OBJECT" << std::endl;
        break;
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: 
        out << "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR" << std::endl;
        break;
    case CL_INVALID_IMAGE_SIZE: 
        out << "CL_INVALID_IMAGE_SIZE" << std::endl;
        break;
    case CL_INVALID_SAMPLER: 
        out << "CL_INVALID_SAMPLER" << std::endl;
        break;
    case CL_INVALID_BINARY: 
        out << "CL_INVALID_BINARY" << std::endl;
        break;
    case CL_INVALID_BUILD_OPTIONS: 
        out << "CL_INVALID_BUILD_OPTIONS" << std::endl;
        break;
    case CL_INVALID_PROGRAM: 
        out << "CL_INVALID_PROGRAM" << std::endl;
        break;
    case CL_INVALID_PROGRAM_EXECUTABLE: 
        out << "CL_INVALID_PROGRAM_EXECUTABLE" << std::endl;
        break;
    case CL_INVALID_KERNEL_NAME: 
        out << "CL_INVALID_KERNEL_NAME" << std::endl;
        break;
    case CL_INVALID_KERNEL_DEFINITION: 
        out << "CL_INVALID_KERNEL_DEFINITION" << std::endl;
        break;
    case CL_INVALID_KERNEL: 
        out << "CL_INVALID_KERNEL" << std::endl;
        break;
    case CL_INVALID_ARG_INDEX: 
        out << "CL_INVALID_ARG_INDEX" << std::endl;
        break;
    case CL_INVALID_ARG_VALUE: 
        out << "CL_INVALID_ARG_VALUE" << std::endl;
        break;
    case CL_INVALID_ARG_SIZE: 
        out << "CL_INVALID_ARG_SIZE" << std::endl;
        break;
    case CL_INVALID_KERNEL_ARGS:
        out << "CL_INVALID_KERNEL_ARGS" << std::endl;
        break;
    case CL_INVALID_WORK_DIMENSION: 
        out << "CL_INVALID_WORK_DIMENSION" << std::endl;
        break;
    case CL_INVALID_WORK_GROUP_SIZE: 
        out << "CL_INVALID_WORK_GROUP_SIZE" << std::endl;
        break;
    case CL_INVALID_WORK_ITEM_SIZE: 
        out << "CL_INVALID_WORK_ITEM_SIZE" << std::endl;
        break;
    case CL_INVALID_GLOBAL_OFFSET: 
        out << "CL_INVALID_GLOBAL_OFFSET" << std::endl;
        break;
    case CL_INVALID_EVENT_WAIT_LIST: 
        out << "CL_INVALID_EVENT_WAIT_LIST" << std::endl;
        break;
    case CL_INVALID_EVENT: 
        out << "CL_INVALID_EVENT" << std::endl;
        break;
    case CL_INVALID_OPERATION: 
        out << "CL_INVALID_OPERATION" << std::endl;
        break;
    case CL_INVALID_GL_OBJECT: 
        out << "CL_INVALID_GL_OBJECT" << std::endl;
        break;
    case CL_INVALID_BUFFER_SIZE: 
        out << "CL_INVALID_BUFFER_SIZE" << std::endl;
        break;
    case CL_INVALID_MIP_LEVEL: 
        out << "CL_INVALID_MIP_LEVEL" << std::endl;
        break;
    case CL_INVALID_GLOBAL_WORK_SIZE: 
        out << "CL_INVALID_GLOBAL_WORK_SIZE" << std::endl;
        break;
    case CL_INVALID_PROPERTY: 
        out << "CL_INVALID_PROPERTY" << std::endl;
        break;
    default:
        out << "Unknown OpenCL error " << error << std::endl;
    }
}
    
/*!
 * \class Ocl_Utilities
 * \ingroup opencl_fastflow
 *
 * \brief TODO
 *
 * This class is defined in \ref ocl_utilities.hpp
 */
class Ocl_Utilities{
    //protected:
    //    int threshold;
public:
    //int  getThreshold(){
    //   return threshold;
    // }
    //int threshold;

    /**
     * TODO
     */
    Ocl_Utilities(){}//threshold=1;}

    /**
     * TODO
     */
    virtual bool  device_rules(cl_device_id id)=0;

    //void  setThreshold(int t){
    //   threshold=t;
    // }
    // if (threshold!=1){
    //      #define THRESHOLD threshold
    //}
    
    /**
     * TODO
     */
    void printStatus(std::string label, cl_int status){
  
      std::cout << label << " status: ";
      printErrorString(status);
  
      if (status != CL_SUCCESS) exit(1);
    }

    /**
     * TODO
     */
    void printErrorString(cl_int error){
        printOCLErrorString(error, std::cout);
    }
};

/*!
 * @}
 * \endlink
 * }
 */

#endif  /* FF_OCL_UTILITIES_HPP */
