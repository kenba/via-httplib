//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "character.hpp"
#include <sstream>
#include <cstdlib>

namespace via
{
  namespace http
  {
    const std::string CRLF{"\r\n"};

    //////////////////////////////////////////////////////////////////////////
    bool is_separator(char c)
    {
      switch (c)
      {
      case '(': case ')': case '<': case '>': case '@':
      case ',': case ';': case ':': case '\\': case '"':
      case '/': case '[': case ']': case '?': case '=':
      case '{': case '}': case ' ': case '\t':
        return true;
      default:
        return false;
      }
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool is_pct_encoded(char const* c)
    { return (c[0] == '%') && std::isxdigit(c[1]) && std::isxdigit(c[2]); }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool is_gen_delim(char c)
    {
      switch (c)
      {
      case ':': case '/': case '?': case '#': case '[': case ']': case '@':
        return true;
      default:
        return false;
      }
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool is_sub_delim(char c)
    {
      switch (c)
      {
      case '!': case '$': case '&': case '\'': case '(': case ')':
      case '*': case '+': case ',': case ';': case '=':
        return true;
      default:
        return false;
      }
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool is_unreserved(char c)
    {
      if (std::isalnum(c))
        return true;
      else
      {
        switch (c)
        {
        case '-': case '.': case '_': case '~':
          return true;
        default:
          return false;
        }
      }
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string http_version(char major_version, char minor_version)
    {
      std::string output{"HTTP/"};
      output.push_back(major_version);
      output.push_back('.');
      output.push_back(minor_version);
      return output;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    size_t from_hex_string(std::string const& hex_string)
    {
      // Ensure that the string only contains hexadecimal characters
      for (auto character : hex_string)
        if (!std::isxdigit(character))
          return std::numeric_limits<size_t>::max();

      // Get the length from the hex_string.
      // Note: strtoul may return zero for a string containing zero or if
      // no valid conversion could be performed. It may also return
      // std::numeric_limits<size_t>::max() if the number is to big...
      size_t length(std::strtoul(hex_string.c_str(), 0, 16));
      if ((length == 0) && (hex_string[0] != '0'))
        return std::numeric_limits<size_t>::max();
      else
        return length;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    size_t from_dec_string(std::string const& dec_string)
    {
      // Ensure that the string only contains decimal characters
      for (auto character : dec_string)
        if (!std::isdigit(character))
          return std::numeric_limits<size_t>::max();

      // Get the length from the dec_string.
      // Note: strtoul may return zero for a string containing zero or if
      // no valid conversion could be performed. It may also return
      // std::numeric_limits<size_t>::max() if the number is to big...
      size_t length(std::strtoul(dec_string.c_str(), 0, 10));
      if ((length == 0) && (dec_string[0] != '0'))
        return std::numeric_limits<size_t>::max();
      else
        return length;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string to_hex_string(size_t number)
    {
      std::stringstream number_stream;
      number_stream << std::hex << number;
      return number_stream.str();
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
