# Process this file with autoconf to produce a configure script.
AC_COPYRIGHT([Orbweb is released under the New BSD License.
Copyright (c) 2010, David L. Dight <fix@fix8.org>
All rights reserved.])

# $Id: configure.ac 496 2010-01-04 10:01:54Z davidd $
# $LastChangedDate: 2010-01-04 21:01:54 +1100 (Mon, 04 Jan 2010) $
AC_REVISION([$Rev: 496 $])
# $URL: svn://catfarm.electro.mine.nu/usr/local/repos/mongod/trunk/configure.ac $

AC_PREREQ(2.61)
AC_INIT([fix8], [m4_esyscmd([./version.sh | tr -d '\n'])],fix@fix8.org)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER(config.h)
AH_TOP([//-----------------------------------------------------------------------------------------
#if 0
fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <fix@fix8.org>
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
PATCH_VERSION_NUM=`[echo $VERSION | sed 's/[^0-9]/ /g' | awk '{printf "%d",$3}']`
AC_DEFINE_UNQUOTED([PATCH_VERSION_NUM], [$PATCH_VERSION_NUM], [Patch version number])
BUILD_VERSION_NUM=`[echo $VERSION | sed 's/[^0-9]/ /g' | awk '{printf "%d",$4}']`
AC_DEFINE_UNQUOTED([BUILD_VERSION_NUM], [$BUILD_VERSION_NUM], [Build number])
AC_DEFINE_UNQUOTED([MAGIC_NUM], [(MAJOR_VERSION_NUM << 29 | MINOR_VERSION_NUM << 23 | PATCH_VERSION_NUM << 16 | BUILD_VERSION_NUM)], [Encoded Version])
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

AC_CHECK_FUNCS([scandir getopt_long sysconf getaddrinfo getnameinfo \
	madvise posix_fadvise writev daemon popen waitpid setlogmask alarm dup2 ftruncate \
	getcwd gettimeofday localtime_r munmap pow regcomp socket strcasecmp strchr strdup strerror \
	strncasecmp strspn strtol strtoul random srandom
	])

AC_CHECK_HEADERS([getopt.h fcntl.h netdb.h regex.h signal.h string.h sys/time.h \
	arpa/inet.h sys/stat.h sys/types.h select.h sys/event.h h \
	netinet/tcp.h \
	sys/wait.h netinet/in.h getopt.h limits.h sys/sendfile.h sys/ioctl.h unistd.h \
	sys/fs/s5param.h sys/socket.h time.h syslog.h termios.h alloca.h
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

DEVURANDOM="/dev/urandom"
AC_CHECK_FILE($DEVURANDOM,
	[
		AC_DEFINE(HAVE_DEV_URANDOM, 1, [Define to 1 if the /dev/urandom device is available])
	])

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

AC_MSG_CHECKING(for valgrind)
AC_ARG_WITH(valgrind, AC_HELP_STRING([--with-valgrind],
	[enable internal support for valgrind (default=no)]), [HAVE_VALGRIND=$withval],[HAVE_VALGRIND=no])
AC_MSG_RESULT([$HAVE_VALGRIND])
if test "$HAVE_VALGRIND" != "no"; then
	AC_CHECK_HEADERS([valgrind/valgrind.h])
fi

AC_MSG_CHECKING(for gprof)
AC_ARG_WITH(gprof, AC_HELP_STRING([--with-gprof],[enable internal support for gprof (default=no)]),
	[HAVE_GPROF=$withval],[HAVE_GPROF=no])
AC_MSG_RESULT([$HAVE_GPROF])
if test "$HAVE_GPROF" != "no"; then
	AC_CHECK_HEADERS([sys/gmon.h],[ have_gprof_headers=yes ])
	if test "$have_gprof_headers" = yes; then
	    AC_DEFINE(PROFILING_BUILD, 1,
			[Define to 1 if your os supports gprof and you wish to enable profiling])
	else
	    AC_MSG_RESULT(no)
	fi
fi

AC_MSG_CHECKING(for TCP_QUICKACK support)
AC_ARG_WITH(tcpquickack, AC_HELP_STRING([--with-tcpquickack],
	[enable support for TCP_QUICKACK (default=yes)]),
	[have_tcp_quickack=$withval],[have_tcp_quickack=yes])
if test "$have_tcp_quickack" != "no"; then
	AC_TRY_COMPILE([
	#include <netinet/tcp.h>
		],[int a = TCP_QUICKACK; ],
		have_tcp_quickack=yes, have_tcp_quickack=no)
	if test "$have_tcp_quickack" = yes; then
	    AC_DEFINE(HAS_TCP_QUICKACK, 1, [Define to 1 if your os supports TCP_QUICKACK (default=yes)])
	    AC_MSG_RESULT(yes)
	else
	    AC_MSG_RESULT(no)
	fi
else
	 AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(for TCP_DEFER_ACCEPT support)
AC_ARG_WITH(tcpdeferaccept, AC_HELP_STRING([--with-tcpdeferaccept],
	[enable support for TCP_DEFER_ACCEPT (default=yes)]),
	[have_tcp_defer=$withval],[have_tcp_defer=yes])
if test "$have_tcp_defer" != "no"; then
	AC_TRY_COMPILE([
	#include <netinet/tcp.h>
		],[int a = TCP_DEFER_ACCEPT; ],
		have_tcp_defer=yes, have_tcp_defer=no)
	if test "$have_tcp_defer" = yes; then
	    AC_DEFINE(HAS_TCP_DEFER_ACCEPT, 1,
			[Define to 1 if your os supports TCP_DEFER_ACCEPT (default=yes)])
	    AC_MSG_RESULT(yes)
	else
	    AC_MSG_RESULT(no)
	fi
else
	 AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(for TCP_NODELAY support)
	AC_TRY_COMPILE([
	#include <netinet/tcp.h>
		],[int a = TCP_NODELAY; ],
		have_tcpnodelay=yes, have_tcpnodelay=no)
	if test "$have_tcpnodelay" = yes; then
	    AC_DEFINE(HAS_TCP_NODELAY, 1, [Define to 1 if your os supports TCP_NODELAY (default=yes)])
	    AC_MSG_RESULT(yes)
	else
	    AC_MSG_RESULT(no)
	fi

AC_MSG_CHECKING(for TCP_CORK support (Linux))
	AC_TRY_COMPILE([
	#include <netinet/tcp.h>
		],[int a = TCP_CORK; ],
		have_tcpcork=yes, have_tcpcork=no)
	if test "$have_tcpcork" = yes; then
	    AC_DEFINE(HAS_TCP_CORK, 1, [Define to 1 if your os supports TCP_CORK (default=yes)])
	    AC_MSG_RESULT(yes)
	else
	    AC_MSG_RESULT(no)
	fi

AC_MSG_CHECKING(for SO_ACCEPTFILTER support (BSD))
AC_ARG_WITH(acceptfilter, AC_HELP_STRING([--with-acceptfilter],
	[enable support for SO_ACCEPTFILTER (default=no)]),
	[have_acceptfilter=$withval],[have_acceptfilter=yes])
if test "$have_acceptfilter" != "no"; then
	AC_TRY_COMPILE([
	#include <netinet/tcp.h>
		],[int a = SO_ACCEPTFILTER; ],
		have_acceptfilter=yes, have_acceptfilter=no)
	if test "$have_acceptfilter" = yes; then
	    AC_DEFINE(HAS_SO_ACCEPTFILTER, 1, [Define to 1 if your os supports SO_ACCEPTFILTER (default=no)])
	    AC_MSG_RESULT(yes)
	else
	    AC_MSG_RESULT(no)
	fi
else
	 AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(for FIONREAD support)
	AC_TRY_COMPILE([
	#include <sys/ioctl.h>
		],[int a = FIONREAD; ],
		have_fionread=yes, have_fionread=no)
	if test "$have_fionread" = yes; then
	    AC_DEFINE(HAS_FIONREAD, 1, [Define to 1 if your os supports ioctl FIONREAD])
	    AC_MSG_RESULT(yes)
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

AC_ARG_ENABLE([bdb],
[AC_HELP_STRING([--enable-bdb],[Enable BDB support (default=no) SDB enabled by default])],
[case "${enableval}" in
	yes)
	AX_PATH_BDB([3],[ LIBS="$BDB_LIBS $LIBS"
	LDFLAGS="$BDB_LDFLAGS $LDFLAGS"
	CPPFLAGS="$CPPFLAGS $BDB_CPPFLAGS"
	AC_DEFINE(HAVE_BDB, 1, [Define to 1 if you have berkeley DB])
	]);;
	no) bdb=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-bdb) ;;
esac],[bdb=no])

# Check for timersub macro in sys/time.h
AC_CACHE_CHECK([for timersub macro in sys/time.h], ac_cv_timersubmacro,
		[AC_TRY_COMPILE([#include <sys/time.h>],
[struct timeval a, b, c; timersub(&a, &b, &c);],
			  timersubmacro=yes, timersubmacro=no)])
if test "$timersubmacro" = "yes"; then
    AC_DEFINE(HAVE_TIMERSUB, 1, [Define to 1 if timersub macro is in sys/time.h])
fi

have_dlopen=no
AC_CHECK_LIB(dl,dlopen,have_dlopen=yes)
if test "$have_dlopen" = "no"; then
		echo
		echo "ERROR: dlopen (libdl) not found. Install and try again."
		echo
		exit 1
fi

AC_MSG_CHECKING(for XMLENTITY_STRICT_BOOL specification)
AC_ARG_WITH(xmlstrictbool, AC_HELP_STRING([--with-xmlstrictbool],
	[enable XMLENTITY_STRICT_BOOL (default=no)]),
	[have_xmlstrictbool=$withval],[have_xmlstrictbool=no])
if test "$have_xmlstrictbool" != "no"; then
	 AC_DEFINE(XMLENTITY_STRICT_BOOL, 1,
		[Define to 1 if you wish to enforce XMLENTITY_STRICT_BOOL (default=no)])
	 AC_MSG_RESULT(yes)
else
	 AC_MSG_RESULT(no)
fi

AC_CHECK_LIB(curses, initscr, [
	AC_CHECK_HEADERS([ncurses.h],
	[
		AC_DEFINE([HAVE_NCURSES], [1], [Define to 1 if libcurses is present])
		AC_DEFINE([HAVE_NCURSES_H], [1], [Define to 1 if ncurses.h is present])
		havecurses=yes
	])
])
if test "x$havecurses" != "xyes"; then
	echo
	echo "ERROR: ncurses dev headers or libraries not found. Install and try again"
	echo
	exit 1
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

m4_include(m4/sendfile.m4)

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

AC_ARG_ENABLE([debug],
[AC_HELP_STRING([--enable-debug],[Enable DEBUG support (default=no)])],
[case "${enableval}" in
	yes) AC_DEFINE(DEBUG, 1, [Define to 1 for debugging extensions]) ;;
	no)  ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac])

AC_ARG_ENABLE([ssl],
[AC_HELP_STRING([--enable-ssl],[Enable SSL support (through OpenSSL, default=no)])],
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
AC_SUBST(BUILD_VERSION_NUM)
AC_SUBST(BRAND_SUMMARY)
AC_SUBST(BRAND_DESCRIPTION)
AC_SUBST(BRAND_COMPANY_SHORTNAME)
AC_SUBST(BRAND_COMPANY_LONGNAME)

AC_CONFIG_FILES([Makefile runtime/Makefile compiler/Makefile test/Makefile])
AC_OUTPUT
#AC_OUTPUT([orbweb.spec base/orbweb.sh base/orbweb.xml \
#	doc/man/b64pw.8.xml doc/man/procmon.8.xml doc/man/md5pw.8.xml \
#	doc/man/orbweb.xml.8.xml doc/man/orbweb.8.xml doc/man/txt2db.8.xml])
AC_SUBST(CONFIG_SHELL)
AM_CONDITIONAL(HAVE_OPENSSL, test "x$openssl" = "xyes")

