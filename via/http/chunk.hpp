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
    /// @class chunk_line
    //////////////////////////////////////////////////////////////////////////
    class chunk_line
    {
      size_t size_;
      std::string hex_size_;
      std::string extension_;

      enum parsing_state
      {
        CHUNK_SIZE_LS,
        CHUNK_SIZE,
        CHUNK_EXTENSION_LS,
        CHUNK_EXTENSION,
        CHUNK_LF,
        CHUNK_END
      };

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @retval state the current state of the parser.
      bool parse_char(char c, parsing_state& state);

    public:

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default constructor
      explicit chunk_line()
        : size_(0)
        , hex_size_("")
        , extension_("")
      {}

      /// Virtual destructor.
      /// Since the class is inherited...
      virtual ~chunk_line()
      {}

      /// Parse an http 1.1 chunk size line
      /// @retval next to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator1, typename ForwardIterator2>
      bool parse(ForwardIterator1& next, ForwardIterator2 end)
      {
        ForwardIterator1 iter(next);
        parsing_state state(CHUNK_SIZE_LS);
        while ((iter != end) && (CHUNK_END != state))
        {
          char c(static_cast<char>(*iter++));
          if (!parse_char(c, state))
            return false;
        }

        if ((CHUNK_END != state) || hex_size_.empty())
          return false;

        size_ = from_hex_string(hex_size_);
        next = iter;
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

      explicit chunk_line(size_t size,
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

    }; // class chunk_line

    //////////////////////////////////////////////////////////////////////////
    /// @class chunk
    //////////////////////////////////////////////////////////////////////////
    template <class Container>
    class chunk : public chunk_line
    {
      Container data_;

    public:

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default Constructor
      explicit chunk()
        : chunk_line()
        , data_()
      {}

      /// Parse an http 1.1 chunk
      /// @retval next to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the buffer.
      /// @retval chunk the body data to be read.
      /// @return true if the data is valid, false otherwise.
      template<typename ForwardIterator1, typename ForwardIterator2>
      bool parse(ForwardIterator1& next, ForwardIterator2 end)
      {
        ForwardIterator1 iter(next);
        if (!chunk_line::parse(iter, end))
          return false;

        if ((iter != end) && (size() > 0))
        {
          int read_size(std::min<size_t>((end - iter), size()));
          data_.assign(iter, iter + read_size);
          iter += read_size;
        }

        next = iter;
        return true;
      }

      /// Accessor for the chunk data.
      /// @return data_
      const Container& data() const
      { return data_; }

      ////////////////////////////////////////////////////////////////////////
      // Encoding interface.

      /// Constructor
      explicit chunk(const Container& data,
                     std::string extension = "")
        : chunk_line(data.size(), extension)
        , data_(data)
      {}

      /// Encode an http 1.1 data chunk.
      /// @return data the data to be encoded, returned as a chunk.
      Container message() const
      {
        std::string length_line(chunk_line::to_string());

        // Prepend length line to the data and terminate with CR LF
        Container data(length_line.begin(), length_line.end());
        data.insert(data.end(), data_.begin(), data_.end());
        data.push_back('\r');
        data.push_back('\n');

        return data;
      }
    }; // class chunk

  }
}

#endif
