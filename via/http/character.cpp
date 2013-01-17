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
    const std::string CRLF("\r\n");

    //////////////////////////////////////////////////////////////////////////
    std::string http_version(int major_version, int minor_version)
    {
      std::string output("HTTP/");
      std::stringstream version;
      version << major_version << '.' << minor_version;
      output += version.str();
      return output;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    size_t from_hex_string(const std::string& hex_string)
    {
      return strtoul(hex_string.c_str(), 0, 16);
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string to_hex_string(size_t number)
    {
      // Note: C++11 to_string would be cleaner, but would it convert to hex?
      std::stringstream number_stream;
      number_stream << std::hex << number;
     // std::string number_string(number_stream.str());
      return number_stream.str();
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string to_dec_string(int number)
    {
      // Note: to_string is a C++11 function!
      std::stringstream number_stream;
      number_stream << number;
     // std::string number_string(number_stream.str());
      return number_stream.str();
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
