//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2018 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/request_method.hpp"
#include "via/http/character.hpp"

namespace
{
  // The following strings are for the standard request methods defined in
  // RFC2616 section 9.
  const char METHOD_OPTIONS[] = {"OPTIONS"};
  const char METHOD_GET[] =     {"GET"};
  const char METHOD_HEAD[]=     {"HEAD"};
  const char METHOD_POST[] =    {"POST"};
  const char METHOD_PUT[] =     {"PUT"};
  const char METHOD_DELETE[] =  {"DELETE"};
  const char METHOD_TRACE[] =   {"TRACE"};
  const char METHOD_CONNECT[] = {"CONNECT"};

  const char EMPTY_STRING[] = {""};
}

namespace via
{
  namespace http
  {
    namespace request_method
    {

      //////////////////////////////////////////////////////////////////////////
      const std::string_view name(id method_id) noexcept
      {
        switch(method_id)
        {
        case id::OPTIONS:                       return METHOD_OPTIONS;
        case id::GET:                           return METHOD_GET;
        case id::HEAD:                          return METHOD_HEAD;
        case id::POST:                          return METHOD_POST;
        case id::PUT:                           return METHOD_PUT;
        case id::DELETE:                        return METHOD_DELETE;
        case id::TRACE:                         return METHOD_TRACE;
        case id::CONNECT:                       return METHOD_CONNECT;

        // Unknown method id
        default:                                return EMPTY_STRING;
        }
      }
      //////////////////////////////////////////////////////////////////////////

    }
  }
}
