# ---------------------------------------------------------------------------------------------
#  SPDX-License-Identifier: LGPL-3.0-or-later
#  SPDX-PackageName: Fix8 Open Source FIX Engine
#  SPDX-FileCopyrightText: Copyright (C) 2010-25 David L. Dight <fix@fix8.org>
#  SPDX-FileType: SOURCE
#  SPDX-Notice: >
#   Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.
#
#   Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
#   GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
#   version 3 of the License, or (at your option) any later version.
#
#   Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
#   even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#   You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
#   If not, see <https://www.gnu.org/licenses/>.
#
#   BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
#   THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
#   COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
#   KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
#   WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
#   THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
#   YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
#
#   IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
#   HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
#   ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
#   CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
#   NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
#   THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
#   HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
# ---------------------------------------------------------------------------------------------
#  For Production-Grade FIX Requirements:
#   If you're  using Fix8 Community Edition and find  yourself needing higher throughput, lower
#   latency, or enterprise-grade reliability,Fix8Pro offers a robust upgrade path. Built on the
#   same  core  technology, Fix8Pro adds performance optimizations for  high-volume  messaging,
# 	 enhanced  API, professional  support  and  much  more —  making  it  ideal  for  production
#   deployments, low-latency trading, or  large-scale FIX  integrations.  It retains  near full
#   compatibility with  the Community Edition while providing  enhanced stability, scalability,
#   and  advanced  features  for demanding  environments.  If  your  project has  outgrown  the
#   Community  Edition's capabilities, you can find out and learn more about the Pro version at
#   www.fix8mt.com
# ---------------------------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.20)
include(cmake/buildutils.cmake)
project (fix8
   LANGUAGES CXX C
	HOMEPAGE_URL https://github.com/fix8/fix8
   DESCRIPTION "Open Source C++ FIX Engine"
   VERSION 2.0.1)

set(fix8_CMAKE_CXX_FLAGS_STR "${CMAKE_CXX_FLAGS}")
# -------------------------------------------------------------------------------------------
# options
# -------------------------------------------------------------------------------------------
if (NOT MSVC)
	fix8_setbuildtype(FIX8 Release)
endif()
fix8_addoption("BUILD_UNITTESTS|enable building unit tests|true")
fix8_addoption("BUILD_ALL_WARNINGS|enable building with all warnings|true")
fix8_addoption("BUILD_DOXYGEN_DOCS|enable building of self documentation|false")
fix8_addoption("BUILD_EP264|enable building of FIX50SP2_EP264 schema library|false")

# -------------------------------------------------------------------------------------------
# deps
# -------------------------------------------------------------------------------------------
if (MSVC)
	fix8_fetch(zip madler/zlib v1.3.1)
	set(ziplib zlib)
	set(poco_ssl_libs "")
	add_library(getopt STATIC msvc/getopt/getopt.c)
	include_directories(msvc/getopt)
	set(getopt_lib $<TARGET_FILE:getopt>)
	copy_libs()
else()
	find_package(ZLIB REQUIRED)
	set(ziplib ZLIB::ZLIB)
	set(poco_ssl_libs Poco::NetSSL Poco::Crypto)
	set(getopt_lib "")
endif()

# -------------------------------------------------------------------------------------------
# workaround for poco's c++17 check that fails for Intel compiler
if (CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM|Intel")
	function(check_for_cxx17_compiler)
		 message(STATUS "Bypassing POCO's C++17 check for Intel compiler")
		 set(CXX17_COMPILER TRUE PARENT_SCOPE)
	endfunction()
	file(WRITE "${CMAKE_BINARY_DIR}/dummy/CXX1x.cmake"
	 "# This empty file ensures our function override takes precedence")
	list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_BINARY_DIR}/dummy")
endif()
fix8_fetch(poco pocoproject/poco poco-1.14.2-release)
set(TBB_TEST OFF)
fix8_fetch(tbb uxlfoundation/oneTBB v2022.1.0)

# -------------------------------------------------------------------------------------------
# compiler settings
# -------------------------------------------------------------------------------------------
cpp_opts()

# -------------------------------------------------------------------------------------------
# runtime
# -------------------------------------------------------------------------------------------
set(fix8_libs Poco::Foundation Poco::Net Poco::Util ${poco_ssl_libs} Poco::XML ${ziplib} TBB::tbb)
macro(build_lib name type)
	add_library(${name} ${type} ${rsrc})
	target_link_libraries(${name} PUBLIC ${fix8_libs})
	target_include_directories(${name} PRIVATE include)
	if (MSVC)
		add_dependencies(${name} lib_copy ${fix8_libs})
	endif()
	comp_opts(${name})
	install(TARGETS ${name})
endmacro()

math(EXPR fix8_MAGIC_NUM "${fix8_VERSION_MAJOR} << 24 | ${fix8_VERSION_MINOR} << 12 | ${fix8_VERSION_PATCH}")
string(TIMESTAMP fix8_SDATE "%Y/%m/%d")
string(TIMESTAMP fix8_TIMESTAMP)
configure_file(${CMAKE_SOURCE_DIR}/f8config.h.in ${CMAKE_SOURCE_DIR}/include/fix8/f8config.h @ONLY)
set(rtdir ${CMAKE_SOURCE_DIR}/runtime)
file(GLOB rsrc "${rtdir}/*.cpp" "${rtdir}/*.c")
build_lib(fix8 SHARED)
set_target_properties(fix8 PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
target_compile_definitions(fix8 PRIVATE BUILD_F8API)
build_lib(fix8stat STATIC)
target_compile_definitions(fix8stat PRIVATE BUILD_F8API_STATIC)
if (NOT MSVC)
	set_target_properties(fix8stat PROPERTIES OUTPUT_NAME fix8)
endif()
install(DIRECTORY include DESTINATION .)
install(DIRECTORY schema DESTINATION share)
install(DIRECTORY contrib DESTINATION share)

# -------------------------------------------------------------------------------------------
# compiler
# -------------------------------------------------------------------------------------------
set(compdir ${CMAKE_SOURCE_DIR}/compiler)
file(GLOB csrc "${compdir}/*.cpp")
add_executable(f8c ${csrc})
target_link_libraries(f8c PUBLIC fix8 ${getopt_lib})
target_include_directories(f8c PRIVATE ${compdir} include)
comp_opts(f8c)
install(TARGETS f8c)

# -------------------------------------------------------------------------------------------
# util
# -------------------------------------------------------------------------------------------
set(utildir ${CMAKE_SOURCE_DIR}/util)
add_executable(seqedit ${utildir}/seqedit.cpp)
target_link_libraries(seqedit PUBLIC fix8 ${getopt_lib})
target_include_directories(seqedit PRIVATE ${utildir} include)
comp_opts(seqedit)
install(TARGETS seqedit)

# -------------------------------------------------------------------------------------------
# shared standard schema libs
# -------------------------------------------------------------------------------------------
fix8_gen_shared_library(FIX40 schema/FIX40.xml --verbose --namespace FIX40 -p FIX40)
fix8_gen_shared_library(FIX41 schema/FIX41.xml --verbose --namespace FIX41 -p FIX41)
fix8_gen_shared_library(FIX42 schema/FIX42.xml --verbose --namespace FIX42 -p FIX42)
fix8_gen_shared_library(FIX43 schema/FIX43.xml --verbose --namespace FIX43 -p FIX43)
fix8_gen_shared_library(FIX44 schema/FIX44.xml --verbose --namespace FIX44 -p FIX44)
fix8_gen_shared_library(FIX50 schema/FIX50.xml --verbose --namespace FIX50 -p FIX50 --fixt ${CMAKE_SOURCE_DIR}/schema/FIXT11.xml)
fix8_gen_shared_library(FIX50SP1 schema/FIX50SP1.xml --verbose --namespace FIX50SP1 -p FIX50SP1 --fixt ${CMAKE_SOURCE_DIR}/schema/FIXT11.xml)
fix8_gen_shared_library(FIX50SP2 schema/FIX50SP2.xml --verbose --namespace FIX50SP2 -p FIX50SP2 --fixt ${CMAKE_SOURCE_DIR}/schema/FIXT11.xml)
install(TARGETS FIX40 FIX41 FIX42 FIX43 FIX44 FIX50 FIX50SP1 FIX50SP2)
if(BUILD_EP264)
	fix8_gen_shared_library(FIX50SP2_EP264 schema/FIX50SP2_EP264.xml --verbose --namespace FIX50SP2_EP264 -p FIX50SP2_EP264 --fixt ${CMAKE_SOURCE_DIR}/schema/FIXT11_EP264.xml)
	install(TARGETS FIX50SP2_EP264)
endif()

# -------------------------------------------------------------------------------------------
# unit tests
# -------------------------------------------------------------------------------------------
if(BUILD_UNITTESTS)
	fix8_fetch(googletest google/googletest main)
	set(EXTRA_FIELDS "<field number='9999' name='SampleUserField'  type='STRING' messages='NewOrderSingle:N ExecutionReport:N OrderCancelRequest:Y' />\
<field number='9991' name='SampleUserField2' type='STRING' messages='NewOrderSingle:N ExecutionReport:N OrderCancelRequest:Y' />")
	fix8_gen_shared_library(utest schema/FIX42UTEST.xml --verbose -F ${EXTRA_FIELDS} -p utest --namespace UTEST)
	include(GoogleTest)
	enable_testing()
	build_test(message_test "utests/message_test.cpp")
	build_test(fileLogger_test "utests/fileLogger_test.cpp")
	build_test(filePersister_test "utests/filePersister_test.cpp")
	build_test(session_test "utests/session_test.cpp;utests/mockConnection.cpp")
endif()

# -------------------------------------------------------------------------------------------
# test apps
# -------------------------------------------------------------------------------------------
add_executable(f8test test/myfix.cpp)
fix8_gen_shared_library(myfix schema/FIX50SP2.xml --verbose --namespace MYFIX --prefix myfix --fixt ${CMAKE_SOURCE_DIR}/schema/FIXT11.xml)
target_link_libraries(f8test PUBLIC fix8 myfix GTest::gtest GTest::gtest_main ${getopt_lib})
target_include_directories(f8test PRIVATE test include ${CMAKE_BINARY_DIR}/generated/myfix)
target_compile_definitions(f8test PRIVATE F8_MYFIX_API_SHARED)
comp_opts(f8test)

add_executable(f8print test/myprint.cpp)
target_link_libraries(f8print PUBLIC fix8 myfix GTest::gtest GTest::gtest_main ${getopt_lib})
target_include_directories(f8print PRIVATE test include ${CMAKE_BINARY_DIR}/generated/myfix)
target_compile_definitions(f8print PRIVATE F8_MYFIX_API_SHARED)
comp_opts(f8print)

add_executable(harness test/harness.cpp)
target_link_libraries(harness PUBLIC fix8 myfix GTest::gtest GTest::gtest_main ${getopt_lib})
target_include_directories(harness PRIVATE test include ${CMAKE_BINARY_DIR}/generated/myfix)
target_compile_definitions(harness PRIVATE F8_MYFIX_API_SHARED)
comp_opts(harness)

add_executable(hftest test/hftest.cpp)
fix8_gen_shared_library(perf schema/FIX42PERF.xml --verbose --namespace PERF --second --noconst --prefix perf --norealm)
target_link_libraries(hftest PUBLIC fix8 perf GTest::gtest GTest::gtest_main ${getopt_lib})
target_include_directories(hftest PRIVATE test include ${CMAKE_BINARY_DIR}/generated/perf)
target_compile_definitions(hftest PRIVATE F8_PERF_API_SHARED)
comp_opts(hftest)

install(TARGETS f8test f8print harness harness)
install(FILES ${CMAKE_SOURCE_DIR}/test/hf_client.xml ${CMAKE_SOURCE_DIR}/test/myfix_client.xml ${CMAKE_SOURCE_DIR}/test/hf_client_include.xml
   ${CMAKE_SOURCE_DIR}/test/hf_server.xml ${CMAKE_SOURCE_DIR}/test/myfix_server.xml DESTINATION share/test)

# -------------------------------------------------------------------------------------------
# generated docs
# -------------------------------------------------------------------------------------------
if(BUILD_DOXYGEN_DOCS)
	find_package(Doxygen REQUIRED)
	find_package(FLEX REQUIRED)
	add_subdirectory(doc)
	add_dependencies(fix8_doxygen myfix)
	install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/html DESTINATION share/doc)
endif()

# -------------------------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# -------------------------------------------------------------------------------------------
# to dump all non-empty cmake variables
# cmake -DCMAKE_DUMP_VARS=true ..
# -------------------------------------------------------------------------------------------
option(CMAKE_DUMP_VARS "dump all cmake vars" false)
if(CMAKE_DUMP_VARS)
   get_cmake_property(varname VARIABLES)
   foreach (var ${varname})
      if(NOT ${var} STREQUAL "")
         message(STATUS "${var}=${${var}}")
      endif()
   endforeach()
endif()

