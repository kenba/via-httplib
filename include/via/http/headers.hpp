#ifndef HEADERS_HPP_VIA_HTTPLIB_
#define HEADERS_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
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
#include "character.hpp"
#include <map>

namespace via
{
  namespace http
  {
    /// @enum Rx is the receiver parsing state and is valid for both the
    /// request and response receivers.
    enum Rx
    {
      RX_INVALID,         ///< the message is invalid
      RX_EXPECT_CONTINUE, ///< the client expects a 100 Continue response
      RX_INCOMPLETE,      ///< the message requires more data
      RX_VALID,           ///< a valid request or response
      RX_CHUNK            ///< a valid chunk received
    };

    //////////////////////////////////////////////////////////////////////////
    /// @class field_line
    /// An HTTP header field.
    //////////////////////////////////////////////////////////////////////////
    class field_line
    {
    public:
      /// @enum Header the state of the header field line parser
      enum Header
      {
        HEADER_NAME,         ///< the header name field
        HEADER_VALUE_LS,     ///< the header value leading white space
        HEADER_VALUE,        ///< the header value
        HEADER_LF,           ///< the line feed (if any)
        HEADER_VALID,        ///< the header line is valid
        HEADER_ERROR_LENGTH, ///< the header line is longer than max_line_length_
        HEADER_ERROR_CRLF,   ///< strict_crlf_ is true and LF was received without CR
        HEADER_ERROR_WS      ///< the whitespace is longer than max_whitespace_
      };

    private:

      /// Parser parameters
      bool           strict_crlf_;     ///< enforce strict parsing of CRLF
      unsigned char  max_whitespace_;  ///< the max no of consectutive whitespace characters.
      unsigned short max_line_length_; ///< the max length of a field line

      /// Field information
      std::string   name_;     ///< the field name (lower case)
      std::string   value_;    ///< the field value
      size_t        length_;   ///< the length of the header line in bytes
      size_t        ws_count_; ///< the current whitespace count
      Header        state_;    ///< the current parsing state

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @retval state the current state of the parser.
      bool parse_char(char c);

    public:

      /// Constructor.
      /// Sets the parser parameters and all member variables to their initial
      /// state.
      /// @param strict_crlf enforce strict parsing of CRLF.
      /// @param max_whitespace the maximum number of consectutive whitespace
      /// characters allowed in a request: min 1, max 254.
      /// @param max_line_length the maximum length of an HTTP header field line:
      /// max 65534.
      explicit field_line(bool           strict_crlf,
                          unsigned char  max_whitespace,
                          unsigned short max_line_length) :
        strict_crlf_(strict_crlf),
        max_whitespace_(max_whitespace),
        max_line_length_(max_line_length),
        name_(""),
        value_(""),
        length_(0),
        ws_count_(0),
        state_(HEADER_NAME)
      {}

      /// clear the field_line.
      /// Sets all member variables to their initial state.
      void clear() NOEXCEPT
      {
        name_.clear();
        value_.clear();
        length_ = 0;
        ws_count_ = 0;
        state_ = HEADER_NAME;
      }

      /// swap member variables with another field_line.
      /// @param other the other field_line
      void swap(field_line& other) NOEXCEPT
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
        while ((iter != end) && (HEADER_VALID != state_))
        {
          char c(static_cast<char>(*iter++));
          if (!parse_char(c))
            return false;
          else if (HEADER_VALID == state_)
          { // determine whether the next line is a continuation header
            if ((iter != end) && is_space_or_tab(*iter))
            {
              value_.push_back(' ');
              state_ = HEADER_VALUE_LS;
            }
          }
        }

        return (HEADER_VALID == state_);
      }

      /// Accessor for the field name.
      /// @return the field name (as a lower case string)
      const std::string& name() const NOEXCEPT
      { return name_; }

      /// Accessor for the field value.
      /// @return the field value in the same case that it was received in.
      const std::string& value() const NOEXCEPT
      { return value_; }

      /// Calculate the length of the header.
      size_t length() const NOEXCEPT
      { return name_.size() + value_.size(); }
    }; // class field_line

    //////////////////////////////////////////////////////////////////////////
    /// @class message_headers
    /// The collection of HTTP headers received with a request, response or a
    /// chunk (trailers).
    /// Note: the parse function converts the received field names into lower
    /// case before storing them in a unordered_map for efficient access.
    /// @see rx_request
    /// @see rx_response
    /// @see rx_chunk
    //////////////////////////////////////////////////////////////////////////
    class message_headers
    {
      /// Parser parameters
      unsigned short max_header_number_; ///< the max no of header fields
      size_t         max_header_length_; ///< the max cumulative length

      /// The HTTP message header fields.
      std::map<std::string, std::string> fields_;
      field_line field_; ///< the current field being parsed
      bool       valid_; ///< true if the headers are valid
      size_t     length_; ///< the length of the message headers

    public:

      /// Constructor.
      /// Sets the parser parameters and all member variables to their initial
      /// state.
      /// @param strict_crlf enforce strict parsing of CRLF.
      /// @param max_whitespace the maximum number of consectutive whitespace
      /// characters allowed in a request: min 1, max 254.
      /// @param max_line_length the maximum length of an HTTP header field line:
      /// max 65534.
      /// @param max_header_number the maximum number of HTTP header field lines:
      /// max 65534.
      /// @param max_header_length the maximum cumulative length the HTTP header
      /// fields: max 4 billion.
      explicit message_headers(bool           strict_crlf,
                               unsigned char  max_whitespace,
                               unsigned short max_line_length,
                               unsigned short max_header_number,
                               size_t         max_header_length) :
        max_header_number_(max_header_number),
        max_header_length_(max_header_length),
        fields_(),
        field_(strict_crlf, max_whitespace, max_line_length),
        valid_(false),
        length_(0)
      {}

      /// Clear the message_headers.
      /// Sets all member variables to their initial state.
      void clear() NOEXCEPT
      {
        fields_.clear();
        field_.clear();
        valid_ = false;
        length_ = 0;
      }

      /// Swap member variables with another message_headers.
      /// @param other the other message_headers
      void swap(message_headers& other) NOEXCEPT
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
      void add(const std::string& name, const std::string& value)
      { fields_.insert(std::map<std::string, std::string>::value_type
                     (name, value)); }

      /// Find the value for a given header name.
      /// Note: the name must be in lowercase for received message_headers.
      /// @param name the name of the header.
      /// @return the value, blank if not found
      const std::string& find(const std::string& name) const;

      /// Find the value for a given header id.
      /// @param field_id the id of the header.
      /// @return the value, blank if not found
      const std::string& find(header_field::id field_id) const
      { return find(header_field::lowercase_name(field_id)); }

      /// If there is a Content-Length field return its size.
      /// @return the value of the Content-Length field or
      /// -1 if it was invalid.
      /// May also return zero if it was not found.
      std::ptrdiff_t content_length() const NOEXCEPT;

      /// Whether Chunked Transfer Coding is applied to the message.
      /// @return true if there is a transfer-encoding header and it does
      /// NOT contain the keyword "identity". See RFC2616 section 4.4 para 2.
      bool is_chunked() const;

      /// Whether the connection should be closed after the response.
      /// @return true if there is a Connection: close header, false otherwise
      bool close_connection() const;

      /// Whether the client expects a "100-continue" response.
      /// @return true if there is an Expect: 100-continue header, false
      /// otherwise
      bool expect_continue() const;

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const NOEXCEPT
      { return valid_; }

      /// Output the message_headers as a string.
      /// Note: it is NOT terminated with an extra CRLF tso that it parses
      /// the are_headers_split function.
      /// @return a string containing all of the message_headers.
      std::string to_string() const;
    };

    /// A function to determine whether the header string contains an extra
    /// CRLF pair, which could cause HTTP message spliting.
    bool are_headers_split(std::string const& headers) NOEXCEPT;
  }
}

#endif
