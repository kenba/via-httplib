#ifndef HTTP_AUTHENTICATION_BASIC_HPP_VIA_HTTPLIB_
#define HTTP_AUTHENTICATION_BASIC_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015 Ken Barker
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

namespace via
{
  namespace http
  {
    namespace authentication
    {
      /// @class basic
      /// This class implements HTTP basic authentication, see:
      /// https://www.ietf.org/rfc/rfc2617.txt &
      /// https://tools.ietf.org/html/rfc7235
      class basic : public authentication
      {
      public:

        /// A map of strings for username/password lookup
        typedef std::unordered_map<std::string, std::string> UserPasswords;

      private:

        /// The map of users and passwords.
        UserPasswords user_passwords_;

      protected:

        /// Function to authenticate a request.
        /// @param headers the request message_headers.
        /// @return true if valid, false otherwise.
        virtual bool is_valid(message_headers const& headers) const override;

        /// The value to be sent in the authenticate response header.
        /// @return the authenticate string.
        virtual std::string authenticate_value() const override;

      public:

        /// Constructor
        /// @param realm the authentication realm, default emopty.
        explicit basic(std::string realm = "")
          : authentication(std::move(realm))
          , user_passwords_()
        {}

        /// Destructor
        /// clears the user_passwords_ collection.
        virtual ~basic()
        { user_passwords_.clear(); }

        /// Add a user and password to the user_passwords_ collection.
        void add_user(std::string user, std::string password)
        {
          user_passwords_.insert(UserPasswords::value_type
                                 (std::move(user), std::move(password)));
        }

        /// Accessor for the user_passwords_ collection.
        UserPasswords const& user_passwords() const
        { return user_passwords_; }
      };
    }
  }
}

#endif // HTTP_AUTHENTICATION_BASIC_HPP_VIA_HTTPLIB_
