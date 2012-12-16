# Process this file with autoconf to produce a configure script.
AC_COPYRIGHT([Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007.
Fix8 Open Source FIX Engine.
Copyright (C) 2010-12 David L. Dight <fix@fix8.org>
All rights reserved.])

AC_PREREQ(2.61)
AC_INIT([fix8], m4_esyscmd([./version.sh|tr -d '\n']),fix@fix8.org,fix8,http://www.fix8.org)
AM_INIT_AUTOMAKE
AH_TOP([//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-12 David L. Dight <fix@fix8.org>

Fix8 is free software: you can redistribute it and/or modify  it under the terms of the GNU
General Public License as  published by the Free Software Foundation,  either version 3  of
the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Fix8.  If not,
see <http://www.gnu.org/licenses/>.

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

#endif
//-----------------------------------------------------------------------------------------])

AC_ENABLE_STATIC(no)
AC_CONFIG_MACRO_DIR([m4])

# generate major_version
VERSION_DASHED=`[echo $VERSION | tr . -]`
MAJOR_VERSION=`[echo $VERSION | sed 's/^\([0-9]*\.[0-9]*\)\..*/\1/']`

MAJOR_VERSION_NUM=`[echo $VERSION | sed 's/[^0-9]/ /g' | awk '{printf "%d",$1}']`
AC_DEFINE_UNQUOTED([MAJOR_VERSION_NUM], [$MAJOR_VERSION_NUM], [Major version number])
MINOR_VERSION_NUM=`[echo $VERSION | sed 's/[^0-9]/ /g' | awk '{printf "%d",$2}']`
AC_DEFINE_UNQUOTED([MINOR_VERSION_NUM], [$MINOR_VERSION_NUM], [Minor version number])
PATCH_VERSION_NUM=`[echo $VERSION | sed 's/[^0-9]/ /g' | awk '{printf "%d",$4}']`
AC_DEFINE_UNQUOTED([PATCH_VERSION_NUM], [$PATCH_VERSION_NUM], [Patch number])
AC_DEFINE_UNQUOTED([MAGIC_NUM], [(MAJOR_VERSION_NUM << 29 | MINOR_VERSION_NUM << 23 | PATCH_VERSION_NUM)], [Encoded Version])
AC_DEFINE_UNQUOTED([CONFIGURE_OPTIONS], ["$ac_configure_args"], [configure options])

BRAND_SUMMARY='Fix8 Open Source FIX Framework'
BRAND_DESCRIPTION='Modern fast FIX Framework'
BRAND_COMPANY_SHORTNAME='fix8'
BRAND_COMPANY_LONGNAME='fix8 by David L. Dight'

# Checks for programs.
AC_PROG_LIBTOOL
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
AC_HEADER_DIRENT
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
esac

# use -fvar-tracking-assignments-toggle
AM_CONDITIONAL(TOGGLETRACKINGASSIGNMENTS, [test x$is_late_gcc = xyes])

if test -z "$DOXYGEN";
	then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
oktodox=yes
AC_ARG_ENABLE([doxygen],
[AC_HELP_STRING([--enable-doxygen],[enable doxygen generation (default=yes if available)])],
[case "${enableval}" in
	yes) ;;
	no)  oktodox=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-doxygen) ;;
esac])

AM_CONDITIONAL([USEDOXYGEN],
	[test -n "$DOXYGEN" && test $oktodox = yes])AM_COND_IF([USEDOXYGEN],[AC_CONFIG_FILES([doc/Makefile])])

AC_CHECK_FUNCS([scandir getopt_long sysconf getaddrinfo getnameinfo \
	daemon popen waitpid setlogmask alarm dup2 ftruncate \
	getcwd gettimeofday localtime_r pow regcomp socket strcasecmp strchr strdup strerror \
	strncasecmp strspn strtol strtoul random srandom
	])

AC_CHECK_LIB(rt, clock_gettime, [AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [Define if you have clock_gettime()])])
AC_CHECK_LIB(rt, clock_nanosleep, [AC_DEFINE(HAVE_CLOCK_NANOSLEEP, 1, [Define if you have clock_nanosleep()])])

AC_CHECK_HEADERS([getopt.h fcntl.h netdb.h regex.h signal.h string.h sys/time.h \
	arpa/inet.h sys/stat.h sys/types.h select.h sys/event.h netinet/tcp.h \
	sys/wait.h netinet/in.h getopt.h limits.h sys/ioctl.h unistd.h \
	sys/socket.h time.h syslog.h termios.h alloca.h
	])

PROCSTAT=/proc/stat
AC_CHECK_FILE("$PROCSTAT",
	[
		AC_DEFINE_UNQUOTED(HAVE_PROC_STAT, 1, [Define to 1 if /proc/stat is available])
	])

PROCPIDSTAT=/proc/"$PPID"/stat
AC_CHECK_FILE("$PROCPIDSTAT",
	[
		AC_DEFINE_UNQUOTED(HAVE_PROC_PID_STAT, 1, [Define to 1 if /proc/[pid]/stat is available])
	])

PROCPIDSTATM=/proc/"$PPID"/statm
AC_CHECK_FILE("$PROCPIDSTATM",
	[
		AC_DEFINE_UNQUOTED(HAVE_PROC_PID_STATM, 1, [Define to 1 if /proc/[pid]/statm is available])
	])

DEVURANDOM=/dev/urandom
AC_CHECK_FILE("$DEVURANDOM",
	[
		AC_DEFINE(HAVE_DEV_URANDOM, 1, [Define to 1 if the /dev/urandom device is available])
	])

AC_DEFINE_UNQUOTED(RESERVE_PERCENT, 30, [Percentage of message fields to reserve for additional fields])
AC_ARG_WITH(reservepercent,
  AC_HELP_STRING([--with-reservepercent=VALUE],[Percentage of message fields to reserve for additional fields]),
[
	AC_DEFINE_UNQUOTED(RESERVE_PERCENT, ${withval},[Percentage of message fields to reserve for additional fields])
],[])

AC_DEFINE_UNQUOTED(MAX_MSG_LENGTH, 8192, [Maximum length of a FIX message in bytes])
AC_ARG_WITH(maxmsglen,
  AC_HELP_STRING([--with-maxmsglen=VALUE],[Maximum length of a FIX message (default=8192)]),
[
	AC_DEFINE_UNQUOTED(MAX_MSG_LENGTH, ${withval}, [Maximum length of a FIX message (default=8192)])
],[])

AC_ARG_WITH(poco,
  AC_HELP_STRING([--with-poco=DIR],[location of Poco]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}/include"
	LDFLAGS="$LDFLAGS -L${withval}/lib"
],[])

AC_ARG_WITH(tbb,
  AC_HELP_STRING([--with-tbb=DIR],[location of TBB]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}/include"
	LDFLAGS="$LDFLAGS -L${withval}/lib"
],[])

AC_MSG_CHECKING(for TBB)
	AC_TRY_COMPILE([
		#include <tbb/atomic.h>
		#include <tbb/mutex.h>
		#include <tbb/concurrent_queue.h>
		],[;],
		have_tbb=yes, have_tbb=no)
	if test "$have_tbb" = yes; then
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(HAS_TBB, 1, [Define to 1 if TBB available])
	else
	    AC_MSG_RESULT(no)
		 echo
		 echo "ERROR: TBB development libraries are required. Install and try again (see wiki:prerequisites)."
		 echo
		 exit 1
	fi

AC_MSG_CHECKING(for Poco)
	AC_TRY_COMPILE([
		#include <Poco/Net/ServerSocket.h>
		#include <Poco/Net/StreamSocket.h>
		#include <Poco/Timestamp.h>
		#include <Poco/DateTime.h>
		#include <Poco/DateTimeParser.h>
		#include <Poco/DateTimeFormatter.h>
		#include <Poco/Net/NetException.h>
		],[;],
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

AX_PATH_BDB([3],[ LIBS="$BDB_LIBS $LIBS"
LDFLAGS="$BDB_LDFLAGS $LDFLAGS"
CPPFLAGS="$CPPFLAGS $BDB_CPPFLAGS"
AC_DEFINE(HAVE_BDB, 1, [Define to 1 if you have berkeley DB])
	])

if test "$have_bdb" = "no"; then
		echo
		echo "ERROR: Oracle Berkeley DB development libraries are required. Install and try again (see wiki:prerequisites)."
		echo
		exit 1
fi

# Check for timersub macro in sys/time.h
AC_CACHE_CHECK([for timersub macro in sys/time.h], ac_cv_timersubmacro,
		[AC_TRY_COMPILE([#include <sys/time.h>],
[struct timeval a, b, c; timersub(&a, &b, &c);],
			  timersubmacro=yes, timersubmacro=no)])
if test "$timersubmacro" = "yes"; then
    AC_DEFINE(HAVE_TIMERSUB, 1, [Define to 1 if timersub macro is in sys/time.h])
fi

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
[AC_HELP_STRING([--enable-xmlstrictbool],[enable xml strict bool behaviour (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(XMLENTITY_STRICT_BOOL, 1, [Define to 1 if you wish to enforce strict bool ]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-xmlstrictbool) ;;
esac])

AC_ARG_ENABLE([bufgloblogging],
[AC_HELP_STRING([--enable-bufgloblogging],[enable buffered global logging behaviour (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(BUFFERED_GLOBAL_LOGGING, 1, [Define to 1 if you wish to enable buffered global logging]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-bufgloblogging) ;;
esac])

AC_ARG_ENABLE([valgrind],
[AC_HELP_STRING([--enable-valgrind],[enable internal support for valgrind (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(HAVE_VALGRIND, 1, [Define to 1 for valgrind support ])
		  AC_CHECK_HEADERS([valgrind/valgrind.h]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-valgrind) ;;
esac])

AC_ARG_ENABLE([gprof],
[AC_HELP_STRING([--enable-gprof],[enable internal support for gprof (default=no)])],
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

AC_ARG_ENABLE([poolalloc],
[AC_HELP_STRING([--enable-poolalloc],[enable Memory Pool Allocator (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(POOLALLOC, 1, [Define to 1 for Memory Pool Allocator]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-poolalloc) ;;
esac])

AC_ARG_ENABLE([msgrecycle],
[AC_HELP_STRING([--enable-msgrecycle],[enable Message Recycling (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(MSGRECYCLING, 1, [Define to 1 to enable Message Recycling]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-msgrecycle) ;;
esac])

AC_ARG_ENABLE([codectiming],
[AC_HELP_STRING([--enable-codectiming],[enable CODEC timing testing code (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(CODECTIMING, 1, [Define to 1 to enable CODEC timing testing code]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-codectiming) ;;
esac])

AC_ARG_ENABLE([customfields],
[AC_HELP_STRING([--enable-customfields],[enable User defined fields (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(PERMIT_CUSTOM_FIELDS, 1, [Define to 1 to enable User Defined Fields]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-customfields) ;;
esac])

AC_ARG_ENABLE([debug],
[AC_HELP_STRING([--enable-debug],[enable DEBUG support (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(DEBUG, 1, [Define to 1 for debugging extensions]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac])

AC_ARG_ENABLE([ssl],
[AC_HELP_STRING([--enable-ssl],[enable SSL support (through OpenSSL, default=no)])],
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
AC_DEFINE_UNQUOTED([CONFIGURE_SDATE],["$CONFIGURE_SDATE"],[Short Date system was configured])
AC_DEFINE_UNQUOTED([CONFIGURE_TIME_NUM],[`date +%s`],[date/time as seconds since start epoch])
AC_DEFINE_UNQUOTED([HOST_SYSTEM],["$host_cpu-$host_vendor-$host_os"],[Default system])

INTSYSCONF=`eval echo $sysconfdir`
if test $prefix/etc = $INTSYSCONF; then
	INTSYSCONF=/etc
fi
AC_DEFINE_UNQUOTED(SYSCONFDIR, "$INTSYSCONF", [Location of the system config directory])

AC_MSG_CHECKING(architecture and if we need to supply scandir)
case "$host_os" in
	solaris* | sunos4*)	sunos=yes
				AC_DEFINE(ISSUNOS, 1, [Define to 1 if this system is running sunos/solaris])
				AC_MSG_RESULT(yes) ;;
	netbsd* | openbsd* | *freebsd*)
				AC_DEFINE(ISBSD, 1, [Define to 1 if this system is running a BSD variant])
			 	AC_MSG_RESULT(no) ;;
	linux*)	AC_DEFINE(ISLINUX, 1, [Define to 1 if this system is running linux])
			 	AC_MSG_RESULT(no) ;;
	*) 		AC_MSG_RESULT(no) ;;
esac

AM_CONDITIONAL(ISSUN, [test x$sunos = xyes])

# substitutions
#
AC_SUBST(PIDDIR)
AC_SUBST(INTSYSCONF)
AC_SUBST(CONFIGURE_TIME)
AC_SUBST(CONFIGURE_SDATE)
AC_SUBST(CONFIGURE_TIME_NUM)
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

AC_CONFIG_FILES([Makefile runtime/Makefile compiler/Makefile test/Makefile doc/fix8.doxygen])
AC_CONFIG_HEADERS(include/f8config.h)
AC_OUTPUT

AC_SUBST(CONFIG_SHELL)
AM_CONDITIONAL(HAVE_OPENSSL, test "x$openssl" = "xyes")

