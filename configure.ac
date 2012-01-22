# Process this file with autoconf to produce a configure script.
AC_COPYRIGHT([Orbweb is released under the New BSD License.
Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
All rights reserved.])

AC_PREREQ(2.61)
AC_INIT([fix8], [m4_esyscmd([./version.sh|tr -d '\n'])],fix@fix8.org)
AM_INIT_AUTOMAKE
AH_TOP([//-----------------------------------------------------------------------------------------
#if 0
fix8 is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of
	conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
	of conditions and the following disclaimer in the documentation and/or other
	materials provided with the distribution.
 * Neither the name of the author nor the names of its contributors may be used to
	endorse or promote products derived from this software without specific prior
	written permission.
 * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
   in their name without written permission from fix8.org

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

BRAND_SUMMARY='Fix8 FIX Framework Server'
BRAND_DESCRIPTION='Modern fast FIX Framework'
BRAND_COMPANY_SHORTNAME='DD'
BRAND_COMPANY_LONGNAME='David L. Dight'

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

if test -z "$DOXYGEN";
	then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
AM_CONDITIONAL([USEDOXYGEN],
	[test -n "$DOXYGEN"])AM_COND_IF([USEDOXYGEN],[AC_CONFIG_FILES([doc/Makefile])])

AC_CHECK_FUNCS([scandir getopt_long sysconf getaddrinfo getnameinfo \
	daemon popen waitpid setlogmask alarm dup2 ftruncate \
	getcwd gettimeofday localtime_r pow regcomp socket strcasecmp strchr strdup strerror \
	strncasecmp strspn strtol strtoul random srandom
	])

AC_CHECK_HEADERS([getopt.h fcntl.h netdb.h regex.h signal.h string.h sys/time.h \
	arpa/inet.h sys/stat.h sys/types.h select.h sys/event.h h netinet/tcp.h \
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

AC_ARG_WITH(poco,
  AC_HELP_STRING([--with-poco=DIR],[location of Poco]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}"
	LDFLAGS="$LDFLAGS -L${withval}"
],[])

AC_ARG_WITH(tbb,
  AC_HELP_STRING([--with-tbb=DIR],[location of TBB]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}"
	LDFLAGS="$LDFLAGS -L${withval}"
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
		 echo "ERROR: TBB development libraries are required. Install and try again."
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
		 echo "ERROR: Poco development libraries are required. Install and try again."
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

have_bdb=no
AX_PATH_BDB([3],[ LIBS="$BDB_LIBS $LIBS"
LDFLAGS="$BDB_LDFLAGS $LDFLAGS"
CPPFLAGS="$CPPFLAGS $BDB_CPPFLAGS"
have_bdb=yes
AC_DEFINE(HAVE_BDB, 1, [Define to 1 if you have berkeley DB])
	])

if test "$have_bdb" = "no"; then
		echo
		echo "ERROR: Berkeley DB development libraries are required. Install and try again."
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
	yes) AC_DEFINE(MSGRECYCLING, 1, [Define to 1 for Message Recycling]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-msgrecycle) ;;
esac])

AC_ARG_ENABLE([customfields],
[AC_HELP_STRING([--enable-customfields],[enable User defined fields (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(PERMIT_CUSTOM_FIELDS, 1, [Define to 1 for User Defined Fields]) ;;
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
AC_SUBST(MIMEFILE)
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

AC_CONFIG_FILES([Makefile runtime/Makefile compiler/Makefile test/Makefile])
AC_CONFIG_HEADERS(include/f8config.h)
AC_OUTPUT

AC_SUBST(CONFIG_SHELL)
AM_CONDITIONAL(HAVE_OPENSSL, test "x$openssl" = "xyes")

