# Copyright (c) 2013-2017 Louis Henry Nayegon.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# The software should be used for Good, not Evil.

cmake_minimum_required (VERSION 2.8)
project (via-httplib)

option(VIA_HTTPLIB_BUILD_SHARED_LIBS "Build via-httplib as shared libraries." ON)
option(VIA_HTTPLIB_BUILD_TESTS "Build the unit tests." OFF)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE includes include/via/*.hpp)

set (sources
  ${SOURCE_DIR}/via/http/character.cpp
  ${SOURCE_DIR}/via/http/chunk.cpp
  ${SOURCE_DIR}/via/http/header_field.cpp
  ${SOURCE_DIR}/via/http/headers.cpp
  ${SOURCE_DIR}/via/http/request.cpp
  ${SOURCE_DIR}/via/http/request_method.cpp
  ${SOURCE_DIR}/via/http/response.cpp
  ${SOURCE_DIR}/via/http/response_status.cpp )

# Add include files to a separate target for QtCreator
# see: https://cmake.org/pipermail/cmake/2012-August/051811.html
add_custom_target(headers SOURCES ${includes})

find_package (Git)
if(Git_FOUND)
  set(GIT_VERSION "COMMAND ${GIT_EXECUTABLE} .git describe")
else()
  set(GIT_VERSION "1.4.0")
endif()

if(VIA_HTTPLIB_BUILD_TESTS)
  file(GLOB_RECURSE sources_test tests/*.cpp)
  set(Boost_COMPONENTS system thread unit_test_framework)
else()
  set(Boost_COMPONENTS)
endif()

# Note: request_router.cpp and authentication files require
# boost::algorithm and boost::archive respectively.
find_package(Boost ${Boost_COMPONENTS})
if(Boost_FOUND)
  set(Boost_USE_MULTITHREADED ON)
  include_directories(${Boost_INCLUDE_DIRS})

  set (sources ${sources}
    ${SOURCE_DIR}/via/http/request_router.cpp
    ${SOURCE_DIR}/via/http/authentication/base64.cpp
    ${SOURCE_DIR}/via/http/authentication/basic.cpp )
endif()

if (MSVC)
  # Don't build dll's for MSVC
  set(VIA_HTTPLIB_BUILD_SHARED_LIBS OFF)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(MSVC)

if (WIN32)
  # Set minimum version to Windows 7
  add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WIN7)
endif(WIN32)

set(VIA_HTTPLIB_LIBRARY_NAME ${PROJECT_NAME})

if(VIA_HTTPLIB_BUILD_SHARED_LIBS)
  set(Boost_USE_STATIC_LIBS OFF)
  set( VIA_HTTPLIB_LIBRARY_TYPE SHARED )
else()
  set(Boost_USE_STATIC_LIBS ON)
  set( VIA_HTTPLIB_LIBRARY_TYPE STATIC )
endif()

## packaging

add_library(${VIA_HTTPLIB_LIBRARY_NAME} ${VIA_HTTPLIB_LIBRARY_TYPE}
  ${includes} ${sources})

if(VIA_HTTPLIB_BUILD_TESTS)
  target_compile_options(${VIA_HTTPLIB_LIBRARY_NAME} PRIVATE --coverage)
  add_executable(unit_tests ${includes} ${sources} ${sources_test}) 
  target_compile_options(unit_tests PRIVATE --coverage)
  target_link_libraries(unit_tests ${Boost_COMPONENTS})
endif(VIA_HTTPLIB_BUILD_TESTS)

install(TARGETS ${VIA_HTTPLIB_LIBRARY_NAME} DESTINATION lib)

install(DIRECTORY include/via DESTINATION include)

set(CPACK_PACKAGE_VERSION ${GIT_VERSION})

include(CPack)
