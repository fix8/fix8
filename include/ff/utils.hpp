/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#ifndef _FF_UTILS_HPP_
#define _FF_UTILS_HPP_
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



#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>
//#if (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
#include <ff/platforms/platform.h>
//#else
//#include <pthread.h>
//#include <sys/time.h>
//#endif

#include <string.h>

#include <ff/cycle.h>
#include <ff/spin-lock.hpp>

namespace ff {

enum { START_TIME=0, STOP_TIME=1, GET_TIME=2 };


static inline ticks ticks_wait(ticks t1) {
    ticks delta;
    ticks t0 = getticks();
	//std::cerr << "should wait " << t1 << " at " << t0 << "\n";
    do { delta = (getticks()) - t0; } while (delta < t1);
    return delta-t1;
}

static inline void error(const char * str, ...) {
    const char err[]="ERROR: ";
    va_list argp;
    char * p=(char *)malloc(strlen(str)+strlen(err)+10);
    if (!p) abort();
    strcpy(p,err);
    strcpy(p+strlen(err), str);
    va_start(argp, str);
    vfprintf(stderr, p, argp);
    va_end(argp);
    free(p);
}

// return current time in usec 
static inline unsigned long getusec() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (unsigned long)(tv.tv_sec*1e6+tv.tv_usec);
}

// compute a-b and return the difference in msec
static inline const double diffmsec(const struct timeval & a, 
                                    const struct timeval & b) {
    long sec  = (a.tv_sec  - b.tv_sec);
    long usec = (a.tv_usec - b.tv_usec);
    
    if(usec < 0) {
        --sec;
        usec += 1000000;
    }
    return ((double)(sec*1000)+ ((double)usec)/1000.0);
}

static inline bool time_compare(struct timeval & a, struct timeval & b) {
    double t1= a.tv_sec*1000 + (double)(a.tv_usec)/1000.0;
    double t2= b.tv_sec*1000 + (double)(b.tv_usec)/1000.0;        
    return (t1<t2);
}

static inline const bool time_iszero(const struct timeval & a) {
    if ((a.tv_sec==0) && (a.tv_usec==0)) return true;
    return false;
}

static inline void time_setzero(struct timeval & a) {
    a.tv_sec=0;  
    a.tv_usec=0;
}

static inline bool isPowerOf2(unsigned long x) {
    return (x==1 || (x & (x-1)) == 0);
}

static inline unsigned long nextPowerOf2(unsigned long x) {
    assert(isPowerOf2(x)==false); // x is not a power of two!
    unsigned long p=1;
    while (x>p) p <<= 1;
    return p;
}

static inline double ffTime(int tag, bool lock=false) {
    static struct timeval tv_start = {0,0};
    static struct timeval tv_stop  = {0,0};
    // needed to protect gettimeofday
    // if multiple threads call ffTime
    static lock_t L = {0};

    double res=0.0;
    switch(tag) {
    case START_TIME:{
        spin_lock(L);
        gettimeofday(&tv_start,NULL);
        spin_unlock(L);
    } break;
    case STOP_TIME:{
        spin_lock(L);
        gettimeofday(&tv_stop,NULL);
        spin_unlock(L);
        res = diffmsec(tv_stop,tv_start);
    } break;
    case GET_TIME: {        
        res = diffmsec(tv_stop,tv_start);
    } break;
    default:
        res=0;
    }    
    return res;
}


} // namespace ff


#endif /* _FF_UTILS_HPP_ */
