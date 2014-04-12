//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
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
    std::string http_version(int major_version, int minor_version)
    {
      std::string output{"HTTP/"};
      std::stringstream version;
      version << major_version << '.' << minor_version;
      output += version.str();
      return output;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    size_t from_hex_string(const std::string& hex_string)
    {
      // Ensure that the string only contains hexadecimal characters
      for (auto character : hex_string)
        if (!std::isxdigit(character))
          return ULONG_MAX;

      // Get the length from the hex_string.
      // Note: strtoul may return zero for a string containing zero or if
      // no valid conversion could be performed. It may also return
      // ULONG_MAX if the number is to big...
      size_t length(std::strtoul(hex_string.c_str(), 0, 16));
      if ((length == 0) && (hex_string[0] != '0'))
        return ULONG_MAX;
      else
        return length;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    size_t from_dec_string(const std::string& dec_string)
    {
      // Ensure that the string only contains decimal characters
      for (auto character : dec_string)
        if (!std::isdigit(character))
          return ULONG_MAX;

      // Get the length from the dec_string.
      // Note: strtoul may return zero for a string containing zero or if
      // no valid conversion could be performed. It may also return
      // ULONG_MAX if the number is to big...
      size_t length(std::strtoul(dec_string.c_str(), 0, 10));
      if ((length == 0) && (dec_string[0] != '0'))
        return ULONG_MAX;
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
