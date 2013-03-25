#ifndef _SPIN_SYSDEP_H
#define _SPIN_SYSDEP_H


/*
 * The following has been taken from Cilk (version 5.4.6) file cilk-sysdep.h. 
 * The Cilk Project web site is  http://supertech.csail.mit.edu/cilk/
 *
 */

/*
 * Copyright (c) 2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or (at
 *  your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
 *  USA.
 *
 */

/* Don't just include config.h, since that is not installed. */
/* Instead, we must actually #define the useful things here. */
/* #include "../config.h" */
/* Compiler-specific dependencies here, followed by the runtime system dependencies.
 * The compiler-specific dependencies were originally written by Eitan Ben Amos.
 * Modified by Bradley.
 */

#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#endif


/***********************************************************\
 * Various types of memory barriers and atomic operations
\***********************************************************/

/*------------------------
       POWERPC 
 ------------------------*/
#if defined(__powerpc__) || defined(__ppc__)
/* This version contributed by Matteo Frigo Wed Jul 13 2005.   He wrote:
 *   lwsync is faster than eieio and has the desired store-barrier
 *   behavior.  The isync in the lock is necessary because the processor is
 *   allowed to speculate on loads following the branch, which makes the
 *   program without isync incorrect (in theory at least---I have never
 *   observed such a speculation).
 */

#define WMB()    __asm__ __volatile__ ("lwsync" : : : "memory")
#define PAUSE()

/* atomic swap operation */
static __inline__ int xchg(volatile int *ptr, int x)
{
    int result;
    __asm__ __volatile__ (
			  "0: lwarx %0,0,%1\n stwcx. %2,0,%1\n bne- 0b\n isync\n" :
			  "=&r"(result) : 
			  "r"(ptr), "r"(x) :
			  "cr0");
    
    return result;
}
#endif

/*------------------------
       IA64
 ------------------------*/
#ifdef __ia64__

#define WMB()    __asm__ __volatile__ ("mf" : : : "memory")
#define PAUSE()

/* atomic swap operation */
static inline int xchg(volatile int *ptr, int x)
{
    int result;
    __asm__ __volatile ("xchg4 %0=%1,%2" : "=r" (result)
			: "m" (*(int *) ptr), "r" (x) : "memory");
    return result;
}
#endif

/*------------------------
         I386 
 ------------------------*/
#ifdef __i386__ 

#define WMB()    __asm__ __volatile__ ("": : :"memory")
#define PAUSE()  __asm__ __volatile__ ("rep; nop" : : : "memory")

/* atomic swap operation 
   Note: no "lock" prefix even on SMP: xchg always implies lock anyway
*/
static inline int xchg(volatile int *ptr, int x)
{
    __asm__("xchgl %0,%1" :"=r" (x) :"m" (*(ptr)), "0" (x) :"memory");
    return x;
}
#endif /* __i386__ */

/*------------------------
         amd_64
 ------------------------*/
#ifdef __x86_64

#define WMB()    __asm__ __volatile__ ("": : :"memory")
#define PAUSE()  __asm__ __volatile__ ("rep; nop" : : : "memory")

/* atomic swap operation */
static inline int xchg(volatile int *ptr, int x)
{
    __asm__("xchgl %0,%1" :"=r" (x) :"m" (*(ptr)), "0" (x) :"memory");
    return x;
}
#endif /* __x86_64 */


static inline void *getAlignedMemory(size_t align, size_t size) {
    void *ptr;

    // malloc should guarantee a sufficiently well aligned memory for any purpose.
#if (defined(MAC_OS_X_VERSION_MIN_REQUIRED) && (MAC_OS_X_VERSION_MIN_REQUIRED < 1060))    
    ptr = ::malloc(size);
#elif (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
    ptr = ::malloc(size); // FIX ME
#else
    if (posix_memalign(&ptr,align,size)!=0)
        return NULL; 
#endif

    /* ptr = (void *)memalign(align, size);
       if (p == NULL) return NULL;
    */
    return ptr;
}

static inline void freeAlignedMemory(void* ptr) {
#if defined(_MSC_VER)
        if (ptr) ::posix_memalign_free(ptr);    
#else	
        if (ptr) ::free(ptr);
#endif  
}



#endif /* _SPIN_SYSDEP_H */
