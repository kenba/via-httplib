# Copyright (c) 2013 Louis Henry Nayegon.
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
  set( VIA_HTTPLIB_LIBRARY_NAME via-httplib )
  set( VIA_HTTPLIB_LIBRARY_TYPE SHARED )
else()
  set(Boost_USE_STATIC_LIBS ON)
  set( VIA_HTTPLIB_LIBRARY_NAME via-httplib )
  set( VIA_HTTPLIB_LIBRARY_TYPE STATIC )
endif()
set(Boost_USE_MULTITHREADED ON)
if(VIA_HTTPLIB_BUILD_TESTS)
  set(Boost_COMPONENTS system regex )
else()
  set(Boost_COMPONENTS system regex )
endif()

find_package( Boost 1.51.0 REQUIRED ${Boost_COMPONENTS} )
find_package( OpenSSL )

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_library( ${VIA_HTTPLIB_LIBRARY_NAME} ${VIA_HTTPLIB_LIBRARY_TYPE}
       via/http/character.cpp
       via/http/chunk.cpp
       via/http/header_field.cpp
       via/http/headers.cpp
       via/http/request.cpp
       via/http/request_method.cpp
       via/http/response.cpp
       via/http/response_status.cpp
       )
