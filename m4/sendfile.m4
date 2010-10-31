dnl
dnl This m4 macro has been used from the current release of samba
dnl
dnl Unix SMB/Netbios implementation.
dnl Version 2.2.x / 3.0.x
dnl sendfile implementations.
dnl Copyright (C) Jeremy Allison 2002.
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl
dnl $Id: sendfile.m4 475 2008-12-25 01:52:13Z davidd $
dnl $Date: 2008-12-25 12:52:13 +1100 (Thu, 25 Dec 2008) $

#################################################
# check for sendfile support

with_sendfile_support=yes
AC_MSG_CHECKING(whether to check to support sendfile)
AC_ARG_WITH(sendfile-support,
[  --with-sendfile-support Check for sendfile support (default=yes)],
[ case "$withval" in
  yes)

	AC_MSG_RESULT(yes);

	case "$host_os" in
	*linux*)
		AC_CACHE_CHECK([for linux sendfile64 support],samba_cv_HAVE_SENDFILE64,[
		AC_TRY_LINK([#include <sys/sendfile.h>],
[\
int tofd, fromfd;
off64_t offset;
size_t total;
ssize_t nwritten = sendfile64(tofd, fromfd, &offset, total);
],
samba_cv_HAVE_SENDFILE64=yes,samba_cv_HAVE_SENDFILE64=no)])

		AC_CACHE_CHECK([for linux sendfile support],samba_cv_HAVE_SENDFILE,[
		AC_TRY_LINK([#include <sys/sendfile.h>],
[\
int tofd, fromfd;
off_t offset;
size_t total;
ssize_t nwritten = sendfile(tofd, fromfd, &offset, total);
],
samba_cv_HAVE_SENDFILE=yes,samba_cv_HAVE_SENDFILE=no)])

# Try and cope with broken Linux sendfile....
		AC_CACHE_CHECK([for broken linux sendfile support],samba_cv_HAVE_BROKEN_LINUX_SENDFILE,[
		AC_TRY_LINK([\
#if defined(_FILE_OFFSET_BITS) && (_FILE_OFFSET_BITS == 64)
#undef _FILE_OFFSET_BITS
#endif
#include <sys/sendfile.h>],
[\
int tofd, fromfd;
off_t offset;
size_t total;
ssize_t nwritten = sendfile(tofd, fromfd, &offset, total);
],
samba_cv_HAVE_BROKEN_LINUX_SENDFILE=yes,samba_cv_HAVE_BROKEN_LINUX_SENDFILE=no)])

	if test x"$samba_cv_HAVE_SENDFILE64" = x"yes"; then
    		AC_DEFINE(HAVE_SENDFILE64,1,[Whether 64-bit sendfile() is available])
		AC_DEFINE(LINUX_SENDFILE_API,1,[Whether linux sendfile() API is available])
		AC_DEFINE(WITH_SENDFILE,1,[Whether sendfile() should be used])
	elif test x"$samba_cv_HAVE_SENDFILE" = x"yes"; then
    		AC_DEFINE(HAVE_SENDFILE,1,[Whether sendfile() is available])
		AC_DEFINE(LINUX_SENDFILE_API,1,[Whether linux sendfile() API is available])
		AC_DEFINE(WITH_SENDFILE,1,[Whether sendfile() should be used])
	elif test x"$samba_cv_HAVE_BROKEN_LINUX_SENDFILE" = x"yes"; then
		AC_DEFINE(LINUX_BROKEN_SENDFILE_API,1,[Whether (linux) sendfile() is broken])
		AC_DEFINE(WITH_SENDFILE,1,[Whether sendfile should be used])
	else
		AC_MSG_RESULT(no);
	fi

	;;
	*freebsd* | *dragonfly* )
		AC_CACHE_CHECK([for freebsd sendfile support],samba_cv_HAVE_SENDFILE,[
		AC_TRY_LINK([\
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>],
[\
	int fromfd, tofd, ret, total=0;
	off_t offset, nwritten;
	struct sf_hdtr hdr;
	struct iovec hdtrl;
	hdr.headers = &hdtrl;
	hdr.hdr_cnt = 1;
	hdr.trailers = NULL;
	hdr.trl_cnt = 0;
	hdtrl.iov_base = NULL;
	hdtrl.iov_len = 0;
	ret = sendfile(fromfd, tofd, offset, total, &hdr, &nwritten, 0);
],
samba_cv_HAVE_SENDFILE=yes,samba_cv_HAVE_SENDFILE=no)])

	if test x"$samba_cv_HAVE_SENDFILE" = x"yes"; then
    		AC_DEFINE(HAVE_SENDFILE,1,[Whether sendfile() support is available])
		AC_DEFINE(FREEBSD_SENDFILE_API,1,[Whether the FreeBSD sendfile() API is available])
		AC_DEFINE(WITH_SENDFILE,1,[Whether sendfile() support should be included])
	else
		AC_MSG_RESULT(no);
	fi
	;;

	*hpux*)
		AC_CACHE_CHECK([for hpux sendfile64 support],samba_cv_HAVE_SENDFILE64,[
		AC_TRY_LINK([\
#include <sys/socket.h>
#include <sys/uio.h>],
[\
	int fromfd, tofd;
	size_t total=0;
	struct iovec hdtrl[2];
	ssize_t nwritten;
	off64_t offset;

	hdtrl[0].iov_base = 0;
	hdtrl[0].iov_len = 0;

	nwritten = sendfile64(tofd, fromfd, offset, total, &hdtrl[0], 0);
],
samba_cv_HAVE_SENDFILE64=yes,samba_cv_HAVE_SENDFILE64=no)])
	if test x"$samba_cv_HAVE_SENDFILE64" = x"yes"; then
    		AC_DEFINE(HAVE_SENDFILE64,1,[Whether sendfile64() is available])
		AC_DEFINE(HPUX_SENDFILE_API,1,[Whether the hpux sendfile() API is available])
		AC_DEFINE(WITH_SENDFILE,1,[Whether sendfile() support should be included])
	else
		AC_MSG_RESULT(no);
	fi

		AC_CACHE_CHECK([for hpux sendfile support],samba_cv_HAVE_SENDFILE,[
		AC_TRY_LINK([\
#include <sys/socket.h>
#include <sys/uio.h>],
[\
	int fromfd, tofd;
	size_t total=0;
	struct iovec hdtrl[2];
	ssize_t nwritten;
	off_t offset;

	hdtrl[0].iov_base = 0;
	hdtrl[0].iov_len = 0;

	nwritten = sendfile(tofd, fromfd, offset, total, &hdtrl[0], 0);
],
samba_cv_HAVE_SENDFILE=yes,samba_cv_HAVE_SENDFILE=no)])
	if test x"$samba_cv_HAVE_SENDFILE" = x"yes"; then
    		AC_DEFINE(HAVE_SENDFILE,1,[Whether sendfile() is available])
		AC_DEFINE(HPUX_SENDFILE_API,1,[Whether the hpux sendfile() API is available])
		AC_DEFINE(WITH_SENDFILE,1,[Whether sendfile() support should be included])
	else
		AC_MSG_RESULT(no);
	fi
	;;

	*solaris*)
		AC_CHECK_LIB(sendfile,sendfilev)
		AC_CACHE_CHECK([for solaris sendfilev64 support],samba_cv_HAVE_SENDFILEV64,[
		AC_TRY_LINK([\
#include <sys/sendfile.h>],
[\
        int sfvcnt;
        size_t xferred;
        struct sendfilevec vec[2];
	ssize_t nwritten;
	int tofd;

	sfvcnt = 2;

	vec[0].sfv_fd = SFV_FD_SELF;
	vec[0].sfv_flag = 0;
	vec[0].sfv_off = 0;
	vec[0].sfv_len = 0;

	vec[1].sfv_fd = 0;
	vec[1].sfv_flag = 0;
	vec[1].sfv_off = 0;
	vec[1].sfv_len = 0;
	nwritten = sendfilev64(tofd, vec, sfvcnt, &xferred);
],
samba_cv_HAVE_SENDFILEV64=yes,samba_cv_HAVE_SENDFILEV64=no)])

	if test x"$samba_cv_HAVE_SENDFILEV64" = x"yes"; then
    		AC_DEFINE(HAVE_SENDFILEV64,1,[Whether sendfilev64() is available])
		AC_DEFINE(SOLARIS_SENDFILE_API,1,[Whether the soloris sendfile() API is available])
		AC_DEFINE(WITH_SENDFILE,1,[Whether sendfile() support should be included])
	else
		AC_MSG_RESULT(no);
	fi

		AC_CACHE_CHECK([for solaris sendfilev support],samba_cv_HAVE_SENDFILEV,[
		AC_TRY_LINK([\
#include <sys/sendfile.h>],
[\
        int sfvcnt;
        size_t xferred;
        struct sendfilevec vec[2];
	ssize_t nwritten;
	int tofd;

	sfvcnt = 2;

	vec[0].sfv_fd = SFV_FD_SELF;
	vec[0].sfv_flag = 0;
	vec[0].sfv_off = 0;
	vec[0].sfv_len = 0;

	vec[1].sfv_fd = 0;
	vec[1].sfv_flag = 0;
	vec[1].sfv_off = 0;
	vec[1].sfv_len = 0;
	nwritten = sendfilev(tofd, vec, sfvcnt, &xferred);
],
samba_cv_HAVE_SENDFILEV=yes,samba_cv_HAVE_SENDFILEV=no)])

	if test x"$samba_cv_HAVE_SENDFILEV" = x"yes"; then
    		AC_DEFINE(HAVE_SENDFILEV,1,[Whether sendfilev() is available])
		AC_DEFINE(SOLARIS_SENDFILE_API,1,[Whether the solaris sendfile() API is available])
		AC_DEFINE(WITH_SENDFILE,1,[Whether to include sendfile() support])
	else
		AC_MSG_RESULT(no);
	fi
	;;
	*aix*)
		AC_CACHE_CHECK([for AIX send_file support],samba_cv_HAVE_SENDFILE,[
		AC_TRY_LINK([\
#include <sys/socket.h>],
[\
	int fromfd, tofd;
	size_t total=0;
	struct sf_parms hdtrl;
	ssize_t nwritten;
	off64_t offset;

	hdtrl.header_data = 0;
	hdtrl.header_length = 0;
	hdtrl.file_descriptor = fromfd;
	hdtrl.file_offset = 0;
	hdtrl.file_bytes = 0;
	hdtrl.trailer_data = 0;
	hdtrl.trailer_length = 0;

	nwritten = send_file(&tofd, &hdtrl, 0);
],
samba_cv_HAVE_SENDFILE=yes,samba_cv_HAVE_SENDFILE=no)])
	if test x"$samba_cv_HAVE_SENDFILE" = x"yes"; then
		AC_DEFINE(HAVE_SENDFILE,1,[Whether sendfile() is available])
		AC_DEFINE(AIX_SENDFILE_API,1,[Whether the AIX send_file() API is available])
		AC_DEFINE(WITH_SENDFILE,1,[Whether to include sendfile() support])
	else
		AC_MSG_RESULT(no);
	fi
	;;
	*)
	;;
        esac
        ;;
  *)
    AC_MSG_RESULT(no)
    ;;
  esac ],
  AC_MSG_RESULT(yes)
)


