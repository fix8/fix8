#############################################################################################
# Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.
#
# Fix8 Open Source FIX Engine.
# Copyright (C) 2010-15 David L. Dight <fix@fix8.org>
#
# Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
# GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
# version 3 of the License, or (at your option) any later version.
#
# Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
# even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
# If not, see <http://www.gnu.org/licenses/>.
#
# BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
# THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
# COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
# KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
# WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
# THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
# YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
#
# IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
# HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
# ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
# CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
# NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
# THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
# HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
#
#############################################################################################
AUTOMAKE_OPTIONS = foreign dist-bzip2 dist-zip

SUBDIRS = runtime compiler test util

if USEGTEST
SUBDIRS += utests
endif

if USEDOXYGEN
SUBDIRS += doc
endif

#############################################################################################
ACLOCAL_AMFLAGS = -I m4

$(distdir).tar.gz:
	make dist

pkg: $(distdir).tar.gz $(PACKAGE).spec
	$(PACKAGER)

src-release:
	make dist
	md5sum $(distdir).tar.gz > $(distdir).md5sum
	cat $(distdir).md5sum

check-release:
	md5sum -c $(distdir).md5sum

prebuilt:
	cd stocklib; $(MAKE)

prebuilt-install:
	cd stocklib; $(MAKE) install

clean-local:
	cd stocklib; $(MAKE) clean

pkgincludedir = ${prefix}/include/fix8
ffincdir = include/fix8/ff
ffincludes = ${ffincdir}/*.hpp ${ffincdir}/*.h ${ffincdir}/d/*.hpp ${ffincdir}/platforms/*.h ${ffincdir}/atomic/*.h \
								  ${ffincdir}/ocl/*.hpp ${ffincdir}/ocl/*.h
pkginclude_HEADERS = include/fix8/*.hpp include/fix8/f8config.h include/fix8/f8dll.h

ffdir = ${prefix}/include/fix8/ff
sharedir = ${prefix}/share/fix8
mandir = doc/man
fix8mandir = ${sharedir}/man/man1
testdir = ${sharedir}/test
fix8htmldir = $(sharedir)/html
schemadir = ${sharedir}/schema
contribdir = ${sharedir}/contrib
exchangecontribdir = ${contribdir}/exchangeSchemas

xmltoman=$(mandir)/xmltoman
PROCESS_MAN_PAGES = $(mandir)/f8c.1 $(mandir)/seqedit.1

INSTALL_DATA_CONTRIB = contrib/FindFix8.cmake
INSTALL_DATA_EXCHANGE_CONTRIB = contrib/exchangeSchemas/*

INSTALL_DATA_LOCAL_TEST = test/hf_client.xml test/myfix_client.xml test/hf_client_include.xml \
	test/hf_server.xml test/myfix_server.xml

INSTALL_DATA_HTML_DOC = doc/html/*.js doc/html/*.html doc/html/*.png doc/html/*.css \
	doc/fix8.doxygen doc/mainpage.doxygen doc/fix8.footer.html

INSTALL_DATA_HTML_SEARCH_DOC = doc/html/search/*.js doc/html/search/*.html doc/html/search/*.png \
	doc/html/search/*.css

INSTALL_DATA_LOCAL_SCHEMA = schema/*.xml schema/QUICKFIX-LICENCE

install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(testdir) $(DESTDIR)$(testdir)/run $(DESTDIR)$(schemadir)
	@$(mkinstalldirs) $(DESTDIR)$(ffdir)/d $(DESTDIR)$(ffdir)/platforms $(DESTDIR)$(ffdir)/atomic $(DESTDIR)$(ffdir)/ocl
	@$(mkinstalldirs) $(DESTDIR)$(fix8mandir) $(DESTDIR)$(contribdir) $(DESTDIR)$(exchangecontribdir)
	$(INSTALL_DATA) $(INSTALL_DATA_LOCAL_TEST) $(DESTDIR)$(testdir)
	$(INSTALL_DATA) $(INSTALL_DATA_LOCAL_SCHEMA) $(DESTDIR)$(schemadir)
	$(INSTALL_DATA) $(INSTALL_DATA_CONTRIB) $(DESTDIR)$(contribdir)
	$(INSTALL_DATA) $(INSTALL_DATA_EXCHANGE_CONTRIB) $(DESTDIR)$(exchangecontribdir)
	$(INSTALL_DATA) ${ffincdir}/*.hpp ${ffincdir}/*.h $(DESTDIR)$(ffdir)
	$(INSTALL_DATA) ${ffincdir}/d/*.hpp $(DESTDIR)$(ffdir)/d
	$(INSTALL_DATA) ${ffincdir}/ocl/*.hpp ${ffincdir}/ocl/*.h $(DESTDIR)$(ffdir)/ocl
	$(INSTALL_DATA) ${ffincdir}/platforms/*.h $(DESTDIR)$(ffdir)/platforms
	$(INSTALL_DATA) ${ffincdir}/atomic/*.h $(DESTDIR)$(ffdir)/atomic
	@for manfile in $(PROCESS_MAN_PAGES) ; do \
		$(xmltoman) $$manfile.xml | gzip > $$manfile.gz ; \
		$(INSTALL_DATA) $$manfile.gz $(DESTDIR)$(fix8mandir); \
	done
if USEDOXYGEN
	@$(mkinstalldirs) $(DESTDIR)$(fix8htmldir) $(DESTDIR)$(fix8htmldir)/search
	$(INSTALL_DATA) $(INSTALL_DATA_HTML_DOC) $(DESTDIR)$(fix8htmldir)
	$(INSTALL_DATA) $(INSTALL_DATA_HTML_SEARCH_DOC) $(DESTDIR)$(fix8htmldir)/search
endif

EXTRA_DIST = $(xmltoman) fix8.spec \
	$(INSTALL_DATA_LOCAL_SCHEMA) $(pkginclude_HEADERS) $(INSTALL_DATA_LOCAL_TEST) ${ffincludes} $(INSTALL_DATA_CONTRIB) $(INSTALL_DATA_EXCHANGE_CONTRIB) \
	version.sh bootstrap README.md LICENSE CONTRIBUTORS runtime/dllmain.cpp stocklib/Makefile.am stocklib/Makefile.in

if USEDOXYGEN
EXTRA_DIST += doc/fix8_Logo_RGB_small.png doc/fix8.footer.html doc/mainpage.doxygen
endif
EXTRA_DIST += utests/precomp.hpp runtime/precomp.hpp util/precomp.hpp compiler/precomp.hpp

