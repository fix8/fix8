#############################################################################################
# Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.
#
# Fix8 Open Source FIX Engine.
# Copyright (C) 2010-13 David L. Dight <fix@fix8.org>
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
bin_PROGRAMS = hftest hfprint
lib_LTLIBRARIES = libhftest.la
if F8TEST
bin_PROGRAMS += f8test f8print harness
lib_LTLIBRARIES += libmyfix.la
endif
f8test_SOURCES = myfix.cpp myfix.hpp
f8print_SOURCES = myprint.cpp myfix.hpp
harness_SOURCES = harness.cpp myfix.hpp
hftest_SOURCES = hftest.cpp hftest.hpp
hfprint_SOURCES = hfprint.cpp hftest.hpp
libmyfix_la_SOURCES = Myfix_classes.cpp Myfix_types.hpp Myfix_types.cpp Myfix_traits.cpp \
					 		 Myfix_router.hpp Myfix_classes.hpp
libhftest_la_SOURCES = Perf_types.hpp Perf_types.cpp Perf_traits.cpp \
					 		 Perf_router.hpp Perf_classes.hpp Perf_classes.cpp

CLEANFILES = $(libmyfix_la_SOURCES) $(libhftest_la_SOURCES)
libmyfix_la_CXXFLAGS = -I$(top_srcdir)/include
libhftest_la_CXXFLAGS = -I$(top_srcdir)/include
AM_CXXFLAGS = -I$(top_srcdir)/include

XML_SCHEMA = $(top_srcdir)/schema/FIX50SP2.xml
XML_HF_SCHEMA = $(top_srcdir)/schema/FIX42PERF.xml
XML_FIXT_SCHEMA = $(top_srcdir)/schema/FIXT11.xml

.NOTPARALLEL: $(XML_SCHEMA) $(XML_HF_SCHEMA)

$(libhftest_la_SOURCES): $(XML_HF_SCHEMA)
	$(top_srcdir)/compiler/f8c -sVp Perf -R -n TEX $(XML_HF_SCHEMA)

$(libmyfix_la_SOURCES): $(XML_SCHEMA)
	$(top_srcdir)/compiler/f8c -rVn TEX $(XML_SCHEMA) -x $(XML_FIXT_SCHEMA)

POCO_LIBS = -lPocoFoundation -lPocoNet -lPocoUtil
GEN_LIBS = -lfix8 -lpthread
if LINK_RT
GEN_LIBS += -lrt
endif
if BDBSUPPORT
GEN_LIBS += -ldb_cxx
endif
if TBBSUPPORT
GEN_LIBS += -ltbb
endif
ALL_LIBS = $(GEN_LIBS) $(POCO_LIBS) -L$(top_srcdir)/runtime

if TOGGLETRACKINGASSIGNMENTS
libmyfix_la_CXXFLAGS += -fno-var-tracking -fno-var-tracking-assignments
endif
f8test_LDFLAGS = -rdynamic $(ALL_LIBS) -lmyfix
f8print_LDFLAGS = -rdynamic $(ALL_LIBS) -lmyfix
hftest_LDFLAGS = -rdynamic $(ALL_LIBS) -lhftest
hfprint_LDFLAGS = -rdynamic $(ALL_LIBS) -lhftest
harness_LDFLAGS = -rdynamic $(ALL_LIBS) -lmyfix

if USECOMPRESSION
f8test_LDFLAGS += -lz
f8print_LDFLAGS += -lz
hftest_LDFLAGS += -lz
hfprint_LDFLAGS += -lz
harness_LDFLAGS += -lz
endif
if USESSL
f8test_LDFLAGS += -lPocoNetSSL -lPocoCrypto
f8print_LDFLAGS += -lPocoNetSSL -lPocoCrypto
hftest_LDFLAGS += -lPocoNetSSL -lPocoCrypto
hfprint_LDFLAGS += -lPocoNetSSL -lPocoCrypto
harness_LDFLAGS += -lPocoNetSSL -lPocoCrypto
endif

