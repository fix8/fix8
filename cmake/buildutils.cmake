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
# cmake build utils
# -------------------------------------------------------------------------------------------
function(fix8_setbuildtype define_prefix default_type)
	if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "")
		if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
			add_compile_definitions(${define_prefix}_DEBUG_BUILD)
		elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
			add_compile_definitions(${define_prefix}_RELWITHDEBINFO_BUILD)
		elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
			add_compile_definitions(${define_prefix}_MINSIZEREL_BUILD)
		elseif(${default_type} STREQUAL "Release")
			add_compile_definitions(${define_prefix}_RELEASE_BUILD)
		else()
			message(FATAL_ERROR "Unsupported build type ${default_type}")
		endif()
		message("-- ${CMAKE_PROJECT_NAME} version ${CMAKE_PROJECT_VERSION}, build type ${CMAKE_BUILD_TYPE}")
	else()
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE ${default_type})
		fix8_setbuildtype(${define_prefix} ${default_type})
	endif()
endfunction()

# -------------------------------------------------------------------------------------------
macro(fix8_addoption option_string)
	string(REPLACE "|" ";" part ${option_string})
	list(GET part 0 opt_name)
	list(GET part 1 opt_description)
	list(GET part 2 opt_default)
	if(NOT DEFINED ${opt_name})
		set(${opt_name} ${opt_default})
	endif()
	option(${opt_name} "${opt_description}" ${${opt_name}})
	message("-- Build: ${opt_description}: ${${opt_name}}")
endmacro()

# -------------------------------------------------------------------------------------------
macro(fix8_fetch modname parturl tag)
	include(FetchContent)
	message(STATUS "Downloading ${modname}...")
	FetchContent_Declare(${modname} GIT_REPOSITORY https://github.com/${parturl}.git GIT_SHALLOW ON GIT_TAG ${tag})
	FetchContent_MakeAvailable(${modname})
endmacro()

# ----------------------------------------------------------------------------------------
function(cpp_opts)
	if (NOT MSVC)
		if(BUILD_ALL_WARNINGS)
			set(CXX_FLAGS -Wall -Wextra -Wpedantic )
		endif()
		set(CXX_FLAGS ${CXX_FLAGS} -fPIC -pthread -Wno-overloaded-virtual -Wno-unused-parameter -Wno-missing-field-initializers)
		set(CXX_FLAGS_DEBUG -g -O0 -D_DEBUG)
		set(CXX_FLAGS_RELEASE -O3 -DNDEBUG)
		set(CXX_FLAGS_RELWITHDEBINFO -g -O1 -D_DEBUG)
		# message("*** ${CXX_FLAGS_DEBUG}")
	else()
		if(BUILD_ALL_WARNINGS)
			set(CXX_FLAGS /W3)
		endif()
		set(CXX_FLAGS ${CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE /D_WINDLL /wd4996 /wd4100 /bigobj)
		set(CXX_FLAGS_DEBUG /DEBUG /D_DEBUG /Od)
		set(CXX_FLAGS_RELEASE /DNDEBUG /O2 /Oi)
		set(CXX_FLAGS_RELWITHDEBINFO /DEBUG /D_DEBUG /O2)
	endif()
	set(FIX8_CXX_FLAGS ${CXX_FLAGS} PARENT_SCOPE)
	set(FIX8_CXX_FLAGS_DEBUG ${CXX_FLAGS_DEBUG} PARENT_SCOPE)
	set(FIX8_CXX_FLAGS_RELEASE ${CXX_FLAGS_RELEASE} PARENT_SCOPE)
	set(FIX8_CXX_FLAGS_RELWITHDEBINFO ${CXX_FLAGS_RELWITHDEBINFO} PARENT_SCOPE)
	set(FIX8_LD_LIBRARY_PATH LD_LIBRARY_PATH=${LIB_OUTPUT_DIR}/lib PARENT_SCOPE)
	if (MSVC)
		set(dlls $<TARGET_FILE_DIR:Poco::Foundation> $<TARGET_FILE_DIR:TBB::tbb> $<TARGET_FILE_DIR:zlib>)
		string(JOIN "\;" dlls_string ${dlls})
		set(FIX8_LD_LIBRARY_PATH "PATH=${dlls_string}" PARENT_SCOPE)
	endif()
endfunction()

# -------------------------------------------------------------------------------------------
function(comp_opts targ)
	target_compile_features(${targ} PRIVATE cxx_std_17)
	target_compile_options(${targ} PRIVATE
			${FIX8_CXX_FLAGS}
			$<$<CONFIG:Debug>:${FIX8_CXX_FLAGS_DEBUG}>
			$<$<CONFIG:Release>:${FIX8_CXX_FLAGS_RELEASE}>
			$<$<CONFIG:RelWithDebInfo>:${FIX8_CXX_FLAGS_RELWITHDEBINFO}>)
	target_link_libraries(${targ} PRIVATE TBB::tbbmalloc_proxy)
	if (MSVC)
		target_link_options(${targ} PRIVATE	/INCLUDE:__TBB_malloc_proxy)
	endif()
endfunction()

# -------------------------------------------------------------------------------------------
function(build_test name files)
	add_executable(${name} ${files})
	target_link_libraries(${name} PUBLIC Poco::Foundation Poco::Net Poco::Util ${poco_ssl_libs} Poco::XML fix8 utest GTest::gtest GTest::gtest_main)
	target_include_directories(${name} PRIVATE include utests ${CMAKE_BINARY_DIR}/generated/utest)
	target_compile_definitions(${name} PRIVATE F8_UTEST_API_SHARED)
	gtest_discover_tests(${name})
	comp_opts(${name})
endfunction()

# -------------------------------------------------------------------------------------------
macro(fix8_gen_library shared name xml extra_fields)
	if ("${extra_fields}" STREQUAL "_")
		set(args ${ARGV4} ${ARGV5} ${ARGV6} ${ARGV7} ${ARGV8} ${ARGV9} ${ARGV10} ${CMAKE_SOURCE_DIR}/${xml})
	else()
		set(args ${ARGV4} ${ARGV5} ${ARGV6} ${ARGV7} ${ARGV8} ${ARGV9} ${ARGV10} ${CMAKE_SOURCE_DIR}/${xml} -F ${extra_fields})
	endif()
	set(prefix ${CMAKE_BINARY_DIR}/generated/${name})
	file(MAKE_DIRECTORY ${prefix})

	string(FIND "${xml}" "/" has_path_delimiters)
	if (${has_path_delimiters} EQUAL -1)
		set(xml ${CMAKE_SOURCE_DIR}/${xml})
	endif()
	message("-- Using ${xml}")
	add_custom_command(
		OUTPUT
			${prefix}/${name}_classes.cpp
			${prefix}/${name}_traits.cpp
			${prefix}/${name}_types.cpp
			${prefix}/${name}_classes.hpp
			${prefix}/${name}_types.hpp
		COMMAND ${CMAKE_COMMAND} -E env ${FIX8_LD_LIBRARY_PATH} $<TARGET_FILE:f8c> ${args}
		MAIN_DEPENDENCY ${xml}
		WORKING_DIRECTORY ${prefix}
		DEPENDS f8c
		VERBATIM)
	if ("${shared}" STREQUAL "shared")
		set(libname ${name})
		add_library(${libname} SHARED
			${prefix}/${name}_classes.cpp
			${prefix}/${name}_traits.cpp
			${prefix}/${name}_types.cpp
			${prefix}/${name}_classes.hpp
			${prefix}/${name}_types.hpp)
		string(TOUPPER ${name} name_upper)
		target_compile_definitions(${libname} PRIVATE F8_${name_upper}_API_SHARED BUILD_F8_${name_upper}_API)
	else()
		set(libname ${name})
		add_library(${libname} STATIC
			${prefix}/${name}_classes.cpp
			${prefix}/${name}_traits.cpp
			${prefix}/${name}_types.cpp
			${prefix}/${name}_classes.hpp
			${prefix}/${name}_types.hpp)
	endif()
	add_dependencies(${libname} f8c)
	target_include_directories(${libname} PUBLIC ${prefix})
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(${libname} PRIVATE -fno-var-tracking -fno-var-tracking-assignments)
	endif()
	target_link_libraries(${libname} PUBLIC fix8)
	comp_opts(${libname})
endmacro()

# -------------------------------------------------------------------------------------------
macro(fix8_gen_shared_library name xml)
	fix8_gen_library(shared ${name} ${xml} "_" ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6} ${ARGV7} ${ARGV8} ${ARGV9} ${ARGV10})
endmacro()

# -------------------------------------------------------------------------------------------
macro(add_gen_static_library name xml)
	fix8_gen_library(static ${name} ${xml} "_" ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6} ${ARGV7} ${ARGV8} ${ARGV9} ${ARGV10})
endmacro()

# -------------------------------------------------------------------------------------------
macro(copy_libs)
	add_custom_target(lib_copy ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bin/.libs_copied)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bin/.libs_copied
		COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:TBB::tbb> ${CMAKE_CURRENT_BINARY_DIR}/bin
		COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${ziplib}> ${CMAKE_CURRENT_BINARY_DIR}/bin
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/bin/.libs_copied
		DEPENDS TBB::tbb TBB::tbbmalloc_proxy TBB::tbbmalloc ${ziplib})
endmacro()
