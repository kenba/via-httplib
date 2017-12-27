# Copyright (c) 2013-2017 Louis Henry Nayegon.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# The software should be used for Good, not Evil.

cmake_minimum_required (VERSION 3.5)
project (via-httplib)

option(VIA_HTTPLIB_BUILD_STATIC_LIBS "Build via-httplib as static libraries." ON)
set(VIA_HTTPLIB_LIBRARY_TYPE SHARED)

if (VIA_HTTPLIB_BUILD_STATIC_LIBS)
  set(VIA_HTTPLIB_LIBRARY_TYPE STATIC)
endif(VIA_HTTPLIB_BUILD_STATIC_LIBS)

set(CMAKE_DEBUG_POSTFIX "-debug")

add_library(${PROJECT_NAME} ${VIA_HTTPLIB_LIBRARY_TYPE} "")

file(GLOB_RECURSE INCLUDE_FILES include/via/*.hpp)
target_sources(${PROJECT_NAME}
  PUBLIC
    ${INCLUDE_FILES}
)

target_sources(${PROJECT_NAME} 
  PRIVATE
    src/via/http/character.cpp
    src/via/http/chunk.cpp
    src/via/http/header_field.cpp
    src/via/http/headers.cpp
    src/via/http/request.cpp
    src/via/http/request_method.cpp
    src/via/http/response.cpp
    src/via/http/response_status.cpp
)

find_package(Boost)
if(Boost_FOUND)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_LIBS ${VIA_HTTPLIB_BUILD_STATIC_LIBS})
  
  # Note: request_router.cpp and authentication files require
  # boost::algorithm and boost::archive respectively.
  target_sources(${PROJECT_NAME} 
    PRIVATE
      src/via/http/request_router.cpp
      src/via/http/authentication/base64.cpp
      src/via/http/authentication/basic.cpp 
  )
else()
  find_package(Asio)
  target_compile_definitions(${PROJECT_NAME} PUBLIC ASIO_STANDALONE)
endif(Boost_FOUND)

if (WIN32)
  # Set minimum version to Windows 7
  # target_compile_definitions(${PROJECT_NAME} PUBLIC WIN32_WINNT=_WIN32_WINNT_WIN7)
  if (MSVC)
      target_compile_definitions(${PROJECT_NAME} PUBLIC SCL_SECURE_NO_WARNINGS)
  endif(MSVC)
endif(WIN32)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_constexpr)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_lambdas)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_noexcept)
  
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

if(Boost_FOUND)
  target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
  # Boost::asio is header only but it requires Boost::system
  target_link_libraries(${PROJECT_NAME} INTERFACE Boost::system)
endif(Boost_FOUND)

install(TARGETS ${PROJECT_NAME} # EXPORT ViaHttpTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

#install(EXPORT ViaHttpTargets 
#    FILE ViaHttp.cmake
#    NAMESPACE ViaHttp::
#    DESTINATION lib/cmake/ViaHttp
#)

find_package (Git)
if(Git_FOUND)
  set(CPACK_PACKAGE_VERSION "COMMAND ${GIT_EXECUTABLE} .git describe")
else()
  set(CPACK_PACKAGE_VERSION "1.4.0")
endif()

include(CPack)
