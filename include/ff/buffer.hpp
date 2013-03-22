/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 *  \file buffer.hpp
 *  \brief This file contains the definition of the bounded \p SWSR circular 
 *  buffer used in FastFlow
 */
 
#ifndef __SWSR_PTR_BUFFER_HPP_
#define __SWSR_PTR_BUFFER_HPP_
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

/* Single-Writer Single-Reader circular buffer.
 * No lock is needed around pop and push methods.
 * 
 * A single NULL value is used to indicate buffer full and 
 * buffer empty conditions.
 * 
 * More details about the SWSR_Ptr_Buffer implementation 
 * can be found in the following report:
 *
 * Massimo Torquati, "Single-Producer/Single-Consumer Queue on Shared Cache 
 * Multi-Core Systems", TR-10-20, Computer Science Department, University
 * of Pisa Italy,2010
 * ( http://compass2.di.unipi.it/TR/Files/TR-10-20.pdf.gz )
 *
 *
 */

#include <stdlib.h>
#include <string.h>

#include <ff/sysdep.h>
#include <ff/config.hpp>

#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#endif


namespace ff {

// 64 bytes is the common size of a cache line
static const int longxCacheLine = (CACHE_LINE_SIZE/sizeof(long));
    
/*!
 *  \ingroup runtime
 *
 *  @{
 */
 
 /*! 
  * \class SWSR_Ptr_Buffer
  *
  * \brief Single-Writer/Single-Reader circular buffer.
  *
  * This class describes the SWSR circular buffer, used in FastFlow to implement
  * a lock-free (wait-free) bounded FIFO queue. No lock is needed around pop and 
  * push methods.\n
  * A single NULL value is used to indicate buffer full and 
  * buffer empty conditions.
  */ 

class SWSR_Ptr_Buffer {
    // experimentally we found that a good value is between 
    // 2 and 6 cache lines (16 to 48 entries respectively)
    enum {MULTIPUSH_BUFFER_SIZE=16};

private:
    // Padding is required to avoid false-sharing between 
    // core's private cache
#if defined(NO_VOLATILE_POINTERS)
    unsigned long    pread;
    long padding1[longxCacheLine-1];
    unsigned long    pwrite;
    long padding2[longxCacheLine-1];
#else
    volatile unsigned long    pread;
    long padding1[longxCacheLine-1];
    volatile unsigned long    pwrite;
    long padding2[longxCacheLine-1];
#endif
    const    unsigned long size;
    void                   ** buf;
    
#if defined(SWSR_MULTIPUSH)
    /* massimot: experimental code (see multipush)
     *
     */
    long padding3[longxCacheLine-2];    
    // local multipush buffer used by the mpush method
    void  * multipush_buf[MULTIPUSH_BUFFER_SIZE];
    int     mcnt;
#endif

public:
    /** 
     *  Constructor.
     *
     *  \param n the size of the buffer
     */
    SWSR_Ptr_Buffer(unsigned long n, const bool=true):
        pread(0),pwrite(0),size(n),buf(0) {
    }
    
    /** Default destructor */
    ~SWSR_Ptr_Buffer() {
        // freeAlignedMemory is a function defined in 'sysdep.h'
        freeAlignedMemory(buf);
	}
    
    /** 
     *  Initialise the buffer. Allocate space (\p size) of possibly
     *  aligned memory and reset the pointers (read pointer and write pointer) 
     *  by placing them at the beginning of the buffer. 
     */
    bool init(const bool startatlineend=false) {
        if (buf || (size==0)) return false;

#if defined(SWSR_MULTIPUSH)
        if (size<MULTIPUSH_BUFFER_SIZE) return false;
#endif
        // getAlignedMemory is a function defined in 'sysdep.h'
        buf=(void**)getAlignedMemory(longxCacheLine*sizeof(long),size*sizeof(void*));
        if (!buf) return false;

        reset(startatlineend);
        return true;
    }

    /** Returns true if the buffer is empty */
    inline bool empty() {
#if defined(NO_VOLATILE_POINTERS)
        return ((*(volatile unsigned long *)(&buf[pread]))==0);
#else
        return (buf[pread]==NULL);
#endif
    }
    
    /** Returns true if there is at least one room in the buffer */
    inline bool available()   { 
#if defined(NO_VOLATILE_POINTERS)
        return ((*(volatile unsigned long *)(&buf[pwrite]))==0);
#else
        return (buf[pwrite]==NULL);
#endif
    }

    /** Returns the size of the buffer */
    inline unsigned long buffersize() const { return size; };
    
    /** 
     *  Push method: push the input value into the queue. \n
     *  A Write Memory Barrier (WMB) ensures that all previous memory writes 
     *  are visible to the other processors before any later
     *  write is executed.  This is an "expensive" memory fence
     *  operation needed in all the architectures with a weak-ordering 
     *  memory model, where stores can be executed out-of-order 
     *  (e.g. PowerPc). This is a no-op on Intel x86/x86-64 CPUs.
     *
     *  \param data Element to be pushed in the buffer
     */
    inline bool push(void * const data) {     /* modify only pwrite pointer */
        if (!data) return false;

        if (available()) {
            /* Write Memory Barrier: ensure all previous memory write 
             * are visible to the other processors before any later
             * writes are executed.  This is an "expensive" memory fence
             * operation needed in all the architectures with a weak-ordering 
             * memory model where stores can be executed out-or-order 
             * (e.g. Powerpc). This is a no-op on Intel x86/x86-64 CPUs.
             */
            WMB(); 
            buf[pwrite] = data;
            pwrite += (pwrite+1 >=  size) ? (1-size): 1; // circular buffer
            return true;
        }
        return false;
    }

    /*
     * the multipush method, which pushes a batch of elements (array) in the queue.
     * NOTE: len should be a multiple of  longxCacheLine/sizeof(void*)
     *
     */
    inline bool multipush(void * const data[], int len) {
        if ((unsigned)len>=size) return false;
        register unsigned long last = pwrite + ((pwrite+ --len >= size) ? (len-size): len);
        register unsigned long r    = len-(last+1), l=last;
		register unsigned long i;
        if (buf[last]==NULL) {
            
            if (last < pwrite) {
                for(i=len;i>r;--i,--l) 
                    buf[l] = data[i];
                for(i=(size-1);i>=pwrite;--i,--r)
                    buf[i] = data[r];
                
            } else 
                for(register int i=len;i>=0;--i) 
                    buf[pwrite+i] = data[i];
            
            WMB();
            pwrite = (last+1 >= size) ? 0 : (last+1);
#if defined(SWSR_MULTIPUSH)
            mcnt = 0; // reset mpush counter
#endif
            return true;
        }
        return false;
    }


#if defined(SWSR_MULTIPUSH)
    
    // massimot: experimental code
    /**
     * This method provides the same interface of the \p push method, but it
     * allows to provide a batch of items to
     * the consumer, thus ensuring better cache locality and 
     * lowering the cache trashing.
     *
     * \param data Element to be pushed in the buffer
     */
    inline bool mpush(void * const data) {
        if (!data) return false;
        
        if (mcnt==MULTIPUSH_BUFFER_SIZE)
            return multipush(multipush_buf,MULTIPUSH_BUFFER_SIZE);

        multipush_buf[mcnt++]=data;

        if (mcnt==MULTIPUSH_BUFFER_SIZE)
            return multipush(multipush_buf,MULTIPUSH_BUFFER_SIZE);

        return true;
    }

    /* REW -- ? */
    inline bool flush() {
        return (mcnt ? multipush(multipush_buf,mcnt) : true);
    }
#endif /* SWSR_MULTIPUSH */
    
    /** 
     *  Pop method: get the next value from the FIFO buffer.
     *
     *  \param data Pointer to the location where to store the 
     *  data popped from the buffer
     */
    inline bool  pop(void ** data) {  /* modify only pread pointer */
        if (!data || empty()) return false;

        *data = buf[pread];
        return inc();
    } 
    
    /* like pop but doesn't copy any data */
    inline bool  inc() {
        buf[pread]=NULL;
        pread += (pread+1 >= size) ? (1-size): 1; // circular buffer       
        return true;
    }           
    
    /** 
     *  Returns the "head" of the buffer, i.e. the element pointed by the 
     *  read pointer (it is a FIFO queue, so \p push on the tail and \p pop from 
     *  the head). 
     */
    inline void * const top() const { 
        return buf[pread];  
    }    

    /** 
     * Reset the buffer and move \p read and \p write pointers to
     * the beginning of the buffer (i.e. position 0).\n
     * Also, the entire buffer is cleaned and set to 0  
     */
    inline void reset(const bool startatlineend=false) { 
        if (startatlineend) {
            /*
             *  This is a good starting point if the multipush method 
             *  will be used in order to reduce cache trashing.
             */
            pwrite = longxCacheLine-1;
            pread  = longxCacheLine-1;
        } else {
            pread=pwrite=0; 
        }
#if defined(SWSR_MULTIPUSH)        
        mcnt   = 0;
#endif        
        memset(buf,0,size*sizeof(void*));
    }

    /** Returns the length of the buffer
     * (i.e. the actual number of elements it contains) 
     */
    inline unsigned long length() const {
        long len = pwrite-pread;
        if (len>=0) return len;
        return size+len;
    }

};

/*!
 *  @}
 */
 

/* Implementation of the well-known 
 * Lamport's wait-free circular buffer.
 *
 */
class Lamport_Buffer {
private:
    // Padding is required to avoid false-sharing between 
    // core's private cache
    volatile unsigned long    pread;
    long padding1[longxCacheLine-1];
    volatile unsigned long    pwrite;
    long padding2[longxCacheLine-1];

    const    unsigned long size;
    void                   ** buf;
    
public:
    Lamport_Buffer(unsigned long n, const bool=true):
        pread(0),pwrite(0),size(n),buf(0) {
    }
    
    ~Lamport_Buffer() {
        freeAlignedMemory(buf);
	}
    
    /* Initialize the circular buffer. */
    bool init() {
        if (buf) return false;
        buf=(void**)getAlignedMemory(longxCacheLine*sizeof(long),size*sizeof(void*));
        if (!buf) return false;
        reset();
        return true;
    }

    /* return true if the buffer is empty */
    inline bool empty() { return (pwrite == pread);  }
    
    /* return true if there is at least one room in the buffer */
    inline bool available()   { 
        const unsigned long next = pwrite + ((pwrite+1>=size)?(1-size):1);
        return (next != pread);
    }
   
    inline unsigned long buffersize() const { return size; };
    
    inline bool push(void * const data) {
        if (!data) return false;

        const unsigned long next = pwrite + ((pwrite+1>=size)?(1-size):1);
        if (next != pread) {
            buf[pwrite] = data;
            /* We have to ensure that all writes have been committed 
             * in memory before we change the value of the pwrite
             * reference otherwise the reader can read stale data.
             */
            WMB(); 
            pwrite =next;
            return true;
        }
        return false;
    }

    inline bool  pop(void ** data) {
        if (!data) return false;       

        if (empty()) return false;
        *data = buf[pread];
        pread += (pread+1 >= size) ? (1-size): 1;
        return true;
    }    
    
    inline void reset() { 
        pread=pwrite=0; 
        memset(buf,0,size*sizeof(void*));
    }

    inline unsigned long length() const {
        long len = pwrite-pread;
        if (len>=0) return len;
        return size+len;
    }

};

} // namespace

#endif /* __SWSR_PTR_BUFFER_HPP_ */
