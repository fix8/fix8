/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 * \link
 * \file utils.hpp
 * \ingroup shared_memory_fastflow
 *
 * \brief TODO
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
 *
 ****************************************************************************
 */

#ifndef FF_UTILS_HPP
#define FF_UTILS_HPP

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <unistd.h> // Not availbe on windows - to be managed
#include <iostream>
//#if (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
#include <fix8/ff/platforms/platform.h>
//#else
//#include <pthread.h>
//#include <sys/time.h>
//#endif

#include <string.h>

#include <fix8/ff/cycle.h>
#include <fix8/ff/spin-lock.hpp>

namespace ff {

/*!
 * \ingroup shared_memory_fastflow
 *
 * @{
 */

enum { START_TIME=0, STOP_TIME=1, GET_TIME=2 };

/* TODO: - nanosleep on Window
 *       - test on Apple
 */
#if defined(__linux__)
/*!!!----Mehdi-- required for DSRIMANAGER NODE----!!*/
static inline void waitCall(double milisec, double sec){
  if(milisec!=0.0 || sec!=0.0){
    struct timespec req = {0};
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    nanosleep(&req, (struct timespec *)NULL);
  }
};

static inline void waitSleep(ticks TICKS2WAIT){
    /*!!!----Mehdi--required to change busy wait with nanosleep ----!!*/
    //struct timespac req = {0};
    //req.tv_sec = static_cast<int>((static_cast<double>(TICKS2WAIT))/CLOCKS_PER_SEC);
    //req.tv_nsec =(((static_cast<double>(TICKS2WAIT))/CLOCKS_PER_SEC)-static_cast<int>((static_cast<double>(TICKS2WAIT))/CLOCKS_PER_SEC))*1.0e9;
    //req.tv_nsec =(((static_cast<double>(TICKS2WAIT))/CLOCKS_PER_SEC)-static_cast<int>((static_cast<double>(TICKS2WAIT))/CLOCKS_PER_SEC))*1.0e9;

    /* NOTE: The following implementation is not correct because we don't take into account
     *       the (current) CPU frequency. Anyway, this works well enough for internal FastFlow usage.
     */
    struct timespec req = {0, static_cast<long>(TICKS2WAIT)};
    nanosleep(&req, NULL);
};
#endif /* __linux__ */

/* NOTE:  nticks should be something less than 1000000 otherwise
 *        better to use something else.
 */
static inline ticks ticks_wait(ticks nticks) {
#if defined(__linux__) && defined(FF_ESAVER)
    waitSleep(nticks);
    return 0;
#else
    ticks delta;
    ticks t0 = getticks();
    do { delta = (getticks()) - t0; } while (delta < nticks);
    return delta-nticks;
#endif
}

/* NOTE: Does not make sense to use 'us' grather than or equal to 1000000 */
static inline void ff_relax(unsigned long us) {
#if defined(__linux__)
    struct timespec req = {0, static_cast<long>(us*1000L)};
    nanosleep(&req, NULL);
#else
    usleep(us);
#endif
    PAUSE();
}

/**
 * TODO
 */
static inline void error(const char * str, ...) {
    const char err[]="ERROR: ";
    va_list argp;
    char * p=(char *)malloc(strlen(str)+strlen(err)+10);
    if (!p) {
        printf("FATAL ERROR: no enough memory!\n");
        abort();
    }
    strcpy(p,err);
    strcpy(p+strlen(err), str);
    va_start(argp, str);
    vfprintf(stderr, p, argp);
    va_end(argp);
    free(p);
}

/**
 * It returns the current time in usec
 */
static inline unsigned long getusec() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (unsigned long)(tv.tv_sec*1e6+tv.tv_usec);
}

/**
 * Compute a-b and return the difference in msec
 */
static inline double diffmsec(const struct timeval & a,
                              const struct timeval & b) {
    long sec  = (a.tv_sec  - b.tv_sec);
    long usec = (a.tv_usec - b.tv_usec);

    if(usec < 0) {
        --sec;
        usec += 1000000;
    }
    return ((double)(sec*1000)+ ((double)usec)/1000.0);
}

/**
 * TODO
 */
static inline bool time_compare(struct timeval & a, struct timeval & b) {
    double t1= a.tv_sec*1000 + (double)(a.tv_usec)/1000.0;
    double t2= b.tv_sec*1000 + (double)(b.tv_usec)/1000.0;
    return (t1<t2);
}

/**
 * TODO
 */
static inline bool time_iszero(const struct timeval & a) {
    if ((a.tv_sec==0) && (a.tv_usec==0)) return true;
    return false;
}

/**
 * TODO
 */
static inline void time_setzero(struct timeval & a) {
    a.tv_sec=0;
    a.tv_usec=0;
}

/**
 * TODO
 */
static inline bool isPowerOf2(unsigned int x) {
    return (x==1 || (x & (x-1)) == 0);
}

/**
 * TODO
 */
static inline unsigned int nextPowerOf2(unsigned int x) {
    assert(isPowerOf2(x)==false); // x is not a power of two!
    unsigned int p=1;
    while (x>p) p <<= 1;
    return p;
}

static inline unsigned int nextMultipleOfIf(unsigned int x, unsigned int m) {
    unsigned r = x % m;
    return (r ? (x-r+m):x);
}


/**
 * TODO
 *
 * \return TODO
 */
static inline double ffTime(int tag, bool lock=false) {
    static struct timeval tv_start = {0,0};
    static struct timeval tv_stop  = {0,0};
    // needed to protect gettimeofday
    // if multiple threads call ffTime
#if (__cplusplus >= 201103L) || (defined __GXX_EXPERIMENTAL_CXX0X__) || (defined(HAS_CXX11_VARIADIC_TEMPLATES))
    static lock_t L;
#else
    static lock_t L = {0};
#endif

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

/*!
 * @}
 * \endlink
 */

} // namespace ff

#endif /* FF_UTILS_HPP */
