//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/character.hpp"
#include <sstream>
#include <cstdlib>
#include <cerrno>

namespace via
{
  namespace http
  {
    const std::string CRLF("\r\n");

    //////////////////////////////////////////////////////////////////////////
    bool is_separator(char c) NOEXCEPT
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
    bool is_pct_encoded(char const* c) NOEXCEPT
    { return (c[0] == '%') && std::isxdigit(c[1]) && std::isxdigit(c[2]); }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    bool is_gen_delim(char c) NOEXCEPT
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
    bool is_sub_delim(char c) NOEXCEPT
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
    bool is_unreserved(char c) NOEXCEPT
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
      std::string output("HTTP/");
      output.push_back(major_version);
      output.push_back('.');
      output.push_back(minor_version);
      return output;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::ptrdiff_t from_hex_string(std::string const& hex_string) NOEXCEPT
    {
      // Ensure that the string only contains hexadecimal characters
      std::string::const_iterator iter(hex_string.begin());
      for (; iter != hex_string.end(); ++iter)
        if (!std::isxdigit(*iter))
          return -1;

      // Get the length from the hex_string.
      // Note: strtol may return zero for a string containing zero or if
      // no valid conversion could be performed. It may also set errno
      // if the number is out of range...
      errno = 0;
      std::ptrdiff_t length(std::strtol(hex_string.c_str(), 0, 16));
      if (errno || ((length == 0) && (hex_string[0] != '0')))
        return -1;
      else
        return length;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::ptrdiff_t from_dec_string(std::string const& dec_string) NOEXCEPT
    {
      // Ensure that the string only contains decimal characters
      std::string::const_iterator iter(dec_string.begin());
      for (; iter != dec_string.end(); ++iter)
        if (!std::isdigit(*iter))
          return -1;

      // Get the length from the dec_string.
      // Note: strtol may return zero for a string containing zero or if
      // no valid conversion could be performed. It may also set errno
      // if the number is out of range...
      errno = 0;
      std::ptrdiff_t length(std::strtol(dec_string.c_str(), 0, 10));
      if (errno || ((length == 0) && (dec_string[0] != '0')))
        return -1;
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

    //////////////////////////////////////////////////////////////////////////
    std::string to_dec_string(size_t number)
    {
      std::stringstream number_stream;
      number_stream << number;
      return number_stream.str();
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
