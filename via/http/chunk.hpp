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
    /// The HTTP header for a data chunk.
    //////////////////////////////////////////////////////////////////////////
    class chunk_header
    {
    public:
      /// @enum parsing_state the state of the chunk header parser.
      enum parsing_state
      {
        CHUNK_SIZE_LS,      ///< leading white space
        CHUNK_SIZE,         ///< the chunk size hex text
        CHUNK_EXTENSION_LS, ///< chunk extension leading white space
        CHUNK_EXTENSION,    ///< the chunk extension
        CHUNK_LF,           ///< the line feed (if any)
        CHUNK_END
      };

    private:

      size_t size_;           ///< the size of the chunk in bytes
      std::string hex_size_;  ///< the chunk size hex string
      std::string extension_; ///< the chunk extesion (if any)
      parsing_state state_;   ///< the current parsing state
      bool size_read_;        ///< true if the chunk size was read
      bool valid_;            ///< true if a chunk header is valid

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @retval state the current state of the parser.
      bool parse_char(char c);

    public:

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit chunk_header() :
        size_(0),
        hex_size_(""),
        extension_(""),
        state_(CHUNK_SIZE_LS),
        size_read_(false),
        valid_(false)
      {}

      /// clear the chunk_header.
      /// Sets all member variables to their initial state.
      void clear()
      {
        size_ = 0;
        hex_size_.clear();
        extension_.clear();
        state_ = CHUNK_SIZE_LS;
        size_read_ =  false;
        valid_ =  false;
      }

      /// swap member variables with another chunk_header.
      /// @param other the other chunk_header
      void swap(chunk_header& other)
      {
        std::swap(size_, other.size_);
        hex_size_.swap(other.hex_size_);
        extension_.swap(other.extension_);
        std::swap(state_, other.state_);
        std::swap(size_read_, other.size_read_);
        std::swap(valid_, other.valid_);
      }

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

        valid_ = (CHUNK_END == state_);
        return valid_;
      }

      /// Accessor for the chunk size.
      /// @return the chunk size in bytes.
      size_t size() const
      { return size_; }

      /// Accessor for the size hex string.
      /// @return the chunk size as a hex string.
      const std::string& hex_size() const
      { return hex_size_; }

      /// Accessor for the chunk extension.
      /// @return the chunk extension, blank if none.
      const std::string& extension() const
      { return extension_; }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const
      { return valid_; }

      ////////////////////////////////////////////////////////////////////////
      // Encoding interface.

      /// Encoding constructor.
      /// Set the chunk size and optionally the extension.
      /// @param size the size of the chunk in bytes.
      /// @param extension the chunk extension (default blank).
      explicit chunk_header(size_t size,
                            std::string extension = "")
        : size_(size)
        , hex_size_(to_hex_string(size))
        , extension_(extension)
      {}

      /// Set the size of the chunk.
      /// @param size the size of the chunk in bytes.
      void set_size(size_t size)
      {
        size_ = size;
        hex_size_ = to_hex_string(size);
      }

      /// Set the chunk extension.
      /// @param extension the chunk extension
      void set_extension(const std::string& extension)
      { extension_ = extension; }

      /// Output as a string.
      /// @return a string containing the chunk line.
      std::string to_string() const;
    }; // class chunk_header

  }
}

#endif
