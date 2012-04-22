#############################################################################################
# Fix8 is released under the New BSD License.
#
# Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
#
#   * Redistributions of source code must retain the above copyright notice, this list of
#	 	conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright notice, this list
#	 	of conditions and the following disclaimer in the documentation and/or other
#		materials provided with the distribution.
#   * Neither the name of the author nor the names of its contributors may be used to
#	 	endorse or promote products derived from this software without specific prior
#		written permission.
#   * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
#	   in their name without written permission from fix8.org
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
# OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
# MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
# THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
# SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
# OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#############################################################################################
bin_PROGRAMS = f8test f8print
lib_LTLIBRARIES = libmyfix.la
INCLUDES = -I$(top_srcdir)/include
libmyfix_la_SOURCES = Myfix_types.hpp Myfix_types.cpp Myfix_traits.cpp \
					 		 Myfix_router.hpp Myfix_classes.hpp Myfix_classes.cpp
f8test_SOURCES = myfix.cpp
f8print_SOURCES = myprint.cpp
XML_SCHEMA = $(top_srcdir)/schema/FIX50SP2.xml
#XML_SCHEMA = $(top_srcdir)/schema/FIX44.xml
#XML_SCHEMA = FIX44TEST.xml
CLEANFILES = $(libmyfix_la_SOURCES)

$(libmyfix_la_SOURCES): $(XML_SCHEMA)
	$(top_srcdir)/compiler/f8c -rn TEX $(XML_SCHEMA) -x $(top_srcdir)/schema/FIXT11.xml

.NOTPARALLEL: $(XML_SCHEMA)
POCO_LIBS = -lPocoFoundation -lPocoNet -lPocoUtil
GEN_LIBS = -lrt -lfix8 -lmyfix
TBB_LIBS = -ltbb
BDB_LIBS = -ldb_cxx
ALL_LIBS = $(GEN_LIBS) $(POCO_LIBS) $(TBB_LIBS) $(BDB_LIBS) -L$(top_srcdir)/runtime

f8test_LDFLAGS = -rdynamic $(ALL_LIBS)
f8print_LDFLAGS = -rdynamic $(ALL_LIBS)

if USECOMPRESSION
f8test_LDFLAGS += -lz
f8print_LDFLAGS += -lz
endif
