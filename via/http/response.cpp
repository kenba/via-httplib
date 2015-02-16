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

    bool response_line::strict_crlf_s(false);

    size_t response_line::max_ws_s(std::numeric_limits<size_t>::max());

    int response_line::max_status_s(std::numeric_limits<int>::max());

    size_t response_line::max_reason_length_s(std::numeric_limits<size_t>::max());

    //////////////////////////////////////////////////////////////////////////
    bool response_line::parse_char(char c)
    {
      switch (state_)
      {
      case Response::HTTP_H:
        // Ignore leading whitespace
        if (is_space_or_tab(c))
        {
          // but only upto to a limit!
          if (++ws_count_ > max_ws_s)
          {
            state_ = Response::ERROR_WS;
            return false;
          }
        }
        else
        {
          if ('H' == c)
            state_ = Response::HTTP_T1;
          else
            return false;
        }
        break;

      case Response::HTTP_T1:
        if ('T' == c)
          state_ =Response::HTTP_T2;
        else
          return false;
        break;

      case Response::HTTP_T2:
        if ('T' == c)
          state_ = Response::HTTP_P;
        else
          return false;
        break;

      case Response::HTTP_P:
        if ('P' == c)
          state_ = Response::HTTP_SLASH;
        else
          return false;
        break;

      case Response::HTTP_SLASH:
        if ('/' == c)
          state_ = Response::HTTP_MAJOR;
        else
          return false;
        break;

      case Response::HTTP_MAJOR:
        if (std::isdigit(c))
        {
          major_version_ = c;
          state_ = Response::HTTP_DOT;
        }
        else
          return false;
        break;

      case Response::HTTP_DOT:
        if ('.' == c)
          state_ = Response::HTTP_MINOR;
        else
          return false;
        break;

      case Response::HTTP_MINOR:
        if (std::isdigit(c))
        {
          minor_version_ = c;
          // must be at least one whitespace before status
          state_ = Response::HTTP_WS;
        }
        else
          return false;
        break;

      case Response::HTTP_WS:
        if (is_space_or_tab(c))
        {
          ws_count_ = 1;
          state_ = Response::STATUS;
        }
        else
          return false;
        break;

      case Response::STATUS:
        if (std::isdigit(c))
        {
          status_read_ = true;
          status_ *= 10;
          status_ += read_digit(c);
          if (status_ > max_status_s)
          {
            state_ = Response::ERROR_STATUS_VALUE;
            return false;
          }
        }
        else if (is_space_or_tab(c))
        {
          if (status_read_)
          {
            ws_count_ = 1;
            state_ = Response::REASON;
          }
          else // Ignore extra leading whitespace
          {
            // but only upto to a limit!
            if (++ws_count_ > max_ws_s)
            {
              state_ = Response::ERROR_WS;
              return false;
            }
          }
        }
        else
          return false;
        break;

      case Response::REASON:
        if (!is_end_of_line(c))
        {
          // Ignore leading whitespace
          if (reason_phrase_.empty() && is_space_or_tab(c))
          {
            // but only upto to a limit!
            if (++ws_count_ > max_ws_s)
            {
              state_ = Response::ERROR_WS;
              return false;
            }
          }
          else
          {
            reason_phrase_.push_back(c);
            if (reason_phrase_.size() > max_reason_length_s)
            {
              state_ = Response::ERROR_REASON_LENGTH;
              return false;
            }
          }
          break;
        }
        // intentional fall-through
      case Response::CR:
        // The HTTP line should end with a \r\n...
        if ('\r' == c)
          state_ = Response::LF;
        else
        {
          // but (if not being strict) permit just \n
          if (!strict_crlf_s && ('\n' == c))
            state_ = Response::VALID;
          else
          {
            state_ = Response::ERROR_CRLF;
            return false;
          }
        }
        break;

      case Response::LF:
        if ('\n' == c)
          state_ = Response::VALID;
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
