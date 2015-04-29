//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/cookie.hpp"
#include "via/http/character.hpp"
#include <algorithm>
#include <functional>

namespace via
{
  namespace http
  {
    namespace
    {
      std::hash<std::string> hash_;

      const std::size_t domain_hash = hash_("domain");
      const std::size_t path_hash = hash_("path");
      const std::size_t expires_hash = hash_("expires");
      const std::size_t max_age_hash = hash_("max-age");
      const std::size_t secure_hash = hash_("secure");
      const std::size_t http_only_hash = hash_("httponly");
    }

    bool cookie::parse_attr(std::string& name, std::string& value)
    {
      const std::size_t name_hash = hash_(name);
      if (name_hash == domain_hash)
      {
        domain_.resize(name.size());
        // lambda is required to build under gcc
        std::transform(name.cbegin(), name.cend(), domain_.begin(), [](int c){ return std::tolower(c); });
      }
      else if (name_hash == path_hash)
      {
        path_.swap(value);
      }
      else if (name_hash == expires_hash)
      {
        if (expires_ == time_point::max())
        {
          auto utime = time_from_string(value, "%a, %d-%b-%Y %T");
          if (utime == -1)
            return false;

          expires_ = time_point::clock::from_time_t(utime);
        }
      }
      else if (name_hash == max_age_hash)
      {
        expires_ = time_point::clock::now() + std::chrono::seconds(from_dec_string(value));
      }
      else if (name_hash == secure_hash)
      {
        secure_ = true;
      }
      else if (name_hash == http_only_hash)
      {
        http_only_ = true;
      }
      return true;
    }

    bool cookie::parse_char(char c, std::string& name, std::string& value)
    {
      std::string* name_ptr = &name;
      std::string* value_ptr = &value;
      auto value_state = COOKIE_ATTR_VALUE;

      switch (state_)
      {
        case COOKIE_NAME:
          name_ptr = &name_;
          value_state = COOKIE_VALUE;
        case COOKIE_ATTR_NAME:
          if (std::isalpha(c) || ('-' == c))
            name_ptr->push_back(static_cast<char>(std::tolower(c)));
          else if ('=' == c)
            state_ = value_state;
          else if (!is_space_or_tab(c))
            return false;
          break;
        case COOKIE_VALUE:
          value_ptr = &value_;
        case COOKIE_ATTR_VALUE:
          if (';' == c && state_ == COOKIE_ATTR_VALUE)
          {
            parse_attr(name, value);
            state_ = COOKIE_ATTR_NAME;
          }
          else if (!is_space_or_tab(c))
            value_ptr->push_back(c);
          break;
        default:
          return false;
      }
      return true;
    }

    std::string cookie::dump() const
    {
      std::string result;
      result.append(name_).append("=").append(value_);
      if (expires_ != time_point::max())
        result.append("; expires=")
          .append(time_to_string(time_point::clock::to_time_t(expires_), "%a, %d-%b-%Y %T"));
      if (!path_.empty())
        result.append("; path=").append(path_);
      if (!domain_.empty())
        result.append("; domain=").append(domain_);
      if (secure_)
        result.append("; secure");
      if (http_only_)
        result.append("; httponly");
      return result;
    }
  }
}

