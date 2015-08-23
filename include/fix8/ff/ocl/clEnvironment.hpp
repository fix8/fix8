/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 * \link
 * \file clEnvironment.hpp
 * \ingroup opencl_fastflow
 *
 * \brief TODO
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
 Mehdi Goli: m.goli@rgu.ac.uk*/

#ifndef FF_OCLENVIRONMENT_HPP
#define FF_OCLENVIRONMENT_HPP

#include <vector>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <fix8/ff/ocl/clDeviceInfo.hpp>
#include <fix8/ff/ocl/ocl_utilities.hpp>

#define THRESHOLD (512*1024*1024)

/*!
 * \ingroup opencl_fastflow
 *
 * @{
 */
pthread_mutex_t instanceMutex = PTHREAD_MUTEX_INITIALIZER;

/*!
 * \class Environment
 * \ingroup opencl_fastflow
 *
 * \brief TODO
 *
 * This class is defined in \ref clEnvironment.hpp
 */

class Environment{

private:

  static Environment * m_Environment;
  std::vector<int> clNodesDevice;
  std::vector<CLDevice*> clDevices;
  pthread_mutex_t mutex_set_policy;
  int nodeId;

  Environment ();
  Environment(Environment const&){};
  Environment& operator=(Environment const&){ return *this;};
  int staticSelectionPolicy(cl_device_type, Ocl_Utilities*);

public:
  cl_device_id getDeviceId(int);
  void createEntry(int&,  Ocl_Utilities*);
  static Environment * instance();
  cl_device_id reallocation(int);
};

#include <fix8/ff/ocl/clEnvironment.cpp>

/*!
 * @}
 * \endlink
 */
#endif /* FF_OCLENVIRONMENT_HPP */
