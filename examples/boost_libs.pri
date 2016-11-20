# Copyright (c) 2014-2016 Ken Barker
# (ken dot barker at via-technology dot co dot uk)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

# Test whether the BOOST_ROOT environment variable has been set
BOOST_ROOT = $$(BOOST_ROOT)
isEmpty(BOOST_ROOT) {
  error("Please set BOOST_ROOT to the location of the boost libraries")
}

win32 {
  INCLUDEPATH += $$BOOST_ROOT
  LIBS += -L$${BOOST_ROOT}/stage/lib
  # MinGW Libraries
  *-g++* {
    message(MinGW boost libs)

    # Read BOOST_VERSION and MINGW_VERSION environment variables
    BOOST_VERSION = $$(BOOST_VERSION)
    isEmpty(BOOST_VERSION) {
      error("Please set BOOST_VERSION to the version of the boost libraries")
    }
    MINGW_VERSION = $$(MINGW_VERSION)
    isEmpty(MINGW_VERSION) {
      error("Please set MINGW_VERSION to the version of the boost libraries")
    }

    BOOST_LIB_PREFIX = boost_

    CONFIG(release, debug|release){
      BOOST_LIB_SUFFIX = $${MINGW_VERSION}-mt$${BOOST_VERSION}
    }
    CONFIG(debug, debug|release){
      BOOST_LIB_SUFFIX = $${MINGW_VERSION}-mt-d$${BOOST_VERSION}
    }
  }

  # Microsoft Visual Studio Libraries
  *-msvc* {
    message(Visual Studio boost libs)

    MSVC_VERSION = $$(MSVC_VERSION)

    BOOST_LIB_PREFIX = libboost_

    CONFIG(release, debug|release){
      BOOST_LIB_SUFFIX = $${MSVC_VERSION}-mt$${BOOST_VERSION}
    }
    CONFIG(debug, debug|release){
      BOOST_LIB_SUFFIX = $${MSVC_VERSION}-mt-gd$${BOOST_VERSION}
    }

    # The following macro disables autolinking when uncommented
    # DEFINES += BOOST_ALL_NO_LIB
  }
} else { # linux or macx
  BOOST_LIB_PREFIX = boost_
  BOOST_LIB_SUFFIX =

  macx {
    # The following macro disables autolinking when uncommented
    # DEFINES += BOOST_ALL_NO_LIB
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
  }
}

for(lib, BOOST_LIBS) {
  message(link: $${BOOST_LIB_PREFIX}$${lib}$${BOOST_LIB_SUFFIX})
  LIBS += -l$${BOOST_LIB_PREFIX}$${lib}$${BOOST_LIB_SUFFIX}
}
