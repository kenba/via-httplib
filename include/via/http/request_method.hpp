#ifndef REQUEST_METHOD_HPP_VIA_HTTPLIB_
#define REQUEST_METHOD_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2019 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file request_method.hpp
/// @brief Enumerations and functions to handle standard HTTP request methods.
//////////////////////////////////////////////////////////////////////////////
#include "character.hpp"
#include <string>
#include <string_view>

#ifdef DELETE
#undef DELETE
#endif

namespace via
{
  namespace http
  {
    namespace request_method
    {
      // The following strings are for the standard request methods defined in
      // RFC2616 section 9.
      constexpr char OPTIONS[] {"OPTIONS"};
      constexpr char GET[]     {"GET"};
      constexpr char HEAD[]    {"HEAD"};
      constexpr char POST[]    {"POST"};
      constexpr char PUT[]     {"PUT"};
      constexpr char DELETE[]  {"DELETE"};
      constexpr char TRACE[]   {"TRACE"};
      constexpr char CONNECT[] {"CONNECT"};

      /// Ids for the standard methods defined in RFC2616.
      /// They are intended to be used in conjunction with the function
      /// method_name to encode and decode the method from a request.
      enum class id
      {
        OPTIONS,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        TRACE,
        CONNECT
      };

      /// The standard method name associated with ids above.
      /// @return the standard method name.
      inline const std::string_view name(id method_id) noexcept
      {
        switch(method_id)
        {
        case id::OPTIONS: return request_method::OPTIONS;
        case id::GET:     return request_method::GET;
        case id::HEAD:    return request_method::HEAD;
        case id::POST:    return request_method::POST;
        case id::PUT:     return request_method::PUT;
        case id::DELETE:  return request_method::DELETE;
        case id::TRACE:   return request_method::TRACE;
        case id::CONNECT: return request_method::CONNECT;

        // Unknown method id
        default:          return std::string_view();
        }
      }
    }
  }
}

#endif
