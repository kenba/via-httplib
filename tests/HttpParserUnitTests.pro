# Copyright (c) 2014-2016 Ken Barker
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
CONFIG += c++14
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

BOOST_LIBS = unit_test_framework
include($$VIAHTTPLIB/examples/boost_libs.pri)

include($$VIAHTTPLIB/via-httplib.pri)
LIBS += -l$$VIA_HTTPLIB_NAME

SOURCES += $$VIAHTTPLIB/tests/test_main.cpp \
  $$VIAHTTPLIB/tests/http/test_character.cpp \
  $$VIAHTTPLIB/tests/http/test_chunk.cpp \
  $$VIAHTTPLIB/tests/http/test_header_field.cpp \
  $$VIAHTTPLIB/tests/http/test_headers.cpp \
  $$VIAHTTPLIB/tests/http/test_request.cpp \
  $$VIAHTTPLIB/tests/http/test_response.cpp
