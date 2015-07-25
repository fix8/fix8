#ifndef _INCLUDE_FIX__F_CONFIG_H
#define _INCLUDE_FIX__F_CONFIG_H 1
 
/* include/fix8/f8config.h. Generated automatically at end of configure. */
/* intermediate_config.h.  Generated from intermediate_config.h.in by configure.  */
/* intermediate_config.h.in.  Generated from configure.ac by autoheader.  */

//-----------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-15 David L. Dight <fix@fix8.org>

Fix8 is free software: you can redistribute it and/or modify  it under the terms of the GNU
Lesser General Public License as  published by the Free Software Foundation, either version
3 of the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-----------------------------------------------------------------------------------------

#ifdef _MSC_VER
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <Windows.h>
#include <WinSock2.h>
#include <io.h>
#include <direct.h>
#include <process.h>
#pragma warning (disable : 4800 ) // forcing value to bool
#pragma warning (disable : 4244 ) // conversion from, possible loss of data
#define HAVE_GMTOFF
#undef Yield
#endif

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define to 1 if you wish to enable buffered global logging */
/* #undef BUFFERED_GLOBAL_LOGGING */

/* Define to 1 if the `closedir' function returns void instead of `int'. */
/* #undef CLOSEDIR_VOID */

/* Define to 1 to enable CODEC timing testing code */
/* #undef CODECTIMING */

/* configure options */
#ifndef FIX8_CONFIGURE_OPTIONS
#define FIX8_CONFIGURE_OPTIONS " '--enable-zeromq=no' '--with-mpmc=tbb' '--with-gtest=/home/sergey/src/f8/gtest-1.7.0' '--enable-doxygen=no' '--enable-f8test' '--enable-extended-metadata' '--enable-codectiming=no' '--with-precision=double' '--enable-doxygen-warnings=yes' '--enable-preencode=yes' '--prefix=/home/sergey/src/f8/fix8/.install' '--with-tbb=' '--with-poco=/usr/local' '--enable-ssl' '--enable-tbbmalloc' 'CXXFLAGS=-O3  -std=c++11 -ggdb'"
#endif

/* Short Date system was configured */
#ifndef FIX8_CONFIGURE_SDATE
#define FIX8_CONFIGURE_SDATE "2015/07/25"
#endif

/* Date system was configured */
#ifndef FIX8_CONFIGURE_TIME
#define FIX8_CONFIGURE_TIME "Sat Jul 25 22:06:35 MSK 2015"
#endif

/* date/time as seconds since start epoch */
#ifndef FIX8_CONFIGURE_TIME_NUM
#define FIX8_CONFIGURE_TIME_NUM 1437851195
#endif

/* compiler spec */
#ifndef FIX8_CPPFLAGS
#define FIX8_CPPFLAGS " -I/include -I/home/sergey/src/f8/gtest-1.7.0/include -I/usr/local/include"
#endif

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Define to 1 for debugging support */
/* #undef DEBUG */

/* Default number of precision digits for floating point fields (default=2) */
#ifndef FIX8_DEFAULT_PRECISION
#define FIX8_DEFAULT_PRECISION 2
#endif

/* Define to 1 to enable experimental socket read */
/* #undef EXPERIMENTAL_BUFFERED_SOCKET_READ */

/* Define to 1 if gtest available */
#ifndef FIX8_HAS_GTEST
#define FIX8_HAS_GTEST 1
#endif

/* Define to 1 if Poco available */
#ifndef FIX8_HAS_POCO
#define FIX8_HAS_POCO 1
#endif

/* Define to 1 if you have zeromq */
/* #undef HAS_ZEROMQ_MBUS */

/* Define to 1 if you have the `alarm' function. */
#ifndef FIX8_HAVE_ALARM
#define FIX8_HAVE_ALARM 1
#endif

/* Define to 1 if you have `alloca', as a function or macro. */
#ifndef FIX8_HAVE_ALLOCA
#define FIX8_HAVE_ALLOCA 1
#endif

/* Define to 1 if you have the <alloca.h> header file. */
#ifndef FIX8_HAVE_ALLOCA_H
#define FIX8_HAVE_ALLOCA_H 1
#endif

/* Define to 1 if you have the <arpa/inet.h> header file. */
/* #undef HAVE_ARPA_INET_H */

/* Define to 1 if you have berkeley DB */
/* #undef HAVE_BDB */

/* Define if you have clock_gettime() */
#ifndef FIX8_HAVE_CLOCK_GETTIME
#define FIX8_HAVE_CLOCK_GETTIME 1
#endif

/* Define if you have clock_nanosleep() */
/* #undef FIX8_HAVE_CLOCK_NANOSLEEP */

/* Define to 1 if zlib headers and library were found */
/* #undef FIX8_HAVE_COMPRESSION */

/* Define to 1 if crypt is present in -lcrypt */
#ifndef FIX8_HAVE_CRYPT
#define FIX8_HAVE_CRYPT 1
#endif

/* Define to 1 if you have the <crypt.h> header file. */
#ifndef FIX8_HAVE_CRYPT_H
#define FIX8_HAVE_CRYPT_H 1
#endif

/* define if the compiler supports basic C++11 syntax */
/* #undef HAVE_CXX11 */

/* Define to 1 if you have the <db.h> header file. */
/* #undef HAVE_DB_H */

/* Define to 1 if you have the declaration of `TCP_CORK', and to 0 if you
   don't. */
#ifndef FIX8_HAVE_DECL_TCP_CORK
#define FIX8_HAVE_DECL_TCP_CORK 1
#endif

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#ifndef FIX8_HAVE_DIRENT_H
#define FIX8_HAVE_DIRENT_H 1
#endif

/* Define to 1 if you have the <dlfcn.h> header file. */
#ifndef FIX8_HAVE_DLFCN_H
#define FIX8_HAVE_DLFCN_H 1
#endif

/* Define to 1 if you have extended Fix8 metadata enabled */
#ifndef FIX8_HAVE_EXTENDED_METADATA
#define FIX8_HAVE_EXTENDED_METADATA 1
#endif

/* Define to 1 if you have the <fcntl.h> header file. */
/* #undef HAVE_FCNTL_H */

/* Define to 1 if you have the `fork' function. */
#ifndef FIX8_HAVE_FORK
#define FIX8_HAVE_FORK 1
#endif

/* Define to 1 if you have the `getcwd' function. */
#ifndef FIX8_HAVE_GETCWD
#define FIX8_HAVE_GETCWD 1
#endif

/* Define to 1 if you have the <getopt.h> header file. */
#ifndef FIX8_HAVE_GETOPT_H
#define FIX8_HAVE_GETOPT_H 1
#endif

/* Define to 1 if you have the `getopt_long' function. */
#ifndef FIX8_HAVE_GETOPT_LONG
#define FIX8_HAVE_GETOPT_LONG 1
#endif

/* Define to 1 if you have the `getpagesize' function. */
#ifndef FIX8_HAVE_GETPAGESIZE
#define FIX8_HAVE_GETPAGESIZE 1
#endif

/* Define to 1 if you have the `gettimeofday' function. */
#ifndef FIX8_HAVE_GETTIMEOFDAY
#define FIX8_HAVE_GETTIMEOFDAY 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef FIX8_HAVE_INTTYPES_H
#define FIX8_HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have libhiredis */
/* #undef HAVE_LIBHIREDIS */

/* Define to 1 if you have libmemcached */
/* #undef HAVE_LIBMEMCACHED */

/* Define to 1 if libz is present */
#ifndef FIX8_HAVE_LIBZ
#define FIX8_HAVE_LIBZ 1
#endif

/* Define to 1 if you have the <limits.h> header file. */
/* #undef HAVE_LIMITS_H */

/* Define to 1 if you have the `localtime_r' function. */
#ifndef FIX8_HAVE_LOCALTIME_R
#define FIX8_HAVE_LOCALTIME_R 1
#endif

/* Define to 1 if the type `long double' works and has more range or precision
   than `double'. */
#ifndef FIX8_HAVE_LONG_DOUBLE
#define FIX8_HAVE_LONG_DOUBLE 1
#endif

/* Define to 1 if the type `long double' works and has more range or precision
   than `double'. */
#ifndef FIX8_HAVE_LONG_DOUBLE_WIDER
#define FIX8_HAVE_LONG_DOUBLE_WIDER 1
#endif

/* Define to 1 if the system has the type 'long long int'. */
#ifndef FIX8_HAVE_LONG_LONG_INT
#define FIX8_HAVE_LONG_LONG_INT 1
#endif

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#ifndef FIX8_HAVE_MALLOC
#define FIX8_HAVE_MALLOC 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#ifndef FIX8_HAVE_MEMORY_H
#define FIX8_HAVE_MEMORY_H 1
#endif

/* Define to 1 if you have a working `mmap' system call. */
#ifndef FIX8_HAVE_MMAP
#define FIX8_HAVE_MMAP 1
#endif

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netdb.h> header file. */
/* #undef HAVE_NETDB_H */

/* Define to 1 if you have the <netinet/in.h> header file. */
/* #undef HAVE_NETINET_IN_H */

/* Define to 1 if you have openssl */
#ifndef FIX8_HAVE_OPENSSL
#define FIX8_HAVE_OPENSSL 1
#endif

/* Define to 1 if you have the <openssl/ssl.h> header file. */
#ifndef FIX8_HAVE_OPENSSL_SSL_H
#define FIX8_HAVE_OPENSSL_SSL_H 1
#endif

/* Define to 1 if you have the `popen' function. */
#ifndef FIX8_HAVE_POPEN
#define FIX8_HAVE_POPEN 1
#endif

/* Have PTHREAD_PRIO_INHERIT. */
/* #undef HAVE_PTHREAD_PRIO_INHERIT */

/* Define to 1 if the system has the type `ptrdiff_t'. */
#ifndef FIX8_HAVE_PTRDIFF_T
#define FIX8_HAVE_PTRDIFF_T 1
#endif

/* Define to 1 if you have the `random' function. */
#ifndef FIX8_HAVE_RANDOM
#define FIX8_HAVE_RANDOM 1
#endif

/* Define to 1 if your system has a GNU libc compatible `realloc' function,
   and to 0 otherwise. */
#ifndef FIX8_HAVE_REALLOC
#define FIX8_HAVE_REALLOC 1
#endif

/* Define to 1 if you have the `regcomp' function. */
#ifndef FIX8_HAVE_REGCOMP
#define FIX8_HAVE_REGCOMP 1
#endif

/* Define to 1 if you have the <regex.h> header file. */
/* #undef HAVE_REGEX_H */

/* Define to 1 if you have the <select.h> header file. */
/* #undef HAVE_SELECT_H */

/* Define to 1 if you have the <signal.h> header file. */
/* #undef HAVE_SIGNAL_H */

/* Define to 1 if you have the `socket' function. */
#ifndef FIX8_HAVE_SOCKET
#define FIX8_HAVE_SOCKET 1
#endif

/* Define to 1 if you have the `srandom' function. */
#ifndef FIX8_HAVE_SRANDOM
#define FIX8_HAVE_SRANDOM 1
#endif

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
/* #undef HAVE_STAT_EMPTY_STRING_BUG */

/* Define to 1 if stdbool.h conforms to C99. */
#ifndef FIX8_HAVE_STDBOOL_H
#define FIX8_HAVE_STDBOOL_H 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef FIX8_HAVE_STDINT_H
#define FIX8_HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#ifndef FIX8_HAVE_STDLIB_H
#define FIX8_HAVE_STDLIB_H 1
#endif

/* Define to 1 if you have the `strcasecmp' function. */
#ifndef FIX8_HAVE_STRCASECMP
#define FIX8_HAVE_STRCASECMP 1
#endif

/* Define to 1 if you have the `strcoll' function and it is properly defined.
   */
#ifndef FIX8_HAVE_STRCOLL
#define FIX8_HAVE_STRCOLL 1
#endif

/* Define to 1 if you have the `strerror' function. */
#ifndef FIX8_HAVE_STRERROR
#define FIX8_HAVE_STRERROR 1
#endif

/* Define to 1 if you have the `strftime' function. */
#ifndef FIX8_HAVE_STRFTIME
#define FIX8_HAVE_STRFTIME 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#ifndef FIX8_HAVE_STRINGS_H
#define FIX8_HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <string.h> header file. */
#ifndef FIX8_HAVE_STRING_H
#define FIX8_HAVE_STRING_H 1
#endif

/* Define to 1 if you have the `strtol' function. */
#ifndef FIX8_HAVE_STRTOL
#define FIX8_HAVE_STRTOL 1
#endif

/* Define to 1 if you have the `strtoul' function. */
#ifndef FIX8_HAVE_STRTOUL
#define FIX8_HAVE_STRTOUL 1
#endif

/* Define to 1 if you have the `sysconf' function. */
#ifndef FIX8_HAVE_SYSCONF
#define FIX8_HAVE_SYSCONF 1
#endif

/* Define to 1 if you have the <syslog.h> header file. */
/* #undef HAVE_SYSLOG_H */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/gmon.h> header file. */
/* #undef HAVE_SYS_GMON_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
/* #undef HAVE_SYS_IOCTL_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#ifndef FIX8_HAVE_SYS_PARAM_H
#define FIX8_HAVE_SYS_PARAM_H 1
#endif

/* Define to 1 if you have the <sys/socket.h> header file. */
/* #undef HAVE_SYS_SOCKET_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#ifndef FIX8_HAVE_SYS_STAT_H
#define FIX8_HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/time.h> header file. */
#ifndef FIX8_HAVE_SYS_TIME_H
#define FIX8_HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#ifndef FIX8_HAVE_SYS_TYPES_H
#define FIX8_HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if you have the <sys/wait.h> header file. */
#ifndef FIX8_HAVE_SYS_WAIT_H
#define FIX8_HAVE_SYS_WAIT_H 1
#endif

/* Define to 1 if you have the <termios.h> header file. */
/* #undef HAVE_TERMIOS_H */

/* Define to 1 if you have the <time.h> header file. */
/* #undef HAVE_TIME_H */

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef FIX8_HAVE_UNISTD_H
#define FIX8_HAVE_UNISTD_H 1
#endif

/* Define to 1 if the system has the type 'unsigned long long int'. */
#ifndef FIX8_HAVE_UNSIGNED_LONG_LONG_INT
#define FIX8_HAVE_UNSIGNED_LONG_LONG_INT 1
#endif

/* Define to 1 for valgrind support */
/* #undef HAVE_VALGRIND */

/* Define to 1 if you have the <valgrind/valgrind.h> header file. */
/* #undef HAVE_VALGRIND_VALGRIND_H */

/* Define to 1 if you have /var/run */
#ifndef FIX8_HAVE_VAR_RUN
#define FIX8_HAVE_VAR_RUN 1
#endif

/* Define to 1 if you have the `vfork' function. */
#ifndef FIX8_HAVE_VFORK
#define FIX8_HAVE_VFORK 1
#endif

/* Define to 1 if you have the <vfork.h> header file. */
/* #undef HAVE_VFORK_H */

/* Define to 1 if `fork' works. */
#ifndef FIX8_HAVE_WORKING_FORK
#define FIX8_HAVE_WORKING_FORK 1
#endif

/* Define to 1 if `vfork' works. */
#ifndef FIX8_HAVE_WORKING_VFORK
#define FIX8_HAVE_WORKING_VFORK 1
#endif

/* Define to 1 if you have the <zlib.h> header file. */
/* #undef FIX8_HAVE_ZLIB_H */

/* Define to 1 if the system has the type `_Bool'. */
#ifndef FIX8_HAVE__BOOL
#define FIX8_HAVE__BOOL 1
#endif

/* Default system */
#ifndef FIX8_HOST_SYSTEM
#define FIX8_HOST_SYSTEM "x86_64-unknown-linux-gnu"
#endif

/* Additional library flags */
#ifndef FIX8_LDFLAGS
#define FIX8_LDFLAGS " -L/lib -L/home/sergey/src/f8/gtest-1.7.0/lib -L/home/sergey/src/f8/gtest-1.7.0/lib/.libs -L/usr/localOD/lib"
#endif

/* Library spec */
#ifndef FIX8_LIBS
#define FIX8_LIBS " -ltbbmalloc -ltbbmalloc_proxy -lssl -lcrypto"
#endif

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
#ifndef FIX8_LSTAT_FOLLOWS_SLASHED_SYMLINK
#define FIX8_LSTAT_FOLLOWS_SLASHED_SYMLINK 1
#endif

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#ifndef FIX8_LT_OBJDIR
#define FIX8_LT_OBJDIR ".libs/"
#endif

/* Encode version */
#ifndef FIX8_MAGIC_NUM
#define FIX8_MAGIC_NUM 16789508
#endif

/* Encoded Version as expresion */
#ifndef FIX8_MAGIC_NUM_EXPR
#define FIX8_MAGIC_NUM_EXPR (FIX8_MAJOR_VERSION_NUM << 24 | FIX8_MINOR_VERSION_NUM << 12 | FIX8_PATCH_VERSION_NUM)
#endif

/* Major version number */
#ifndef FIX8_MAJOR_VERSION_NUM
#define FIX8_MAJOR_VERSION_NUM 1
#endif

/* std malloc */
#ifndef FIX8_MALLOC_STD
#define FIX8_MALLOC_STD 3
#endif

/* malloc system to use */
#ifndef FIX8_MALLOC_SYSTEM
#define FIX8_MALLOC_SYSTEM FIX8_MALLOC_TBB
#endif

/* TBB malloc */
#ifndef FIX8_MALLOC_TBB
#define FIX8_MALLOC_TBB 1
#endif

/* tcmalloc */
#ifndef FIX8_MALLOC_TCMALLOC
#define FIX8_MALLOC_TCMALLOC 2
#endif

/* Maximum length of a FIX field (default=1024) */
#ifndef FIX8_MAX_FLD_LENGTH
#define FIX8_MAX_FLD_LENGTH 1024
#endif

/* Maximum length of a FIX message (default=8192) */
#ifndef FIX8_MAX_MSG_LENGTH
#define FIX8_MAX_MSG_LENGTH 8192
#endif

/* Minor version number */
#ifndef FIX8_MINOR_VERSION_NUM
#define FIX8_MINOR_VERSION_NUM 3
#endif

/* FIX8_FF MPMC */
#ifndef FIX8_MPMC_FF
#define FIX8_MPMC_FF 2
#endif

/* MPMC system used */
#ifndef FIX8_MPMC_SYSTEM
#define FIX8_MPMC_SYSTEM FIX8_MPMC_TBB
#endif

/* FIX8_TBB MPMC */
#ifndef FIX8_MPMC_TBB
#define FIX8_MPMC_TBB 1
#endif

/* Name of package */
#ifndef FIX8_PACKAGE
#define FIX8_PACKAGE "fix8"
#endif

/* Define to the address where bug reports for this package should be sent. */
#ifndef FIX8_PACKAGE_BUGREPORT
#define FIX8_PACKAGE_BUGREPORT "fix@fix8.org"
#endif

/* Define to the full name of this package. */
#ifndef FIX8_PACKAGE_NAME
#define FIX8_PACKAGE_NAME "fix8"
#endif

/* Define to the full name and version of this package. */
#ifndef FIX8_PACKAGE_STRING
#define FIX8_PACKAGE_STRING "fix8 1.3.4"
#endif

/* Define to the one symbol short name of this package. */
#ifndef FIX8_PACKAGE_TARNAME
#define FIX8_PACKAGE_TARNAME "fix8"
#endif

/* Define to the home page for this package. */
#ifndef FIX8_PACKAGE_URL
#define FIX8_PACKAGE_URL "http://www.fix8.org"
#endif

/* Define to the version of this package. */
#ifndef FIX8_PACKAGE_VERSION
#define FIX8_PACKAGE_VERSION "1.3.4"
#endif

/* Patch number */
#ifndef FIX8_PATCH_VERSION_NUM
#define FIX8_PATCH_VERSION_NUM 4
#endif

/* Define to 1 to enable metatdata population in encode/decodes */
/* #undef POPULATE_METADATA */

/* Define to 1 if you wish to pre-encode (prepare) message support */
#ifndef FIX8_PREENCODE_MSG_SUPPORT
#define FIX8_PREENCODE_MSG_SUPPORT 1
#endif

/* Define to 1 if your os supports gprof and you wish to enable profiling */
/* #undef PROFILING_BUILD */

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* Define to 1 if you wish to enable raw FIX message support */
/* #undef RAW_MSG_SUPPORT */

/* Poco regex system */
#ifndef FIX8_REGEX_POCO
#define FIX8_REGEX_POCO 1
#endif

/* regex.h regex system */
#ifndef FIX8_REGEX_REGEX_H
#define FIX8_REGEX_REGEX_H 2
#endif

/* regex.h system used */
#ifndef FIX8_REGEX_SYSTEM
#define FIX8_REGEX_SYSTEM FIX8_REGEX_POCO
#endif

/* Percentage of message fields to reserve for additional fields */
#ifndef FIX8_RESERVE_PERCENT
#define FIX8_RESERVE_PERCENT 30
#endif

/* Define as the return type of signal handlers (`int' or `void'). */
#ifndef FIX8_RETSIGTYPE
#define FIX8_RETSIGTYPE void
#endif

/* The size of `unsigned long', as computed by sizeof. */
/* #undef FIX8_SIZEOF_UNSIGNED_LONG */

/* Define to 1 if when using fastflow, sleep for VAL ns instead of yield when
   waiting for input */
/* #undef SLEEP_NO_YIELD */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#ifndef FIX8_STDC_HEADERS
#define FIX8_STDC_HEADERS 1
#endif

/* Location of the system config directory */
#ifndef FIX8_SYSCONFDIR
#define FIX8_SYSCONFDIR "/etc"
#endif

/* pthread thread system */
#ifndef FIX8_THREAD_PTHREAD
#define FIX8_THREAD_PTHREAD 2
#endif

/* std::thread thread system */
#ifndef FIX8_THREAD_STDTHREAD
#define FIX8_THREAD_STDTHREAD 4
#endif

/* pthread used for threading */
#ifndef FIX8_THREAD_SYSTEM
#define FIX8_THREAD_SYSTEM FIX8_THREAD_STDTHREAD
#endif

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#ifndef FIX8_TIME_WITH_SYS_TIME
#define FIX8_TIME_WITH_SYS_TIME 1
#endif

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Define to 1 to enable rdtsc for interval timer if available */
/* #undef USE_RDTSC */

/* Define to 1 if using float precision */
/* #undef USE_SINGLE_PRECISION */

/* Version number of package */
#ifndef FIX8_VERSION
#define FIX8_VERSION "1.3.4"
#endif

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to 1 if you wish to enforce strict bool */
/* #undef XMLENTITY_STRICT_BOOL */

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to rpl_malloc if the replacement function should be used. */
/* #undef malloc */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to rpl_realloc if the replacement function should be used. */
/* #undef realloc */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */

/* Define as `fork' if `vfork' does not work. */
/* #undef vfork */
 
/* once: _INCLUDE_FIX__F_CONFIG_H */
#endif
