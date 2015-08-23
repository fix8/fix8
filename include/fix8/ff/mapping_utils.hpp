/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*!
 *  \link
 *  \file mapping_utils.hpp
 *  \ingroup streaming_network_arbitrary_shared_memory
 *
 *  \brief This file contains utilities for thread pinning to cores and thread
 *  mapping.
 *
 *  This file provides support for thread pinning and mapping at linux and
 *  MacOS.
 *
 */

/* ***************************************************************************
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

/*
 *
 * Author: Massimo Aldinucci, Massimo Torquati
 * Date: Oct 5, 2010: basic linux and apple
 * Date: Mar 27, 2011: some win platform support
 *
 */

#ifndef FF_MAPPING_UTILS_HPP
#define FF_MAPPING_UTILS_HPP


#include <iostream>
#include <errno.h>
#include <fix8/ff/config.hpp>
#if defined(__linux__)
#include <sys/types.h>
#include <sys/resource.h>
#include <asm/unistd.h>
#include <stdio.h>
#include <unistd.h>
#define gettid() syscall(__NR_gettid)
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/syscall.h>
#include <mach/mach.h>
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
// If you don't include mach/mach.h, it doesn't work.
// In theory mach/thread_policy.h should be enough
#elif (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
#include <fix8/ff/platforms/platform.h>
//extern "C" {
//#include <Powrprof.h>
//}
#endif

/**
 *  \brief Returns the ID of the calling thread
 *
 *  It returns the ID of the calling thread. It works on Linux OS, Apple OS,
 *  Windows.
 *
 *  \return if successful the identifier of the thread is returned. Otherwise a
 *  negative vlue is returned.
 *
 */
static inline long ff_getThreadID() {
#if (defined(__GNUC__) && defined(__linux))
    return  gettid();
#elif defined(__APPLE__) && MAC_OS_X_HAS_AFFINITY
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    return (long) tid; // > 10.6 only
#elif  (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
    return GetCurrentThreadId();
#endif
    return -1;
}

/**
 *  \brief Returnes the frequency of the CPU
 *
 *  It returns the frequency of the CPUs (On a shared memory system, all cores
 *  have the same frequency). It works on Linux OS and Apple OS.
 *
 *  \return An integer value showing the frequency of the core.
 */
static inline unsigned long ff_getCpuFreq() {
    unsigned long  t = 0;
#if defined(__linux__)
    FILE       *f;
    float       mhz;

    f = popen("cat /proc/cpuinfo |grep MHz |head -1|sed 's/^.*: //'", "r");
    if (fscanf(f, "%f", &mhz) == EOF) {pclose(f); return t;}
    t = (unsigned long)(mhz * 1000000);
    pclose(f);
#elif defined(__APPLE__) && MAC_OS_X_HAS_AFFINITY
    size_t len = 8;
    if (sysctlbyname("hw.cpufrequency", &t, &len, NULL, 0) != 0) {
        perror("sysctl");
    }
#elif (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
//SYSTEM_POWER_CAPABILITIES pow;
//GetPwrCapabilities(&pow);
#else
//#warning "Cannot detect CPU frequency"
#endif
    return (t);
}

/**
 *  \brief Returns the number of cores in the system
 *
 *  It returns the number of cores present in the system. (Note that it does
 *  take into account hyperthreading). It works on Linux OS, Apple OS and
 *  Windows.
 *
 *  \return An integer value showing the number of cores.
 */
static inline int ff_numCores() {
    int  n=-1;
#if defined(__linux__)
    FILE       *f;
    f = popen("cat /proc/cpuinfo |grep processor | wc -l", "r");
    if (fscanf(f, "%d", &n) == EOF) { pclose(f); return n;}
    pclose(f);
#elif defined(__APPLE__) // BSD
    size_t len = 4;
    if (sysctlbyname("hw.ncpu", &n, &len, NULL, 0) == -1)
        perror("sysctl");
#elif  (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
    SYSTEM_INFO sysinfo;
    GetSystemInfo( &sysinfo );
    n = sysinfo.dwNumberOfProcessors;
#else
//#warning "Cannot detect num. of cores."
#endif
    return n;
}


/**
 *  \brief Returns the real number of cores in the system without considering
 *  HT or HMT
 *
 *  It returns the number of cores present in the system. It works on Linux OS
 *
 *  \return An integer value showing the number of cores.
 */
static inline int ff_realNumCores() {
    int  n=-1;
#if defined(__linux__)
    FILE       *f;
    f = popen("cat /proc/cpuinfo|egrep 'core id|physical id'|tr -d '\n'|sed 's/physical/\\nphysical/g'|grep -v ^$|sort|uniq|wc -l", "r");
    if (fscanf(f, "%d", &n) == EOF) { pclose(f); return n;}
    pclose(f);
#else
#warning "ff_realNumCores not supported on this platform"
#endif
    return n;
}

/**
 *  \brief Returns the number of CPUs (physical sockets) on the system.
 *
 *  It returns the number physical sockets on the system. It works on Linux OS.
 *
 *  \return An integer value showing the number of sockets.
 */
static inline int ff_numSockets() {
    int  n=-1;
#if defined(__linux__)
    FILE       *f;
    f = popen("cat /proc/cpuinfo|grep 'physical id'|sort|uniq|wc -l", "r");
    if (fscanf(f, "%d", &n) == EOF) { pclose(f); return n;}
    pclose(f);
#else
#warning "ff_numSockets not supported on this platform"
#endif
    return n;
}


/**
 * \brief Sets the scheduling priority
 *
 * It sets the scheduling priority of the process (or thread). The
 * priority_level is a value in the range -20 to 19. The default priority is 0,
 * lower priorities cause more favorable scheduling.
 *
 * MA: This should be redesigned since it might have different behaviours in
 * different systems.
 *
 * \parm priority_level defines the level of the priority. Default is set to
 * 0.
 *
 * \return An integer value showing the priority of the scheduling policy.
 *
 */
static inline int ff_setPriority(int priority_level=0) {
    int ret=0;
#if (defined(__GNUC__) && defined(__linux))
    //if (priority_level) {
        if (setpriority(PRIO_PROCESS, gettid(),priority_level) != 0) {
            perror("setpriority:");
            ret = EINVAL;
        }
    //}
#elif defined(__APPLE__)
    if (setpriority(PRIO_DARWIN_THREAD, 0 /*myself */ ,priority_level) != 0) {
            perror("setpriority:");
            ret = EINVAL;
    }
#elif  (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
    int pri = (priority_level + 20)/10;
    int subpri = ((priority_level + 20)%10)/2;
    switch (pri) {
        case 0: ret = !(SetPriorityClass(GetCurrentThread(),HIGH_PRIORITY_CLASS));
            break;
        case 1: ret = !(SetPriorityClass(GetCurrentThread(),ABOVE_NORMAL_PRIORITY_CLASS));
            break;
        case 2: ret = !(SetPriorityClass(GetCurrentThread(),NORMAL_PRIORITY_CLASS));
            break;
        case 3: ret = !(SetPriorityClass(GetCurrentThread(),BELOW_NORMAL_PRIORITY_CLASS));
            break;
        default: ret = EINVAL;
    }

    switch (pri) {
        case 0: ret |= !(SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST));
            break;
        case 1: ret |= !(SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL));
            break;
        case 2: ret |= !(SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL));
            break;
        case 3: ret |= !(SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL));
            break;
        case 4: ret |= !(SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_LOWEST));
            break;
        default: ret = EINVAL;
}
#else
// do nothing
#endif
if (ret!=0) perror("ff_setPriority");
    return ret;
}

/**
 *  \brief Returns the ID of the core
 *
 *  It returns the ID of the core where the calling thread is running. It works
 *  on Linux OS and Apple OS.
 *
 *  \return An integer value showing the ID of the core. If the ID of the core
 *  is not found, then -1 is returned.
 */
static inline int ff_getMyCore() {
#if defined(__linux__) && defined(CPU_SET)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    if (sched_getaffinity(gettid(), sizeof(mask), &mask) != 0) {
        perror("sched_getaffinity");
        return EINVAL;
    }
    for(int i=0;i<CPU_SETSIZE;++i)
        if (CPU_ISSET(i,&mask)) return i;
#elif defined(__APPLE__) && MAC_OS_X_HAS_AFFINITY
    // Not tested
    struct thread_affinity_policy mypolicy;
    boolean_t get_default;
    mach_msg_type_number_t thread_info_count = THREAD_AFFINITY_POLICY_COUNT;
    thread_policy_get(mach_thread_self(), THREAD_AFFINITY_POLICY,
                      (integer_t*) &mypolicy,
                      &thread_info_count, &get_default);
    int res = mypolicy.affinity_tag;
    return(res);
#else
#if __GNUC__
#warning "ff_getMyCpu not supported"
#else
std::cerr << "---> ff_getMyCpu not supported\n";
#pragma message( "ff_getMyCpu not supported")
#endif
#endif
return -1;
}
// NOTE: this function will be discarded, please use ff_getMyCore() instead
static inline int ff_getMyCpu() { return ff_getMyCore(); }

/**
 *  \brief Maps the calling thread to the given CPU.
 *
 *  It maps the calling thread to the given core. It works on Linux OS, Apple
 *  OS, Windows.
 *
 *  \param cpu_id the ID of the CPU to which the thread will be attached.
 *  \param priority_level TODO
 *
 *  \return An integet value showing the priority level is returned if
 *  successful. Otherwise \p EINVAL is returned.
 */
static inline int ff_mapThreadToCpu(int cpu_id, int priority_level=0) {
    if (cpu_id > ff_numCores()) return EINVAL;
#if defined(__linux__) && defined(CPU_SET)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    if (sched_setaffinity(gettid(), sizeof(mask), &mask) != 0)
        return EINVAL;
    return (ff_setPriority(priority_level));
#elif defined(__APPLE__) && MAC_OS_X_HAS_AFFINITY
    // Mac OS does not implement direct pinning of threads onto cores.
    // Threads can be organised in affinity set. Using requested CPU
    // tag for the set. Cores under the same L2 cache are not distinguished.
    // Should be called before running the thread.
#define CACHE_LEVELS 3
    #define CACHE_L2 2
    size_t len;

    if (sysctlbyname("hw.cacheconfig",NULL, &len, NULL, 0) != 0) {
        perror("sysctl");
    } else {
      int64_t cacheconfig[len];
      if (sysctlbyname("hw.cacheconfig", &cacheconfig[0], &len, NULL, 0) != 0)
        perror("sysctl: unable to get hw.cacheconfig");
      else {
      /*
          for (size_t i=0;i<CACHE_LEVELS;i++)
          std::cerr << " Cache " << i << " shared by " <<  cacheconfig[i] << " cores\n";
      */
      struct thread_affinity_policy mypolicy;
      // Define sets taking in account pinning is performed on L2
      mypolicy.affinity_tag = cpu_id/cacheconfig[CACHE_L2];
      if ( thread_policy_set(mach_thread_self(), THREAD_AFFINITY_POLICY, (integer_t*) &mypolicy, THREAD_AFFINITY_POLICY_COUNT) != KERN_SUCCESS ) {
      std::cerr << "Setting affinity of thread ? (" << mach_thread_self() << ") failed!" << std::endl;
      return EINVAL;
      } // else {
      //   std::cerr << "Sucessfully set affinity of thread (" <<
      //   mach_thread_self() << ") to core " << cpu_id/cacheconfig[CACHE_L2] << "\n";
      // }
      }
   }

    return(ff_setPriority(priority_level));
#elif (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
    if (-1==SetThreadIdealProcessor(GetCurrentThread(),cpu_id)) {
        perror("ff_mapThreadToCpu:SetThreadIdealProcessor");
        return EINVAL;
    }
    //std::cerr << "Successfully set affinity of thread " << GetCurrentThreadId() << " to core " << cpu_id << "\n";
#else
#warning "CPU_SET not defined, cannot map thread to specific CPU"
#endif
    return 0;
}

// Author: Nick Strupat
// Date: October 29, 2010
// Returns the cache line size (in bytes) of the processor, or 0 on failure

#include <stddef.h>

#if defined(__APPLE__)
//#include <sys/sysctl.h>
/**
 *  \brief Gets cache line size in Apple systems
 *
 *  It returns the size of the cache line only in Apple Systems
 *
 *  \return A \size_t value, showing the size of the cache line.
 */
static inline size_t cache_line_size() {

    u_int64_t line_size = 0;
    size_t sizeof_line_size = sizeof(line_size);
    if (sysctlbyname("hw.cachelinesize", &line_size, &sizeof_line_size, NULL, 0) !=0) {
        perror("cachelinesize:");
        line_size=0;
    }
    return line_size;
}

#elif defined(_WIN32)
//#include <stdlib.h>
//#include <windows.h>

/**
 *  \brief Gets cache line size in Windows systems
 *
 *  It returns the size of the cache line only in Windows Systems
 *
 *  \return A \size_t value, showing the size of the cache line.
 */
static inline size_t cache_line_size() {
    size_t line_size = 0;
    DWORD buffer_size = 0;
    DWORD i = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

    GetLogicalProcessorInformation(0, &buffer_size);
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
    GetLogicalProcessorInformation(&buffer[0], &buffer_size);

    for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
            line_size = buffer[i].Cache.LineSize;
            break;
        }
    }

    free(buffer);
    return line_size;
}

#elif defined(__linux__)
//#include <stdio.h>

/**
 *  \brief Gets cache line size in Linux systems
 *
 *  It returns the size of the cache line only in Linux Systems
 *
 *  \return A \size_t value, showing the size of the cache line.
 */

static inline size_t cache_line_size() {
    FILE * p = 0;
    p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
    unsigned int i = 0;
    if (p) {
        if (fscanf(p, "%ud", &i) == EOF) {
            perror("fscanf");
            if (fclose(p) != 0) perror("fclose");
            return 0;
        }
        if (fclose(p) != 0) {
            perror("fclose");
            return 0;
        }
    }
    return i;
}

#else
#error Unrecognized platform
#endif

/*!
 * @}
 * \endlink
 */

#endif /* FF_MAPPING_UTILS_HPP */
