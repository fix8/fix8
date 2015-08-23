/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 *  \link
 *  \file ubuffer.hpp
 *  \ingroup streaming_network_simple_shared_memory
 *
 *  \brief This file contains the definition of the unbounded \p SWSR circular buffer used in
 *  FastFlow
 *
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
/* Author: Massimo Torquati
 *
 */
/**
 * Single-Writer/Single-Reader (SWSR) lock-free (wait-free) unbounded FIFO
 * queue.  No lock is needed around pop and push methods!!
 *
 * The key idea underneath the implementation is quite simple: the unbounded
 * queue is based on a pool of wait-free SWSR circular buffers (see
 * buffer.hpp).  The pool of buffers automatically grows and shrinks on demand.
 * The implementation of the pool of buffers carefully try to minimize the
 * impact of dynamic memory allocation/deallocation by using caching
 * strategies.
 *
 * More information about the uSWSR_Ptr_Buffer implementation and correctness
 * proof can be found in the following report:
 *
 * Massimo Torquati, "Single-Producer/Single-Consumer Queue on Shared Cache
 * Multi-Core Systems", TR-10-20, Computer Science Department, University
 * of Pisa Italy,2010
 * ( http://compass2.di.unipi.it/TR/Files/TR-10-20.pdf.gz )
 *
 * IMPORTANT:
 *
 * This implementation has been optimized for 1 producer and 1 consumer.
 * If you need to use more producers and/or more consumers you have
 * several possibilities (top-down order):
 *  1. to use an high level construct like the farm skeleton and compositions
 *     of multiple farms (in other words, use FastFlow ;-) ).
 *  2. to use one of the implementations in the MPMCqueues.hpp file
 *  3. to use the SWSR_Ptr_Buffer but with the mp_push and the mc_pop methods
 *     both protected by (spin-)locks in order to protect the internal data
 *     structures.
 *
 */


#ifndef FF_uSWSR_PTR_BUFFER_HPP
#define FF_uSWSR_PTR_BUFFER_HPP

#include <assert.h>
#include <cassert>
#include <new>
#include <fix8/ff/dynqueue.hpp>
#include <fix8/ff/buffer.hpp>
#include <fix8/ff/spin-lock.hpp>
#if defined(HAVE_ATOMIC_H)
#include <asm/atomic.h>
#else
#include <fix8/ff/atomic/atomic.h>
#endif
namespace ff {

/*!
 *
 *  \ingroup streaming_network_simple_shared_memory
 *
 * @{
 */

/* Do not change the following define unless you know what you're doing */
#define INTERNAL_BUFFER_T SWSR_Ptr_Buffer  /* bounded SPSC buffer */

/*!
 *
 * \class BufferPool
 *  \ingroup streaming_network_simple_shared_memory
 *
 * \breif It defines a pool of buffers used to create and unbounded SWSR buffer.
 *
 * This class is defined in \ref ubuffer.hpp
 *
 */
class BufferPool {
public:
    BufferPool(int cachesize, const bool fillcache=false, unsigned long size=-1)
        :inuse(cachesize),bufcache(cachesize) {
        bufcache.init(); // initialise the internal buffer and allocates memory

        if (fillcache) {
            assert(size>0);
            union { INTERNAL_BUFFER_T * buf; void * buf2;} p;
            for(int i=0;i<cachesize;++i) {
                p.buf = (INTERNAL_BUFFER_T*)malloc(sizeof(INTERNAL_BUFFER_T));
                new (p.buf) INTERNAL_BUFFER_T(size);
#if defined(uSWSR_MULTIPUSH)
                p.buf->init(true);
#else
                p.buf->init();
#endif
                bufcache.push(p.buf);
            }
        }

#if defined(UBUFFER_STATS)
        miss=0;hit=0;
        (void)padding1;
#endif
    }

    /**
     *
     * TODO
     */
    ~BufferPool() {
        union { INTERNAL_BUFFER_T * b1; void * b2;} p;

        while(inuse.pop(&p.b2)) {
            p.b1->~INTERNAL_BUFFER_T();
            free(p.b2);
        }
        while(bufcache.pop(&p.b2)) {
            p.b1->~INTERNAL_BUFFER_T();
            free(p.b2);
        }
    }

    /**
     * It returns a pointer to the next internal buffer to write
     */
    inline INTERNAL_BUFFER_T * next_w(unsigned long size)  {
        union { INTERNAL_BUFFER_T * buf; void * buf2;} p;
        if (!bufcache.pop(&p.buf2)) {
#if defined(UBUFFER_STATS)
            ++miss;
#endif
            p.buf = (INTERNAL_BUFFER_T*)malloc(sizeof(INTERNAL_BUFFER_T));
            new (p.buf) INTERNAL_BUFFER_T(size);
#if defined(uSWSR_MULTIPUSH)
            if (!p.buf->init(true)) return NULL;
#else
            if (!p.buf->init()) return NULL;
#endif
        }
#if defined(UBUFFER_STATS)
        else  ++hit;
#endif
        inuse.push(p.buf);
        return p.buf;
    }

    /**
     * It returns a pointer to the next internal buffer to read
     */
    inline INTERNAL_BUFFER_T * next_r()  {
        union { INTERNAL_BUFFER_T * buf; void * buf2;} p;
        return (inuse.pop(&p.buf2)? p.buf : NULL);
    }

    /**
     * TODO
     */
    inline void release(INTERNAL_BUFFER_T * const buf) {
        buf->reset();
        if (!bufcache.push(buf)) {
            buf->~INTERNAL_BUFFER_T();
            free(buf);
        }
    }

#if defined(UBUFFER_STATS)
    /**
     * TODO
     */
    inline unsigned long readPoolMiss() {
        unsigned long m = miss;
        miss = 0;
        return m;
    }

    /**
     * TODO
     */
    inline unsigned long readPoolHit() {
        unsigned long h = hit;
        hit = 0;
        return h;
    }
#endif

    // just empties the inuse bucket putting data in the cache
    void reset() {
        union { INTERNAL_BUFFER_T * b1; void * b2;} p;
        while(inuse.pop(&p.b2)) release(p.b1);
    }

private:
#if defined(UBUFFER_STATS)
    unsigned long      miss,hit;
    long padding1[longxCacheLine-2];
#endif

    dynqueue           inuse;    // of type dynqueue, that is a Dynamic (list-based)
                                 // SWSR unbounded queue.
                                 // No lock is needed around pop and push methods.
    INTERNAL_BUFFER_T  bufcache; // This is a bounded buffer
};

// --------------------------------------------------------------------------------------

/*!
 *  \ingroup streaming_network_simple_shared_memory
 *
 *  @{
 */

 /*!
  * \class uSWSR_Ptr_Buffer
 *  \ingroup streaming_network_simple_shared_memory
  *
  * \brief Unbounded Single-Writer/Single-Reader circular buffer.
  *
  * The unbounded SWSR circular buffer is based on a pool of wait-free SWSR
  * circular buffers (see buffer.hpp). The pool of buffers automatically grows
  * and shrinks on demand. The implementation of the pool of buffers carefully
  * tries to minimize the impact of dynamic memory allocation/deallocation by
  * using caching strategies. The unbounded buffer is based on the
  * INTERNAL_BUFFER_T.
  *
  * This class is defined in \ref ubuffer.hpp
  */
class uSWSR_Ptr_Buffer {
private:
    enum {CACHE_SIZE=32};

#if defined(uSWSR_MULTIPUSH)
    enum { MULTIPUSH_BUFFER_SIZE=16};

    // Multipush: push a bach of items.
    inline bool multipush() {
        if (buf_w->multipush(multipush_buf,MULTIPUSH_BUFFER_SIZE)) {
            mcnt=0;
            return true;
        }

        if (fixedsize) return false;
        // try to get a new buffer
        INTERNAL_BUFFER_T * t = pool.next_w(size);
        assert(t); // if (!t) return false; // EWOULDBLOCK
        buf_w = t;
        in_use_buffers++;
        buf_w->multipush(multipush_buf,MULTIPUSH_BUFFER_SIZE);
        mcnt=0;
#if defined(UBUFFER_STATS)
        atomic_long_inc(&numBuffers);
#endif
        return true;
    }
#endif

public:
    /**
     *  Constructor
     *
     *  \param n the size of the buffer
     *  \param fixedsize a boolean flag that asserts whether the buffer can be
     *  resized. Default is \p false.
     *  \param fillcache a flag.
     */
    uSWSR_Ptr_Buffer(unsigned long n, const bool fixedsize=false, const bool fillcache=false):
        buf_r(0),buf_w(0),in_use_buffers(1),size(n),fixedsize(fixedsize),
        pool(CACHE_SIZE,fillcache,size) {
        init_unlocked(P_lock); init_unlocked(C_lock);
#if defined(UBUFFER_STATS)
        atomic_long_set(&numBuffers,0);
#endif
        // Avoid unused private field warning on padding fields
        (void)padding1; (void)padding2; (void)padding3; (void)padding4;

    }

    /** Destructor */
    ~uSWSR_Ptr_Buffer() {
        if (buf_r) {
            buf_r->~INTERNAL_BUFFER_T();
            free(buf_r);
        }
        // buf_w either is equal to buf_w or is freed by BufferPool destructor
    }

    /**
     *  Initialise the unbounded buffer.
     */
    bool init() {
        if (buf_w || buf_r) return false;
#if defined(uSWSR_MULTIPUSH)
        if (size<=MULTIPUSH_BUFFER_SIZE) return false;
#endif
        buf_r = (INTERNAL_BUFFER_T*)::malloc(sizeof(INTERNAL_BUFFER_T));
        if (!buf_r) return false;
        new ((void *)buf_r) INTERNAL_BUFFER_T(size);
#if defined(uSWSR_MULTIPUSH)
        if (!buf_r->init(true)) return false;
#else
        if (!buf_r->init()) return false;
#endif
        buf_w = buf_r;
        return true;
    }

    /**
     * It returns true if the buffer is empty.
     *
     * \return If successful \p true is returned, otherwise \p false is
     * returned.
     */
    inline bool empty() {
        //return buf_r->empty();
        if (buf_r->empty()) { // current buffer is empty
            if (buf_r == buf_w) return true;
        }
        return false;
    }

    /**
     * It returns true if there is at least one room in the buffer.
     *
     * \return If available \p true is returned, otherwise \p false is
     * returned.
     */
    inline bool available()   {
        return buf_w->available();
    }

    /**
     *  Push Method: push the input value into the queue.\n
     *  If fixedsize has been set to \p true, this method may
     *  return false. This means EWOULDBLOCK
     *  and the call should be retried.
     *
     *  \param data Data to be pushed in the buffer
     *  \return \p false if \p fixedsize is set to \p true OR if \p data is NULL
     *  OR if there is not a buffer to write to.
     *
     *  \return \p true if the push succedes.
     */
    inline bool push(void * const data) {
        /* NULL values cannot be pushed in the queue */
        if (!data || !buf_w) return false;

        // If fixedsize has been set to \p true, this method may
        // return false. This means EWOULDBLOCK
        if (!available()) {

            if (fixedsize) return false;

            // try to get a new buffer
            INTERNAL_BUFFER_T * t = pool.next_w(size);
            assert(t); //if (!t) return false; // EWOULDBLOCK
            buf_w = t;
            in_use_buffers++;
#if defined(UBUFFER_STATS)
            atomic_long_inc(&numBuffers);
#endif
        }
        //DBG(assert(buf_w->push(data)); return true;);
        buf_w->push(data);
        return true;
    }

    /**
     *  Multi-producers push method (lock protected)
     *
     *  \return TODO
     *
     */
    inline bool mp_push(void *const data) {
        spin_lock(P_lock);
        bool r=push(data);  // should it be mpush(data)?
        spin_unlock(P_lock);
        return r;
    }

#if defined(uSWSR_MULTIPUSH)
    /**
     *
     * massimot: experimental code
     *
     * This method provides the same interface of the push one but uses the
     * multipush method to provide a batch of items to the consumer thus
     * ensuring better cache locality and lowering the cache trashing.
     *
     * \return TODO
     */
    inline bool mpush(void * const data) {
        if (!data) return false;

        if (mcnt==MULTIPUSH_BUFFER_SIZE)
            return multipush();

        multipush_buf[mcnt++]=data;

        if (mcnt==MULTIPUSH_BUFFER_SIZE) return multipush();

        return true;
    }

    inline bool flush() {
        return (mcnt ? multipush() : true);
    }
#endif /* uSWSR_MULTIPUSH */

    /**
     *  Pop method: get the next data from the buffer
     *
     *  \param data Double pointer to the location where to store the data
     *  popped from the buffer
     *
     */
    inline bool  pop(void ** data) {
        if (!data || !buf_r) return false;
        if (buf_r->empty()) { // current buffer is empty
            if (buf_r == buf_w) return false;
            if (buf_r->empty()) { // we have to check again
                INTERNAL_BUFFER_T * tmp = pool.next_r();
                if (tmp) {
                    // there is another buffer, release the current one
                    pool.release(buf_r);
                    in_use_buffers--;
                    buf_r = tmp;

#if defined(UBUFFER_STATS)
                    atomic_long_dec(&numBuffers);
#endif
                }
            }
        }
        //DBG(assert(buf_r->pop(data)); return true;);
        return buf_r->pop(data);
    }

#if defined(UBUFFER_STATS)

    /**
     * TODO
     */
    inline unsigned long queue_status() {
        return (unsigned long)atomic_long_read(&numBuffers);
    }

    /**
     * TODO
     */
    inline unsigned long readMiss() {
        return pool.readPoolMiss();
    }

    /**
     * TODO
     */
    inline unsigned long readHit() {
        return pool.readPoolHit();
    }
#endif

    /**
     * It defines multi-consumers pop method. lock protected.
     *
     * \return TODO
     */
    inline bool mc_pop(void ** data) {
        spin_lock(C_lock);
        bool r=pop(data);
        spin_unlock(C_lock);
        return r;
    }

    /**
     * It returns the length of the queue.
     * Note that this is just a rough estimation of the actual queue length.
     *
     * \return TODO
     */
    inline unsigned long length() const {
        unsigned long len = buf_r->length();
        if (buf_r == buf_w) return len;
        assert(in_use_buffers>2);
        return len+(in_use_buffers-2)*size+buf_w->length();
    }

    /**
     *  Resets the buffer
     */
    inline void reset() {
        if (buf_r) buf_r->reset();
        if (buf_w) buf_w->reset();
        buf_w = buf_r;
        pool.reset();
    }

private:
    // Padding is required to avoid false-sharing between
    // core's private cache
    INTERNAL_BUFFER_T * buf_r;
    long padding1[longxCacheLine-1];
    INTERNAL_BUFFER_T * buf_w;
    long padding2[longxCacheLine-1];

    /* ----- two-lock used only in the mp_push and mc_pop methods ------- */
    union {
        lock_t P_lock;
        char padding3[CACHE_LINE_SIZE];
    };
    union {
        lock_t C_lock;
        char padding4[CACHE_LINE_SIZE];
    };
    /* -------------------------------------------------------------- */
#if defined(UBUFFER_STATS)
    atomic_long_t numBuffers;
#endif

#if defined(uSWSR_MULTIPUSH)
    /* massimot: experimental code (see multipush)
     *
     */
    // local multipush buffer used by the mpush method
    void  * multipush_buf[MULTIPUSH_BUFFER_SIZE];
    int     mcnt;
#endif
    unsigned long       in_use_buffers; // used to estimate queue length
    const unsigned long	size;
    const bool			fixedsize;
    BufferPool			pool;
};

/*!
 * @}
 * \endlink
 */

} // namespace ff

#endif /* FF_uSWSR_PTR_BUFFER_HPP */
