# Process this file with autoconf to produce a configure script.
AC_COPYRIGHT([Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.
Fix8 Open Source FIX Engine.
Copyright (C) 2010-13 David L. Dight <fix@fix8.org>
All rights reserved.])

AC_PREREQ(2.61)
AC_INIT([fix8], m4_esyscmd([./version.sh|tr -d '\n']),fix@fix8.org,fix8,http://www.fix8.org)
AM_INIT_AUTOMAKE
AH_TOP([//-----------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-13 David L. Dight <fix@fix8.org>

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
//-----------------------------------------------------------------------------------------])

AC_ENABLE_STATIC(no)
AC_CONFIG_MACRO_DIR([m4])

# generate major_version
VERSION_DASHED=`[echo $VERSION | tr . -]`
MAJOR_VERSION=`[echo $VERSION | sed 's/^\([0-9]*\.[0-9]*\)\..*/\1/']`
echo "checking version... $VERSION"

MAJOR_VERSION_NUM=`[echo $VERSION | sed 's/[^0-9]/ /g' | awk '{printf "%d",$1}']`
AC_DEFINE_UNQUOTED([MAJOR_VERSION_NUM], [$MAJOR_VERSION_NUM], [Major version number])
MINOR_VERSION_NUM=`[echo $VERSION | sed 's/[^0-9]/ /g' | awk '{printf "%d",$2}']`
AC_DEFINE_UNQUOTED([MINOR_VERSION_NUM], [$MINOR_VERSION_NUM], [Minor version number])
PATCH_VERSION_NUM=`[echo $VERSION | sed 's/[^0-9]/ /g' | awk '{printf "%d",$3}']`
AC_DEFINE_UNQUOTED([PATCH_VERSION_NUM], [$PATCH_VERSION_NUM], [Patch number])
AC_DEFINE_UNQUOTED([MAGIC_NUM], [(MAJOR_VERSION_NUM << 24 | MINOR_VERSION_NUM << 12 | PATCH_VERSION_NUM)], [Encoded Version])
AC_DEFINE_UNQUOTED([CONFIGURE_OPTIONS], ["$ac_configure_args"], [configure options])

BRAND_SUMMARY='Fix8 Open Source FIX Framework'
BRAND_DESCRIPTION='Modern fast FIX Framework'
BRAND_COMPANY_SHORTNAME='fix8'
BRAND_COMPANY_LONGNAME='fix8 by David L. Dight'

# Checks for programs.
LT_INIT
AC_PROG_CXX
AC_PROG_CC
AC_C_BIGENDIAN
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_C_LONG_DOUBLE
AC_TYPE_LONG_LONG_INT

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME
AC_STRUCT_TM
AC_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
# these are checked before the libraries are added to the compilation flags so that they will link
# properly
AC_PROG_GCC_TRADITIONAL
AC_LANG_CPLUSPLUS
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_TYPE_SIGNAL
AC_FUNC_ALLOCA
AC_HEADER_SYS_WAIT
AC_FUNC_FORK
AC_HEADER_STDC
AC_FUNC_MMAP
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_REALLOC
AC_FUNC_STRCOLL

AC_CHECK_TOOL([STRIP],[strip])
AC_CHECK_TOOL([DOXYGEN],[doxygen])

# check what version of gcc we're using

# test for gcc >= 4.7
is_late_gcc=no
case "$CXX" in
  *g++*)
		GCC_VERSION=`$CC -dumpversion`
		gcc_major=`echo $GCC_VERSION | cut -d. -f1`
		gcc_minor=`echo $GCC_VERSION | cut -d. -f2`
		if test $gcc_major -ge 4 -a $gcc_minor -ge 7; then
			is_late_gcc=yes
		fi ;;
	*clang*)
		LIBS="$LIBS -lstdc++ -lm " ;;
esac

oktotogglevar=yes
AC_ARG_ENABLE([toggleassignmentvars],
[AS_HELP_STRING([--enable-toggleassignmentvars],[enable toggleassignmentvars for gcc (default=yes if available)])],
[case "${enableval}" in
	yes) ;;
	no)  oktotogglevar=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-toggleassignmentvars) ;;
esac])

# use -fvar-tracking-assignments-toggle
AM_CONDITIONAL(TOGGLETRACKINGASSIGNMENTS, [test x$is_late_gcc = xyes && test x$oktotogglevar = xyes])

if test -z "$DOXYGEN";
	then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
oktodox=yes
AC_ARG_ENABLE([doxygen],
[AS_HELP_STRING([--enable-doxygen],[enable doxygen generation (default=yes if available)])],
[case "${enableval}" in
	yes) ;;
	no)  oktodox=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-doxygen) ;;
esac])

AM_CONDITIONAL([USEDOXYGEN],
	[test -n "$DOXYGEN" && test $oktodox = yes])AM_COND_IF([USEDOXYGEN],[AC_CONFIG_FILES([doc/Makefile])])

DOXYGEN_WARNINGS=YES
AC_ARG_ENABLE([doxygen-warnings],
[AS_HELP_STRING([--enable-doxygen-warnings],[permit doxygen to generate warnings (default=yes)])],
[case "${enableval}" in
	no) DOXYGEN_WARNINGS=NO ;;
	yes) ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-doxygen-warnings) ;;
esac])

AC_CHECK_FUNCS([getopt_long sysconf popen getcwd gettimeofday localtime_r regcomp socket strcasecmp \
		strerror strtol strtoul random srandom
	])

AC_CHECK_DECLS([TCP_CORK], [], [], [[#include <netinet/tcp.h>]])

fillmetadata=yes
AC_ARG_ENABLE([fillmetadata],
[AS_HELP_STRING([--enable-fillmetadata],[enable metadata population (default=yes)])],
[case "${enableval}" in
	yes) AC_DEFINE(POPULATE_METADATA, 1, [Define to 1 to enable metatdata population in encode/decodes]) ;;
	no)  fillmetadata=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-fillmetadata) ;;
esac])

AC_ARG_ENABLE([rdtsc],
[AS_HELP_STRING([--enable-rdtsc],[use rdtsc for interval timer if available])],
[case "${enableval}" in
	yes) AC_DEFINE(USE_RDTSC, 1, [Define to 1 to enable rdtsc for interval timer if available]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-rdtsc) ;;
esac])

oktolinkrt=yes
AC_CHECK_LIB(rt, clock_gettime, [AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [Define if you have clock_gettime()])], oktolinkrt=no)
AC_CHECK_LIB(rt, clock_nanosleep, [AC_DEFINE(HAVE_CLOCK_NANOSLEEP, 1, [Define if you have clock_nanosleep()])], oktolinkrt=no)
AM_CONDITIONAL(LINK_RT, [test x$oktolinkrt = xyes])

AC_CHECK_HEADERS([getopt.h fcntl.h netdb.h regex.h signal.h string.h sys/time.h \
	arpa/inet.h sys/stat.h sys/types.h select.h \
	sys/wait.h netinet/in.h getopt.h limits.h sys/ioctl.h unistd.h \
	sys/socket.h time.h syslog.h termios.h alloca.h], [], [],
	[[
     #ifdef HAVE_GETOPT_H
     # include <getop.h>
     #endif
     #ifdef HAVE_FCNTL_H
     # include <fcntl.h>
     #endif
     #ifdef HAVE_NETDB_H
     # include <netdb.h>
     #endif
     #ifdef HAVE_REGEX_H
     # include <regex.h>
     #endif
     #ifdef HAVE_SIGNAL_H
     # include <signal.h>
     #endif
     #ifdef HAVE_ARPA_INET_H
     # include <arpa/inet.h>
     #endif
     #ifdef HAVE_NETINET_IN_H
     # include <netinet/in.h>
     #endif
     #ifdef HAVE_LIMITS_H
     # include <limits.h>
     #endif
     #ifdef HAVE_SYS_IOCTL_H
     # include <sys/ioctl.h>
     #endif
     #ifdef HAVE_SYS_SOCKET_H
     # include <sys/socket.h>
     #endif
     #ifdef HAVE_TIME_H
     # include <time.h>
     #endif
     #ifdef HAVE_SYSLOG_H
     # include <syslog.h>
     #endif
     #ifdef HAVE_TERMIOS_H
     # include <termios.h>
     #endif
     #ifdef HAVE_ALLOCA_H
     # include <alloca.h>
     #endif
	]])


AC_DEFINE_UNQUOTED(RESERVE_PERCENT, 30, [Percentage of message fields to reserve for additional fields])
AC_ARG_WITH(reservepercent,
  AS_HELP_STRING([--with-reservepercent=VALUE],[Percentage of message fields to reserve for additional fields]),
[
	AC_DEFINE_UNQUOTED(RESERVE_PERCENT, ${withval},[Percentage of message fields to reserve for additional fields])
],[])

AC_DEFINE_UNQUOTED(MAX_MSG_LENGTH, 8192, [Maximum length of a FIX message in bytes])
AC_ARG_WITH(maxmsglen,
  AS_HELP_STRING([--with-maxmsglen=VALUE],[Maximum length of a FIX message (default=8192)]),
[
	AC_DEFINE_UNQUOTED(MAX_MSG_LENGTH, ${withval}, [Maximum length of a FIX message (default=8192)])
],[])

AC_DEFINE_UNQUOTED(MAX_FLD_LENGTH, 1024, [Maximum length of a FIX field in bytes])
AC_ARG_WITH(maxfldlen,
  AS_HELP_STRING([--with-maxfldlen=VALUE],[Maximum length of a FIX field (default=1024)]),
[
	AC_DEFINE_UNQUOTED(MAX_FLD_LENGTH, ${withval}, [Maximum length of a FIX field (default=1024)])
],[])

AC_DEFINE_UNQUOTED(DEFAULT_PRECISION, 2, [Default precision for floating point fields])
AC_ARG_WITH(precision,
  AS_HELP_STRING([--with-precision=VALUE],[Default precision for floating point fields (default=2)]),
[
	AC_DEFINE_UNQUOTED(DEFAULT_PRECISION, ${withval}, [Default precision for floating point fields (default=2)])
],[])

AC_ARG_WITH(tbb,
  AS_HELP_STRING([--with-tbb=DIR],[location of TBB]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}/include"
	LDFLAGS="$LDFLAGS -L${withval}/lib"
],[])

AC_ARG_WITH(gtest,
  AS_HELP_STRING([--with-gtest=DIR],[location of gtest]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}/include"
	LDFLAGS="$LDFLAGS -L${withval}/lib -L${withval}/lib/.libs"
],[])

usetcmalloc=yes
AC_ARG_ENABLE([tcmalloc],
[AS_HELP_STRING([--enable-tcmalloc],[enable gperftools tcmalloc (default=yes)])],
[case "${enableval}" in
	yes) ;;
	no)  usetcmalloc=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-tcmalloc) ;;
esac])

usetbbmalloc=no
AC_ARG_ENABLE([tbbmalloc],
[AS_HELP_STRING([--enable-tbbmalloc],[enable TBB tbbmalloc (default=no)])],
[case "${enableval}" in
	yes) usetbbmalloc=yes;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-tbbmalloc) ;;
esac])

AC_ARG_WITH(gperf,
  AS_HELP_STRING([--with-gperf=DIR],[location of gperf libraries]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}/include"
	LDFLAGS="$LDFLAGS -L${withval}/lib"
],[])

AC_DEFINE(MPMC_TBB, 1, [TBB MPMC])
AC_DEFINE(MPMC_FF, 2, [FF MPMC])

mpmc_set=no
using_ff=no
AC_ARG_WITH([mpmc],
[AS_HELP_STRING([--with-mpmc=VAL],[select which mpmc system will be used (tbb|ff, default=ff)])],
[case "${withval}" in
	tbb) AC_MSG_CHECKING(for TBB)
				AC_TRY_COMPILE([
					#include <tbb/atomic.h>
					#include <tbb/mutex.h>
					#include <tbb/concurrent_queue.h>
					],,
					have_tbb=yes, have_tbb=no)
				if test "$have_tbb" = yes; then
					 AC_MSG_RESULT(yes)
					 AC_DEFINE(MPMC_SYSTEM, MPMC_TBB, [MPMC system used])
				else
					 AC_MSG_RESULT(no)
					 echo
					 echo "ERROR: TBB development libraries not found. Install and try again."
					 echo
					 exit 1
				fi
		 mpmc_set=yes ;;
	ff) using_ff=yes;;
	*) echo
		echo "ERROR: Unknown value ${withval} for MPMC given."
		echo
		exit 1
esac])

if test "$mpmc_set" = no; then
	using_ff=yes
	AC_DEFINE(MPMC_SYSTEM, MPMC_FF, [MPMC system used])
fi

AM_CONDITIONAL(TBBSUPPORT, [test x$have_tbb = xyes])

AC_CHECK_LIB(tcmalloc_minimal, malloc, have_tcmalloc_minimal_lib=yes)
AC_CHECK_LIB(tcmalloc, malloc, have_tcmalloc_lib=yes)

if test "$have_tbb" = yes && test x$usetbbmalloc = xyes; then
	LIBS="$LIBS -ltbbmalloc -ltbbmalloc_proxy"
else if test "$have_tcmalloc_minimal_lib" = yes && test x$usetcmalloc = xyes; then
	LIBS="$LIBS -ltcmalloc_minimal"
else if test "$have_tcmalloc_lib" = yes && test x$usetcmalloc = xyes; then
	LIBS="$LIBS -ltcmalloc"
fi
fi
fi

AC_ARG_WITH(sleep,
	AS_HELP_STRING([--with-sleep=VAL],[If using fastflow, sleep for VAL ns instead of yield when waiting for input(default=no)]),
[
	AC_DEFINE_UNQUOTED(SLEEP_NO_YIELD, ${withval}, [Define to 1 if when using fastflow, sleep for VAL ns instead of yield when waiting for input])
],[])

AC_ARG_WITH(poco,
  AS_HELP_STRING([--with-poco=DIR],[location of Poco]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}/include"
	LDFLAGS="$LDFLAGS -L${withval}/lib"
],[])

AC_MSG_CHECKING(for Poco)
	AC_TRY_COMPILE([
		#include <Poco/Net/ServerSocket.h>
		#include <Poco/Net/StreamSocket.h>
		#include <Poco/Timestamp.h>
		#include <Poco/DateTime.h>
		#include <Poco/DateTimeParser.h>
		#include <Poco/DateTimeFormatter.h>
		#include <Poco/Net/NetException.h>
		],,
		have_poco=yes, have_poco=no)
	if test "$have_poco" = yes; then
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(HAS_POCO, 1, [Define to 1 if Poco available])
	else
	    AC_MSG_RESULT(no)
		 echo
		 echo "ERROR: Poco development libraries are required. Install and try again (see wiki:prerequisites)."
		 echo
		 exit 1
	fi

AC_DEFINE(MPMC_TBB, 1, [TBB MPMC])
AC_DEFINE(MPMC_FF, 2, [FF MPMC])

regex_set=no
using_poco_regex=no
AC_ARG_WITH([regex],
[AS_HELP_STRING([--with-regex=VAL],[select which regex system will be used (regex|poco, default=poco)])],
[case "${withval}" in
	regex) AC_MSG_CHECKING(for regex)
				AC_TRY_COMPILE([
					#include <regex.h>
					],,
					have_regex=yes, have_regex=no)
				if test "$have_regex" = yes; then
					 AC_MSG_RESULT(yes)
					 AC_DEFINE(REGEX_SYSTEM, REGEX_REGEX_H, [regex.h system used])
				else
					 AC_MSG_RESULT(no)
					 echo
					 echo "ERROR: regex.h header not found. Install and try again."
					 echo
					 exit 1
				fi
		 regex_set=yes ;;
	poco) using_poco_regex=yes;;
	*) echo
		echo "ERROR: Unknown value ${withval} for REGEX given."
		echo
		exit 1
esac])

AC_DEFINE(REGEX_POCO, 1, [Poco regex system])
AC_DEFINE(REGEX_REGEX_H, 2, [regex.h regex system])
if test "$regex_set" = no; then
	using_poco_regex=yes
	AC_DEFINE(REGEX_SYSTEM, REGEX_POCO, [POCO regex system used])
fi

oktogtest=yes
AC_ARG_ENABLE([gtest],
[AS_HELP_STRING([--enable-gtest],[enable gtest unit test case building if available (default=yes if available)])],
[case "${enableval}" in
	yes) ;;
	no)  oktogtest=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-gtest) ;;
esac])

AC_MSG_CHECKING(for gtest)
    AC_TRY_COMPILE([
        #include <gtest/gtest.h>
        ],[;],
        have_gtest=yes, have_gtest=no)
    if test "$have_gtest" = yes && test "$oktogtest" = yes; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(HAS_GTEST, 1, [Define to 1 if gtest available])
    else
        AC_MSG_RESULT(no)
    fi
AM_CONDITIONAL(USEGTEST, [test x$have_gtest = xyes && test "$oktogtest" = yes])


AC_MSG_CHECKING(for tr1/unordered_map)
	AC_TRY_COMPILE([
	#include <tr1/unordered_map>
	#include <string>
		],[std::tr1::unordered_map<std::string, std::string> uom; uom["blah"]="blather";],
		have_tr1_unordered_map=yes, have_tr1_unordered_map=no)
	if test "$have_tr1_unordered_map" = yes; then
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(HAS_TR1_UNORDERED_MAP, 1, [Define to 1 if TR1 hashmaps are available])
	else
	    AC_MSG_RESULT(no)
	fi

AC_CHECK_LIB(crypt, crypt, [
	AC_CHECK_HEADERS([crypt.h],
	[
		AC_DEFINE([HAVE_CRYPT], [1], [Define to 1 if crypt is present in -lcrypt])
		havecrypt=yes
	])
])
AM_CONDITIONAL(USECCRYPT, [test x$havecrypt = xyes])

m4_include(m4/ax_compare_version.m4)
m4_include(m4/ax_path_bdb.m4)

oktof8test=yes
AC_ARG_ENABLE([f8test],
[AS_HELP_STRING([--enable-f8test],[permit f8test client/server to be built (default=yes)])],
[case "${enableval}" in
	no) oktof8test=no ;;
	yes) ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-f8test) ;;
esac])

AM_CONDITIONAL(F8TEST, [test x$oktof8test = xyes])

oktobdb=no
AC_ARG_ENABLE([bdb],
[AS_HELP_STRING([--enable-bdb],[enable support for Oracle Berkeley DB persistence (default=no)])],
[case "${enableval}" in
	yes) oktobdb=yes
		AX_PATH_BDB([3],[ LIBS="$BDB_LIBS $LIBS" LDFLAGS="$BDB_LDFLAGS $LDFLAGS" CPPFLAGS="$CPPFLAGS $BDB_CPPFLAGS"
			AC_DEFINE(HAVE_BDB, 1, [Define to 1 if you have berkeley DB])]) ;;
	no) ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-bdb) ;;
esac])

AM_CONDITIONAL(BDBSUPPORT, [test x$oktobdb = xyes])

AC_CHECK_LIB(z, deflate, [
	AC_CHECK_HEADERS([zlib.h],
	[
		Z_LIB=-lz
		AC_DEFINE([HAVE_LIBZ], [1], [Define to 1 if libz is present])
		AC_DEFINE([HAVE_ZLIB_H], [1])
		AC_DEFINE(HAVE_COMPRESSION, 1, [Define to 1 if zlib headers and library were found])
		havecompression=yes
	])
])
AM_CONDITIONAL(USECOMPRESSION, [test x$havecompression = xyes])
AC_SUBST(Z_LIB)
if test "$havecompression" != "yes"; then
		echo
		echo "ERROR: zlib (libz) development libraries are required. Install and try again."
		echo
		exit 1
fi

if test x"$GCC" != x"yes"; then
	case x"$build" in
		xsparc-sun-solaris*)
		CFLAGS="$CFLAGS -xCC"
		AC_TRY_COMPILE(
			[ // a comment ],
			[],
			[],
			AC_MSG_ERROR([-xCC does not allow compilation of C++ comments])
		)
		esac
else
	CFLAGS="$CFLAGS -Wall"
	CXXFLAGS="$CXXFLAGS -Wall"
fi

AC_ARG_ENABLE([xmlstrictbool],
[AS_HELP_STRING([--enable-xmlstrictbool],[enable xml strict bool behaviour (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(XMLENTITY_STRICT_BOOL, 1, [Define to 1 if you wish to enforce strict bool ]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-xmlstrictbool) ;;
esac])

AC_ARG_ENABLE([bufgloblogging],
[AS_HELP_STRING([--enable-bufgloblogging],[enable buffered global logging behaviour (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(BUFFERED_GLOBAL_LOGGING, 1, [Define to 1 if you wish to enable buffered global logging]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-bufgloblogging) ;;
esac])

AC_ARG_ENABLE([valgrind],
[AS_HELP_STRING([--enable-valgrind],[enable internal support for valgrind (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(HAVE_VALGRIND, 1, [Define to 1 for valgrind support ])
		  AC_CHECK_HEADERS([valgrind/valgrind.h]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-valgrind) ;;
esac])

AC_ARG_ENABLE([gprof],
[AS_HELP_STRING([--enable-gprof],[enable internal support for gprof (default=no)])],
[case "${enableval}" in
	yes) AC_CHECK_HEADERS([sys/gmon.h],[ have_gprof_headers=yes ])
			if test "$have_gprof_headers" = yes; then
				 AC_DEFINE(PROFILING_BUILD, 1,
					[Define to 1 if your os supports gprof and you wish to enable profiling])
				 CFLAGS="$CFLAGS -pg"
				 CXXFLAGS="$CXXFLAGS -pg"
			fi ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-gprof) ;;
esac])

AC_ARG_ENABLE([codectiming],
[AS_HELP_STRING([--enable-codectiming],[enable CODEC timing testing code (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(CODECTIMING, 1, [Define to 1 to enable CODEC timing testing code]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-codectiming) ;;
esac])

AC_ARG_ENABLE([debug],
[AS_HELP_STRING([--enable-debug],[enable DEBUG support (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(DEBUG, 1, [Define to 1 for debugging extensions]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac])

AC_ARG_ENABLE([ssl],
[AS_HELP_STRING([--enable-ssl],[enable SSL support (through OpenSSL, default=no)])],
[case "${enableval}" in
	yes) openssl=yes ;;
	no)  openssl=no ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-https) ;;
esac],[openssl=no])

if test "$openssl" = yes; then
		unset openssl_path ac_cv_lib_ssl_SSL_new ac_cv_header_openssl_ssl_h
		if test "$openssl" != yes; then
				 openssl_path=$openssl
		fi
		openssl=no
		SAVED_LIBS=$LIBS
		LIBS="$LIBS -lcrypto"
		if test -n "$openssl_path"; then
				 LIBS="-L$openssl_path/lib $LIBS"
		fi
		AC_CHECK_LIB(ssl, SSL_new, have_openssl=yes)
		AC_CHECK_HEADERS(openssl/ssl.h, have_openssl_h=yes)
		LIBS=$SAVED_LIBS
		AC_MSG_CHECKING(if we can enable OpenSSL support)
		if test "$have_openssl" = yes; then
			SAVED_CPPFLAGS=$CPPFLAGS
			if test -n "$openssl_path"; then
				CPPFLAGS="-I$openssl_path/include $CPPFLAGS"
			fi
			if test "$have_openssl_h" = yes; then
			AC_MSG_RESULT(yes)
			openssl=yes
			AC_DEFINE(HAVE_OPENSSL, 1, [Define to 1 if you have openssl])
			LIBS="$LIBS -lssl -lcrypto"
			if test -n "$openssl_path"; then
				LIBS="-L$openssl_path/lib $LIBS"
			fi
			else
				CPPFLAGS=$SAVED_CPPFLAGS
				AC_MSG_RESULT(no)
			fi
		else
			AC_MSG_RESULT(no)
			echo
			echo "ERROR: SSL not supported in your build environment. Install and retry."
			echo
			exit 1
		fi
fi
AM_CONDITIONAL(USESSL, [test x$have_openssl = xyes])

AC_MSG_CHECKING(checking for /var/run/)
if test -d /var/run; then
	AC_DEFINE(HAVE_VAR_RUN, 1, [Define to 1 if you have /var/run])
	AC_MSG_RESULT(yes)
	PIDDIR=/var/run
else
	PIDDIR=/tmp
	AC_MSG_RESULT([no, using (${PIDDIR})])
fi

AC_DEFINE_UNQUOTED([CONFIGURE_TIME],["`date`"],[Date system was configured])
CONFIGURE_SDATE=`date +%d/%m/%Y`
BUILD_NUMBER_DATE=`date +%d%m%y`

AC_DEFINE_UNQUOTED([CONFIGURE_SDATE],["$CONFIGURE_SDATE"],[Short Date system was configured])
AC_DEFINE_UNQUOTED([CONFIGURE_TIME_NUM],[`date +%s`],[date/time as seconds since start epoch])
AC_DEFINE_UNQUOTED([HOST_SYSTEM],["$host_cpu-$host_vendor-$host_os"],[Default system])
AC_DEFINE_UNQUOTED([CPPFLAGS],["$CPPFLAGS"],[compiler spec])
AC_DEFINE_UNQUOTED([LDFLAGS],["$LDFLAGS"],[Additional library flags])
AC_DEFINE_UNQUOTED([LIBS],["$LIBS"],[Library spec])

INTSYSCONF=`eval echo $sysconfdir`
if test $prefix/etc = $INTSYSCONF; then
	INTSYSCONF=/etc
fi
AC_DEFINE_UNQUOTED(SYSCONFDIR, "$INTSYSCONF", [Location of the system config directory])

AM_CONDITIONAL(ISSUN, [test x$sunos = xyes])

# substitutions
#
AC_SUBST(PIDDIR)
AC_SUBST(INTSYSCONF)
AC_SUBST(CONFIGURE_TIME)
AC_SUBST(CONFIGURE_SDATE)
AC_SUBST(DOXYGEN_WARNINGS)
AC_SUBST(CONFIGURE_TIME_NUM)
AC_SUBST(BUILD_NUMBER_DATE)
AC_SUBST(LIBS)
AC_SUBST(VERSION_DASHED)
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MAJOR_VERSION_DASHED)
AC_SUBST(PROJ_RELEASE_VERSION)
AC_SUBST(CONFIGURE_OPTIONS)
AC_SUBST(MAJOR_VERSION_NUM)
AC_SUBST(MINOR_VERSION_NUM)
AC_SUBST(PATCH_VERSION_NUM)
AC_SUBST(BRAND_SUMMARY)
AC_SUBST(BRAND_DESCRIPTION)
AC_SUBST(BRAND_COMPANY_SHORTNAME)
AC_SUBST(BRAND_COMPANY_LONGNAME)
AC_SUBST_FILE([CHANGELOG])
CHANGELOG=$srcdir/ChangeLog

AC_CONFIG_FILES([Makefile runtime/Makefile compiler/Makefile test/Makefile utests/Makefile util/Makefile doc/fix8.doxygen \
						fix8.spec doc/man/f8c.1.xml doc/man/seqedit.1.xml])
AC_CONFIG_HEADERS(include/fix8/f8config.h)
AC_OUTPUT

AC_SUBST(CONFIG_SHELL)
AM_CONDITIONAL(HAVE_OPENSSL, test "x$openssl" = "xyes")

