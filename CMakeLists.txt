# Copyright (c) 2013-2019 Louis Henry Nayegon.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# The software should be used for Good, not Evil.

cmake_minimum_required (VERSION 3.12)
cmake_policy(SET CMP0074 NEW)
project (via-httplib)

option(VIA_HTTPLIB_COVERAGE "Enable code coverage." OFF)

add_library(${PROJECT_NAME} INTERFACE)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
  target_include_directories(${PROJECT_NAME} INTERFACE ${Boost_INCLUDE_DIRS})

  # Boost::asio is header only but it requires Boost::system
  target_link_libraries(${PROJECT_NAME} INTERFACE Boost::system)
else()
  find_package(Asio)
  target_compile_definitions(${PROJECT_NAME} INTERFACE ASIO_STANDALONE)
endif(Boost_FOUND)

target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

find_package(Boost REQUIRED COMPONENTS thread unit_test_framework)
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
  
  target_compile_definitions(${PROJECT_NAME}_test PRIVATE BOOST_ALL_DYN_LINK)
  target_include_directories(${PROJECT_NAME}_test PRIVATE ${Boost_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME}_test
    PRIVATE
    ${PROJECT_NAME}
    Boost::system
    Boost::thread
    Boost::unit_test_framework)

  if (MSVC)
    target_compile_options(${PROJECT_NAME}_test PRIVATE /W4)
  else()
    target_compile_options(${PROJECT_NAME}_test PRIVATE -Wall -Wextra -Wpedantic)
    
    if (VIA_HTTPLIB_COVERAGE)
      target_compile_options(${PROJECT_NAME} PRIVATE --coverage)
      target_link_libraries(${PROJECT_NAME}_test PRIVATE --coverage)
    endif()
    
  endif()
	
  add_test(NAME via_http_Parsers.test COMMAND ${PROJECT_NAME}_test)
  
endif()

# Introduce variables:
# * CMAKE_INSTALL_INCLUDEDIR
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME} EXPORT ViaHttpLibTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Install headers:
install(
    DIRECTORY "include/via"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING PATTERN "*.hpp"
)

set(ConfigPackageLocation lib/cmake/ViaHttpLib)
install(EXPORT ViaHttpLibTargets 
    FILE ViaHttpLibTargets.cmake
    NAMESPACE ViaHttpLib::
    DESTINATION ${ConfigPackageLocation}
)

add_library(ViaHttpLib::httplib ALIAS ${PROJECT_NAME})

set(CPACK_PACKAGE_VERSION "1.8.0")

include(CMakePackageConfigHelpers)
write_basic_package_version_file("cmake/ViaHttpLibConfigVersion.cmake"
  VERSION ${CPACK_PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(FILES "cmake/ViaHttpLibConfig.cmake" "cmake/ViaHttpLibConfigVersion.cmake"
  DESTINATION ${ConfigPackageLocation}
)

include(CPack)
