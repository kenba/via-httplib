#ifndef HEADERS_HPP_VIA_HTTPLIB_
#define HEADERS_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file headers.hpp
/// @brief Classes to parse and encode HTTP headers.
//////////////////////////////////////////////////////////////////////////////
#include "header_field.hpp"
#include <unordered_map>

namespace via
{
  namespace http
  {
    constexpr char COOKIE[]   {"cookie"};
    constexpr char IDENTITY[] {"identity"};
    constexpr char CLOSE[]    {"close"};
    constexpr char CONTINUE[] {"100-continue"};

    /// @enum Rx is the receiver parsing state and is valid for both the
    /// request and response receivers.
    enum class Rx
    {
      INVALID,         ///< the message is invalid
      EXPECT_CONTINUE, ///< the client expects a 100 Continue response
      INCOMPLETE,      ///< the message requires more data
      VALID,           ///< a valid request or response
      CHUNK            ///< a valid chunk received
    };

    //////////////////////////////////////////////////////////////////////////
    /// @class field_line
    /// An HTTP header field.
    /// @tparam MAX_LINE_LENGTH the maximum length of an HTTP header field line:
    /// min 1, max 65534.
    /// @tparam MAX_WHITESPACE_CHARS the maximum number of consectutive whitespace
    /// characters allowed in a request: min 1, max 254.
    /// @tparam STRICT_CRLF enforce strict parsing of CRLF.
    //////////////////////////////////////////////////////////////////////////
    template <unsigned short MAX_LINE_LENGTH,
              unsigned char  MAX_WHITESPACE_CHARS,
              bool           STRICT_CRLF>
    class field_line
    {
    public:
      /// @enum Header the state of the header field line parser
      enum class Header
      {
        NAME,         ///< the header name field
        VALUE_LS,     ///< the header value leading white space
        VALUE,        ///< the header value
        LF,           ///< the line feed (if any)
        VALID,        ///< the header line is valid
        ERROR_LENGTH, ///< the header line is longer than MAX_LINE_LENGTH
        ERROR_CRLF,   ///< STRICT_CRLF is true and LF was received without CR
        ERROR_WS      ///< the whitespace is longer than MAX_WHITESPACE_CHARS
      };

    private:
      /// Field information
      std::string   name_ {};                ///< the field name (lower case)
      std::string   value_ {};               ///< the field value
      size_t        length_ { 0u };          ///< the length of the header line in bytes
      size_t        ws_count_ { 0u };        ///< the current whitespace count
      Header        state_ { Header::NAME }; ///< the current parsing state

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @retval state the current state of the parser.
      bool parse_char(char c)
      {
        // Ensure that the overall header length is within limits
        if (++length_ > MAX_LINE_LENGTH)
          state_ = Header::ERROR_LENGTH;

        switch (state_)
        {
        case Header::NAME:
          if (std::isalpha(c) || ('-' == c))
            name_.push_back(static_cast<char>(std::tolower(c)));
          else if (':' == c)
            state_ = Header::VALUE_LS;
          else
            return false;
          break;

        case Header::VALUE_LS:
          // Ignore leading whitespace
          if (std::isblank(c))
            // but only upto to a limit!
            if (++ws_count_ > MAX_WHITESPACE_CHARS)
            {
              state_ = Header::ERROR_WS;
              return false;
            }
            else
              break;
          else
            state_ = Header::VALUE;
          [[fallthrough]]; // intentional fall-through

        case Header::VALUE:
          // The header line should end with an \r\n...
          if (!is_end_of_line(c))
            value_.push_back(c);
          else if ('\r' == c)
            state_ = Header::LF;
          else // ('\n' == c)
          {
            if (STRICT_CRLF)
            {
              state_ = Header::ERROR_CRLF;
              return false;
            }
            else
              state_ = Header::VALID;
          }
          break;

        case Header::LF:
          if ('\n' == c)
            state_ = Header::VALID;
          else
            return false;
          break;

        default:
          return false;
        }

        return true;
      }

    public:

      /// Default Constructor.
      field_line() = default;

      /// clear the field_line.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        name_.clear();
        value_.clear();
        length_ = 0u;
        ws_count_ = 0u;
        state_ = Header::NAME;
      }

      /// swap member variables with another field_line.
      /// @param other the other field_line
      void swap(field_line& other) noexcept
      {
        name_.swap(other.name_);
        value_.swap(other.value_);
        std::swap(length_, other.length_);
        std::swap(ws_count_, other.ws_count_);
        std::swap(state_, other.state_);
      }

      /// Parse an individual http header field and extract the field name
      /// (transformed to lower case) and value.
      /// @retval iter an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the buffer.
      /// @return true if a valid HTTP header, false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        while ((iter != end) && (Header::VALID != state_))
        {
          char c(static_cast<char>(*iter++));
          if (!parse_char(c))
            return false;
          else if (Header::VALID == state_)
          { // determine whether the next line is a continuation header
            if ((iter != end) && std::isblank(*iter))
            {
              value_.push_back(' ');
              state_ = Header::VALUE_LS;
            }
          }
        }

        return (Header::VALID == state_);
      }

      /// Accessor for the field name.
      /// @return the field name (as a lower case string)
      const std::string& name() const noexcept
      { return name_; }

      /// Accessor for the field value.
      /// @return the field value in the same case that it was received in.
      const std::string& value() const noexcept
      { return value_; }

      /// Calculate the length of the header.
      size_t length() const noexcept
      { return name_.size() + value_.size(); }
    }; // class field_line

    /// An unordered_map of strings indexed by strings.
    typedef std::unordered_map<std::string, std::string> StringMap;

    //////////////////////////////////////////////////////////////////////////
    /// @class message_headers
    /// The collection of HTTP headers received with a request, response or a
    /// chunk (trailers).
    /// Note: the parse function converts the received field names into lower
    /// case before storing them in a unordered_map for efficient access.
    /// @tparam MAX_HEADER_NUMBER the maximum number of HTTP header field lines:
    /// default 100, max 65534.
    /// @tparam MAX_HEADER_LENGTH the maximum cumulative length the HTTP header
    /// fields: default 8190, max 4 billion.
    /// @tparam MAX_LINE_LENGTH the maximum length of an HTTP header field line:
    /// default 1024, min 1, max 65534.
    /// @tparam MAX_WHITESPACE_CHARS the maximum number of consectutive whitespace
    /// characters allowed in a request: default 8, min 1, max 254.
    /// @tparam STRICT_CRLF enforce strict parsing of CRLF, default true.
    /// @see rx_request
    /// @see rx_response
    /// @see rx_chunk
    //////////////////////////////////////////////////////////////////////////
    template <unsigned short MAX_HEADER_NUMBER    = 100,
              size_t         MAX_HEADER_LENGTH    = 8190,
              unsigned short MAX_LINE_LENGTH      = 1024,
              unsigned char  MAX_WHITESPACE_CHARS = 8,
              bool           STRICT_CRLF          = true>
    class message_headers
    {
      /// Parser parameters
      unsigned short max_header_number_ { 8u };    ///< the max no of header fields
      size_t         max_header_length_ { 1024u }; ///< the max cumulative length

      /// The HTTP message header fields.
      StringMap fields_ {};
      /// The current field being parsed
      field_line<MAX_LINE_LENGTH, MAX_WHITESPACE_CHARS, STRICT_CRLF> field_ {};
      bool       valid_ { false }; ///< true if the headers are valid
      size_t     length_ { 0u };   ///< the length of the message headers

    public:

      /// Default Constructor.
      message_headers() = default;

      /// Clear the message_headers.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        fields_.clear();
        field_.clear();
        valid_ = false;
        length_ = 0;
      }

      /// Swap member variables with another message_headers.
      /// @param other the other message_headers
      void swap(message_headers& other) noexcept
      {
        fields_.swap(other.fields_);
        field_.swap(other.field_);
        std::swap(valid_, other.valid_);
        std::swap(length_, other.length_);
      }

      /// Parse message_headers from a received request or response.
      /// @retval iter reference to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        while (iter != end && !is_end_of_line(*iter))
        {
         // field_line field;
          if (!field_.parse(iter, end))
            return false;

          length_ += field_.length();
          add(field_.name(), field_.value());
          field_.clear();

          if ((length_ > max_header_length_)
           || (fields_.size() > max_header_number_))
            return false;
        }

        // Parse the blank line at the end of message_headers and
        // chunk trailers
        if (iter == end || !is_end_of_line(*iter))
          return false;

        // allow \r\n or just \n
        if ('\r' == *iter)
          ++iter;

        if ((iter == end) || ('\n' != *iter))
           return false;

        ++iter;
        valid_ = true;
        return valid_;
      }

      /// Add a header to the collection.
      /// @param name the field name (in lower case)
      /// @param value the field value.
      void add(std::string_view name, std::string_view value)
      {
        auto iter(fields_.find(name.data()));
        // if the field name was found previously
        if (iter != fields_.end())
        {
          char separator((name.find(COOKIE) != std::string::npos) ? ';' : ',');
          iter->second.append({separator});
          iter->second.append(value);
        }
        else
          fields_.insert(StringMap::value_type(name, value));
      }

      /// Find the value for a given header name.
      /// Note: the name must be in lowercase for received message_headers.
      /// @param name the name of the header.
      /// @return the value, blank if not found
      std::string_view find(std::string_view name) const
      {
        const auto iter(fields_.find(name.data()));
        return (iter != fields_.end()) ? iter->second : std::string_view();
      }

      /// Find the value for a given header id.
      /// @param field_id the id of the header.
      /// @return the value, blank if not found
      std::string_view find(header_field::id field_id) const
      { return find(header_field::lowercase_name(field_id)); }

      /// If there is a Content-Length field return its size.
      /// @return the value of the Content-Length field or
      /// -1 if it was invalid.
      /// May also return zero if it was not found.
      std::ptrdiff_t content_length() const noexcept
      {
        // Find whether there is a content length field.
        auto content_length(find(header_field::LC_CONTENT_LENGTH));
        return (content_length.empty()) ? 0 : from_dec_string(content_length);
      }

      /// Whether Chunked Transfer Coding is applied to the message.
      /// @return true if there is a transfer-encoding header and it does
      /// NOT contain the keyword "identity". See RFC2616 section 4.4 para 2.
      bool is_chunked() const
      {
        // Find whether there is a transfer encoding header.
        std::string xfer_encoding(find(header_field::LC_TRANSFER_ENCODING));
        if (xfer_encoding.empty())
          return false;

        std::transform(xfer_encoding.begin(), xfer_encoding.end(),
                       xfer_encoding.begin(), ::tolower);
        // Note: is transfer encoding if "identity" is NOT found.
        return (xfer_encoding.find(IDENTITY) == std::string::npos);
      }

      /// Whether the connection should be closed after the response.
      /// @return true if there is a Connection: close header, false otherwise
      bool close_connection() const
      {
        // Find whether there is a connection header.
        std::string connection(find(header_field::LC_CONNECTION));
        if (connection.empty())
          return false;

        std::transform(connection.begin(), connection.end(),
                       connection.begin(), ::tolower);
        return (connection.find(CLOSE) != std::string::npos);
      }

      /// Whether the client expects a "100-continue" response.
      /// @return true if there is an Expect: 100-continue header, false
      /// otherwise
      bool expect_continue() const
      {
        // Find whether there is a expect header.
        std::string expect(find(header_field::LC_EXPECT));
        if (expect.empty())
          return false;

        std::transform(expect.begin(), expect.end(),
                       expect.begin(), ::tolower);
        return (expect.find(CONTINUE) != std::string::npos);
      }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const noexcept
      { return valid_; }

      /// Accessor for the header fields.
      /// @return headers as a map
      const StringMap& fields() const
      { return fields_; }
      
      /// Output the message_headers as a string.
      /// Note: it is NOT terminated with an extra CRLF so that it parses
      /// the are_headers_split function.
      /// @return a string containing all of the message_headers.
      std::string to_string() const
      {
        std::string output;
        for (StringMap::const_iterator
             iter(fields_.begin()); iter != fields_.end(); ++iter)
          output += header_field::to_header(iter->first, iter->second);

        return output;
      }
    };

    /// A function to determine whether the header string contains an extra
    /// CRLF pair, which could cause HTTP message spliting.
    inline bool are_headers_split(std::string_view headers) noexcept
    {
      char prev('0');
      char pprev('0');

      if (!headers.empty())
      {
        auto iter(headers.cbegin());
        for(; iter != headers.cend(); ++iter)
        {
          if (*iter == '\n')
          {
            if (prev == '\n')
              return true;
            else if ((prev == '\r') && (pprev == '\n'))
              return true;
          }

          pprev = prev;
          prev = *iter;
        }
      }

      return false;
    }
  }
}

#endif
