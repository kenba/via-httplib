//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/request.hpp"
#include "via/http/character.hpp"
#include <limits>

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
        {
          method_.push_back(c);
          if (method_.size() > max_method_length_)
          {
            state_ = REQ_ERROR_METHOD_LENGTH;
            return false;
          }
        }
        // If this char is whitespace and method has been read
        else if (is_space_or_tab(c) && !method_.empty())
        {
          ws_count_ = 1;
          state_ = REQ_URI;
        }
        else
          return false;
        break;

      case REQ_URI:
        if (is_end_of_line(c))
          return false;
        else if (is_space_or_tab(c))
        {
          // Ignore leading whitespace
          // but only upto to a limit!
          if (++ws_count_ > max_whitespace_)
          {
            state_ = REQ_ERROR_WS;
            return false;
          }

          if (!uri_.empty())
          {
            ws_count_ = 1;
            state_ = REQ_HTTP_H;
          }
        }
        else
        {
          uri_.push_back(c);
          if (uri_.size() > max_uri_length_)
          {
            state_ = REQ_ERROR_URI_LENGTH;
            return false;
          }
        }
        break;

      case REQ_HTTP_H:
        // Ignore leading whitespace
        if (is_space_or_tab(c))
        {
          // but only upto to a limit!
          if (++ws_count_ > max_whitespace_)
          {
            state_ = REQ_ERROR_WS;
            return false;
          }
        }
        else
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
          major_version_ = c;
          state_ = REQ_HTTP_DOT;
        }
        else
          return false;
        break;

      case REQ_HTTP_DOT:
        if ('.' == c)
          state_ = REQ_HTTP_MINOR;
        else
          return false;
        break;

      case REQ_HTTP_MINOR:
        if (std::isdigit(c))
        {
          minor_version_ = c;
          state_ = REQ_CR;
        }
        else
          return false;
        break;

      case REQ_CR:
        // The HTTP line should end with a \r\n...
        if ('\r' == c)
          state_ = REQ_LF;
        else
        {
          // but (if not being strict) permit just \n
          if (!strict_crlf_ && ('\n' == c))
            state_ = REQ_VALID;
          else
          {
            state_ = REQ_ERROR_CRLF;
            return false;
          }
        }
        break;

      case REQ_LF:
        if ('\n' == c)
        {
          state_ = REQ_VALID;
          break;
        }
        // intentional fall-through (for code coverage)

       default:
        return false;
      }

      return true;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string request_line::to_string() const
    {
      std::string output(method_);
      output += ' ' + uri_ + ' '
             + http_version(major_version_, minor_version_)
             + CRLF;
      return output;
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
