#ifndef REQUEST_METHOD_HPP_VIA_HTTPLIB_
#define REQUEST_METHOD_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
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

#ifdef DELETE
#undef DELETE
#endif

namespace via
{
  namespace http
  {
    namespace request_method
    {
      /// A namespace to mimic C++11 enum class naming.
      namespace id
      {
      /// Ids for the standard methods defined in RFC2616.
      /// They are intended to be used in conjunction with the function
      /// method_name to encode and decode the method from a request.
      enum method
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
      }

      /// The standard method name associated with ids above.
      /// @return the standard method name.
      const std::string& name(id::method method_id) NOEXCEPT;
    }
  }
}

#endif
