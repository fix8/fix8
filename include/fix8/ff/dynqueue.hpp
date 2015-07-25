/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 * \link
 * \file dynqueue.hpp
 * \ingroup streaming_network_simple_shared_memory
 *
 * \brief This file defines the dynamic queue.
 *
 */

#ifndef FF_DYNQUEUE_HPP
#define FF_DYNQUEUE_HPP

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

/* Dynamic (list-based) Single-Writer Single-Reader
 * (or Single-Producer Single-Consumer) unbounded queue.
 *
 * No lock is needed around pop and push methods.
 * See also ubuffer.hpp for a more efficient SPSC unbounded queue.
 *
 *
 */

#include <stdlib.h>
#include <fix8/ff/buffer.hpp>
#include <fix8/ff/spin-lock.hpp> // used only for mp_push and mp_pop
#include <fix8/ff/sysdep.h>

namespace ff {

/*!
 * \ingroup streaming_network_simple_shared_memory
 *
 * @{
 */

#if !defined(_FF_DYNQUEUE_OPTIMIZATION)

/*!
 * \class dynqueue
 * \ingroup streaming_network_simple_shared_memory
 *
 * \brief TODO
 *
 * This class is defined in \ref dynqueue.hpp
 */
class dynqueue {
private:
    struct Node {
        void        * data;
        struct Node * next;
    };

    Node * volatile   head;
    long padding1[longxCacheLine-(sizeof(Node *)/sizeof(long))];
    Node * volatile   tail;
    long padding2[longxCacheLine-(sizeof(Node*)/sizeof(long))];

    /* ----- two-lock used only in the mp_push and mp_pop methods ------- */
    /*                                                                    */
    /*  By using the mp_push and mp_pop methods as standard push and pop, */
    /*  the dynqueue algorithm basically implements the well-known        */
    /*  Michael and Scott 2-locks MPMC queue.                             */
    /*                                                                    */
    /*                                                                    */
    union {
        lock_t P_lock;
        char padding3[CACHE_LINE_SIZE];
    };
    union {
        lock_t C_lock;
        char padding4[CACHE_LINE_SIZE];
    };

    /* -------------------------------------------------------------- */

    // internal cache
    // if mp_push and mp_pop methods are used the cache access is lock protected
#if defined(STRONG_WAIT_FREE)
    Lamport_Buffer     cache;
#else
    SWSR_Ptr_Buffer    cache;
#endif

private:
    /**
     * TODO
     */
    inline Node * allocnode() {
        union { Node * n; void * n2; } p;
#if !defined(NO_CACHE)
        if (cache.pop(&p.n2)) return p.n;
#endif
        p.n = (Node *)::malloc(sizeof(Node));
        return p.n;
    }

    /**
     * TODO
     */
    inline Node * mp_allocnode() {
        union { Node * n; void * n2; } p;
#if !defined(NO_CACHE)
        spin_lock(P_lock);
        if (cache.pop(&p.n2)) {
            spin_unlock(P_lock);
            return p.n;
        }
        spin_unlock(P_lock);
#endif
        p.n = (Node *)::malloc(sizeof(Node));
        return p.n;
    }

public:
    /**
     * TODO
     */
    enum {DEFAULT_CACHE_SIZE=1024};

    /**
     * TODO
     */
    dynqueue(int cachesize=DEFAULT_CACHE_SIZE, bool fillcache=false):cache(cachesize) {
        Node * n = (Node *)::malloc(sizeof(Node));
        n->data = NULL; n->next = NULL;
        head=tail=n;
        cache.init();
        if (fillcache) {
            for(int i=0;i<cachesize;++i) {
                n = (Node *)::malloc(sizeof(Node));
                if (n) cache.push(n);
            }
        }
        init_unlocked(P_lock);
        init_unlocked(C_lock);
        // Avoid unused private field warning on padding vars
        (void) padding1; (void) padding2 ; (void) padding3; (void) padding4;
    }

    /**
     * TODO
     */
    bool init() { return true;}

    /**
     * TODO
     */
    ~dynqueue() {
        union { Node * n; void * n2; } p;
        if (cache.buffersize()>0) while(cache.pop(&p.n2)) free(p.n);
        while(head != tail) {
            p.n = (Node*)head;
            head = head->next;
            free(p.n);
        }
        if (head) free((void*)head);
    }

    /**
     * TODO
     */
    inline bool push(void * const data) {
        if (!data) return false;
        Node * n = allocnode();
        n->data = data; n->next = NULL;
        WMB();
        tail->next = n;
        tail       = n;

        return true;
    }

    /**
     * TODO
     */
    inline bool  pop(void ** data) {
        if (!data) return false;
#if defined(STRONG_WAIT_FREE)
        if (head == tail) return false;
#else
        if (head->next)
#endif
        {
            Node * n = (Node *)head;
            *data    = (head->next)->data;
            head     = head->next;
#if !defined(NO_CACHE)
            if (!cache.push(n)) ::free(n);
#else
            ::free(n);
#endif
            return true;
        }
        return false;
    }

    /**
     * TODO
     */
    inline unsigned long length() const { return 0;}

    /**
     * MS 2-lock MPMC algorithm PUSH method
     */
    inline bool mp_push(void * const data) {
        if (!data) return false;
        Node* n = mp_allocnode();
        n->data = data; n->next = NULL;
        spin_lock(P_lock);
        tail->next = n;
        tail       = n;
        spin_unlock(P_lock);
        return true;
    }

    /**
     * MS 2-lock MPMC algorithm POP method
     */
    inline bool  mp_pop(void ** data) {
        if (!data) return false;
        spin_lock(C_lock);
        if (head->next) {
            Node * n = (Node *)head;
            *data    = (head->next)->data;
            head     = head->next;
            bool f   = cache.push(n);
            spin_unlock(C_lock);
            if (!f) ::free(n);
            return true;
        }
        spin_unlock(C_lock);
        return false;
    }
};

#else // _FF_DYNQUEUE_OPTIMIZATION
/*
 * Experimental code
 */

/*!
 * \class dynqueue
 * \ingroup streaming_network_simple_shared_memory
 *
 * \brief TODO
 *
 * This class is defined in \ref dynqueue.hpp
 */

class dynqueue {
private:
    struct Node {
        void        * data;
        struct Node * next;
    };

    Node * volatile         head;
    volatile unsigned long  pwrite;
    long padding1[longxCacheLine-((sizeof(Node *)+sizeof(unsigned long))/sizeof(long))];
    Node * volatile        tail;
    volatile unsigned long pread;
    long padding2[longxCacheLine-((sizeof(Node*)+sizeof(unsigned long))/sizeof(long))];

    const   size_t cachesize;
    void ** cache;

private:

    /**
     * TODO
     */
    inline bool cachepush(void * const data) {

        if (!cache[pwrite]) {
            /* Write Memory Barrier: ensure all previous memory write
             * are visible to the other processors before any later
             * writes are executed.  This is an "expensive" memory fence
             * operation needed in all the architectures with a weak-ordering
             * memory model where stores can be executed out-or-order
             * (e.g. Powerpc). This is a no-op on Intel x86/x86-64 CPUs.
             */
            WMB();
            cache[pwrite] = data;
            pwrite += (pwrite+1 >= cachesize) ? (1-cachesize): 1;
            return true;
        }
        return false;
    }

    /**
     * TODO
     */
    inline bool  cachepop(void ** data) {
        if (!cache[pread]) return false;

        *data = cache[pread];
        cache[pread]=NULL;
        pread += (pread+1 >= cachesize) ? (1-cachesize): 1;
        return true;
    }

public:
    /**
     * TODO
     */
    enum {DEFAULT_CACHE_SIZE=1024};

    /**
     * TODO
     */
    dynqueue(int cachesize=DEFAULT_CACHE_SIZE, bool fillcache=false):cachesize(cachesize) {
        Node * n = (Node *)::malloc(sizeof(Node));
        n->data = NULL; n->next = NULL;
        head=tail=n;

        cache=(void**)getAlignedMemory(longxCacheLine*sizeof(long),cachesize*sizeof(void*));
        if (!cache) {
            error("FATAL ERROR: dynqueue no memory available!\n");
            abort();
        }

        if (fillcache) {
            for(int i=0;i<cachesize;++i) {
                n = (Node *)::malloc(sizeof(Node));
                if (n) cachepush(n);
            }
        }
    }

    /**
     * TODO
     */
    ~dynqueue() {
        union { Node * n; void * n2; } p;
        while(cachepop(&p.n2)) free(p.n);
        while(head != tail) {
            p.n = (Node*)head;
            head = head->next;
            free(p.n);
        }
        if (head) free((void*)head);
        if (cache) freeAlignedMemory(cache);
    }

    /**
     * TODO
     */
    inline bool push(void * const data) {
        if (!data) return false;

        union { Node * n; void * n2; } p;
        if (!cachepop(&p.n2))
            p.n = (Node *)::malloc(sizeof(Node));

        p.n->data = data; p.n->next = NULL;
        WMB();
        tail->next = p.n;
        tail       = p.n;

        return true;
    }

    /**
     * TODO
     */
    inline bool  pop(void ** data) {
        if (!data) return false;
        if (head->next) {
            Node * n = (Node *)head;
            *data    = (head->next)->data;
            head     = head->next;

            if (!cachepush(n)) free(n);
            return true;
        }
        return false;
    }
};

#endif // _FF_DYNQUEUE_OPTIMIZATION

/*!
 * @}
 * \endlink
 */

} // namespace

#endif /* FF_DYNQUEUE_HPP */
