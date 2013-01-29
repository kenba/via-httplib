#pragma once

#ifndef CHUNK_HPP_VIA_HTTPLIB_
#define CHUNK_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "character.hpp"

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class chunk_header
    //////////////////////////////////////////////////////////////////////////
    class chunk_header
    {
    public:
      enum parsing_state
      {
        CHUNK_SIZE_LS,
        CHUNK_SIZE,
        CHUNK_EXTENSION_LS,
        CHUNK_EXTENSION,
        CHUNK_LF,
        CHUNK_END
      };

    private:

      size_t size_;
      std::string hex_size_;
      std::string extension_;
      parsing_state state_;

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @retval state the current state of the parser.
      bool parse_char(char c);

    public:

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default constructor
      explicit chunk_header() :
        size_(0),
        hex_size_(""),
        extension_(""),
        state_(CHUNK_SIZE_LS)
      {}

      /// Parse an http 1.1 chunk size line
      /// @retval iter to an iterator to the start of the http chunk.
      /// If parsed sucessfully, it will refer to the start of the data.
      /// @param end the end of the buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator1, typename ForwardIterator2>
      bool parse(ForwardIterator1& iter, ForwardIterator2 end)
      {
        while ((iter != end) && (CHUNK_END != state_))
        {
          char c(static_cast<char>(*iter++));
          if (!parse_char(c))
            return false;
        }

        if ((CHUNK_END != state_) || hex_size_.empty())
          return false;

        size_ = from_hex_string(hex_size_);
        return true;
      }

      size_t size() const
      { return size_; }

      const std::string& hex_size() const
      { return hex_size_; }

      const std::string& extension() const
      { return extension_; }

      ////////////////////////////////////////////////////////////////////////
      // Encoding interface.

      explicit chunk_header(size_t size,
                          std::string extension = "")
        : size_(size)
        , hex_size_(to_hex_string(size))
        , extension_(extension)
      {}

      void set_size(size_t size)
      {
        size_ = size;
        hex_size_ = to_hex_string(size);
      }

      void set_extension(const std::string& extension)
      { extension_ = extension; }

      /// Output as a string.
      /// @return a string containing the chunk line.
      std::string to_string() const;

    }; // class chunk_header

  }
}

#endif
