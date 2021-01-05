# Copyright (c) 2014-2021 Ken Barker
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

    # Read boost library version environment variables
    Boost_LIB_VERSION = $$(Boost_LIB_VERSION)
    isEmpty(Boost_LIB_VERSION) {
      error("Please set Boost_LIB_VERSION to the version string of the boost libraries")
    }

    Boost_ARCHITECTURE = $$(Boost_ARCHITECTURE)
    isEmpty(Boost_ARCHITECTURE) {
      error("Please set Boost_ARCHITECTURE to: -x64 or -x32")
    }

    MINGW_VERSION = $$(MINGW_VERSION)
    isEmpty(MINGW_VERSION) {
      error("Please set MINGW_VERSION to the version of the MinGW compiler")
    }

    BOOST_LIB_PREFIX = boost_

    CONFIG(release, debug|release){
      BOOST_LIB_SUFFIX = $${MINGW_VERSION}-mt$${Boost_ARCHITECTURE}$${Boost_LIB_VERSION}
    }
    CONFIG(debug, debug|release){
      BOOST_LIB_SUFFIX = $${MINGW_VERSION}-mt-d$${Boost_ARCHITECTURE}$${Boost_LIB_VERSION}
    }
  }

  # Microsoft Visual Studio Libraries
  *-msvc* {
    message(Visual Studio boost libs)

    MSVC_VERSION = $$(MSVC_VERSION)

    BOOST_LIB_PREFIX = libboost_

    CONFIG(release, debug|release){
      BOOST_LIB_SUFFIX = $${MSVC_VERSION}-mt$${Boost_LIB_VERSION}
    }
    CONFIG(debug, debug|release){
      BOOST_LIB_SUFFIX = $${MSVC_VERSION}-mt-gd$${Boost_LIB_VERSION}
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
