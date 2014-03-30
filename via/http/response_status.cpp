//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "response_status.hpp"
#include "character.hpp"

namespace
{
  // The following strings are for the standard response reason phrases
  // associated with the given response status code.

  const std::string REASONS[] =
  {
    // Empty string
    {""},

    // Informational 1xx
    {"Continue"},
    {"Switching Protocols"},

    // Successful 2xx
    {"OK"},
    {"Created"},
    {"Accepted"},
    {"Non-Authoritative Information"},
    {"No Content"},
    {"Reset Content"},
    {"Partial Content"},

    // Redirection 3xx
    {"Multiple Choices"},
    {"Moved Permanently"},
    {"Found"},
    {"See Other"},
    {"Not Modified"},
    {"Use Proxy"},
    {"Temporary Redirect"},

    // Client Error 4xx
    {"Bad Request"},
    {"Unauthorized"},
    {"Payment Required"},
    {"Forbidden"},
    {"Not Found"},
    {"Method Not Allowed"},
    {"Not Acceptable"},
    {"Proxy Authentication Required"},
    {"Request Time-out"},
    {"Conflict"},
    {"Gone"},
    {"Length Required"},
    {"Precondition Failed"},
    {"Request Entity Too Large"},
    {"Request-URI Too Large"},
    {"Unsupported Media Type"},
    {"Requested range not satisfiable"},
    {"Expectation Failed"},

    // Server Error 5xx
    {"Internal Server Error"},
    {"Not Implemented"},
    {"Bad Gateway"},
    {"Service Unavailable"},
    {"Gateway Time-out"},
    {"HTTP Version not supported"}
  };
}

namespace via
{
  namespace http
  {
    namespace response_status
    {
    //////////////////////////////////////////////////////////////////////////
    const std::string& reason_phrase(status_code status)
    {
      int index{0};

      switch(status)
      {
      // Informational 1xx
      case CONTINUE:                      index =  1; break;
      case SWITCHING_PROTOCOLS:           index =  2; break;

      // Successful 2xx
      case OK:                            index =  3; break;
      case CREATED:                       index =  4; break;
      case ACCEPTED:                      index =  5; break;
      case NON_AUTHORITATIVE:             index =  6; break;
      case NO_CONTENT:                    index =  7; break;
      case RESET_CONTENT:                 index =  8; break;
      case PARTIAL_CONTENT:               index =  9; break;

      // Redirection 3xx
      case MULTIPLE_CHOICES:              index = 10; break;
      case MOVED_PERMANENTLY:             index = 11; break;
      case FOUND:                         index = 12; break;
      case SEE_OTHER:                     index = 13; break;
      case NOT_MODIFIED:                  index = 14; break;
      case USE_PROXY:                     index = 15; break;
      case TEMPORARY_REDIRECT:            index = 16; break;

      // Client Error 4xx
      case BAD_REQUEST:                   index = 17; break;
      case UNAUTHORISED:                  index = 18; break;
      case PAYMENT_REQUIRED:              index = 19; break;
      case FORBIDDEN:                     index = 20; break;
      case NOT_FOUND:                     index = 21; break;
      case METHOD_NOT_ALLOWED:            index = 22; break;
      case NOT_ACCEPTABLE:                index = 23; break;
      case PROXY_AUTHENTICATION_REQUIRED: index = 24; break;
      case REQUEST_TIMEOUT:               index = 25; break;
      case CONFLICT:                      index = 26; break;
      case GONE:                          index = 27; break;
      case LENGTH_REQUIRED:               index = 28; break;
      case PRECONDITION_FAILED:           index = 29; break;
      case REQUEST_ENTITY_TOO_LARGE:      index = 30; break;
      case REQUEST_URI_TOO_LONG:          index = 31; break;
      case UNSUPPORTED_MEDIA_TYPE:        index = 32; break;
      case REQUEST_RANGE_NOT_SATISFIABLE: index = 33; break;
      case EXPECTATION_FAILED:            index = 34; break;

      // Server Error 5xx
      case INTERNAL_SERVER_ERROR:         index = 35; break;
      case NOT_IMPLEMENTED:               index = 36; break;
      case BAD_GATEWAY:                   index = 37; break;
      case SERVICE_UNAVAILABLE:           index = 38; break;
      case GATEWAY_TIMEOUT:               index = 39; break;
      case HTTP_VERSION_NOT_SUPPORTED:    index = 40; break;

      // Unknown Error Status Code
      default:                            index =  0;
      }

      return REASONS[index];
    }
    //////////////////////////////////////////////////////////////////////////

    }
  }
}
