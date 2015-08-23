/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 * \link
 * \file dynlinkedlist.hpp
 * \ingroup shared_memory_fastflow
 *
 * \brief Dynamic linked list Single-Writer Single-Reader unbounded queue.
 * No lock is needed around pop and push methods.
 *
 */
#ifndef FF_DYNLINKEDLIST_HPP
#define FF_DYNLINKEDLIST_HPP

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

#include <stdlib.h>
#include <fix8/ff/buffer.hpp>
#include <fix8/ff/sysdep.h>
#include <assert.h>

namespace ff {

/*!
 * \ingroup shared_memory_fastflow
 *
 * @{
 */

/*!
 * \class dynlinkedlist
 * \ingroup shared_memory_fastflow
 *
 * \brief TODO
 *
 * This class is defined in \ref dynlinkedlist.hpp
 *
 */
class dynlinkedlist {

#define CAST_TO_UL(X) ((unsigned long)X)

private:
    /*!
     * \class Node
     * \ingroup shared_memory_fastflow
     *
     * \brief TODO
     *
     * This class is defined in \ref dynlinkedlist.hpp
     */
    struct Node {
        void        * data;
        struct Node * next;
        void        * next_data;
        long padding[longxCacheLine-((sizeof(void*)*3)/sizeof(long))];
    };

    volatile Node *    head;
    long padding1[longxCacheLxine-(sizeof(Node *)/sizeof(long))];
    volatile Node *    tail;
    long padding2[longxCacheLine-(sizeof(Node*)/sizeof(long))];
    //SWSR_Ptr_Buffer    cache;
    /*
      This is a vector of Node elemens.
      The len is equal to cachesize
     */
    Node * min_cache;
    int min_cache_size;
    void * cache_mem;
private:
    /**
     * TODO
     */
    bool isincahce(Node * n){
        if(((unsigned long) n ) - ((unsigned long)min_cache) < 0){
            return false;
        }

        if(((unsigned long) n ) - ((unsigned long)min_cache) > min_cache_size - sizeof(Node)){
            return false;
        }
    }
public:
    /**
     * TODO
     */
    enum {DEFAULT_CACHE_SIZE=1024};

    /**
     * TODO
     */
    dynlinkedlist(int cachesize=DEFAULT_CACHE_SIZE, bool fillcache=false){
        //Node * n = (Node *)::malloc(sizeof(Node));
        assert(sizeof(Node) == longxCacheLine);
        cache_mem = malloc((sizeof(Node)+1)*cachesize);
        if(CAST_TO_UL(cache_mem)%longxCacheLine){
            min_cache = (Node *)(
                                 ((CAST_TO_UL(cache_mem)
                                   /longxCacheLine)+1
                                  )*longxCacheLine
                                 );
        }
        min_cache_size = cachesize;


        for(int i=0; i<cachesize-1; i++){
            min_cache[i].next = &min_cache[i+1];
            min_cache[i].next_data = &min_cache[i+1].data;
            min_cache[i].data = NULL;
        }

        min_cache[cachesize-1].next = &min_cache[0];
        min_cache[cachesize-1].next_data = &min_cache[0].data;
        min_cache[cachesize-1].data = NULL;

        head = &min_cache[0];
        tail = &min_cache[0];
    }

    /**
     * TODO
     */
    ~dynlinkedlist() {
        Node * start_free = min_cache[0].next;
        min_cache[0].next=NULL;
        while(start_free){
            Node *tmp = start_free;
            start_free = start_free->next;
            tmp->next = NULL;
            tmp->next_data = NULL;
            if(!isincache(tmp)){
                free(tmp);
            }
        }
        free(cache_mem);
    }

    /**
     * TODO
     */
    inline bool push(void * const data) {
        assert(data!=NULL);
        if(likely(tail->next_data == NULL)){
            tail->data = data;
            tail = tail->next;
            WMB();
            return true;
        }

        Node * n = (Node *)::malloc(sizeof(Node*));
        n->data = data;
        n->next = tail->next;
        n->next_data = &tail->next.data;
        tail->next = n;
        WMB();
        return true;
    }

    /**
     * TODO
     */
    inline bool  pop(void ** data) {
        if (likely(head->data)) {
            *data = head->data;
            head->data = NULL;
            head = nead->next;
            return true;
        }
        return false;
    }
};

/*!
 * @}
 * \endlink
 */

} // namespace ff

#endif /* FF_DYNLINKEDLIST_HPP */
