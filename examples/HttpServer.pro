# Copyright (c) 2014-2018 Ken Barker
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

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++17
CONFIG += debug

# output directories

# message(Output to: $$OUT_PWD)
CONFIG(debug, debug|release) {
  DESTDIR = $${OUT_PWD}/debug
} else {
  DESTDIR = $${OUT_PWD}/release
}

# Tidy the output of intermediate files, e.g. object files
OBJECTS_DIR = $${DESTDIR}/obj

# Compiler options
*-g++* {
  message(Setting flags for GCC or MinGw)

  QMAKE_CXXFLAGS += -Wall
  QMAKE_CXXFLAGS += -Wextra
  QMAKE_CXXFLAGS += -Wpedantic
  QMAKE_CXXFLAGS += -Wno-type-limits

  win32 {
    # Set debug options for MinGw in QtCreator
    QMAKE_CXXFLAGS_DEBUG = -g -O0
  }
}

*-clang* {
  message(Setting flags for clang)
  QMAKE_CXXFLAGS += -stdlib=libc++
}

*-msvc* {
  message(Setting flags for Visual Studio)
  QMAKE_CXXFLAGS_WARN_ON = ""
  QMAKE_CXXFLAGS += /W4
}

# Test whether the ASIO_ROOT environment variable has been set
ASIO_ROOT = $$(ASIO_ROOT)
isEmpty(ASIO_ROOT) {
  BOOST_LIBS = system
  include(boost_libs.pri)
}

include(openssl.pri)

include($${VIAHTTPLIB}/via-httplib.pri)
LIBS += -l$$VIA_HTTPLIB_NAME

win32 {
  # Set the minimum target Windows version to Windows 7
  # See: http://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx
  DEFINES += NTDDI_VERSION=NTDDI_WIN7
  DEFINES += _WIN32_WINNT=_WIN32_WINNT_WIN7

  LIBS += -lgdi32
  LIBS += -lwsock32
  LIBS += -lws2_32
} else {
  # assume macx or linux

  LIBS += -lpthread
}

HEADERS += \
  $${VIAHTTPLIB}/examples/cacert.hpp \
  $${VIAHTTPLIB}/examples/privkey.hpp

SOURCES += \
  $${VIAHTTPLIB}/examples/cacert.cpp \
  $${VIAHTTPLIB}/examples/privkey.cpp \
  $${VIAHTTPLIB}/examples/server/routing_http_server.cpp

#  $${VIAHTTPLIB}/examples/server/example_http_server.cpp
#  $${VIAHTTPLIB}/examples/server/chunked_http_server.cpp
#  $${VIAHTTPLIB}/examples/server/example_https_server.cpp
#  $${VIAHTTPLIB}/examples/server/simple_http_server.cpp
#  $${VIAHTTPLIB}/examples/server/simple_https_server.cpp
#  $${VIAHTTPLIB}/examples/server/thread_pool_http_server.cpp
