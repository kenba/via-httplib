//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "request_method.hpp"
#include "character.hpp"

namespace
{
  // The following strings are for the standard request methods defined in 
  // RFC2616 section 9.
  const std::string METHODS[] =
  {
    {"OPTIONS"},
    {"GET"},
    {"HEAD"},
    {"POST"},
    {"PUT"},
    {"DELETE"},
    {"TRACE"},
    {"CONNECT"}
  };
}

namespace via
{
  namespace http
  {
    namespace request_method
    {
      //////////////////////////////////////////////////////////////////////////
      const std::string& name(method_id id)
      { return METHODS[static_cast<int>(id)]; }
      //////////////////////////////////////////////////////////////////////////
    }
  }
}
