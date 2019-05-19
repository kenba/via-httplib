#ifndef RESPONSE_STATUS_HPP_VIA_HTTPLIB_
#define RESPONSE_STATUS_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2019 Ken Barker
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
#include <string_view>

namespace via
{
  namespace http
  {
    namespace response_status
    {
      /// The standard HTTP 1.1 response status codes.
      enum class code
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
        PERMANENT_REDIRECT            = 308,

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
        PAYLOAD_TOO_LARGE             = 413,
        REQUEST_ENTITY_TOO_LARGE      = 413, // PAYLOAD_TOO_LARGE before RFC7232
        REQUEST_URI_TOO_LONG          = 414,
        UNSUPPORTED_MEDIA_TYPE        = 415,
        REQUEST_RANGE_NOT_SATISFIABLE = 416,
        EXPECTATION_FAILED            = 417,
        UPGRADE_REQUIRED              = 426,
        PRECONDITION_REQUIRED         = 428,
        TOO_MANY_REQUESTS             = 429,
        REQUEST_HEADER_FIELDS_TOO_LARGE = 431,

        // Server Error 5xx
        INTERNAL_SERVER_ERROR         = 500,
        NOT_IMPLEMENTED               = 501,
        BAD_GATEWAY                   = 502,
        SERVICE_UNAVAILABLE           = 503,
        GATEWAY_TIMEOUT               = 504,
        HTTP_VERSION_NOT_SUPPORTED    = 505,
        NETWORK_AUTHENTICATION_REQUIRED = 511
      };

      // The following strings are for the standard response reason phrases
      // associated with the given response status code.

      // Informational 1xx
      constexpr char REASON_CONTINUE[]            {"Continue"};
      constexpr char REASON_SWITCHING_PROTOCOLS[] {"Switching Protocols"};

      // Successful 2xx
      constexpr char REASON_OK[]                  {"OK"};
      constexpr char REASON_CREATED[]             {"Created"};
      constexpr char REASON_ACCEPTED[]            {"Accepted"};
      constexpr char REASON_NON_AUTHORITATIVE[]   {"Non-Authoritative Information"};
      constexpr char REASON_NO_CONTENT[]          {"No Content"};
      constexpr char REASON_RESET_CONTENT[]       {"Reset Content"};
      constexpr char REASON_PARTIAL_CONTENT[]     {"Partial Content"};


      // Redirection 3xx
      constexpr char REASON_MULTIPLE_CHOICES[]    {"Multiple Choices"};
      constexpr char REASON_MOVED_PERMANENTLY[]   {"Moved Permanently"};
      constexpr char REASON_FOUND[]               {"Found"};
      constexpr char REASON_SEE_OTHER[]           {"See Other"};
      constexpr char REASON_NOT_MODIFIED[]        {"Not Modified"};
      constexpr char REASON_USE_PROXY[]           {"Use Proxy"};
      constexpr char REASON_TEMPORARY_REDIRECT[]  {"Temporary Redirect"};
      constexpr char REASON_PERMANENT_REDIRECT[]  {"Permanent Redirect"};

      // Client Error 4xx
      constexpr char REASON_BAD_REQUEST[]         {"Bad Request"};
      constexpr char REASON_UNAUTHORISED[]        {"Unauthorized"};
      constexpr char REASON_PAYMENT_REQUIRED[]    {"Payment Required"};
      constexpr char REASON_FORBIDDEN[]           {"Forbidden"};
      constexpr char REASON_NOT_FOUND[]           {"Not Found"};
      constexpr char REASON_METHOD_NOT_ALLOWED[]  {"Method Not Allowed"};
      constexpr char REASON_NOT_ACCEPTABLE[]      {"Not Acceptable"};
      constexpr char REASON_PROXY_AUTHENTICATION_REQUIRED[] =
                                                {"Proxy Authentication Required"};
      constexpr char REASON_REQUEST_TIMEOUT[]     {"Request Time-out"};
      constexpr char REASON_CONFLICT[]            {"Conflict"};
      constexpr char REASON_GONE[]                {"Gone"};
      constexpr char REASON_LENGTH_REQUIRED[]     {"Length Required"};
      constexpr char REASON_PRECONDITION_FAILED[] {"Precondition Failed"};
      constexpr char REASON_PAYLOAD_TOO_LARGE[]   {"Payload Too Large"};
      constexpr char REASON_REQUEST_URI_TOO_LONG[] {"Request-URI Too Long"};
      constexpr char REASON_UNSUPPORTED_MEDIA_TYPE[] =
                                                {"Unsupported Media Type"};
      constexpr char REASON_REQUEST_RANGE_NOT_SATISFIABLE[] =
                                                {"Requested range not satisfiable"};
      constexpr char REASON_EXPECTATION_FAILED[]  {"Expectation Failed"};
      constexpr char REASON_UPGRADE_REQUIRED[]    {"Upgrade Required"};
      constexpr char REASON_PRECONDITION_REQUIRED[] =
                                                {"Precondition Required"};
      constexpr char REASON_TOO_MANY_REQUESTS[]   {"Too Many Requests"};
      constexpr char REASON_REQUEST_HEADER_FIELDS_TOO_LARGE[] =
                                                {"Request Header Fields Too Large"};

      // Server Error 5xx
      constexpr char REASON_INTERNAL_SERVER_ERROR[] =
                                                {"Internal Server Error"};
      constexpr char REASON_NOT_IMPLEMENTED[]     {"Not Implemented"};
      constexpr char REASON_BAD_GATEWAY[]         {"Bad Gateway"};
      constexpr char REASON_SERVICE_UNAVAILABLE[] {"Service Unavailable"};
      constexpr char REASON_GATEWAY_TIMEOUT[]     {"Gateway Time-out"};
      constexpr char REASON_HTTP_VERSION_NOT_SUPPORTED[]
                                                {"HTTP Version not supported"};
      constexpr char REASON_NETWORK_AUTHENTICATION_REQUIRED[]
                                                {"Network Authentication Required"};

      /// The standard reason phrase associated with the response status code.
      /// See RFC2616 Section 6.1.1.
      /// @param status_code the reason status code
      /// @return the standard reason phrase associated with the status code.
      constexpr std::string_view reason_phrase(code status_code) noexcept
      {
        switch(status_code)
        {
        // Informational 1xx
        case code::CONTINUE:                      return REASON_CONTINUE;
        case code::SWITCHING_PROTOCOLS:           return REASON_SWITCHING_PROTOCOLS;

        // Successful 2xx
        case code::OK:                            return REASON_OK;
        case code::CREATED:                       return REASON_CREATED;
        case code::ACCEPTED:                      return REASON_ACCEPTED;
        case code::NON_AUTHORITATIVE:             return REASON_NON_AUTHORITATIVE;
        case code::NO_CONTENT:                    return REASON_NO_CONTENT;
        case code::RESET_CONTENT:                 return REASON_RESET_CONTENT;
        case code::PARTIAL_CONTENT:               return REASON_PARTIAL_CONTENT;

        // Redirection 3xx
        case code::MULTIPLE_CHOICES:              return REASON_MULTIPLE_CHOICES;
        case code::MOVED_PERMANENTLY:             return REASON_MOVED_PERMANENTLY;
        case code::FOUND:                         return REASON_FOUND;
        case code::SEE_OTHER:                     return REASON_SEE_OTHER;
        case code::NOT_MODIFIED:                  return REASON_NOT_MODIFIED;
        case code::USE_PROXY:                     return REASON_USE_PROXY;
        case code::TEMPORARY_REDIRECT:            return REASON_TEMPORARY_REDIRECT;
        case code::PERMANENT_REDIRECT:            return REASON_PERMANENT_REDIRECT;

        // Client Error 4xx
        case code::BAD_REQUEST:                   return REASON_BAD_REQUEST;
        case code::UNAUTHORISED:                  return REASON_UNAUTHORISED;
        case code::PAYMENT_REQUIRED:              return REASON_PAYMENT_REQUIRED;
        case code::FORBIDDEN:                     return REASON_FORBIDDEN;
        case code::NOT_FOUND:                     return REASON_NOT_FOUND;
        case code::METHOD_NOT_ALLOWED:            return REASON_METHOD_NOT_ALLOWED;
        case code::NOT_ACCEPTABLE:                return REASON_NOT_ACCEPTABLE;
        case code::PROXY_AUTHENTICATION_REQUIRED:
                                    return REASON_PROXY_AUTHENTICATION_REQUIRED;
        case code::REQUEST_TIMEOUT:               return REASON_REQUEST_TIMEOUT;
        case code::CONFLICT:                      return REASON_CONFLICT;
        case code::GONE:                          return REASON_GONE;
        case code::LENGTH_REQUIRED:               return REASON_LENGTH_REQUIRED;
        case code::PRECONDITION_FAILED:           return REASON_PRECONDITION_FAILED;
        case code::PAYLOAD_TOO_LARGE:             return REASON_PAYLOAD_TOO_LARGE;
        case code::REQUEST_URI_TOO_LONG:          return REASON_REQUEST_URI_TOO_LONG;
        case code::UNSUPPORTED_MEDIA_TYPE:       return REASON_UNSUPPORTED_MEDIA_TYPE;
        case code::REQUEST_RANGE_NOT_SATISFIABLE:
                                    return REASON_REQUEST_RANGE_NOT_SATISFIABLE;
        case code::EXPECTATION_FAILED:            return REASON_EXPECTATION_FAILED;
        case code::UPGRADE_REQUIRED:              return REASON_UPGRADE_REQUIRED;
        case code::PRECONDITION_REQUIRED:         return REASON_PRECONDITION_REQUIRED;
        case code::TOO_MANY_REQUESTS:             return REASON_TOO_MANY_REQUESTS;
        case code::REQUEST_HEADER_FIELDS_TOO_LARGE:
                                                  return REASON_REQUEST_HEADER_FIELDS_TOO_LARGE;

        // Server Error 5xx
        case code::INTERNAL_SERVER_ERROR:         return REASON_INTERNAL_SERVER_ERROR;
        case code::NOT_IMPLEMENTED:               return REASON_NOT_IMPLEMENTED;
        case code::BAD_GATEWAY:                   return REASON_BAD_GATEWAY;
        case code::SERVICE_UNAVAILABLE:           return REASON_SERVICE_UNAVAILABLE;
        case code::GATEWAY_TIMEOUT:               return REASON_GATEWAY_TIMEOUT;
        case code::HTTP_VERSION_NOT_SUPPORTED:
                                       return REASON_HTTP_VERSION_NOT_SUPPORTED;
        case code::NETWORK_AUTHENTICATION_REQUIRED:
                                  return REASON_NETWORK_AUTHENTICATION_REQUIRED;

        // Unknown Error Status Code
        default:                            return std::string_view();
        }
      }

      /// Find whether there's a standard reason phrase for the status_code.
      /// See RFC2616 Section 6.1.1.
      /// @param status_code the reason status code
      /// @return the standard reason phrase associated with the status code.
      constexpr std::string_view reason_phrase(int status_code)
      { return reason_phrase(static_cast<code>(status_code)); }

      /// Whether the response may contain a message body.
      /// See RFC7230 Section 3.3.
      /// @param status_code the reason status code.
      /// @return true if the response may contain a message body,
      /// false otherwise.
      inline bool content_permitted(int status_code) noexcept
      {
        return (status_code >= static_cast<int>(code::OK))
            && (status_code != static_cast<int>(code::NO_CONTENT))
            && (status_code != static_cast<int>(code::NOT_MODIFIED));
      }
    }
  }
}

#endif
