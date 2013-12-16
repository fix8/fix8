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

if USEGTEST
test_programs = message_test fileLogger_test filePersister_test session_test
noinst_PROGRAMS = $(test_programs)
lib_LTLIBRARIES = libutest.la
AM_CPPFLAGS = -I$(top_srcdir)/include
pkginclude_HEADERS = f8headers.hpp

TESTS = $(test_programs)

message_test_SOURCES = message_test.cpp
fileLogger_test_SOURCES = fileLogger_test.cpp
filePersister_test_SOURCES = filePersister_test.cpp
session_test_SOURCES = mockConnection.hpp mockConnection.cpp session_test.cpp

libutest_la_SOURCES = utest_types.hpp utest_types.cpp utest_traits.cpp \
							 utest_router.hpp utest_classes.hpp utest_classes.cpp

CLEANFILES = $(libutest_la_SOURCES)

.NOTPARALLEL: $(XML_42_UTEST_SCHEMA)

XML_42_UTEST_SCHEMA = $(top_srcdir)/schema/FIX42UTEST.xml

$(libutest_la_SOURCES): $(XML_42_UTEST_SCHEMA)
	$(top_srcdir)/compiler/f8c -sVp utest -n UTEST $(XML_42_UTEST_SCHEMA)

AM_LDFLAGS = -lPocoFoundation -lPocoNet -lPocoUtil -lgtest -lgtest_main -lpthread

if BDBSUPPORT
AM_LDFLAGS += -ldb_cxx
endif
if USECOMPRESSION
AM_LDFLAGS += -lz
endif
if TBBSUPPORT
AM_LDFLAGS += -ltbb
endif
if USESSL
AM_LDFLAGS += -lPocoNetSSL -lPocoCrypto
endif

built = $(top_srcdir)/runtime

required_objs = -lutest $(built)/logger.lo $(built)/f8utils.lo $(built)/gzstream.lo \
					 $(built)/message.lo $(built)/modp_numtoa.lo
persist_objs = $(built)/configuration.lo $(built)/xml.lo $(built)/persist.lo $(built)/filepersist.lo
session_objs = $(built)/connection.lo $(built)/session.lo

message_test_LDADD = $(required_objs) $(persist_objs)
fileLogger_test_LDADD = $(required_objs)
filePersister_test_LDADD = $(required_objs) $(persist_objs) $(session_objs)
session_test_LDADD = $(required_objs) $(persist_objs)

endif

