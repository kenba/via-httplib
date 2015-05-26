//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file basic.cpp
/// @brief Contains the basic authentication class.
//////////////////////////////////////////////////////////////////////////////
#include "via/http/authentication/basic.hpp"
#include "via/http/authentication/base64.hpp"

namespace
{
  const std::string BASIC("Basic");
  const std::string REALM(" realm=");
  const std::string QUOTE("\"");
  const std::string BASIC_REALM_QUOTE(BASIC + REALM + QUOTE);
}

namespace via
{
  namespace http
  {
    namespace authentication
    {
      ////////////////////////////////////////////////////////////////////////
      bool basic::is_valid(message_headers const& headers) const
      {
        // Does the request contain an AUTHORIZATION header?
        std::string authorization(headers.find(header_field::id::AUTHORIZATION));
        if (authorization.empty())
          return false;

        // Is it Basic?
        auto basic_pos(authorization.find(BASIC));
        if (basic_pos == std::string::npos)
          return false;

        // Strip the BASIC identifier from the string
        basic_pos += BASIC.size() +1;
        authorization = authorization.substr(basic_pos);

        // Decode the authorization value from Base 64
        std::string decoded_authorization(base64::decode(authorization));

        // Split the username from the password
        auto user_end(decoded_authorization.find(':'));
        if (user_end == std::string::npos)
          return false;

        // Search for the username
        std::string username(decoded_authorization.substr(0, user_end));
        auto iter(user_passwords_.find(username));
        if (iter == user_passwords_.cend())
          return false;

        // Test the passowrd
        std::string password(decoded_authorization.substr(user_end +1));
        return (password == iter->second);
      }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string basic::authenticate_value() const
      {
        return realm().empty() ? BASIC
                               : BASIC_REALM_QUOTE + realm() + QUOTE;
      }
      ////////////////////////////////////////////////////////////////////////
    }
  }
}
