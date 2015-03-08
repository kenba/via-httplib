#ifndef CHUNK_HPP_VIA_HTTPLIB_
#define CHUNK_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file chunk.hpp
/// @brief Classes to parse and encode HTTP chunks.
//////////////////////////////////////////////////////////////////////////////
#include "headers.hpp"

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

      /// @enum Chunk the parsing state of the chunk header parser.
      enum Chunk
      {
        CHUNK_SIZE_LS,      ///< leading white space
        CHUNK_SIZE,         ///< the chunk size hex text
        CHUNK_EXTENSION_LS, ///< chunk extension leading white space
        CHUNK_EXTENSION,    ///< the chunk extension
        CHUNK_LF,           ///< the line feed
        CHUNK_VALID,        ///< the chunk header is valid
        CHUNK_ERROR_LENGTH, ///< the header is longer than max_length_s
        CHUNK_ERROR_CRLF,   ///< strict_crlf_s is true and LF was received without CR
        CHUNK_ERROR_WS,     ///< the whitespace is longer than max_ws_s
        CHUNK_ERROR_SIZE    ///< the chunk size is greater than max_data_size_s
      };

    private:

      size_t size_;           ///< the size of the chunk in bytes
      size_t length_;         ///< the length of the chunk header in bytes
      size_t ws_count_;       ///< the current whitespace count
      size_t size_count_;     ///< the size character count
      std::string hex_size_;  ///< the chunk size hex string
      std::string extension_; ///< the chunk extesion (if any)
      Chunk state_;           ///< the current parsing state
      bool size_read_;        ///< true if the chunk size was read
      bool valid_;            ///< true if a chunk header is valid

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @return true if the character is valid, false otherwise.
      bool parse_char(char c);

    public:

      /// whether to enforce strict parsing of CRLF
      static bool strict_crlf_s;

      /// the maximum number of consectutive whitespace characters.
      static size_t max_ws_s;

      /// the maximum number of size digits.
      static size_t max_size_digits_s;

      /// the maximum length of the chunk header.
      static size_t max_length_s;

      /// the maximum size of a chunk's data
      static size_t max_data_size_s;

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit chunk_header() :
        size_(0),
        length_(0),
        ws_count_(0),
        hex_size_(""),
        extension_(""),
        state_(CHUNK_SIZE_LS),
        size_read_(false),
        valid_(false)
      {}

      /// Clear the chunk_header.
      /// Sets all member variables to their initial state.
      void clear()
      {
        size_ = 0;
        length_ = 0;
        ws_count_ = 0;
        hex_size_.clear();
        extension_.clear();
        state_ = CHUNK_SIZE_LS;
        size_read_ =  false;
        valid_ =  false;
      }

      /// Swap member variables with another chunk_header.
      /// @param other the other chunk_header
      void swap(chunk_header& other)
      {
        std::swap(size_, other.size_);
        std::swap(length_, other.length_);
        std::swap(ws_count_, other.ws_count_);
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
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        while ((iter != end) && (CHUNK_VALID != state_))
        {
          char c(static_cast<char>(*iter++));
          if (!parse_char(c))
            return false;
        }

        valid_ = (CHUNK_VALID == state_);
        return valid_;
      }

      /// Accessor for the chunk size.
      /// @return the chunk size in bytes.
      size_t size() const
      { return size_; }

      /// Accessor for the size hex string.
      /// @return the chunk size as a hex string.
      std::string const& hex_size() const
      { return hex_size_; }

      /// Accessor for the chunk extension.
      /// @return the chunk extension, blank if none.
      std::string const& extension() const
      { return extension_; }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const
      { return valid_; }

      /// Function to determine whether this is the last chunk.
      /// @return true if the last chunk, false otherwise.
      bool is_last() const
      { return size() == 0; }

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
      void set_extension(std::string const& extension)
      { extension_ = extension; }

      /// Output as a string.
      /// @return a string containing the chunk line.
      std::string to_string() const;
    }; // class chunk_header

    //////////////////////////////////////////////////////////////////////////
    /// @class rx_chunk
    /// A class to receive an HTTP chunk.
    //////////////////////////////////////////////////////////////////////////
    template <typename Container>
    class rx_chunk : public chunk_header
    {
      Container data_;         ///< the data contained in the chunk
      message_headers trailers_; ///< the HTTP field headers for the last chunk
      bool valid_;               ///< true if the chunk is valid

    public:

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit rx_chunk() :
        chunk_header(),
        data_(),
        trailers_(),
        valid_(false)
      {}

      /// clear the rx_chunk.
      /// Sets all member variables to their initial state.
      void clear()
      {
        chunk_header::clear();
        data_.clear();
        trailers_.clear();
        valid_ =  false;
      }

      /// swap member variables with another rx_chunk.
      /// @param other the other rx_chunk
      void swap(rx_chunk& other)
      {
        chunk_header::swap(other);
        data_.swap(other.data_);
        trailers_.swap(other.trailers_);
        std::swap(valid_, other.valid_);
      }

      /// Parse an HTTP chunk.
      /// @retval iter reference to an iterator to the start of the data.
      /// If the chunk is valid it will refer to:
      ///   - the start of the next data chunk,
      ///   - the start of the next http message, or
      ///   - the end of the data buffer.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        if (!chunk_header::valid() && !chunk_header::parse(iter, end))
          return false;

        // Only the last chunk has a trailer.
        if (chunk_header::is_last())
        {
          if (!trailers_.parse(iter, end))
            return false;
        }
        else // get the data
        {
          size_t required(chunk_header::size() - data_.size());
          size_t rx_size(end - iter);

          // received buffer contains more than just the required data
          if (rx_size > required)
          {
            if (required > 0)
            {
              ForwardIterator next(iter + required);
              data_.insert(data_.end(), iter, next);
              iter = next;
            }

            // Chunk must end in CRLF, allow just LF
            if ('\r' == *iter)
              ++iter;

            if ((iter == end) || ('\n' != *iter))
              return false;

            ++iter;
          }
          else // not enough received data, just add it to data
          {
            data_.insert(data_.end(), iter, end);
            iter = end;
            return false;
          }
        }

        valid_ = true;
        return valid_;
      }

      /// Accessor for the chunk message trailers.
      /// @return a constant reference to the trailer message_headers
      const message_headers& trailers() const
      { return trailers_; }

      /// Accessor for the chunk message data.
      /// @return a constant reference to the data
      const Container& data() const
      { return data_; }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const
      { return valid_; }
    }; // rx_chunk

    //////////////////////////////////////////////////////////////////////////
    /// @class last_chunk
    /// A class to send the last HTTP chunk and any trailers.
    //////////////////////////////////////////////////////////////////////////
    class last_chunk
    {
      std::string extension_;
      std::string trailer_string_;

    public:

      /// Send the last HTTP chunk for a request or response.
      /// @param extension the (optional) chunk extension.
      /// @param trailer_string the (optional) chunk trailers.
      explicit last_chunk(std::string const& extension,
                          std::string const& trailer_string) :
        extension_(extension),
        trailer_string_(trailer_string)
      {}

      /// Add a free form trailer to the chunk.
      void add_trailer(std::string const& field, std::string const& value)
      { trailer_string_ += header_field::to_header(field, value);  }

      /// Add a standard trailer to the chunk.
      void add_trailer(header_field::id::field field_id, std::string const& value)
      { trailer_string_ += header_field::to_header(field_id, value);  }

      /// The http message header string.
      std::string message() const;
    };

  }
}

#endif
