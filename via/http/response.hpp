#pragma once

#ifndef RESPONSE_HPP_VIA_HTTPLIB_
#define RESPONSE_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file response.hpp
/// @brief Classes to parse and encode HTTP responses.
//////////////////////////////////////////////////////////////////////////////
#include "response_status.hpp"
#include "headers.hpp"
#include "chunk.hpp"
#include <algorithm>
#include <cassert>

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class response_line
    /// The HTTP response start line.
    //////////////////////////////////////////////////////////////////////////
    class response_line
    {
    public:
      /// @enum parsing_state the state of the response line parser.
      enum parsing_state
      {
        RESP_HTTP,        ///< HTTP/ -H
        RESP_HTTP_T1,     ///< HTTP/ first T
        RESP_HTTP_T2,     ///< HTTP/ second T
        RESP_HTTP_P,      ///< HTTP/ P
        RESP_HTTP_SLASH,  ///< HTTP/ slash
        RESP_HTTP_MAJOR,  ///< HTTP major version number
        RESP_HTTP_MINOR,  ///< HTTP minor version number
        RESP_HTTP_STATUS, ///< response status code
        RESP_HTTP_REASON, ///< response status reason
        RESP_HTTP_LF,     ///< the line feed (if any)
        RESP_HTTP_END
      };

    private:

      int major_version_;         ///< the HTTP major version number
      int minor_version_;         ///< the HTTP minor version number
      int status_;                ///< the response status code
      std::string reason_phrase_; ///< response status reason phrase
      parsing_state state_;       ///< the current parsing state
      bool major_read_;           ///< true if major version was read
      bool minor_read_;           ///< true if minor version was read
      bool status_read_;          ///< true if status code was read
      bool valid_;                ///< true if the response line is valid
      bool fail_;                 ///< true if the response line failed validation

      /// Parse an individual character.
      /// @param c the character to be parsed.
      /// @return true if valid, false otherwise.
      bool parse_char(char c);

    public:

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit response_line() :
        major_version_(0),
        minor_version_(0),
        status_(0),
        reason_phrase_(""),
        state_(RESP_HTTP),
        major_read_(false),
        minor_read_(false),
        status_read_(false),
        valid_(false),
        fail_(false)
      {}

      /// Clear the response_line.
      /// Sets all member variables to their initial state.
      void clear()
      {
        major_version_ = 0;
        minor_version_ = 0;
        status_ = 0;
        reason_phrase_.clear();
        state_ = RESP_HTTP;
        major_read_ =  false;
        minor_read_ =  false;
        status_read_ = false;
        valid_ =  false;
        fail_ = false;
      }

      /// Swap member variables with another response_line.
      /// @param other the other response_line
      void swap(response_line& other)
      {
        std::swap(major_version_, other.major_version_);
        std::swap(minor_version_, other.minor_version_);
        std::swap(status_, other.status_);
        reason_phrase_.swap(other.reason_phrase_);
        std::swap(state_, other.state_);
        std::swap(major_read_, other.major_read_);
        std::swap(minor_read_, other.minor_read_);
        std::swap(status_read_, other.status_read_);
        std::swap(valid_, other.valid_);
        std::swap(fail_, other.fail_);
      }

      /// Virtual destructor.
      /// Since the class is inherited...
      virtual ~response_line()
      {}

      /// Parse the line as an HTTP response.
      /// @retval iter reference to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator1, typename ForwardIterator2>
      bool parse(ForwardIterator1& iter, ForwardIterator2 end)
      {
        while ((iter != end) && (RESP_HTTP_END != state_))
        {
          char c(static_cast<char>(*iter++));
          if ((fail_ = !parse_char(c))) // Note: deliberate assignment
            return false;
        }
        valid_ = (RESP_HTTP_END == state_);
        return valid_;
      }

      /// Accessor for the HTTP major version number.
      /// @return the major version number.
      int major_version() const
      { return major_version_; }

      /// Accessor for the HTTP minor version number.
      /// @return the minor version number.
      int minor_version() const
      { return minor_version_; }

      /// Accessor for the response status.
      /// @return the response status number.
      int status() const
      { return status_; }

      /// Accessor for the response reason string.
      /// @return the response reason string.
      const std::string& reason_phrase() const
      { return reason_phrase_; }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const
      { return valid_; }

      /// Accessor for the fail flag.
      /// @return the fail flag.
      bool fail() const
      { return fail_; }

      ////////////////////////////////////////////////////////////////////////
      // Encoding interface.

      /// Constructor for creating a response for one of the standard
      /// responses defined in RFC2616.
      /// @see http::response_status::status_code
      /// @param status the response status code
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit response_line(response_status::status_code status,
                             int minor_version = 1,
                             int major_version = 1) :
        major_version_(major_version),
        minor_version_(minor_version),
        status_(status),
        reason_phrase_(response_status::reason_phrase(status)),
        state_(RESP_HTTP_END),
        major_read_(true),
        minor_read_(true),
        status_read_(true),
        valid_(true),
        fail_(false)
      {}

      /// Constructor for creating a non-standard response.
      /// @param status the response status
      /// @param reason_phrase the reason phrase for the response status.
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit response_line(int status,
                             std::string reason_phrase,
                             int minor_version = 1,
                             int major_version = 1) :
        major_version_(major_version),
        minor_version_(minor_version),
        status_(status),
        reason_phrase_(!reason_phrase.empty() ? reason_phrase :
               response_status::reason_phrase
                    (static_cast<response_status::status_code>(status))),
        state_(RESP_HTTP_END),
        major_read_(true),
        minor_read_(true),
        status_read_(true),
        valid_(true),
        fail_(false)
      {}

      /// Set the response status for standard responses.
      /// @see http::response_status::status_code
      /// @param status the response status.
      void set_status(response_status::status_code status)
      {
        status_ = status;
        reason_phrase_ = response_status::reason_phrase(status);
      }

      /// Set the response status and reason phrase.
      /// @param status the response status.
      /// @param reason_phrase the the response reason phrase.
      void set_status_and_reason(int status, const std::string& reason_phrase)
      {
        status_ = status;
        reason_phrase_ = reason_phrase;
      }

      /// Set the HTTP minor version.
      /// @param minor_version the HTTP minor version.
      void set_minor_version(int minor_version)
      { minor_version_ = minor_version; }

      /// Set the HTTP major version.
      /// @param major_version the HTTP major version.
      void set_major_version(int major_version)
      { major_version_ = major_version; }

      /// Output as a string.
      /// @return a string containing the response line.
      std::string to_string() const;
    }; // class response_line

    //////////////////////////////////////////////////////////////////////////
    /// @class rx_response
    /// A class to receive an HTTP response.
    //////////////////////////////////////////////////////////////////////////
    class rx_response : public response_line
    {
      message_headers headers_; ///< the HTTP headers for the response
      bool valid_;               ///< true if the response is valid

    public:

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit rx_response() :
        response_line(),
        headers_(),
        valid_(false)
      {}

      /// Clear the rx_response.
      /// Sets all member variables to their initial state.
      void clear()
      {
        response_line::clear();
        headers_.clear();
        valid_ =  false;
      }

      /// Swap member variables with another rx_response.
      /// @param other the other rx_response
      void swap(rx_response& other)
      {
        response_line::swap(other);
        headers_.swap(other.headers_);
        std::swap(valid_, other.valid_);
      }

      /// Parse an HTTP response.
      /// @retval iter reference to an iterator to the start of the data.
      /// If the response is valid it will refer to:
      ///   - the start of the response body if content_length > 0,
      ///   - the start of the first data chunk if is_chunked(),
      ///   - the start of the next http response, or
      ///   - the end of the data buffer.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        if (!response_line::valid() && !response_line::parse(iter, end))
          return false;

        if (!headers_.valid() && !headers_.parse(iter, end))
          return false;

        valid_ = true;
        return valid_;
      }

      /// Accessor for the response message headers.
      /// @return a constant reference to the message_headers
      const message_headers& headers() const
      { return headers_; }

      /// The size in the content_length header (if there is one)
      /// @return the content_length header value.
      size_t content_length() const
      { return headers_.content_length(); }

      /// Whether chunked transfer encoding is enabled.
      /// @return true if chunked transfer encoding is enabled.
      bool is_chunked() const
      { return headers_.is_chunked(); }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const
      { return valid_; }

      /// Whether the connection should be kept alive.
      /// I.e. if the request is HTTP 1.1 and there is not a connection: close
      /// header field.
      /// @return true if it should be kept alive, false otherwise.
      bool keep_alive() const
      {
        return major_version() >= 1 &&
               minor_version() >= 1 &&
               !headers_.close_connection();
      }
    }; // class rx_response

    //////////////////////////////////////////////////////////////////////////
    /// @class tx_response
    /// A class to encode an HTTP response.
    //////////////////////////////////////////////////////////////////////////
    class tx_response : public response_line
    {
      std::string header_string_; ///< The headers as a string.

    public:

      /// Constructor for creating a response for one of the standard
      /// responses defined in RFC2616.
      /// @see http::response_status::status_code
      /// @param status the response status code
      /// @param header_string default blank
      explicit tx_response(response_status::status_code status,
                           std::string header_string = "") :
        response_line(status),
        header_string_(header_string)
      {}

      /// Constructor for creating a non-standard response.
      /// @param reason_phrase the reason phrase for the response status.
      /// @param status the response status
      /// @param header_string default blank
      explicit tx_response(const std::string& reason_phrase,
                           int status,
                           std::string header_string = "") :
        response_line(status, reason_phrase) ,
        header_string_(header_string)
      {}

      /// Add a standard header to the response.
      /// @see http::header_field::field_id
      /// @param id the header field id
      /// @param value the header field value
      void add_header(header_field::field_id id, const std::string& value)
      { header_string_ += header_field::to_header(id, value);  }

      /// Add a free form header to the response.
      /// @param field the header field name
      /// @param value the header field value
      void add_header(std::string const& field, const std::string& value)
      { header_string_ += header_field::to_header(field, value);  }

      /// Add a Date header to the response.
      void add_date_header()
      { header_string_ += header_field::date_header(); }

      /// Add a Server header to the response.
      void add_server_header()
      { header_string_ += header_field::server_header(); }

      /// Add an http content length header line for the given size.
      void add_content_length_header(size_t size)
      { header_string_ += header_field::content_length(size); }

      /// Whether this is a continue response.
      /// @return true if this is a continue response, false otherwise.
      bool is_continue() const
      { return status() == response_status::CONTINUE; }

      /// The http message header string.
      /// @param content_length the size of the message body for the
      /// content_length header.
      /// @return The http message header as a std:string.
      std::string message(size_t content_length = 0) const
      {
        std::string output(response_line::to_string());
        output += header_string_;

        // Ensure that it's got a content length header unless
        // a tranfer encoding is being applied.
        bool no_content_length(std::string::npos == header_string_.find
              (header_field::standard_name(header_field::CONTENT_LENGTH)));
        bool no_transfer_encoding(std::string::npos == header_string_.find
              (header_field::standard_name(header_field::TRANSFER_ENCODING)));
        if (no_content_length && no_transfer_encoding)
          output += header_field::content_length(content_length);
        output += CRLF;

        return output;
      }
    }; // class tx_response

    //////////////////////////////////////////////////////////////////////////
    /// @class response_receiver
    /// A template class to receive HTTP responses and any associated data.
    //////////////////////////////////////////////////////////////////////////
    template <typename Container>
    class response_receiver
    {
      rx_response response_; ///< the received response
      rx_chunk<Container> chunk_;    ///< the received chunk
      Container   body_;     ///< the response body or data for the last chunk

    public:

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit response_receiver() :
        response_(),
        chunk_(),
        body_()
      {}

      /// clear the response_receiver.
      /// Sets all member variables to their initial state.
      void clear()
      {
        response_.clear();
        chunk_.clear();
        body_.clear();
      }

      /// Accessor for the HTTP response header.
      /// @return a constant reference to an rx_response.
      rx_response const& response() const
      { return response_; }

      /// Accessor for the received chunk.
      /// @return a constant reference to the received chunk.
      rx_chunk<Container> const& chunk() const
      { return chunk_; }

      /// Accessor for the response body / last chunk data.
      /// @return a constant reference to the data.
      Container const& body() const
      { return body_; }

      /// Receive data for an HTTP response, body or data chunk.
      /// @param iter an iterator to the beginning of the received data.
      /// @param end an iterator to the end of the received data.
      template<typename ForwardIterator>
      receiver_parsing_state receive(ForwardIterator& iter,
                                     ForwardIterator end)
      {
        // building a response
        bool response_parsed(!response_.valid());
        if (response_parsed)
        {
          chunk_.clear();

          // failed to parse response
          if (!response_.parse(iter, end))
          {
            // if a parsing error (not run out of data)
            if ((iter != end) || response_.fail())
            {
              clear();
              return RX_INVALID;
            }
            else
              return RX_INCOMPLETE;
          }
        }

        // build a response body or receive a chunk
        assert(response_.valid());
        if (!response_.is_chunked())
        {
          // if there is a content length header, ensure it's valid
          auto content_length(response_.content_length());
          if (content_length == CONTENT_LENGTH_INVALID)
          {
            clear();
            return RX_INVALID;
          }

          // if there's a message body then insist on a content length header
          long rx_size(end - iter);
          long required(content_length - body_.size());
          if ((rx_size > 0) && (content_length == 0) &&
              response_.headers().find(header_field::CONTENT_LENGTH).empty())
          {
            clear();
            return RX_LENGTH_REQUIRED;
          }

          // received buffer contains more than the required data
          if (rx_size > required)
          {
              auto next(iter + required);
              body_.insert(body_.end(), iter, next);
          }
          else // received buffer <= required data
          {
            if (end > iter)
              body_.insert(body_.end(), iter, end);
          }

          // return whether the body is complete
          if (body_.size() == response_.content_length())
            return RX_VALID;
        }
        else // response_.is_chunked()
        {
          // If parsed a chunk and its data previously,
          // then clear it ready for the next chunk
          if (chunk_.valid())
            chunk_.clear();

          // failed to parse request
          if (!chunk_.parse(iter, end))
          {
            // if a parsing error (not run out of data)
            if (iter != end)
            {
              clear();
              return RX_INVALID;
            }
          }

          // If parsed the response header, pass it to the application
          if (response_parsed)
            return RX_VALID;

          // A complete chunk has been parsed..
          if (chunk_.valid())
            return RX_CHUNK;
        }

        return RX_INCOMPLETE;
      }

    };

  }
}

#endif
