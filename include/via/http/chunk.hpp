#ifndef CHUNK_HPP_VIA_HTTPLIB_
#define CHUNK_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Ken Barker
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
#include <algorithm>

namespace via
{
  namespace http
  {
    /// The default maximum size of an HTTP chunk, 1M.
    constexpr size_t DEFAULT_MAX_CHUNK_SIZE = 1048576U;

    //////////////////////////////////////////////////////////////////////////
    /// @class chunk_header
    /// The HTTP header for a data chunk.
    /// @tparam MAX_LINE_LENGTH the maximum length of an HTTP header field line:
    /// min 1, max 65534.
    /// @tparam MAX_WHITESPACE_CHARS the maximum number of consecutive
    /// whitespace characters allowed in a request: min 1, max 254.
    /// @tparam STRICT_CRLF enforce strict parsing of CRLF.
    //////////////////////////////////////////////////////////////////////////
    template <unsigned short MAX_LINE_LENGTH,
              unsigned char  MAX_WHITESPACE_CHARS,
              bool           STRICT_CRLF>
    class chunk_header
    {
    public:

      /// @enum Chunk the parsing state of the chunk header parser.
      enum class Chunk
      {
        SIZE_LS,      ///< leading white space
        SIZE,         ///< the chunk size hex text
        EXTENSION_LS, ///< chunk extension leading white space
        EXTENSION,    ///< the chunk extension
        LF,           ///< the line feed
        VALID,        ///< the chunk header is valid
        ERROR_LENGTH, ///< the header is longer than MAX_LINE_LENGTH
        ERROR_CRLF,   ///< STRICT_CRLF is true and LF was received without CR
        ERROR_WS,     ///< the whitespace is longer than MAX_WHITESPACE_CHARS
        ERROR_SIZE    ///< the chunk size is greater than max_chunk_size_
      };

    private:

      /// the maximum size of a chunk body
      size_t max_chunk_size_ { DEFAULT_MAX_CHUNK_SIZE };
      size_t size_ { 0 };              ///< the size of the chunk in bytes
      size_t length_ { 0 };            ///< the length of the chunk header in bytes
      size_t ws_count_ { 0 };          ///< the current whitespace count
      std::string hex_size_ {};        ///< the chunk size hex string
      std::string extension_ {};       ///< the chunk extension (if any)
      Chunk state_ { Chunk::SIZE_LS }; ///< the current parsing state
      bool size_read_ { false };       ///< true if the chunk size was read
      bool valid_ { false };           ///< true if a chunk header is valid

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @return true if the character is valid, false otherwise.
      bool parse_char(char c)
      {
        static constexpr size_t MAX_SIZE_DIGITS(16); // enough for a 64 bit number

        // Ensure that the overall header length is within limits
        if (++length_ > MAX_LINE_LENGTH)
          state_ = Chunk::ERROR_LENGTH;

        switch (state_)
        {
        case Chunk::SIZE_LS:
          // Ignore leading whitespace
          if (std::isblank(c))
          {
            // but only upto to a limit!
            if (++ws_count_ > MAX_WHITESPACE_CHARS)
            {
              state_ = Chunk::ERROR_WS;
              return false;
            }
            else
              break;
          }
          else
            state_ = Chunk::SIZE;
          [[fallthrough]];

        case Chunk::SIZE:
          if (std::isxdigit (c))
          {
            hex_size_.push_back(c);
            // limit the length of the hex string
            if (hex_size_.size() > MAX_SIZE_DIGITS)
            {
              state_ = Chunk::ERROR_SIZE;
              return false;
            }
          }
          else
          {
            if (is_end_of_line(c) || (';' == c))
            {
              size_ = from_hex_string(hex_size_);
              size_read_ = true;
              if (size_ > max_chunk_size_)
              {
                state_ = Chunk::ERROR_SIZE;
                return false;
              }

              if (';' == c)
              {
                ws_count_ = 0;
                state_ = Chunk::EXTENSION_LS;
              }
              else
              {
                if ('\r' == c)
                  state_ = Chunk::LF;
                else // ('\n' == c)
                {
                  if (STRICT_CRLF)
                    return false;
                  else
                    state_ = Chunk::VALID;
                }
              }
            }
            else
              return false;
          }
          break;

        case Chunk::EXTENSION_LS:
          // Ignore leading whitespace
          if (std::isblank(c))
          {
            // but only upto to a limit!
            if (++ws_count_ > MAX_WHITESPACE_CHARS)
              return false;
            else
              break;
          }
          else
            state_ = Chunk::EXTENSION;
          [[fallthrough]];

        case Chunk::EXTENSION:
          if (!is_end_of_line(c))
            extension_.push_back(c);
          else if ('\r' == c)
            state_ = Chunk::LF;
          else // ('\n' == c)
          {
            if (STRICT_CRLF)
            {
              state_ = Chunk::ERROR_CRLF;
              return false;
            }
            else
              state_ = Chunk::VALID;
          }
          break;

        case Chunk::LF:
          if ('\n' == c)
            state_ = Chunk::VALID;
          else
            return false;
          break;

        default:
          return false;
        }

        return true;
      }

    public:

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default Constructor.
      chunk_header() = default;

      /// Clear the chunk_header.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        size_ = 0;
        length_ = 0;
        ws_count_ = 0;
        hex_size_.clear();
        extension_.clear();
        state_ = Chunk::SIZE_LS;
        size_read_ =  false;
        valid_ =  false;
      }

      /// Swap member variables with another chunk_header.
      /// @param other the other chunk_header
      void swap(chunk_header& other) noexcept
      {
        std::swap(max_chunk_size_, other.max_chunk_size_);
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
      /// If parsed successfully, it will refer to the start of the data.
      /// @param end the end of the buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        while ((iter != end) && (Chunk::VALID != state_))
        {
          char c(*iter++);
          if (!parse_char(c))
            return false;
        }

        valid_ = (Chunk::VALID == state_);
        return valid_;
      }

      /// Accessor for the chunk size.
      /// @return the chunk size in bytes.
      size_t size() const noexcept
      { return size_; }

      /// Accessor for the size hex string.
      /// @return the chunk size as a hex string.
      std::string const& hex_size() const noexcept
      { return hex_size_; }

      /// Accessor for the chunk extension.
      /// @return the chunk extension, blank if none.
      std::string const& extension() const noexcept
      { return extension_; }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const noexcept
      { return valid_; }

      /// Function to determine whether this is the last chunk.
      /// @return true if the last chunk, false otherwise.
      bool is_last() const noexcept
      { return size() == 0; }

      ////////////////////////////////////////////////////////////////////////
      // Encoding interface.

      /// Constructor.
      /// Set the chunk size and optionally the extension.
      /// @param size the size of the chunk in bytes.
      /// @param extension the chunk extension (default blank).
      explicit chunk_header(size_t size,
                             std::string_view extension = std::string_view())
        : max_chunk_size_(size)
        , size_(size)
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
      void set_extension(std::string_view extension)
      { extension_ = extension; }

      /// Output as a string.
      /// @return a string containing the chunk line.
      std::string to_string() const
      {
        std::string output(hex_size_);
        if (!extension_.empty())
          output += "; " + extension_;

        output += CRLF;
        return output;
      }
    }; // class chunk_header

    //////////////////////////////////////////////////////////////////////////
    /// @class rx_chunk
    /// A class to receive an HTTP chunk.
    /// @tparam MAX_HEADER_NUMBER the maximum number of HTTP header field lines:
    /// max 65534.
    /// @tparam MAX_HEADER_LENGTH the maximum cumulative length the HTTP header
    /// fields: max 4 billion.
    /// @tparam MAX_LINE_LENGTH the maximum length of an HTTP chunk header
    /// field line: max 65534.
    /// @tparam MAX_WHITESPACE_CHARS the maximum number of consecutive whitespace
    /// characters allowed in a request: min 1, max 254.
    /// @tparam STRICT_CRLF enforce strict parsing of CRLF.
    //////////////////////////////////////////////////////////////////////////
    template <typename Container,
              unsigned short MAX_HEADER_NUMBER,
              size_t         MAX_HEADER_LENGTH,
              unsigned short MAX_LINE_LENGTH,
              unsigned char  MAX_WHITESPACE_CHARS,
              bool           STRICT_CRLF>
    class rx_chunk : public chunk_header<MAX_LINE_LENGTH,
                                          MAX_WHITESPACE_CHARS,
                                          STRICT_CRLF>
    {
      using MessageHeaders = message_headers<MAX_HEADER_NUMBER,
                                             MAX_HEADER_LENGTH,
                                             MAX_LINE_LENGTH,
                                             MAX_WHITESPACE_CHARS,
                                             STRICT_CRLF>;

      using ChunkHeader = chunk_header<MAX_LINE_LENGTH,
                                       MAX_WHITESPACE_CHARS,
                                       STRICT_CRLF>;

      Container data_ {};           ///< the data contained in the chunk
      MessageHeaders trailers_ {}; ///< the HTTP field headers for the last chunk
      bool valid_ { false };        ///< true if the chunk is valid

    public:

      /// Default Constructor.
      rx_chunk() = default;

      /// Constructor.
      /// Set the chunk maximum size.
      /// @param max_chunk_size the maximum size of the chunk in bytes.
      explicit rx_chunk(size_t max_chunk_size)
        : ChunkHeader(max_chunk_size)
      { clear(); }

      /// clear the rx_chunk.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        ChunkHeader::clear();
        data_.clear();
        trailers_.clear();
        valid_ =  false;
      }

      /// swap member variables with another rx_chunk.
      /// @param other the other rx_chunk
      void swap(rx_chunk& other) noexcept
      {
        ChunkHeader::swap(other);
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
        if (!ChunkHeader::valid() && !ChunkHeader::parse(iter, end))
          return false;

        // Only the last chunk has a trailer.
        if (ChunkHeader::is_last())
        {
          if (!trailers_.parse(iter, end))
            return false;
        }
        else
        {
          // get the data and the CRLF after it
          std::ptrdiff_t data_required(static_cast<std::ptrdiff_t>(ChunkHeader::size()) -
                                       static_cast<std::ptrdiff_t>(data_.size()));
          std::ptrdiff_t rx_size(std::distance(iter, end));

          // received buffer contains more than just the required data
          if (rx_size > data_required)
          {
            if (data_required > 0)
            {
              ForwardIterator next(iter + data_required);
              data_.insert(data_.end(), iter, next);
              iter = next;
            }

            // Chunk should end in CRLF
            if ('\r' == *iter)
              ++iter;
            else
            { // enforce if strict
              if (STRICT_CRLF)
                return false;
            }

            // But it must end with an LF
            if ((iter == end) || ('\n' != *iter))
              return false;
            else // ('\n' == *iter)
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
      const MessageHeaders& trailers() const noexcept
      { return trailers_; }

      /// Accessor for the chunk message data.
      /// @return a constant reference to the data
      const Container& data() const noexcept
      { return data_; }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const noexcept
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
      explicit last_chunk(std::string_view extension,
                           std::string_view trailer_string) :
        extension_(extension),
        trailer_string_(trailer_string)
      {}

      /// Add a free form trailer to the chunk.
      void add_trailer(std::string_view field, std::string_view value)
      { trailer_string_ += header_field::to_header(field, value);  }

      /// Add a standard trailer to the chunk.
      void add_trailer(header_field::id field_id, std::string_view value)
      { trailer_string_ += header_field::to_header(field_id, value);  }

      /// The http chunk header string.
      std::string to_string() const
      {
        std::string output("0");
        if (!extension_.empty())
          output += "; " + extension_;

        output += CRLF;
        output += trailer_string_;

        output += CRLF;
        return output;
      }
    };

  }
}

#endif
