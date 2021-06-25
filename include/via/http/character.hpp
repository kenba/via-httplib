#ifndef CHARACTER_HPP_VIA_HTTPLIB_
#define CHARACTER_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file character.hpp
/// @brief Low level functions to classify characters and manipulate strings.
//////////////////////////////////////////////////////////////////////////////
#include <cctype>
#include <string>
#include <string_view>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <cerrno>

namespace via
{
  namespace http
  {
    /// The standard HTTP line terminator.
    constexpr char CRLF[]{"\r\n"};

    /// Test whether a character is an end of line character,
    /// i.e. CR or LF.
    /// @param c the character
    /// @return true if character is CR or LF, false otherwise.
    constexpr bool is_end_of_line(char c) noexcept
    { return ('\r' == c) || ('\n' == c); }

    /// Test whether a character is a separator character.
    /// @param c the character
    /// @return true if character is a separator character, false otherwise.
    inline bool is_separator(char c) noexcept
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

    /// Test whether a sequence of three characters is a percent encoding
    /// character according to RFC 3986.
    /// @param c the characters
    /// @return true if character is a percent encoding character, false otherwise.
    inline bool is_pct_encoded(char const* c) noexcept
    { return (c[0] == '%') && std::isxdigit(c[1]) && std::isxdigit(c[2]); }

    /// Test whether a character is a gen-delim according to RFC3986.
    /// @param c the character
    /// @return true if character is a gen-delim character, false otherwise.
    inline bool is_gen_delim(char c) noexcept
    {
      switch (c)
      {
      case ':': case '/': case '?': case '#': case '[': case ']': case '@':
        return true;
      default:
        return false;
      }
    }

    /// Test whether a character is a sub-delim according to RFC 3986.
    /// @param c the character
    /// @return true if character is a sub-delim character, false otherwise.
    inline bool is_sub_delim(char c) noexcept
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

    /// Test whether a character is a reserved character according to RFC3986.
    /// I.e. whether it is a gen-delim or a sub-delim character.
    /// @param c the character
    /// @return true if character is a reserved character, false otherwise.
    inline bool is_reserved(char c) noexcept
    { return is_gen_delim(c) || is_sub_delim(c); }

    /// Test whether a character is a unreserved character according to RFC3986.
    /// @param c the character
    /// @return true if character is a unreserved character, false otherwise.
    inline bool is_unreserved(char c) noexcept
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

    /// Test whether a character is a token character.
    /// i.e. not a control or a separator character.
    /// @param c the character
    /// @return true if character is a token character, false otherwise.
    inline bool is_token(char c) noexcept
    { return !std::iscntrl(c) && !is_separator(c); }

    /// Convert a digit character to an integer.
    /// @pre the character must be a valid digit character.
    /// @param c the character
    /// @return the integer equivalent of the character
    constexpr int read_digit(char c) noexcept
    { return (c -'0');}

    /// The http version string, i.e. HTTP/1.1
    /// @param major_version the http major version number.
    /// @param minor_version the http minor version number.
    /// @return the http string for the given version.
    inline std::string http_version(char major_version, char minor_version)
    {
      std::string output("HTTP/0.0");
      output[5] = major_version;
      output[7] = minor_version;
      return output;
    }

    /// Convert a string representing a hexadecimal number to an unsigned int.
    /// @param hex_string the string containing a valid hexadecimal number
    /// @return the number represented by the string, -1 if invalid.
    inline std::ptrdiff_t from_hex_string(std::string_view hex_string) noexcept
    {
      // Ensure that the string only contains hexadecimal characters
      if (!hex_string.empty() &&
          std::all_of(hex_string.cbegin(), hex_string.cend(),
                      [](auto c){ return std::isxdigit(c); }))
      {
        // Get the length from the hex_string.
        // Note: strtol may return zero for a string containing zero or if
        // no valid conversion could be performed. It may also set errno
        // if the number is out of range...
        errno = 0;
        std::ptrdiff_t value(std::strtol(hex_string.data(), 0, 16));
        if (errno || ((value == 0) && (hex_string[0] != '0')))
          return -1;
        else
          return value;
      }
      else
        return -1;
    }

    /// Convert an unsigned int into a hexadecimal string.
    /// @param number to be represented
    /// @return the string containing the number in hexadecimal.
    inline std::string to_hex_string(size_t number)
    {
      std::stringstream number_stream;
      number_stream << std::hex << number;
      return number_stream.str();
    }

    /// Convert a string representing a decimal number to an unsigned int.
    /// @param dec_string the string containing a valid decimal number
    /// @return the number represented by the string, -1 if invalid.
    inline std::ptrdiff_t from_dec_string(std::string_view dec_string) noexcept
    {
      // Ensure that the string only contains decimal characters
      if (!dec_string.empty() &&
          std::all_of(dec_string.cbegin(), dec_string.cend(),
                      [](auto c){ return std::isdigit(c); }))
      {
        // Get the length from the dec_string.
        // Note: strtol may return zero for a string containing zero or if
        // no valid conversion could be performed. It may also set errno
        // if the number is out of range...
        errno = 0;
        std::ptrdiff_t value(std::strtol(dec_string.data(), 0, 10));
        if (errno || ((value == 0) && (dec_string[0] != '0')))
          return -1;
        else
          return value;
      }
      else
        return -1;
    }
  }
}

#endif

