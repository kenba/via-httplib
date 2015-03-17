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
  # Set the minimum target Windows version to Windows 7
  DEFINES += _WIN32_WINNT=_WIN32_WINNT_WIN7

  # Ensure that the BOOST_ROOT environment variable has been set
  BOOST_ROOT = $$(BOOST_ROOT)
  isEmpty(BOOST_ROOT) {
    error("Please set BOOST_ROOT to the location of the Boost libraries")
  } else {
    message(Using Boost from: $$BOOST_ROOT)
  }

  INCLUDEPATH += $$BOOST_ROOT

  # Set debug options for MinGw
  *-g++* {
    QMAKE_CXXFLAGS_DEBUG = -O0
    QMAKE_CXXFLAGS_DEBUG += -gdwarf-3
  }
}

# Compiler options
*-g++* {
  message(Setting flags for GCC or MinGw)
  QMAKE_CXXFLAGS += -std=c++11

  QMAKE_CXXFLAGS += -Wall
  QMAKE_CXXFLAGS += -Wextra
  QMAKE_CXXFLAGS += -Wpedantic
}

*-clang* {
  message(Setting flags for clang)
  QMAKE_CXXFLAGS += -std=c++11
  QMAKE_CXXFLAGS += -Wno-c++11-extensions
  QMAKE_CXXFLAGS += -stdlib=libc++
}

TEMPLATE = lib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += create_prl

INCLUDEPATH += $$VIAHTTPLIB

HEADERS += $${VIAHTTPLIB}/via/http/*.hpp
HEADERS += $${VIAHTTPLIB}/via/comms/*.hpp
HEADERS += $${VIAHTTPLIB}/via/comms/ssl/*.hpp
HEADERS += $${VIAHTTPLIB}/via/*.hpp

SOURCES += $${VIAHTTPLIB}/via/http/character.cpp
SOURCES += $${VIAHTTPLIB}/via/http/chunk.cpp
SOURCES += $${VIAHTTPLIB}/via/http/header_field.cpp
SOURCES += $${VIAHTTPLIB}/via/http/headers.cpp
SOURCES += $${VIAHTTPLIB}/via/http/request_method.cpp
SOURCES += $${VIAHTTPLIB}/via/http/request.cpp
SOURCES += $${VIAHTTPLIB}/via/http/response_status.cpp
SOURCES += $${VIAHTTPLIB}/via/http/response.cpp

