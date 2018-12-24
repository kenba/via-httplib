//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2018 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/headers.hpp"
#include <cstdlib>
#include <algorithm>

namespace
{
  const char COOKIE[] =   {"cookie"};
  const char IDENTITY[] = {"identity"};
  const char CLOSE[] =    {"close"};
  const char CONTINUE[] = {"100-continue"};
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
        [[fallthrough]]; // intentional fall-through

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
    void message_headers::add(std::string_view name, std::string_view value)
    {
      std::unordered_map<std::string, std::string>::iterator iter
        (fields_.find(name.data()));
      // if the field name was found previously
      if (iter != fields_.end())
      {
        char separator((name.find(COOKIE) != std::string::npos) ? ';' : ',');
        iter->second.append({separator});
        iter->second.append(value);
      }
      else
        fields_.insert(std::unordered_map<std::string, std::string>::value_type
                             (name, value));
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string_view message_headers::find(std::string_view name) const
    {
      std::unordered_map<std::string, std::string>::const_iterator iter
        (fields_.find(name.data()));
      return (iter != fields_.end()) ? iter->second : std::string_view();
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::ptrdiff_t message_headers::content_length() const noexcept
    {
      // Find whether there is a content length field.
      auto content_length(find(header_field::id::CONTENT_LENGTH));
      return (content_length.empty()) ? 0 : from_dec_string(content_length);
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::is_chunked() const
    {
      // Find whether there is a transfer encoding header.
      std::string xfer_encoding(find(header_field::id::TRANSFER_ENCODING));
      if (xfer_encoding.empty())
        return false;

      std::transform(xfer_encoding.begin(), xfer_encoding.end(),
                     xfer_encoding.begin(), ::tolower);
      // Note: is transfer encoding if "identity" is NOT found.
      return (xfer_encoding.find(IDENTITY) == std::string::npos);
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::close_connection() const
    {
      // Find whether there is a connection header.
      std::string connection(find(header_field::id::CONNECTION));
      if (connection.empty())
        return false;

      std::transform(connection.begin(), connection.end(),
                     connection.begin(), ::tolower);
      return (connection.find(CLOSE) != std::string::npos);
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::expect_continue() const
    {
      // Find whether there is a expect header.
      std::string expect(find(header_field::id::EXPECT));
      if (expect.empty())
        return false;

      std::transform(expect.begin(), expect.end(),
                     expect.begin(), ::tolower);
      return (expect.find(CONTINUE) != std::string::npos);
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
    bool are_headers_split(std::string_view headers) noexcept
    {
      char prev('0');
      char pprev('0');

      if (!headers.empty())
      {
        auto iter(headers.cbegin());
        for(; iter != headers.cend(); ++iter)
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
      }

      return false;
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
