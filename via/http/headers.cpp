//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "headers.hpp"
#include <cstdlib>
#include <algorithm>
#include <regex>
#include <limits>

namespace
{
  const std::string EMPTY_STRING{""};

  const std::regex REGEX_IDENTITY{".*identity.*", std::regex::icase};
  const std::regex REGEX_CLOSE{".*close.*", std::regex::icase};
  const std::regex REGEX_CONTINUE{".*100-continue.*", std::regex::icase};
}

namespace via
{
  namespace http
  {

    bool field_line::strict_crlf_s(false);

    size_t field_line::max_ws_s(8);

    size_t field_line::max_length_s(1024);

    //////////////////////////////////////////////////////////////////////////
    bool field_line::parse_char(char c)
    {
      // Ensure that the overall header length is within limitts
      if (++length_ > max_length_s)
        state_ = Header::ERROR_LENGTH;

      switch (state_)
      {
      case Header::NAME:
        if (std::isalpha(c) || ('-' == c))
          name_.push_back(static_cast<char>(std::tolower(c)));
        else if (':' == c)
          state_ = Header::VALUE_LS;
        else
          return false;
        break;

      case Header::VALUE_LS:
        // Ignore leading whitespace
        if (is_space_or_tab(c))
          // but only upto to a limit!
          if (++ws_count_ > max_ws_s)
          {
            state_ = Header::ERROR_WS;
            return false;
          }
          else
            break;
        else
          state_ = Header::VALUE;
        // intentional fall-through
      case Header::VALUE:
        // The header line should end with an \r\n...
        if (!is_end_of_line(c))
          value_.push_back(c);
        else if ('\r' == c)
          state_ = Header::LF;
        else // ('\n' == c)
        {
          if (strict_crlf_s)
          {
            state_ = Header::ERROR_CRLF;
            return false;
          }
          else
            state_ = Header::VALID;
        }
        break;

      case Header::LF:
        if ('\n' == c)
          state_ = Header::VALID;
        else
          return false;
        break;

      default:
        return false;
      }

      return true;
    }
    //////////////////////////////////////////////////////////////////////////

    size_t message_headers::max_length_s(std::numeric_limits<size_t>::max());
    size_t message_headers::max_content_length_s(std::numeric_limits<size_t>::max());

    //////////////////////////////////////////////////////////////////////////
    const std::string& message_headers::find(const std::string& name) const
    {
      auto iter(fields_.find(name));

      if (iter != fields_.end())
        return iter->second;
      else
        return EMPTY_STRING;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    size_t message_headers::content_length() const
    {
      // Find whether there is a content length field.
      const std::string& content_length{find(header_field::id::CONTENT_LENGTH)};
      if (content_length.empty())
        return 0;

      // Get the length from the content length field.
      auto length(from_dec_string(content_length));
      if (length < max_content_length_s)
        return length;
      else
        return std::numeric_limits<size_t>::max();
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::is_chunked() const
    {
      // Find whether there is a transfer encoding header.
      const std::string& xfer_encoding{find(header_field::id::TRANSFER_ENCODING)};
      if (xfer_encoding.empty())
        return false;

      return (!std::regex_match(xfer_encoding, REGEX_IDENTITY));
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::close_connection() const
    {
      // Find whether there is a connection header.
      const std::string& connection{find(header_field::id::CONNECTION)};
      if (connection.empty())
        return false;

      return (std::regex_match(connection, REGEX_CLOSE));
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::expect_continue() const
    {
      // Find whether there is a expect header.
      const std::string& connection{find(header_field::id::EXPECT)};
      if (connection.empty())
        return false;

      return (std::regex_match(connection, REGEX_CONTINUE));
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string message_headers::to_string() const
    {
      std::string output;
      for (const auto& elem : fields_)
        output += header_field::to_header(elem.first, elem.second);

      return output;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool are_headers_split(std::string const& headers)
    {
      auto prev('0');
      auto pprev('0');

      for(auto const& elem : headers)
      {
        if (elem == '\n')
        {
          if (prev == '\n')
            return true;
          else if ((prev == '\r') && (pprev == '\n'))
            return true;
        }

        pprev = prev;
        prev = elem;
      }

      return false;
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
