# Copyright (c) 2013-2015 Ken Barker
# (ken dot barker at via-technology dot co dot uk)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

win32 {
  # Set the minimum target Windows version to Windows 7
  DEFINES *= _WIN32_WINNT=_WIN32_WINNT_WIN7

  INCLUDEPATH *= $$BOOST_ROOT
}

INC_DIR = $$VIAHTTPLIB/include

INCLUDEPATH *= $${INC_DIR}

HEADERS += $${INC_DIR}/via/*.hpp
HEADERS += $${INC_DIR}/via/http/*.hpp
HEADERS += $${INC_DIR}/via/comms/*.hpp
HEADERS += $${INC_DIR}/via/comms/ssl/*.hpp

release: LIBS += -L$${VIAHTTPLIB}/release
debug:   LIBS += -L$${VIAHTTPLIB}/debug

# Ensure that the dubug library has a different name
VIA_HTTPLIB_NAME = via-httplib
CONFIG(debug, debug|release) {
  win32 {
   VIA_HTTPLIB_NAME = $$join(VIA_HTTPLIB_NAME,,,d)
  } else {
   VIA_HTTPLIB_NAME = $$join(VIA_HTTPLIB_NAME,,,_debug)
  }
}
