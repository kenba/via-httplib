//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
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
      case RESP_HTTP_H:
        // Ignore leading whitespace
        if (is_space_or_tab(c))
        {
          // but only upto to a limit!
          if (++ws_count_ > max_whitespace_)
          {
            state_ = RESP_ERROR_WS;
            return false;
          }
        }
        else
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
          major_version_ = c;
          state_ = RESP_HTTP_DOT;
        }
        else
          return false;
        break;

      case RESP_HTTP_DOT:
        if ('.' == c)
          state_ = RESP_HTTP_MINOR;
        else
          return false;
        break;

      case RESP_HTTP_MINOR:
        if (std::isdigit(c))
        {
          minor_version_ = c;
          // must be at least one whitespace before status
          state_ = RESP_HTTP_WS;
        }
        else
          return false;
        break;

      case RESP_HTTP_WS:
        if (is_space_or_tab(c))
        {
          ws_count_ = 1;
          state_ = RESP_STATUS;
        }
        else
          return false;
        break;

      case RESP_STATUS:
        if (std::isdigit(c))
        {
          status_read_ = true;
          status_ *= 10;
          status_ += read_digit(c);
          if (status_ > max_status_no_)
          {
            state_ = RESP_ERROR_STATUS_VALUE;
            return false;
          }
        }
        else if (is_space_or_tab(c))
        {
          if (status_read_)
          {
            ws_count_ = 1;
            state_ = RESP_REASON;
          }
          else // Ignore extra leading whitespace
          {
            // but only upto to a limit!
            if (++ws_count_ > max_whitespace_)
            {
              state_ = RESP_ERROR_WS;
              return false;
            }
          }
        }
        else
          return false;
        break;

      case RESP_REASON:
        if (!is_end_of_line(c))
        {
          // Ignore leading whitespace
          if (reason_phrase_.empty() && is_space_or_tab(c))
          {
            // but only upto to a limit!
            if (++ws_count_ > max_whitespace_)
            {
              state_ = RESP_ERROR_WS;
              return false;
            }
          }
          else
          {
            reason_phrase_.push_back(c);
            if (reason_phrase_.size() > max_reason_length_)
            {
              state_ = RESP_ERROR_REASON_LENGTH;
              return false;
            }
          }
          break;
        }
        // intentional fall-through
      case RESP_CR:
        // The HTTP line should end with a \r\n...
        if ('\r' == c)
          state_ = RESP_LF;
        else
        {
          // but (if not being strict) permit just \n
          if (!strict_crlf_ && ('\n' == c))
            state_ = RESP_VALID;
          else
          {
            state_ = RESP_ERROR_CRLF;
            return false;
          }
        }
        break;

      case RESP_LF:
        if ('\n' == c)
        {
          state_ = RESP_VALID;
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
    std::string response_line::to_string() const
    {
      std::string output(http_version(major_version_, minor_version_));
      output += ' ' + to_dec_string(status_)
              + ' ' + reason_phrase_
              + CRLF;
      return output;
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
