# Find fix8 include paths and libraries
# Copyright 2015 (c) Ido Rosen <ido@kernel.org>

# To pass additional search paths/hints:
#   set(FIX8_INCLUDE_HINTS "lib/fix8/include/")
#   set(FIX8_LIBRARY_HINTS "lib/fix8/lib/")
#   set(FIX8_BINPATH_HINTS "lib/fix8/bin/")

# This sets the following variables:
#   FIX8_INCLUDE_DIRS - directory containing "fix8" folder
#   FIX8_LIBRARY_DIRS - directory containing "libfix8.so".
#   FIX8_LIBRARIES - fix8 library(/ies)
#   FIX8_VERSION_STRING - version of f8c executable
#   FIX8_F8C_EXECUTABLE - path of f8c executable for generating code
#   FIX8_COMPILER - path of f8c executable for generating code
#
# To generate code using the fix8 compiler:
#   include_directories(${CMAKE_CURRENT_BINARY_DIR})
#   FIX8_GENERATE_CPP(FIX8_SRCS FIX8_HDRS /usr/share/fix8/schema/FIX42.xml)
#   add_executable(bar bar.cpp ${FIX8_SRCS} ${FIX8_HDRS})
#   target_link_libraries(bar ${FIX8_LIBRARIES} ${POCO_LIBRARIES} ${BerkeleyDB_CXX_LIBRARIES})
#

function(FIX8_GENERATE_CPP SRCS HDRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: FIX8_GENERATE_CPP() called without any XML schema files")
    return()
  endif()

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    list(APPEND ${SRCS}
        "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_classes.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_traits.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_types.cpp")
    list(APPEND ${HDRS}
        "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_classes.hpp"
        "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_router.hpp"
        "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_types.hpp")
    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_classes.cpp"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_classes.hpp"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_router.hpp"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_traits.cpp"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_types.cpp"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}fix_types.hpp"
      COMMAND  ${FIX8_COMPILER}
      ARGS -o ${CMAKE_CURRENT_BINARY_DIR} -p ${FIL_WE}fix -n ${FIL_WE} ${ABS_FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C++ fix8 compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

if(FIX8_INCLUDE_DIRS)
    set(FIX8_FIND_QUIETLY TRUE)
endif()

find_package(PkgConfig)
pkg_check_modules(PC_FIX8 QUIET fix8)

find_path(FIX8_INCLUDE_DIR NAMES fix8/f8config.h
    HINTS ${FIX8_INCLUDE_HINTS}
          ${PC_FIX8_INCLUDEDIR} ${PC_FIX8_INCLUDE_DIRS})

find_library(FIX8_LIBRARY NAMES fix8
    HINTS ${PC_FIX8_LIBDIR} ${PC_FIX8_LIBRARY_DIRS})

find_program(FIX8_F8C_EXECUTABLE NAMES f8c
             DOC "The fix8 compiler."
             HINTS ${FIX8_BINPATH_HINTS}
                   ${PC_FIX8_BINDIR})
if (FIX8_F8C_EXECUTABLE)
    execute_process(COMMAND "${FIX8_F8C_EXECUTABLE}" "--version"
                    TIMEOUT 2
                    RESULT_VARIABLE _fix8_F8C_VERSION_RESULT
                    OUTPUT_VARIABLE FIX8_VERSION_STRING_VERBOSE
                    )
    if (_fix8_F8C_VERSION_RESULT EQUAL "0")
        if("${FIX8_VERSION_STRING_VERBOSE}" MATCHES "fix8 version ([^ \r\n]+)[ \r\n]+")
            set(FIX8_VERSION_STRING "${CMAKE_MATCH_1}")
        endif()
    endif()
endif()

set(FIX8_INCLUDE_DIRS ${FIX8_INCLUDE_DIR})
set(FIX8_LIBRARY_DIRS ${PC_FIX8_LIBDIR} ${PC_FIX8_LIBRARY_DIRS})
set(FIX8_LIBRARIES ${FIX8_LIBRARY})
set(FIX8_COMPILER ${FIX8_F8C_EXECUTABLE})

find_package_handle_standard_args(Fix8
                                  REQUIRED_VARS
                                    FIX8_INCLUDE_DIRS
                                    FIX8_F8C_EXECUTABLE
                                    FIX8_COMPILER
                                    FIX8_VERSION_STRING
                                  VERSION_VAR FIX8_VERSION_STRING)
mark_as_advanced(FIX8_INCLUDE_DIRS
    FIX8_LIBRARY_DIRS
    FIX8_LIBRARIES
    FIX8_F8C_EXECUTABLE
    FIX8_COMPILER
    FIX8_VERSION_STRING
    FIX8_VERSION_STRING_VERBOSE)
