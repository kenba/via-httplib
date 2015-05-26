#ifndef HTTP_AUTHENTICATION_BASE64_HPP_VIA_HTTPLIB_
#define HTTP_AUTHENTICATION_BASE64_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file base64.hpp
/// @brief Contains the base64 encoder and decoder.
//////////////////////////////////////////////////////////////////////////////
#include <string>

namespace via
{
  namespace http
  {
    namespace authentication
    {
      namespace base64
      {
        /// Encode a string into Base64 format.
        /// @param input the string to encode
        /// @return the string encoded into base64 format.
        std::string encode(std::string input);

        /// Decode a string from Base64 format.
        /// @param input the string to decode
        /// @return the decoded string.
        std::string decode(std::string input);
      }
    }
  }
}

#endif // HTTP_AUTHENTICATION_BASE64_HPP_VIA_HTTPLIB_
