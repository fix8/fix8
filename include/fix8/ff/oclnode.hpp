/* -*- Mode: C++; tab-width: 2; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 * \link
 * \file oclnode.hpp
 * \ingroup streaming_network_arbitrary_shared_memory
 *
 * \brief Defines the OpenCL implementation of FastFlow node
 *
 * This files defines the FastFlow implementation of OpenCL. This
 * implementation enables us to support FastFlow on the GPGPUs.
 *
 */

/* ***************************************************************************
 *
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
  Mehdi Goli: m.goli@rgu.ac.uk*/

#ifndef FF_OCLNODE_HPP
#define FF_OCLNODE_HPP

#include <fix8/ff/ocl/clEnvironment.hpp>
#include <fix8/ff/node.hpp>

namespace ff{

/*!
 * \ingroup streaming_network_arbitrary_shared_memory
 *
 * @{
 */

/*!
 * \class ff_oclNode
 * \ingroup streaming_network_arbitrary_shared_memory
 *
 * \brief OpenCL implementation of FastFlow node
 *
 * This class defines the OpenCL implementation of FastFlow node.
 *
 * This class is defined in \ref ff_oclnode.hpp
 *
 */
class ff_oclNode : public ff_node, public Ocl_Utilities {
public:

    /**
     * \brief Setup OCL object
     *
     * It is a pure virtual function. It sets up the OpenCL object of the FastFlow node.
     *
     * \parm id is the identifier of the opencl device
     */
    virtual void svc_SetUpOclObjects(cl_device_id id)=0;

    /**
     * \brief Releases OCL object
     *
     * It is a pure virtual function, and releases the OpenCL object of the
     * FastFlow node.
     *
     */
    virtual void svc_releaseOclObjects()=0;

protected:

    /**
     * \brief Intializes OpenCL instance
     *
     * It initializes the OpenCL instance.
     *
     * \return If successful \p true is returned, otherwise \p false is
     * returned.
     */
    bool initOCLInstance() {
        Environment::instance()->createEntry(tId, this);
        if(tId>=0) {
            baseclass_ocl_node_deviceId = Environment::instance()->getDeviceId(tId);
            return true;
        }
        return false;
    }

    /**
     * \brief Constructor
     *
     * It construct the OpenCL node for the device.
     *
     */
    ff_oclNode():tId(-1), baseclass_ocl_node_deviceId(NULL) { }

    /**
     * \brief Device rules
     *
     * It defines the ruls for the device.
     *
     * \parm id is the identifier of the device
     *
     * \return \p true is always returned
     */
    bool  device_rules(cl_device_id id){ return true;}

    /**
     * \brief Creates OpenCL
     *
     * It initializes OpenCL instance,
     */
    inline void svc_createOCL(){
        if (baseclass_ocl_node_deviceId==NULL) {
            if (initOCLInstance()){
                svc_SetUpOclObjects(baseclass_ocl_node_deviceId);
            }else{
                error("FATAL ERROR: Instantiating the Device: Failed to instatiate the device!\n");
                abort();
            }
        }
        else if (evaluation()) {
            svc_releaseOclObjects();
            svc_SetUpOclObjects(baseclass_ocl_node_deviceId);
        }
    }

    /**
     * \brief Releases OpenCL
     *
     * It releases OpenCL instance.
     */
    inline void svc_releaseOCL(){
        svc_releaseOclObjects();
    }

    /**
     * \brief Evaluation
     *
     * it evalautes the OpenCL node on the device.
     *
     * \return If successful \p true, otherwise \p false
     */
    inline bool evaluation(){
        cl_device_id nDId = Environment::instance()->reallocation(tId);
        if (nDId != baseclass_ocl_node_deviceId) {
            baseclass_ocl_node_deviceId = nDId;
            return true;
        }
        return false;
    }

private:
    int tId; // the node id
    cl_device_id baseclass_ocl_node_deviceId; // is the id which is provided for user
};

/*!
 * @}
 * \endlink
 */
}
#endif /* FF_OCLNODE_HPP */
