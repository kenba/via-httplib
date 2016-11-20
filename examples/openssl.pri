# Copyright (c) 2014-2016 Ken Barker
# (ken dot barker at via-technology dot co dot uk)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

# Check OPENSSL_ROOT has been set
# Note: isEmpty doesn't work on environ var directly so put it in a local first
OPENSSL_ROOT = $$(OPENSSL_ROOT)
isEmpty(OPENSSL_ROOT) {
  message("The OPENSSL_ROOT environment variable has not been set, please set it to the location of the OpenSsl libraries")
} else {
  message(Using OpenSsl from: $${OPENSSL_ROOT})

  INCLUDEPATH += $${OPENSSL_ROOT}/include

  win32 {
    # Check whether using visual studio compiler
    *-msvc* {
      LIBS += -L$${OPENSSL_ROOT}/lib

      CONFIG(release, debug|release){
        LIBS += -llibeay32
        LIBS += -llibssleay32
      }

      CONFIG(debug, debug|release){
        LIBS += -llibeay32d
        LIBS += -llibssleay32d
      }
    }

    # Check whether using mingw compiler
    *-g++* {
      LIBS += -L$${OPENSSL_ROOT}/lib/mingw

      CONFIG(release, debug|release){
        LIBS += -lssl
        LIBS += -lcrypto
      }

      CONFIG(debug, debug|release){
        LIBS += -lssl
        LIBS += -lcrypto
      }
    }
  }
  else {
    # assume linux
    LIBS += -L$${OPENSSL_ROOT}/lib

    LIBS += -lssl
    LIBS += -lcrypto
    LIBS += -ldl
  }
}
