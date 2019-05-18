#ifndef HTTP_AUTHENTICATION_BASE64_HPP_VIA_HTTPLIB_
#define HTTP_AUTHENTICATION_BASE64_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015-2019 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file base64.hpp
/// @brief Contains the base64 encoder and decoder.
//////////////////////////////////////////////////////////////////////////////
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <algorithm>
#include <string>

namespace via
{
  namespace http
  {
    namespace authentication
    {
      namespace base64
      {
        /// The pad character for base64: =
        constexpr char PAD_CHARACTER{'='};

        /// Encode a string into Base64 format.
        /// @param input the string to encode
        /// @return the string encoded into base64 format.
        inline std::string encode(std::string input)
        {
          // The input must be in multiples of 3, otherwise the transformation
          // may overflow the input buffer, so pad with zero.
          size_t num_pad_chars((3 - input.size() % 3) % 3);
          input.append(num_pad_chars, 0);

          // Transform to Base64 with line breaks every 76 characters
          using namespace boost::archive::iterators;
          typedef insert_linebreaks<base64_from_binary<transform_width
              <std::string::const_iterator, 6, 8> >, 76> ItBase64T;
          std::string output(ItBase64T(input.begin()),
                             ItBase64T(input.end() - num_pad_chars));

          // Pad blank characters with =
          output.append(num_pad_chars, PAD_CHARACTER);

          return output;
        }

        /// Decode a string from Base64 format.
        /// @param input the string to decode
        /// @return the decoded string.
        inline std::string decode(std::string input)
        {
          using namespace boost::archive::iterators;
          typedef transform_width<binary_from_base64<remove_whitespace
              <std::string::const_iterator> >, 8, 6> ItBinaryT;

          try
          {
            // If the input isn't a multiple of 4, pad with =
            size_t num_pad_chars((4 - input.size() % 4) % 4);
            input.append(num_pad_chars, PAD_CHARACTER);

            size_t pad_chars(std::count(input.begin(), input.end(), PAD_CHARACTER));
            std::replace(input.begin(), input.end(), PAD_CHARACTER, 'A');
            std::string output(ItBinaryT(input.begin()), ItBinaryT(input.end()));
            output.erase(output.end() - pad_chars, output.end());
            return output;
          }
          catch (std::exception const&)
          {
            return std::string("");
          }
        }
      }
    }
  }
}

#endif // HTTP_AUTHENTICATION_BASE64_HPP_VIA_HTTPLIB_
