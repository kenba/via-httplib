# Copyright (c) 2013-2015 Louis Henry Nayegon.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# The software should be used for Good, not Evil.

cmake_minimum_required (VERSION 2.8)
project (VIA-HTTPLIB)

option( VIA_HTTPLIB_BUILD_SHARED_LIBS "Build via-httplib as shared libraries." OFF )
option( VIA_HTTPLIB_BUILD_TESTS "Build the unit tests." ON )

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
  set(Boost_COMPONENTS system)
endif()

find_package( Boost 1.51.0 REQUIRED ${Boost_COMPONENTS} )
find_package( OpenSSL )

if (OPENSSL_FOUND)
    add_definitions(-DBOOST_NETWORK_ENABLE_HTTPS)
endif()

if(Boost_FOUND)
  if (MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif(MSVC)
  if (WIN32)
    add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WIN7)
  endif(WIN32)
  include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

  add_library( ${VIA_HTTPLIB_LIBRARY_NAME} ${VIA_HTTPLIB_LIBRARY_TYPE}
    src/via/http/character.cpp
    src/via/http/chunk.cpp
    src/via/http/header_field.cpp
    src/via/http/headers.cpp
    src/via/http/request.cpp
    src/via/http/request_method.cpp
    src/via/http/response.cpp
    src/via/http/response_status.cpp
  )

  install(TARGETS ${VIA_HTTPLIB_LIBRARY_NAME}
    DESTINATION lib)

  install(DIRECTORY include/via
    DESTINATION include)
endif()
