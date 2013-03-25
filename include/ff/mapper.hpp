/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 *  \file mapper.hpp
 *  \brief This file contains the thread mapper definition used in FastFlow
 */
  
#ifndef __THREAD_MAPPER_HPP_
#define __THREAD_MAPPER_HPP_
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

#include <stdlib.h>
#include <ff/svector.hpp>
#include <ff/utils.hpp>
#include <ff/mapping_utils.hpp>

namespace ff {

/*!
 *  \ingroup runtime
 *
 *  @{
 */

/*! 
  * \class threadMapper
  *
  * \brief The thread mapper allows to map threads to specific core using a 
  *  predefined mapping policy.
  *
  * The threadMapper stores a list of CPU ids. By default the list is simply a 
  * linear sequence of core ids of the system, for example in a quad-core system 
  * the default list is 0 1 2 3. It is possible to change the default list using 
  * the method setMappingList by passing a string of space-serated 
  * (or comma-separated) CPU ids. The policy implemented in the threadManager is 
  * to pick up a CPU id from the list using a round-robin policy.
  *
  */ 
class threadMapper {
public:
    /** Get a static instance of the threadMapper object */
    static inline threadMapper* instance() {
        static threadMapper thm;
        return &thm;
    }
    
    /**
     * Default constructor. 
     */
    threadMapper():rrcnt(-1),mask(0),num_cores(ff_numCores()),CList(num_cores) {
        if (num_cores <= 0) {
            error("threadMapper: invalid num_cores\n");
            return;
        }
        
        unsigned long size=num_cores;
        // usually num_cores is a power of two....!
        if (!isPowerOf2(size)) size = nextPowerOf2(size);
        CList.reserve(size);
 
        mask = size-1;
        
        for(size_t i=0;i<num_cores;++i) CList.push_back(i);
        for(size_t i=num_cores,j=0; i<size;++i,j++) CList.push_back(j);
        
        rrcnt=0;
    }
    /*!
     * Allows to set a new list of CPU ids.
     *
     * The str variable should contain a space-separated or a comma-separated list 
     * of CPU ids.
     * For example if the string str is "0 1 1 2 3", then the first thread will be 
     * bind to CPU 0, the second to CPU 1, the third to CPU 1, the fourth to CPU 2, 
     * the fifth to CPU 3. Then it follows the same rule for the subsequent 
     * threads.
     */
    void setMappingList(const char* str) {
        rrcnt=0;// reset rrcnt

        if (str==NULL) return; // use the previous mapping list
        char* _str=const_cast<char*>(str), *_str_end;
        svector<int> List(mask+1);
        do {
            while (*_str==' ' || *_str=='\t' || *_str==',') ++_str;
            unsigned cpuid = strtoul(_str, &_str_end, 0);
            if (_str == _str_end) {
                error("setMapping, invalid mapping string\n");
                return;
            }
            if (cpuid>(num_cores-1)) {
                error("setMapping, invalid cpu id in the mapping string\n");
                return;
            }
            _str=_str_end;
            List.push_back(cpuid);

            if (*_str == '\0')	break;
        } while (1);

        unsigned long size=List.size();
        if (!isPowerOf2(size)) {
            size=nextPowerOf2(size);
            List.reserve(size);
        }
        mask = size-1;
        for(size_t i=List.size(),j=0; i<size;++i,j++) List.push_back(List[j]);
        CList=List;
    }
    
    /**
     *  Returns the next CPU id using a round-robin mapping access on the 
     *  mapping list. 
     */ 
    int getCoreId() { 
        assert(rrcnt>=0);
        int id=CList[rrcnt++];
        rrcnt &= mask;
        return id;
    }

    int getCoreId(unsigned long tid) { 
        int id=CList[tid & mask];
        return id;
    }

    inline const bool checkCPUId(const int cpuId) const {
        return ((unsigned)cpuId < num_cores);
    }

protected:
    long          rrcnt;
    unsigned long mask;
    unsigned long num_cores;
    svector<int> CList;
};

} // namespace ff

/*!
 *
 * @}
 */

#endif /* __THREAD_MAPPER_HPP_ */
