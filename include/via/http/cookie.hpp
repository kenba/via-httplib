#ifndef HEADER_COOKIE_HPP_VIA_HTTPLIB_
#define HEADER_COOKIE_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file cookie.hpp
/// @brief Enumerations and functions to handle HTTP cookies
//////////////////////////////////////////////////////////////////////////////

#include "via/no_except.hpp"
#include <string>
#include <functional>
#include <ctime>

namespace via
{
  namespace http
  {
    class cookie
    {
    public:
      /// @enum parsing_state the state of the cookie line parser
      enum parsing_state
      {
        COOKIE_NAME,     ///< the cookie name
        COOKIE_VALUE,    ///< the cookie value
        COOKIE_ATTR_NAME, ///< the cookie option name (expires, domain, ...)
        COOKIE_ATTR_VALUE, ///< the cookie option value
        COOKIE_ATTR_END,   ///< the cookie option end
      };

    private:
      std::string   name_;  ///< the cookie name
      std::string   value_; ///< the cookie value
      std::string   domain_; ///< the cookie domain
      std::string   path_; ///< the cookie path
      std::time_t   expires_; ///< the cookie expiration time
      bool          secure_; ///< the cookie is secure
      bool          http_only_; ///< the cookie is http only
      parsing_state state_; ///< the current parsing state

      /// Parse an the option
      /// @param name the option name
      /// @param name the option value
      /// @retval true if success otherwise false
      bool parse_attr(std::string& name, std::string& value);

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @retval state the current state of the parser.
      bool parse_char(char c, std::string& name, std::string& value);
    public:

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit cookie() :
        path_("/"),
        expires_(-1),
        secure_(false),
        http_only_(false),
        state_{ COOKIE_NAME }
      {}

      cookie(const std::string& name,
             const std::string& value,
             const std::string& domain="",
             const std::string& path="/",
             std::time_t expires=-1,
             bool secure=false,
             bool httponly=false) :
        name_(name),
        value_(value),
        domain_(domain),
        path_(path),
        expires_(expires),
        secure_(secure),
        http_only_(httponly),
        state_{ COOKIE_NAME }
      {}

      /// clear the cookie_line.
      /// Sets all member variables to their initial state.
      void clear() NOEXCEPT
      {
        name_.clear();
        value_.clear();
        domain_.clear();
        path_.clear();
        expires_ = -1;
        secure_ = false;
        http_only_ = false;
        state_ = COOKIE_NAME;
      }

      /// swap member variables with another cookie_line.
      /// @param other the other cookie_line
      void swap(cookie & other) NOEXCEPT
      {
        name_.swap(other.name_);
        value_.swap(other.value_);
        domain_.swap(other.domain_);
        path_.swap(other.path_);
        std::swap(expires_, other.expires_);
        std::swap(expires_, other.expires_);
        std::swap(secure_, other.secure_);
        std::swap(http_only_, other.http_only_);
        std::swap(state_, other.state_);
      }

      /// Parse an individual http cookie  and extract the parameters
      /// If valid it will refer to the next char of data to be read.
      /// @param s the set-cookie header value
      /// @return true if a valid HTTP header, false otherwise.
      bool parse(const std::string& s)
      {
        std::string name, value;
        for (auto begin = s.cbegin(), end = s.cend(); begin != end; ++begin)
        {
          if (!parse_char(*begin, name, value))
            return false;
        }
        if (state_ == COOKIE_ATTR_VALUE && !parse_attr(name, value))
          return false;
        return !(state_ == COOKIE_ATTR_NAME && !name.empty());
      }

      /// Accessor for the cookie name.
      /// @return the cookie name
      const std::string& name() const NOEXCEPT
      { return name_; }

      /// Accessor for the cookie value.
      /// @return the cookie value in the same case that it was received in.
      const std::string& value() const NOEXCEPT
      { return value_; }

      /// Accessor for the cookie domain.
      /// @return the cookie domain in the same case that it was received in.
      const std::string& domain() const NOEXCEPT
      { return domain_; }

      /// Accessor for the cookie path.
      /// @return the cookie value in the same case that it was received in.
      const std::string& path() const NOEXCEPT
      { return path_; }

      /// Accessor for the cookie expiration time.
      /// @return the true if cookie is expired otherwise false
      bool expired() const NOEXCEPT
      { return expires_ != -1 && expires_ < time(0); }

      /// Accessor for the cookie secure attribute.
      /// @return the true if cookie is secure otherwise false
      bool is_secure() const NOEXCEPT
      { return secure_; }

      /// Accessor for the cookie httponly attribute.
      /// @return the true if cookie is http-only otherwise false
      bool is_http_only() const NOEXCEPT
      { return http_only_; }

      /// Output the cookie as a string.
      /// @return a serialized cookie.
      std::string to_string() const NOEXCEPT
      { return name_ + "=" + value_; }

      /// Dump cookie
      /// @return a serialized cookie.
      std::string dump() const;
    }; // class cookie
  }
}

namespace std
{
  template <>
  struct hash<via::http::cookie> : private hash<std::string>
  {
    std::size_t operator()(const via::http::cookie& k) const
    {
      return hash<std::string>::operator()(k.name());
    }
  };

  template<>
  struct equal_to<via::http::cookie> : private equal_to<std::string>
  {
    bool operator()(const via::http::cookie& l, const via::http::cookie& r)
    {
      return equal_to<std::string>::operator()(l.name(), r.name());
    }
  };

  template<>
  struct less<via::http::cookie> : private less<std::string>
  {
    bool operator()(const via::http::cookie& l, const via::http::cookie& r) const
    {
      return less<std::string>::operator()(l.name(), r.name());
    }
  };
}



#endif
