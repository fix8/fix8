/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

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
 * **************************************************************************/

/*
 *   Author:
 *      Massimo Torquati
 *
 *  - August 2015 first version
 *
 */

#ifndef FF_OCLALLOCATOR_HPP
#define FF_OCLALLOCATOR_HPP

#if !defined(FF_OPENCL)
#error FF_OPENCL not defined
#endif

#include <cassert>
#include <map>
#include <fix8/ff/ocl/clEnvironment.hpp>

namespace ff {

class ff_oclallocator {
    typedef std::map<const void*, cl_mem>            inner_map_t;
    typedef std::map<cl_context, inner_map_t>  outer_map_t;
public:

    cl_mem createBuffer(const void *key,
                        cl_context ctx, cl_mem_flags flags, size_t size, cl_int *status) {
        cl_mem ptr = clCreateBuffer(ctx,flags,size, NULL,status);
        if (*status == CL_SUCCESS) allocated[ctx][key] = ptr;
        return ptr;
    }

    cl_mem createBufferUnique(const void *key,
                              cl_context ctx, cl_mem_flags flags, size_t size, cl_int *status) {
        if (allocated.find(ctx) != allocated.end()) {
            inner_map_t::iterator it = allocated[ctx].find(key);
            if (it != allocated[ctx].end()) {
                *status = CL_SUCCESS;
                return it->second;
            }
        }
        return createBuffer(key,ctx,flags,size,status);
    }

    void updateKey(const void *oldkey, const void *newkey, cl_context ctx) {
        assert(allocated.find(ctx) != allocated.end());
        assert(allocated[ctx].find(oldkey) != allocated[ctx].end());

        cl_mem ptr = allocated[ctx][oldkey];
        allocated[ctx][newkey] = ptr;
        allocated[ctx].erase(oldkey);
    }


    cl_int releaseBuffer(const void *key, cl_context ctx, cl_mem ptr) {
        assert(allocated.find(ctx) != allocated.end());
        assert(allocated[ctx].find(key) != allocated[ctx].end());
        if (clReleaseMemObject(ptr) == CL_SUCCESS) {
            allocated[ctx].erase(key);
            return CL_SUCCESS;
        }
        return CL_INVALID_MEM_OBJECT;
    }

    cl_int releaseAllBuffers(cl_context ctx) {
        outer_map_t::iterator it = allocated.find(ctx);
        if (it == allocated.end()) return CL_SUCCESS;

        inner_map_t::iterator b=it->second.begin();
        inner_map_t::iterator e=it->second.end();
        while(b != e) {
            if (clReleaseMemObject(b->second) != CL_SUCCESS) return CL_INVALID_MEM_OBJECT;
            ++b;
        }
        it->second.clear();
        return CL_SUCCESS;
    }

protected:
    std::map<cl_context, std::map<const void*, cl_mem> > allocated;
};

} // namespace

#endif /* FF_OCLALLOCATOR_HPP */
