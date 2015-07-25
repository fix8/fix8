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
lib_LTLIBRARIES = libfix8.la
inc = $(top_srcdir)/include
AM_CPPFLAGS = -I$(inc)

libfix8_la_SOURCES = xml.cpp f8utils.cpp message.cpp traits.cpp \
                     session.cpp logger.cpp persist.cpp \
                     connection.cpp configuration.cpp \
                     consolemenu.cpp filepersist.cpp modp_numtoa.c precomp.cpp

libfix8_la_LDFLAGS = -version-info $(libfix8_la_version_info)

libfix8_la_LDFLAGS += -ggdb -shared -lpthread
if !USEOSX
libfix8_la_LDFLAGS += -rdynamic
endif

if USECOMPRESSION
libfix8_la_SOURCES += gzstream.cpp
libfix8_la_LDFLAGS += -lz
endif
libfix8_la_LDFLAGS += -lz

if USECCRYPT
libfix8_la_LDFLAGS += -lcrypt
endif
POCO_LIBS = -lPocoFoundation -lPocoNet -lPocoUtil
if USESSL
POCO_LIBS += -lPocoNetSSL -lPocoCrypto -lPocoXML
endif
libfix8_la_LDFLAGS += $(POCO_LIBS)
if TBBSUPPORT
libfix8_la_LDFLAGS += -ltbb
endif

if LIBMEMCACHEDSUPPORT
libfix8_la_SOURCES += memcachedpersist.cpp
libfix8_la_LDFLAGS += -lmemcached
endif
if LIBHIREDISSUPPORT
libfix8_la_SOURCES += redispersist.cpp
libfix8_la_LDFLAGS += -lhiredis
endif

