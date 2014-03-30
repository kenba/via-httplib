//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "response.hpp"
#include "character.hpp"

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    bool response_line::parse_char(char c)
    {
      switch (state_)
      {
      case RESP_HTTP:
        // Ignore leading whitespace
        if (!is_space_or_tab(c))
        {
          if ('H' == c)
            state_ = RESP_HTTP_T1;
          else
            return false;
        }
        break;

      case RESP_HTTP_T1:
        if ('T' == c)
          state_ = RESP_HTTP_T2;
        else
          return false;
        break;

      case RESP_HTTP_T2:
        if ('T' == c)
          state_ = RESP_HTTP_P;
        else
          return false;
        break;

      case RESP_HTTP_P:
        if ('P' == c)
          state_ = RESP_HTTP_SLASH;
        else
          return false;
        break;

      case RESP_HTTP_SLASH:
        if ('/' == c)
          state_ = RESP_HTTP_MAJOR;
        else
          return false;
        break;

      case RESP_HTTP_MAJOR:
        if (std::isdigit(c))
        {
          major_read_ = true;
          major_version_ *= 10;
          major_version_ += read_digit(c);
        }
        else
        {
          if (major_read_ && ('.' == c))
            state_ = RESP_HTTP_MINOR;
          else
            return false;
        }
        break;

      case RESP_HTTP_MINOR:
        if (std::isdigit(c))
        {
          minor_read_ = true;
          minor_version_ *= 10;
          minor_version_ += read_digit(c);
        }
        else if (minor_read_ && (is_space_or_tab(c)))
          state_ = RESP_HTTP_STATUS;
        else
          return false;
        break;

      case RESP_HTTP_STATUS:
        if (std::isdigit(c))
        {
          status_read_ = true;
          status_ *= 10;
          status_ += read_digit(c);
        }
        else if (is_space_or_tab(c))
        {
          if (status_read_)
            state_ = RESP_HTTP_REASON;
          // Ignore leading whitespace
        }
        else
          return false;
        break;

      case RESP_HTTP_REASON:
        if (is_end_of_line(c))
        {
          if ('\r' == c)
            state_ = RESP_HTTP_LF;
          else // ('\n' == *iter) \\ but permit just \n
            state_ = RESP_HTTP_END;
        }
        // Ignore leading whitespace
        else if (!reason_phrase_.empty() || !is_space_or_tab(c))
          reason_phrase_.push_back(c);
        break;

      case RESP_HTTP_LF:
        if ('\n' == c)
          state_ = RESP_HTTP_END;
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
    std::string response_line::to_string() const
    {
      std::string output{http_version(major_version_, minor_version_)};
      output += ' ' + to_dec_string(status_)
              + ' ' + reason_phrase_
              + CRLF;
      return output;
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
