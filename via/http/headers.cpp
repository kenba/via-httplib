//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
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

namespace
{
  const std::string EMPTY_STRING("");

  const std::regex REGEX_IDENTITY(".*identity.*", std::regex::icase);
  const std::regex REGEX_CLOSE(".*close.*", std::regex::icase);
  const std::regex REGEX_CONTINUE(".*100-continue.*", std::regex::icase);
}

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    bool field_line::parse_char(char c)
    {
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
        if (is_space_or_tab(c))
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
        else // ('\n' == c) \\ but permit just \n
          state_ = HEADER_END;
        break;

      case HEADER_LF:
        if ('\n' == c)
          state_ = HEADER_END;
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
      const std::string& content_length(find(header_field::CONTENT_LENGTH));
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
      const std::string& xfer_encoding(find(header_field::TRANSFER_ENCODING));
      if (xfer_encoding.empty())
        return false;

      return (!std::regex_match(xfer_encoding, REGEX_IDENTITY));
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::close_connection() const
    {
      // Find whether there is a connection header.
      const std::string& connection(find(header_field::CONNECTION));
      if (connection.empty())
        return false;

      return (std::regex_match(connection, REGEX_CLOSE));
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool message_headers::expect_continue() const
    {
      // Find whether there is a expect header.
      const std::string& connection(find(header_field::EXPECT));
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

      output += "\r\n";
      return output;
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
