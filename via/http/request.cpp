//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "request.hpp"
#include "character.hpp"

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    bool request_line::parse_char(char c)
    {


      switch (state_)
      {
      case REQ_METHOD:
        // Valid HTTP methods must be uppercase chars
        if (std::isupper(c))
          method_.push_back(c);
        // If this char is whitespace and method has been read
        else if (is_space_or_tab(c) && !method_.empty())
          state_ = REQ_URI;
        else
          return false;
        break;

      case REQ_URI:
        if (is_end_of_line(c))
          return false;
        else if (is_space_or_tab(c))
        {
          // Ignore leading whitespace
          if (!uri_.empty())
            state_ = REQ_HTTP;
        }
        else
          uri_.push_back(c);
        break;

      case REQ_HTTP:
        // Ignore leading whitespace
        if (!is_space_or_tab(c))
        {
          if ('H' == c)
            state_ = REQ_HTTP_T1;
          else
            return false;
        }
        break;

      case REQ_HTTP_T1:
        if ('T' == c)
          state_ = REQ_HTTP_T2;
        else
          return false;
        break;

      case REQ_HTTP_T2:
        if ('T' == c)
          state_ = REQ_HTTP_P;
        else
          return false;
        break;

      case REQ_HTTP_P:
        if ('P' == c)
          state_ = REQ_HTTP_SLASH;
        else
          return false;
        break;

      case REQ_HTTP_SLASH:
        if ('/' == c)
          state_ = REQ_HTTP_MAJOR;
        else
          return false;
        break;

      case REQ_HTTP_MAJOR:
        if (std::isdigit(c))
        {
          major_read_ = true;
          major_version_ *= 10;
          major_version_ += read_digit(c);
        }
        else
        {
          if (major_read_ && ('.' == c))
            state_ = REQ_HTTP_MINOR;
          else
            return false;
        }
        break;

      case REQ_HTTP_MINOR:
        if (std::isdigit(c))
        {
          minor_read_ = true;
          minor_version_ *= 10;
          minor_version_ += read_digit(c);
        }
        else
        {
          // The HTTP version should end with a \r\n...
          if (is_end_of_line(c) && minor_read_)
          {
            if ('\r' == c)
              state_ = REQ_HTTP_LF;
            else // ('\n' == c) \\ but permit just \n
              state_ = REQ_HTTP_END;
          }
          else
            return false;
        }
        break;

      case REQ_HTTP_LF:
        if ('\n' == c)
          state_ = REQ_HTTP_END;
        else 
          return false;
        break;

      default:
        return false;
      }

      return true;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string request_line::to_string() const
    {
      std::string output{method_};
      output += ' ' + uri_ + ' '
             + http_version(major_version_, minor_version_)
             + "\r\n";
      return output;
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
