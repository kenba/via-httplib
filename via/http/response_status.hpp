#pragma once

#ifndef RESPONSE_STATUS_HPP_VIA_HTTPLIB_
#define RESPONSE_STATUS_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file response_status.hpp
/// @brief Enumerations and functions to handle HTTP response statuses.
//////////////////////////////////////////////////////////////////////////////
#include <string>

namespace via
{
  namespace http
  {
    namespace response_status
    {
      /// The standard HTTP 1.1 response status codes.
      enum status_code
      {
        // Informational 1xx
        CONTINUE                      = 100,
        SWITCHING_PROTOCOLS           = 101,

        // Successful 2xx
        OK                            = 200,
        CREATED                       = 201,
        ACCEPTED                      = 202,
        NON_AUTHORITATIVE             = 203,
        NO_CONTENT                    = 204,
        RESET_CONTENT                 = 205,
        PARTIAL_CONTENT               = 206,

        // Redirection 3xx
        MULTIPLE_CHOICES              = 300,
        MOVED_PERMANENTLY             = 301,
        FOUND                         = 302,
        SEE_OTHER                     = 303,
        NOT_MODIFIED                  = 304,
        USE_PROXY                     = 305,
        TEMPORARY_REDIRECT            = 307,

        // Client Error 4xx
        BAD_REQUEST                   = 400,
        UNAUTHORISED                  = 401,
        PAYMENT_REQUIRED              = 402,
        FORBIDDEN                     = 403,
        NOT_FOUND                     = 404,
        METHOD_NOT_ALLOWED            = 405,
        NOT_ACCEPTABLE                = 406,
        PROXY_AUTHENTICATION_REQUIRED = 407,
        REQUEST_TIMEOUT               = 408,
        CONFLICT                      = 409,
        GONE                          = 410,
        LENGTH_REQUIRED               = 411,
        PRECONDITION_FAILED           = 412,
        REQUEST_ENTITY_TOO_LARGE      = 413,
        REQUEST_URI_TOO_LONG          = 414,
        UNSUPPORTED_MEDIA_TYPE        = 415,
        REQUEST_RANGE_NOT_SATISFIABLE = 416,
        EXPECTATION_FAILED            = 417,

        // Server Error 5xx
        INTERNAL_SERVER_ERROR         = 500,
        NOT_IMPLEMENTED               = 501,
        BAD_GATEWAY                   = 502,
        SERVICE_UNAVAILABLE           = 503,
        GATEWAY_TIMEOUT               = 504,
        HTTP_VERSION_NOT_SUPPORTED    = 505
      };

      /// The standard reason phrase associated with the response status code.
      /// See RFC2616 Section 6.1.1.
      /// @param status the reason status code
      /// @return the standard reason phrase associated with the status code.
      const std::string& reason_phrase(status_code status);

    }
  }
}

#endif
