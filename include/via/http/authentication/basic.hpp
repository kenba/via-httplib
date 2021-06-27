#ifndef HTTP_AUTHENTICATION_BASIC_HPP_VIA_HTTPLIB_
#define HTTP_AUTHENTICATION_BASIC_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015-2021 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file basic.hpp
/// @brief Contains the basic authentication class.
//////////////////////////////////////////////////////////////////////////////
#include "authentication.hpp"
#include "base64.hpp"

namespace via
{
  namespace http
  {
    namespace authentication
    {

      constexpr char BASIC[]{"Basic"};
      constexpr char REALM[]{" realm="};
      constexpr char QUOTE[]{"\""};

      /// @class basic
      /// This class implements HTTP basic authentication, see:
      /// https://www.ietf.org/rfc/rfc2617.txt &
      /// https://tools.ietf.org/html/rfc7235
      class basic : public authentication
      {
        /// The map of users and passwords.
        StringMap user_passwords_;

      protected:

        /// Function to authenticate a request.
        /// @param header_fields the request message header fields.
        /// @return true if valid, false otherwise.
        virtual bool is_valid(StringMap const& header_fields) const override
        {
          // Does the request contain an AUTHORIZATION header?
          const auto h_iter(header_fields.find(header_field::LC_AUTHORIZATION));
          if (h_iter == header_fields.end())
            return false;

          std::string authorization(h_iter->second);

          // Is it Basic?
          auto basic_pos(authorization.find(BASIC));
          if (basic_pos == std::string::npos)
            return false;

          // Strip the BASIC identifier from the string
          basic_pos += 6;
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

        /// The value to be sent in the authenticate response header.
        /// @return the authenticate string.
        virtual std::string authenticate_value() const override
        {
          return realm().empty() ? BASIC :
                         std::string(BASIC) + REALM + QUOTE + realm() + QUOTE;
        }

      public:

        /// Constructor
        /// @param realm the authentication realm, default emopty.
        explicit basic(std::string realm = "")
          : authentication(std::move(realm))
          , user_passwords_()
        {}

        /// Destructor
        /// clears the user_passwords_ collection.
        virtual ~basic() override
        { user_passwords_.clear(); }

        /// Add a user and password to the user_passwords_ collection.
        void add_user(std::string user, std::string password)
        {
          user_passwords_.insert(StringMap::value_type
                                 (std::move(user), std::move(password)));
        }

        /// Accessor for the user_passwords_ collection.
        StringMap const& user_passwords() const
        { return user_passwords_; }
      };
    }
  }
}

#endif // HTTP_AUTHENTICATION_BASIC_HPP_VIA_HTTPLIB_
