# Copyright (c) 2013-2015 Ken Barker
# (ken dot barker at via-technology dot co dot uk)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

# Check that VIAHTTPLIB has been set
# Note: isEmpty doesn't work on environ var directly so put it in a local first
VIAHTTPLIB = $$(VIAHTTPLIB)
isEmpty(VIAHTTPLIB) {
  error("The VIAHTTPLIB environment variable has not been set. Please set it to the location of the via-httplib libraries")
} else {
  message(Using via-httplib from: $$VIAHTTPLIB)
}

win32 {
  # Min version is Windows 7
  DEFINES += _WIN32_WINNT=_WIN32_WINNT_WIN7

  ASIO_ROOT = $$(ASIO_ROOT)
  isEmpty(ASIO_ROOT) {
    # Ensure that the BOOST_ROOT environment variable has been set
    BOOST_ROOT = $$(BOOST_ROOT)
    isEmpty(BOOST_ROOT) {
      error("Please set BOOST_ROOT to the location of the Boost libraries")
    } else {
      message(Using Boost from: $$BOOST_ROOT)
    }
  } else {
    message(Using Asio from: $$ASIO_ROOT)
  }
} else {
  # Library version numbers
  VER_MAJ = 1
  VER_MIN = 2
  VER_PAT = 0
}

TEMPLATE = lib
CONFIG -= lib_bundle
CONFIG -= qt
CONFIG += c++14
CONFIG += thread
CONFIG += shared
CONFIG += separate_debug_info

# Compiler options
*-g++* {
  message(Setting flags for GCC or MinGw)

  QMAKE_CXXFLAGS += -Wall
  QMAKE_CXXFLAGS += -Wextra
  QMAKE_CXXFLAGS += -Wpedantic

  win32 {
    # Set debug options for MinGw in QtCreator
    QMAKE_CXXFLAGS_DEBUG = -O0
    QMAKE_CXXFLAGS_DEBUG += -gdwarf-3
  }
}

*-clang* {
  message(Setting flags for clang)
  QMAKE_CXXFLAGS += -stdlib=libc++
}

include (via-httplib.pri)

SRC_DIR = $$VIAHTTPLIB/src

SOURCES += $${SRC_DIR}/via/http/character.cpp
SOURCES += $${SRC_DIR}/via/http/chunk.cpp
SOURCES += $${SRC_DIR}/via/http/header_field.cpp
SOURCES += $${SRC_DIR}/via/http/headers.cpp
SOURCES += $${SRC_DIR}/via/http/request_method.cpp
SOURCES += $${SRC_DIR}/via/http/request.cpp
SOURCES += $${SRC_DIR}/via/http/response_status.cpp
SOURCES += $${SRC_DIR}/via/http/response.cpp

CONFIG(release, debug|release) {
  DESTDIR = $${OUT_PWD}/release
} else {
  DESTDIR = $${OUT_PWD}/debug
}

OBJECTS_DIR = $${DESTDIR}/obj

# To run install after a build:
win32 {
  contains(QMAKE_TARGET.arch, x86_64) {
    DLL_DIR = /Windows/system
  } else {
    DLL_DIR = /Windows/system32
  }
} else {
  macx {
    DLL_DIR = /usr/local/lib
  } else {
    # Qt doesn't set QMAKE_TARGET.arch on linux
    # see: http://qt-project.org/faq/answer/how_can_i_detect_in_the_.pro_file_if_i_am_compiling_for_a_32_bit_or_a_64_bi
    *g++:QMAKE_TARGET.arch = $$QMAKE_HOST.arch
    *g++-32:QMAKE_TARGET.arch = x86
    *g++-64:QMAKE_TARGET.arch = x86_64

    contains(QMAKE_TARGET.arch, x86_64) {
      DLL_DIR = /usr/lib64
    } else {
      DLL_DIR = /usr/lib
    }
  }
}
message (Install DLL_DIR is: $$DLL_DIR)

# Use the name from via-httplib.pri
TARGET = $$VIA_HTTPLIB_NAME

target.path = $$DLL_DIR
INSTALLS += target
