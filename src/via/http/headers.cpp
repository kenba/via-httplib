//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/headers.hpp"
#include <cstdlib>
#include <algorithm>
#include <regex>

namespace
{
  const std::string EMPTY_STRING("");

  const std::regex REGEX_IDENTITY(".*identity.*",     std::regex::icase);
  const std::regex REGEX_CLOSE   (".*close.*",        std::regex::icase);
  const std::regex REGEX_CONTINUE(".*100-continue.*", std::regex::icase);
}

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    bool field_line::parse_char(char c)
    {
      // Ensure that the overall header length is within limitts
      if (++length_ > max_line_length_)
        state_ = HEADER_ERROR_LENGTH;

      switch (state_)
      {
      case HEADER_NAME:
        if (std::isalpha(c) || ('-' == c))
          name_.push_back(static_cast<char>(std::tolower(c)));
        else if (':' == c)
          state_ = HEADER_VALUE_LS;
        else
          return false;
        break;

      case HEADER_VALUE_LS:
        // Ignore leading whitespace
        if (is_space_or_tab(c))
          // but only upto to a limit!
          if (++ws_count_ > max_whitespace_)
          {
            state_ = HEADER_ERROR_WS;
            return false;
          }
          else
          break;
        else
          state_ = HEADER_VALUE;
        // intentional fall-through
      case HEADER_VALUE:
        // The header line should end with an \r\n...
        if (!is_end_of_line(c))
          value_.push_back(c);
        else if ('\r' == c)
          state_ = HEADER_LF;
        else // ('\n' == c)
        {
          if (strict_crlf_)
          {
            state_ = HEADER_ERROR_CRLF;
            return false;
          }
          else
            state_ = HEADER_VALID;
        }
        break;

      case HEADER_LF:
        if ('\n' == c)
          state_ = HEADER_VALID;
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
    const std::string& message_headers::find(const std::string& name) const
    {
      std::unordered_map<std::string, std::string>::const_iterator iter
        (fields_.find(name));

      if (iter != fields_.end())
        return iter->second;
      else
        return EMPTY_STRING;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::ptrdiff_t message_headers::content_length() const NOEXCEPT
    {
      // Find whether there is a content length field.
      const std::string& content_length(find(header_field::id::CONTENT_LENGTH));
      if (content_length.empty())
        return 0;

      // Get the length from the content length field.
      return from_dec_string(content_length);
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::is_chunked() const
    {
      // Find whether there is a transfer encoding header.
      const std::string& xfer_encoding(find(header_field::id::TRANSFER_ENCODING));
      if (xfer_encoding.empty())
        return false;

      return (!std::regex_match(xfer_encoding, REGEX_IDENTITY));
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::close_connection() const
    {
      // Find whether there is a connection header.
      const std::string& connection(find(header_field::id::CONNECTION));
      if (connection.empty())
        return false;

      return (std::regex_match(connection, REGEX_CLOSE));
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::expect_continue() const
    {
      // Find whether there is a expect header.
      const std::string& connection(find(header_field::id::EXPECT));
      if (connection.empty())
        return false;

      return (std::regex_match(connection, REGEX_CONTINUE));
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string message_headers::to_string() const
    {
      std::string output;
      for (std::unordered_map<std::string, std::string>::const_iterator
           iter(fields_.begin()); iter != fields_.end(); ++iter)
        output += header_field::to_header(iter->first, iter->second);

      return output;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool are_headers_split(std::string const& headers) NOEXCEPT
    {
      char prev('0');
      char pprev('0');

      std::string::const_iterator iter(headers.begin());
      for(; iter != headers.end(); ++iter)
      {
        if (*iter == '\n')
        {
          if (prev == '\n')
            return true;
          else if ((prev == '\r') && (pprev == '\n'))
            return true;
        }

        pprev = prev;
        prev = *iter;
      }

      return false;
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
