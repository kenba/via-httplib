#pragma once

#ifndef HEADERS_HPP_VIA_HTTPLIB_
#define HEADERS_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "header_field.hpp"
#include "character.hpp"
#include <map>

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class field_line
    //////////////////////////////////////////////////////////////////////////
    class field_line
    {
    public:
      enum parsing_state
      {
        HEADER_NAME,
        HEADER_VALUE_LS,
        HEADER_VALUE,
        HEADER_LF,
        HEADER_END
      };

    private:

      std::string name_;
      std::string value_;
      parsing_state state_;

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @retval state the current state of the parser.
      bool parse_char(char c);

    public:

      explicit field_line() :
        name_(""),
        value_(""),
        state_(HEADER_NAME)
      {}

      void clear()
      {
        name_.clear();
        value_.clear();
        state_ = HEADER_NAME;
      }

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
      /// @retval name the header field name (lowercase).
      /// @retval value the header field value.
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

      const std::string& name() const
      { return name_; }

      const std::string& value() const
      { return value_; }
    }; // class field_line

    //////////////////////////////////////////////////////////////////////////
    /// @class message_headers
    //////////////////////////////////////////////////////////////////////////
    class message_headers
    {
      /// The HTTP message header fields.
      /// Note: A C++11 unordered_map or a hash_map would be better
      /// But hash_map is non-standard. TODO template?
      std::map<std::string, std::string> fields_;
      field_line field_;
      bool valid_;

    public:

      explicit message_headers() :
        fields_(),
        field_(),
        valid_(false)
      {}

      void clear()
      {
        fields_.clear();
        field_.clear();
        valid_ = false;
      }

      void swap(message_headers& other)
      {
        fields_.swap(other.fields_);
        field_.swap(other.field_);
        std::swap(valid_, other.valid_);
      }

      /// Parse message_headers from received data.
      /// @retval next reference to an iterator to the start of the data.
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

        // Parse the blank line at the end of the message_headers (and footers)
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
      void add(const std::string& name, const std::string& value)
      { fields_.insert(std::map<std::string, std::string>::value_type
                     (name, value)); }

      /// Find the value for a given header name.
      /// Note: the name must be in lowercase for received message_headers.
      /// @param name the name of the header.
      /// @return the value, blank if not found
      const std::string& find(const std::string& name) const;

      /// Find the value for a given header name.
      /// Note: the name must be in lowercase for received message_headers.
      /// @param name the name of the header.
      /// @return the value, blank if not found
      const std::string& find(header_field::field_id id) const
      { return find(header_field::lowercase_name(id)); }

      /// If there is a Content-Length field return its size.
      /// @return the size of the Content-Length field.
      /// Zero if it was not found or invalid.
      size_t content_length() const;

      /// Whether Chunked Transfer Coding is applied to the message.
      /// @return true if there is a transfer-encoding header and it does
      /// NOT contain the keyword "identity". See RFC2616 section 4.4 para 2.
      bool is_chunked() const;

      bool valid() const
      { return valid_; }

      /// Output the message_headers as a string.
      /// @return a string containing all of the message_headers.
      std::string to_string() const;
    };

  }
}

#endif
