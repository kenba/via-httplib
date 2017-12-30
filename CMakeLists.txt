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

add_library(${PROJECT_NAME} ${VIA_HTTPLIB_LIBRARY_TYPE}
    src/via/http/character.cpp
    src/via/http/chunk.cpp
    src/via/http/header_field.cpp
    src/via/http/headers.cpp
    src/via/http/request.cpp
    src/via/http/request_method.cpp
    src/via/http/response.cpp
    src/via/http/response_status.cpp
)

find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
  target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})

  # Boost::asio is header only but it requires Boost::system
  target_link_libraries(${PROJECT_NAME} INTERFACE Boost::system)

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

if (MSVC)
  target_compile_definitions(${PROJECT_NAME} PUBLIC SCL_SECURE_NO_WARNINGS)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif(MSVC)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
  
target_include_directories(${PROJECT_NAME} PUBLIC 
  $<BUILD_INTERFACE:${${PROJECT_NAME}_BINARY_DIR}/include>
  $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_include_directories(${PROJECT_NAME} PRIVATE src)

if(Boost_FOUND)
  add_executable(${PROJECT_NAME}_test
    tests/test_main.cpp
    tests/http/test_character.cpp
    tests/http/test_chunk.cpp
    tests/http/test_header_field.cpp
    tests/http/test_headers.cpp
    tests/http/test_request.cpp
    tests/http/test_request_router.cpp
    tests/http/test_request_uri.cpp
    tests/http/test_response.cpp
    tests/http/authentication/test_base64.cpp
    tests/http/authentication/test_basic_authentication.cpp
    tests/thread/test_threadsafe_hash_map.cpp
  )
  
  file(GLOB_RECURSE INCLUDE_FILES include/via/*.hpp)
  target_sources(${PROJECT_NAME}_test
    PRIVATE
      ${INCLUDE_FILES}
  )
  
  target_link_libraries(${PROJECT_NAME}_test 
    PRIVATE
   ${PROJECT_NAME})
  
  if (MSVC)
    target_compile_options(${PROJECT_NAME}_test PRIVATE /W4)
  else()
    target_compile_options(${PROJECT_NAME}_test PRIVATE -Wall -Wextra -Wpedantic)
    
    find_package(Boost REQUIRED COMPONENTS thread unit_test_framework)
    target_link_libraries(${PROJECT_NAME}_test
      PRIVATE
      Boost::thread
      Boost::unit_test_framework)
  endif()
	
  add_test(NAME via_http_Parsers.test COMMAND ${PROJECT_NAME}_test)
  
endif(Boost_FOUND)

install(TARGETS ${PROJECT_NAME} EXPORT ViaHttpLibTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

set(ConfigPackageLocation lib/cmake/ViaHttpLib)
install(EXPORT ViaHttpLibTargets 
    FILE ViaHttpLibTargets.cmake
    NAMESPACE ViaHttpLib::
    DESTINATION ${ConfigPackageLocation}
)

add_library(ViaHttpLib::httplib ALIAS ${PROJECT_NAME})

set(CPACK_PACKAGE_VERSION "1.4.1")

include(CMakePackageConfigHelpers)
write_basic_package_version_file("ViaHttpLibConfigVersion.cmake"
  VERSION ${CPACK_PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(FILES "ViaHttpLibConfig.cmake" "ViaHttpLibConfigVersion.cmake"
  DESTINATION ${ConfigPackageLocation}
)

include(CPack)
