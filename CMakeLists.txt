# Copyright (c) 2013-2017 Louis Henry Nayegon.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# The software should be used for Good, not Evil.

cmake_minimum_required (VERSION 2.8)
project (via-httplib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE includes include/via/*.hpp)
file(GLOB_RECURSE sources src/via/http/*.cpp)

# Add include files to a separate target for QtCreator
# see: https://cmake.org/pipermail/cmake/2012-August/051811.html
add_custom_target(headers SOURCES ${includes})

option(VIA_HTTPLIB_BUILD_SHARED_LIBS "Build via-httplib as shared libraries." OFF)
option(VIA_HTTPLIB_BUILD_TESTS "Build the unit tests." ON)

if(VIA_HTTPLIB_BUILD_SHARED_LIBS)
  set(Boost_USE_STATIC_LIBS OFF)
  set( VIA_HTTPLIB_LIBRARY_TYPE SHARED )
else()
  set(Boost_USE_STATIC_LIBS ON)
  set( VIA_HTTPLIB_LIBRARY_TYPE STATIC )
endif()
set( VIA_HTTPLIB_LIBRARY_NAME via-httplib )

set(Boost_USE_MULTITHREADED ON)
if(VIA_HTTPLIB_BUILD_TESTS)
else()
  set(Boost_COMPONENTS system thread)
endif()

find_package (Git)
if(Git_FOUND)
  set(GIT_VERSION "COMMAND ${GIT_EXECUTABLE} .git describe")
else()
  set(GIT_VERSION "1.0.0")
endif()

find_package(Boost 1.51.0 REQUIRED ${Boost_COMPONENTS})
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package( OpenSSL )
if (OpenSSL_FOUND)
  add_definitions(-DBOOST_NETWORK_ENABLE_HTTPS)
  include_directories(${OpenSSL_INCLUDE_DIRS})
endif()

find_package( Asio )
if (Asio_FOUND)
  add_definitions(-DASIO_STANDALONE)
  include_directories(${Asio_INCLUDE_DIRS})
endif()

if (MSVC)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(MSVC)
if (WIN32)
  add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WIN7)
endif(WIN32)

## packaging

add_library(${VIA_HTTPLIB_LIBRARY_NAME} ${VIA_HTTPLIB_LIBRARY_TYPE}
  ${includes} ${sources})

install(TARGETS ${VIA_HTTPLIB_LIBRARY_NAME} DESTINATION lib)

install(DIRECTORY include/via DESTINATION include)

set(CPACK_PACKAGE_VERSION ${GIT_VERSION})

include(CPack)
