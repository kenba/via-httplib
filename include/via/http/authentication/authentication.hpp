#ifndef HTTP_AUTHENTICATION_AUTHENTICATION_HPP_VIA_HTTPLIB_
#define HTTP_AUTHENTICATION_AUTHENTICATION_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2021 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file authentication.hpp
/// @brief Contains the authentication base class.
//////////////////////////////////////////////////////////////////////////////
#include "via/http/request.hpp"
#include <memory>

namespace via
{
  namespace http
  {
    namespace authentication
    {
      ////////////////////////////////////////////////////////////////////////
      /// @class Authentication
      /// The is an abstract base class for authenticating a "realm", see:
      /// https://tools.ietf.org/html/rfc7235
      /// The is_valid and authenticate_value functions are pure virtual and
      /// must be overridden by the derived class to implement a specific
      /// authentication protocol.
      ////////////////////////////////////////////////////////////////////////
      class authentication
      {
        /// The name of this realm (for the authenticate response header);
        std::string realm_;

      protected:

        /// Pure virtual function to authenticate a request
        /// @param header_fields the request message header fields.
        /// @return true if valid, false otherwise.
        virtual bool is_valid(StringMap const& header_fields) const = 0;

        /// The value to be sent in the authenticate response header.
        /// @return the authenticate string.
        virtual std::string authenticate_value() const = 0;

        /// Constructor
        /// @param realm the authentication realm.
        explicit authentication(std::string realm)
          : realm_(std::move(realm))
        {}

      public:

        /// Destructor
        virtual ~authentication()
        {}

        /// The authenticate function.
        /// It calls is_valid to authenticate the request.
        /// If valid it returns an empty string, otherwise it returns the
        /// value for the authenticate response header.
        /// @param request the input request to be authenticated.
        /// @return an empty string if valid or authenticate_value() if invalid.
        template <typename R>
        std::string authenticate(R const& request) const
        {
          if (is_valid(request.headers().fields()))
            return std::string("");
          else
            return authenticate_value();
        }

        /// Accessor for the realm.
        /// @return realm
        const std::string& realm() const
        { return realm_; }
      };
    }
  }
}

#endif // HTTP_AUTHENTICATION_AUTHENTICATION_HPP_VIA_HTTPLIB_

