#############################################################################################
# Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.
#
# Fix8 Open Source FIX Engine.
# Copyright (C) 2010-16 David L. Dight <fix@fix8.org>
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
lib_LTLIBRARIES = libFIX40.la libFIX41.la libFIX42.la libFIX43.la libFIX44.la libFIX50.la libFIX50SP1.la libFIX50SP2.la
libFIX40_la_SOURCES = FIX40_classes.cpp FIX40_types.hpp FIX40_types.cpp FIX40_traits.cpp \
                      FIX40_router.hpp FIX40_classes.hpp
libFIX41_la_SOURCES = FIX41_classes.cpp FIX41_types.hpp FIX41_types.cpp FIX41_traits.cpp \
                      FIX41_router.hpp FIX41_classes.hpp
libFIX42_la_SOURCES = FIX42_classes.cpp FIX42_types.hpp FIX42_types.cpp FIX42_traits.cpp \
                      FIX42_router.hpp FIX42_classes.hpp
libFIX43_la_SOURCES = FIX43_classes.cpp FIX43_types.hpp FIX43_types.cpp FIX43_traits.cpp \
                      FIX43_router.hpp FIX43_classes.hpp
libFIX44_la_SOURCES = FIX44_classes.cpp FIX44_types.hpp FIX44_types.cpp FIX44_traits.cpp \
                      FIX44_router.hpp FIX44_classes.hpp
libFIX50_la_SOURCES = FIX50_classes.cpp FIX50_types.hpp FIX50_types.cpp FIX50_traits.cpp \
                      FIX50_router.hpp FIX50_classes.hpp
libFIX50SP1_la_SOURCES = FIX50SP1_classes.cpp FIX50SP1_types.hpp FIX50SP1_types.cpp FIX50SP1_traits.cpp \
                      FIX50SP1_router.hpp FIX50SP1_classes.hpp
libFIX50SP2_la_SOURCES = FIX50SP2_classes.cpp FIX50SP2_types.hpp FIX50SP2_types.cpp FIX50SP2_traits.cpp \
                      FIX50SP2_router.hpp FIX50SP2_classes.hpp

ALLSOURCES = $(libFIX40_la_SOURCES) $(libFIX41_la_SOURCES) $(libFIX42_la_SOURCES) $(libFIX43_la_SOURCES) \
				 $(libFIX44_la_SOURCES) $(libFIX50_la_SOURCES) $(libFIX50SP1_la_SOURCES) $(libFIX50SP2_la_SOURCES)

FIX40_XML_SCHEMA = $(top_srcdir)/schema/FIX40.xml
FIX41_XML_SCHEMA = $(top_srcdir)/schema/FIX41.xml
FIX42_XML_SCHEMA = $(top_srcdir)/schema/FIX42.xml
FIX43_XML_SCHEMA = $(top_srcdir)/schema/FIX43.xml
FIX44_XML_SCHEMA = $(top_srcdir)/schema/FIX44.xml
FIX50_XML_SCHEMA = $(top_srcdir)/schema/FIX50.xml
FIX50SP1_XML_SCHEMA = $(top_srcdir)/schema/FIX50SP1.xml
FIX50SP2_XML_SCHEMA = $(top_srcdir)/schema/FIX50SP2.xml
FIXT11_XML_SCHEMA = $(top_srcdir)/schema/FIXT11.xml

ALLSCHEMA = $(FIX40_XML_SCHEMA) $(FIX41_XML_SCHEMA) $(FIX42_XML_SCHEMA) $(FIX43_XML_SCHEMA) $(FIX44_XML_SCHEMA) \
				$(FIX50_XML_SCHEMA) $(FIX50SP1_XML_SCHEMA) $(FIX50SP2_XML_SCHEMA)

AM_CXXFLAGS = -I$(top_srcdir)/include -fno-var-tracking -fno-var-tracking-assignments

BUILT_SOURCES = $(ALLSOURCES)
CLEANFILES = $(ALLSOURCES)
.NOTPARALLEL: $(ALLSCHEMA)

$(libFIX40_la_SOURCES): $(FIX40_XML_SCHEMA)
	$(top_srcdir)/compiler/f8c --verbose --namespace FIX40 $(FIX40_XML_SCHEMA) -p FIX40
$(libFIX41_la_SOURCES): $(FIX41_XML_SCHEMA)
	$(top_srcdir)/compiler/f8c --verbose --namespace FIX41 $(FIX41_XML_SCHEMA) -p FIX41
$(libFIX42_la_SOURCES): $(FIX42_XML_SCHEMA)
	$(top_srcdir)/compiler/f8c --verbose --namespace FIX42 $(FIX42_XML_SCHEMA) -p FIX42
$(libFIX43_la_SOURCES): $(FIX43_XML_SCHEMA)
	$(top_srcdir)/compiler/f8c --verbose --namespace FIX43 $(FIX43_XML_SCHEMA) -p FIX43
$(libFIX44_la_SOURCES): $(FIX44_XML_SCHEMA)
	$(top_srcdir)/compiler/f8c --verbose --namespace FIX44 $(FIX44_XML_SCHEMA) -p FIX44
$(libFIX50_la_SOURCES): $(FIX50_XML_SCHEMA) $(FIXT11_XML_SCHEMA)
	$(top_srcdir)/compiler/f8c --verbose --namespace FIX50 $(FIX50_XML_SCHEMA) -p FIX50 --fixt $(FIXT11_XML_SCHEMA)
$(libFIX50SP1_la_SOURCES): $(FIX50SP1_XML_SCHEMA) $(FIXT11_XML_SCHEMA)
	$(top_srcdir)/compiler/f8c --verbose --namespace FIX50SP1 $(FIX50SP1_XML_SCHEMA) -p FIX50SP1 --fixt $(FIXT11_XML_SCHEMA)
$(libFIX50SP2_la_SOURCES): $(FIX50SP2_XML_SCHEMA) $(FIXT11_XML_SCHEMA)
	$(top_srcdir)/compiler/f8c --verbose --namespace FIX50SP2 $(FIX50SP2_XML_SCHEMA) -p FIX50SP2 --fixt $(FIXT11_XML_SCHEMA)

FIX40:
	make libFIX40.la
FIX41:
	make libFIX41.la
FIX42:
	make libFIX42.la
FIX43:
	make libFIX43.la
FIX44:
	make libFIX44.la
FIX50:
	make libFIX50.la
FIX50SP1:
	make libFIX50SP1.la
FIX50SP2:
	make libFIX50SP2.la

