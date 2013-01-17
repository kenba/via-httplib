//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
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
  const std::string	METHOD_OPTIONS ("OPTIONS");
  const std::string	METHOD_GET     ("GET");
  const std::string	METHOD_HEAD    ("HEAD");
  const std::string	METHOD_POST    ("POST");
  const std::string	METHOD_PUT     ("PUT");
  const std::string	METHOD_DELETE  ("DELETE");
  const std::string	METHOD_TRACE   ("TRACE");
  const std::string	METHOD_CONNECT ("CONNECT");

  const std::string EMPTY_STRING("");
}

namespace via
{
  namespace http
  {
    namespace request_method
    {

      //////////////////////////////////////////////////////////////////////////
      const std::string& name(method_id id)
      {
        switch(id)
        {
        case OPTIONS:                       return METHOD_OPTIONS;
        case GET:                           return METHOD_GET;
        case HEAD:                          return METHOD_HEAD;
        case POST:                          return METHOD_POST;
        case PUT:                           return METHOD_PUT;
        case DELETE:                        return METHOD_DELETE;
        case TRACE:                         return METHOD_TRACE;
        case CONNECT:                       return METHOD_CONNECT;

        // Unknown method id
        default:                            return EMPTY_STRING;
        }
        return EMPTY_STRING;
      }
      //////////////////////////////////////////////////////////////////////////

    }
  }
}
