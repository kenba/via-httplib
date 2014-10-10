#ifndef HEADERS_HPP_VIA_HTTPLIB_
#define HEADERS_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
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
    /// @enum content_length_error gives an error states for an invalid
    /// content length header.
    enum content_length_error
    { CONTENT_LENGTH_INVALID = ULONG_MAX };

    /// @enum receiver_parsing_state is valid for both the request and
    /// response receivers.
    enum receiver_parsing_state
    {
      RX_INVALID,         ///< the message is invalid
      RX_LENGTH_REQUIRED, ///< the message requires a content-length header
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
      /// @enum parsing_state the state of the field line parser
      enum parsing_state
      {
        HEADER_NAME,     ///< the header name field
        HEADER_VALUE_LS, ///< the header value leading white space
        HEADER_VALUE,    ///< the header value
        HEADER_LF,       ///< the line feed (if any)
        HEADER_END
      };

    private:

      std::string   name_;  ///< the field name (lower case)
      std::string   value_; ///< the field value
      parsing_state state_; ///< the current parsing state

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @retval state the current state of the parser.
      bool parse_char(char c);

    public:

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit field_line() :
        name_(""),
        value_(""),
        state_(HEADER_NAME)
      {}

      /// clear the field_line.
      /// Sets all member variables to their initial state.
      void clear()
      {
        name_.clear();
        value_.clear();
        state_ = HEADER_NAME;
      }

      /// swap member variables with another field_line.
      /// @param other the other field_line
      void swap(field_line& other)
      {
        name_.swap(other.name_);
        value_.swap(other.value_);
        std::swap(state_, other.state_);
      }

      /// Parse an individual http header field and extract the field name
      /// (transformed to lower case) and value.
      /// @retval iter an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the buffer.
      /// @return true if a valid HTTP header, false otherwise.
      template<typename ForwardIterator1, typename ForwardIterator2>
      bool parse(ForwardIterator1& iter, ForwardIterator2 end)
      {
        while ((iter != end) && (HEADER_END != state_))
        {
          // following line added to compile with vectors of Unsigned chars
          // using Visual Studio 2012 (Beta)
          char c(static_cast<char>(*iter++));
          if (!parse_char(c))
            return false;
          else if (HEADER_END == state_)
          { // determine whether the next line is a continuation header
            if ((iter != end) && is_space_or_tab(*iter))
            {
              value_.push_back(' ');
              state_ = HEADER_VALUE_LS;
            }
          }
        }

        return (HEADER_END == state_);
      }

      /// Accessor for the field name.
      /// @return the field name (as a lower case string)
      const std::string& name() const
      { return name_; }

      /// Accessor for the field value.
      /// @return the field value in the same case that it was received in.
      const std::string& value() const
      { return value_; }
    }; // class field_line

    //////////////////////////////////////////////////////////////////////////
    /// @class message_headers
    /// The collection of HTTP headers received with a request, response or a
    /// chunk (trailers).
    /// Note: the parse function converts the received field names into lower
    /// case before storing them in a map for efficient access.
    /// @see rx_request
    /// @see rx_response
    /// @see rx_chunk
    //////////////////////////////////////////////////////////////////////////
    class message_headers
    {
      /// The HTTP message header fields.
      /// Note: A C++11 unordered_map or a hash_map would be better
      /// But hash_map is non-standard. TODO template?
      std::map<std::string, std::string> fields_;
      field_line field_; ///< the current field being parsed
      bool       valid_; ///< true if the headers are valid

    public:

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit message_headers() :
        fields_(),
        field_(),
        valid_(false)
      {}

      /// Clear the message_headers.
      /// Sets all member variables to their initial state.
      void clear()
      {
        fields_.clear();
        field_.clear();
        valid_ = false;
      }

      /// Swap member variables with another message_headers.
      /// @param other the other message_headers
      void swap(message_headers& other)
      {
        fields_.swap(other.fields_);
        field_.swap(other.field_);
        std::swap(valid_, other.valid_);
      }

      /// Parse message_headers from a received request or response.
      /// @retval iter reference to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator1, typename ForwardIterator2>
      bool parse(ForwardIterator1& iter, ForwardIterator2 end)
      {
        while (iter != end && !is_end_of_line(*iter))
        {
         // field_line field;
          if (!field_.parse(iter, end))
            return false;

          add(field_.name(), field_.value());
          field_.clear();
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
      /// @param id the id of the header.
      /// @return the value, blank if not found
      const std::string& find(header_field::field_id id) const
      { return find(header_field::lowercase_name(id)); }

      /// If there is a Content-Length field return its size.
      /// @return the value of the Content-Length field or
      /// CONTENT_LENGTH_INVALID if it was invalid.
      /// May also return zero if it was not found.
      size_t content_length() const;

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
      bool valid() const
      { return valid_; }

      /// Output the message_headers as a string.
      /// @return a string containing all of the message_headers.
      std::string to_string() const;
    };

  }
}

#endif
