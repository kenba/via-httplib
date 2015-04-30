#ifndef CHARACTER_HPP_VIA_HTTPLIB_
#define CHARACTER_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file character.hpp
/// @brief Low level functions to classify characters and manipulate strings.
//////////////////////////////////////////////////////////////////////////////
#include "via/no_except.hpp"
#include <cctype>
#include <string>
#include <ctime>

namespace via
{
  namespace http
  {
    /// The standard HTTP line terminator.
    extern const std::string CRLF;

    /// Test whether a character is an end of line character,
    /// i.e. CR or LF.
    /// @param c the character
    /// @return true if character is CR or LF, false otherwise.
    inline bool is_end_of_line(char c) NOEXCEPT
    { return ('\r' == c) || ('\n' == c); }

    /// Test whether a character is a space or tab character.
    /// Note: equivalent to C++11 std::isblank in <cctype>
    /// @param c the character
    /// @return true if character is space or tab, false otherwise.
    inline bool is_space_or_tab(char c) NOEXCEPT
    { return (' ' == c) || ('\t' == c); }

    /// Test whether a character is a control character.
    /// @param c the character
    /// @return true if character is control character, false otherwise.
    inline bool is_ctl(char c) NOEXCEPT
    { return ((0 <= c) && (31 >= c)) || (127 ==c); }

    /// Test whether a character is a separator character.
    /// @param c the character
    /// @return true if character is a separator character, false otherwise.
    bool is_separator(char c) NOEXCEPT;

    /// Test whether a sequence of three characters is a percent encoding
    /// character according to RFC 3986.
    /// @param c the characters
    /// @return true if character is a percent encoding character, false otherwise.
    bool is_pct_encoded(char const* c) NOEXCEPT;

    /// Test whether a character is a gen-delim according to RFC3986.
    /// @param c the character
    /// @return true if character is a gen-delim character, false otherwise.
    bool is_gen_delim(char c) NOEXCEPT;

    /// Test whether a character is a sub-delim according to RFC 3986.
    /// @param c the character
    /// @return true if character is a sub-delim character, false otherwise.
    bool is_sub_delim(char c) NOEXCEPT;

    /// Test whether a character is a reserved character according to RFC3986.
    /// I.e. whether it is a gen-delim or a sub-delim character.
    /// @param c the character
    /// @return true if character is a reserved character, false otherwise.
    inline bool is_reserved(char c) NOEXCEPT
    { return is_gen_delim(c) || is_sub_delim(c); }

    /// Test whether a character is a unreserved character according to RFC3986.
    /// @param c the character
    /// @return true if character is a unreserved character, false otherwise.
    bool is_unreserved(char c) NOEXCEPT;

    /// Test whether a character is a token character.
    /// i.e. not a control or separator character.
    /// @param c the character
    /// @return true if character is a token character, false otherwise.
    inline bool is_token(char c) NOEXCEPT
    { return !is_ctl(c) && !is_separator(c); }

    /// Convert a digit charcter to an integer.
    /// @pre the character must be a valid digit character.
    /// @param c the character
    /// @return the integer equivalent of the character
    inline int read_digit(char c) NOEXCEPT
    { return (c -'0');}

    /// The http version string, i.e. HTTP/1.1
    /// @param major_version the http major version number.
    /// @param minor_version the http minor version number.
    /// @return the http string for the given version.
    std::string http_version(char major_version, char minor_version);

    /// Convert a string representing a hexadecimal number to an unsigned int.
    /// @param hex_string the string containing a vald hexadecimal number
    /// @return the number represented by the string, -1 if invalid.
    std::ptrdiff_t from_hex_string(std::string const& hex_string) NOEXCEPT;

    /// Convert an unsigned int into a hexadecimal string.
    /// @param number to be represented
    /// @return the string containing the number in hexadecimal.
    std::string to_hex_string(size_t number);

    /// Convert a string representing a decimal number to an unsigned int.
    /// @param dec_string the string containing a vald decimal number
    /// @return the number represented by the string, -1 if invalid.
    std::ptrdiff_t from_dec_string(std::string const& dec_string) NOEXCEPT;

    /// Convert an int into a decimal string.
    /// @param number to be represented
    /// @return the string containing the number in decimal.
    std::string to_dec_string(size_t number);

    /// Convert an string to time
    /// @param s, the encoded time
    /// @return the unix time
    std::time_t time_from_string(const std::string& s, const std::string& fmt="%a, %d-%b-%Y %T");

    /// Convert an time to the string
    /// @param t, the unix time
    /// @return the encoded time
    std::string time_to_string(const std::time_t& t, const std::string& fmt="%a, %d-%b-%Y %T");
  }
}

#endif
