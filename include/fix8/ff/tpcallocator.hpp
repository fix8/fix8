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
 *  - September 2015 first version
 *
 */

#ifndef FF_TPCALLOCATOR_HPP
#define FF_TPCALLOCATOR_HPP


#include <cassert>
#include <map>
#include <fix8/ff/tpc/tpcEnvironment.hpp>
#include <fix8/ff/tpc/tpc_api.h>

using namespace rpr;
using namespace tpc;

namespace ff {

class ff_tpcallocator {
    typedef std::map<const void*, tpc_handle_t>           inner_map_t;
    typedef std::map<const tpc_dev_ctx_t *, inner_map_t>  outer_map_t;
public:

    tpc_handle_t createBuffer(const void *key,
                              tpc_dev_ctx_t *dev_ctx, tpc_device_alloc_flag_t flags, size_t size) {

        tpc_handle_t handle = 0;
        tpc_device_alloc(dev_ctx, &handle, size, flags);
        if (handle) allocated[dev_ctx][key] = handle;
        return handle;
    }

    // FIX: should be atomic !!
    tpc_handle_t createBufferUnique(const void *key,
                                    tpc_dev_ctx_t *dev_ctx, tpc_device_alloc_flag_t flags, size_t size) {
        if (allocated.find(dev_ctx) != allocated.end()) {
            inner_map_t::iterator it = allocated[dev_ctx].find(key);
            if (it != allocated[dev_ctx].end()) return it->second;
        }
        return createBuffer(key,dev_ctx,flags,size);
    }


    //FIX: we need an updateKeyUnique that is atomic
    void updateKey(const void *oldkey, const void *newkey, tpc_dev_ctx_t *dev_ctx) {
        assert(allocated.find(dev_ctx) != allocated.end());
        assert(allocated[dev_ctx].find(oldkey) != allocated[dev_ctx].end());

        tpc_handle_t handle = allocated[dev_ctx][oldkey];
        allocated[dev_ctx][newkey] = handle;
        allocated[dev_ctx].erase(oldkey);
    }


    void releaseBuffer(const void *key, tpc_dev_ctx_t *dev_ctx, tpc_handle_t handle) {
        assert(allocated.find(dev_ctx) != allocated.end());
        assert(allocated[dev_ctx].find(key) != allocated[dev_ctx].end());
        tpc_device_free(dev_ctx, handle, TPC_DEVICE_ALLOC_FLAGS_NONE);
        allocated[dev_ctx].erase(key);
    }

    void releaseAllBuffers(tpc_dev_ctx_t *dev_ctx) {
        outer_map_t::iterator it = allocated.find(dev_ctx);
        if (it == allocated.end()) return;

        inner_map_t::iterator b=it->second.begin();
        inner_map_t::iterator e=it->second.end();
        while(b != e) {
            tpc_device_free(dev_ctx, b->second, TPC_DEVICE_ALLOC_FLAGS_NONE);
            ++b;
        }
        it->second.clear();
    }

protected:
    std::map<const tpc_dev_ctx_t *, inner_map_t > allocated;
};

} // namespace

#endif /* FF_TPCALLOCATOR_HPP */
