# Copyright (c) 2017-2019 Ken Barker
# (ken dot barker at via-technology dot aero)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required (VERSION 3.5)
project (HttpServer)

add_executable(${PROJECT_NAME} 
    ../cacert.cpp
	../privkey.cpp
	example_https_server.cpp
	
	# thread_pool_http_server.cpp
	# example_http_server.cpp
	# chunked_http_server.cpp
	# example_https_server.cpp
	# simple_http_server.cpp
	# simple_https_server.cpp
)

if (WIN32)
  # Set the minimum target Windows version to Windows 7
  target_compile_definitions(${PROJECT_NAME} PUBLIC NTDDI_VERSION=NTDDI_WIN7)
  target_compile_definitions(${PROJECT_NAME} PUBLIC _WIN32_WINNT=_WIN32_WINNT_WIN7)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ../../examples)

find_package(ViaHttpLib REQUIRED via-httplib)
if(ViaHttpLib_FOUND)
  target_include_directories(${PROJECT_NAME} PUBLIC ${ViaHttpLib_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE ViaHttpLib::via-httplib)
endif()

find_package(OpenSSL REQUIRED SSL)
if(OpenSSL_FOUND)
  target_include_directories(${PROJECT_NAME} PUBLIC ${OpenSSL_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ALL_DYN_LINK)

find_package(Boost REQUIRED COMPONENTS system date_time regex)
target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  Boost::system
  Boost::date_time
  Boost::regex)
