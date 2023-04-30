# Copyright (c) 2017-2023 Ken Barker
# (ken dot barker at via-technology dot aero)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required (VERSION 3.13)
cmake_policy(SET CMP0074 NEW)
project (HttpServer)

add_executable(${PROJECT_NAME} 
	example_https_server.cpp
	
	# thread_pool_http_server.cpp
	# example_http_server.cpp
	# chunked_http_server.cpp
	# example_https_server.cpp
	# simple_http_server.cpp
  # simple_https_server.cpp
  # simple_mutual_authentication_https_server.cpp
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

# Get the via-httplib code and examples
target_include_directories(${PROJECT_NAME} PRIVATE ../../include)
target_include_directories(${PROJECT_NAME} PRIVATE ../../examples)

if (WIN32)
  # Boost_ARCHITECTURE not defined for mingw on Windows
  if(MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(Boost_ARCHITECTURE "-x32")
  else()
    set(Boost_ARCHITECTURE "-x64")
  endif()

  # Set the minimum target Windows version to Windows 10
  target_compile_definitions(${PROJECT_NAME} PUBLIC NTDDI_VERSION=NTDDI_WIN10)
  target_compile_definitions(${PROJECT_NAME} PUBLIC _WIN32_WINNT=_WIN32_WINNT_WIN10)

  # Get the windows socket libraries
  target_link_libraries(${PROJECT_NAME} PRIVATE gdi32 wsock32 ws2_32)
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif(WIN32)

# OpenSLL is required for HTTPS examples
find_package(OpenSSL COMPONENTS SSL)
if(OpenSSL_FOUND)
  target_include_directories(${PROJECT_NAME} PUBLIC ${OpenSSL_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL)
endif(OpenSSL_FOUND)

if (DEFINED ENV{Asio_DIR})
  message("Using ASIO_STANDALONE")
  target_compile_definitions(${PROJECT_NAME} PRIVATE ASIO_STANDALONE)
  target_include_directories(${PROJECT_NAME} PRIVATE $ENV{Asio_DIR}/include)
endif()

find_package(Boost COMPONENTS system date_time regex)
if(Boost_FOUND)
  target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
  target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ALL_DYN_LINK)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Boost::system
    Boost::date_time
    Boost::regex)
endif(Boost_FOUND)
