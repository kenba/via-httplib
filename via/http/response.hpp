#ifndef RESPONSE_HPP_VIA_HTTPLIB_
#define RESPONSE_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
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
      /// @enum Response the state of the response line parser.
      enum Response
      {
        RESP_HTTP_H,             ///< HTTP/ H
        RESP_HTTP_T1,            ///< HTTP/ first T
        RESP_HTTP_T2,            ///< HTTP/ second T
        RESP_HTTP_P,             ///< HTTP/ P
        RESP_HTTP_SLASH,         ///< HTTP/ slash
        RESP_HTTP_MAJOR,         ///< HTTP major version number
        RESP_HTTP_DOT,           ///< HTTP . between major and minor versions
        RESP_HTTP_MINOR,         ///< HTTP minor version number
        RESP_HTTP_WS,            ///< HTTP space of tab before status
        RESP_STATUS,             ///< response status code
        RESP_REASON,             ///< response reason
        RESP_CR,                 ///< the carriage return (if any)
        RESP_LF,                 ///< the line feed
        RESP_VALID,              ///< the response line is valid
        RESP_ERROR_CRLF,         ///< strict_crlf_s is true and LF was received without CR
        RESP_ERROR_WS,           ///< the whitespace is longer than max_ws_s
        RESP_ERROR_STATUS_VALUE, ///< the method name is longer than max_method_length_s
        RESP_ERROR_REASON_LENGTH ///< then uri is longer than max_uri_length_s
      };

    private:

      int status_;                ///< the response status code
      std::string reason_phrase_; ///< the response reason phrase
      size_t ws_count_;           ///< the current whitespace count
      char major_version_;        ///< the HTTP major version number
      char minor_version_;        ///< the HTTP minor version number
      Response state_;            ///< the current parsing state
      bool status_read_;          ///< true if status code was read
      bool valid_;                ///< true if the response line is valid
      bool fail_;                 ///< true if the response line failed validation

      /// Parse an individual character.
      /// @param c the character to be parsed.
      /// @return true if valid, false otherwise.
      bool parse_char(char c);

    public:

      /// whether to enforce strict parsing of CRLF
      static bool strict_crlf_s;

      /// the maximum number of consectutive whitespace characters.
      static size_t max_ws_s;

      /// the maximum number of a response status
      static int max_status_s;

      /// the maximum length of a response reason,
      static size_t max_reason_length_s;

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit response_line() :
        status_(0),
        reason_phrase_(""),
        ws_count_(0),
        major_version_(0),
        minor_version_(0),
        state_(RESP_HTTP_H),
        status_read_(false),
        valid_(false),
        fail_(false)
      {}

      /// Clear the response_line.
      /// Sets all member variables to their initial state.
      void clear()
      {
        status_ = 0;
        reason_phrase_.clear();
        ws_count_ = 0;
        major_version_ = 0;
        minor_version_ = 0;
        state_ = RESP_HTTP_H;
        status_read_ = false;
        valid_ =  false;
        fail_ = false;
      }

      /// Swap member variables with another response_line.
      /// @param other the other response_line
      void swap(response_line& other)
      {
        std::swap(status_, other.status_);
        reason_phrase_.swap(other.reason_phrase_);
        std::swap(ws_count_, other.ws_count_);
        std::swap(major_version_, other.major_version_);
        std::swap(minor_version_, other.minor_version_);
        std::swap(state_, other.state_);
        std::swap(status_read_, other.status_read_);
        std::swap(valid_, other.valid_);
        std::swap(fail_, other.fail_);
      }

      /// Virtual destructor.
      /// Since the class is inherited...
      virtual ~response_line() {}

      /// Parse the line as an HTTP response.
      /// @retval iter reference to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        while ((iter != end) && (RESP_VALID != state_))
        {
          char c(*iter++);
          if ((fail_ = !parse_char(c))) // Note: deliberate assignment
            return false;
        }
        valid_ = (RESP_VALID == state_);
        return valid_;
      }

      /// Accessor for the HTTP major version number.
      /// @return the major version number.
      char major_version() const
      { return major_version_; }

      /// Accessor for the HTTP minor version number.
      /// @return the minor version number.
      char minor_version() const
      { return minor_version_; }

      /// Accessor for the response status.
      /// @return the response status number.
      int status() const
      { return status_; }

      /// Whether this is a continue response.
      /// @return true if this is a continue response, false otherwise.
      bool is_continue() const
      { return status_ == static_cast<int>(response_status::code::CONTINUE); }

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

      /// Test for early HTTP versions
      /// @return true if HTTP/1.0 or earlier.
      bool is_http_1_0_or_earlier() const
      {
        return (major_version_ <= '0') ||
              ((major_version_ == '1') && (minor_version_ == '0'));
      }

      ////////////////////////////////////////////////////////////////////////
      // Encoding interface.

      /// Constructor for creating a response for one of the standard
      /// responses defined in RFC2616.
      /// @see http::response_status::code
      /// @param status_code the response status code
      /// @param major_version default '1'
      /// @param minor_version default '1'
      explicit response_line(response_status::code::status status_code,
                             char major_version = '1',
                             char minor_version = '1') :
        status_(static_cast<int>(status_code)),
        reason_phrase_(response_status::reason_phrase(status_code)),
        major_version_(major_version),
        minor_version_(minor_version),
        state_(RESP_VALID),
        status_read_(true),
        valid_(true),
        fail_(false)
      {}

      /// Constructor for creating a non-standard response.
      /// @param status the response status code
      /// @param reason_phrase the reason phrase for the response status.
      /// @param major_version default '1'
      /// @param minor_version default '1'
      explicit response_line(int status,
                             std::string const& reason_phrase,
                             char major_version = '1',
                             char minor_version = '1') :
        status_(status),
        reason_phrase_(!reason_phrase.empty() ? reason_phrase :
               response_status::reason_phrase
                    (static_cast<response_status::code::status>(status))),
        major_version_(major_version),
        minor_version_(minor_version),
        state_(RESP_VALID),
        status_read_(true),
        valid_(true),
        fail_(false)
      {}

      /// Set the response status for standard responses.
      /// @see http::response_status::status_code
      /// @param status the response status.
      void set_status(response_status::code::status status_code)
      {
        status_ = static_cast<int>(status_code);
        reason_phrase_ = response_status::reason_phrase(status_code);
      }

      /// Set the response status and reason phrase.
      /// @param status the response status.
      /// @param reason_phrase the the response reason phrase.
      void set_status_and_reason(int status, std::string const& reason_phrase)
      {
        status_ = status;
        reason_phrase_ = reason_phrase;
      }

      /// Set the HTTP minor version.
      /// @param minor_version the HTTP minor version.
      void set_minor_version(char minor_version)
      { minor_version_ = minor_version; }

      /// Set the HTTP major version.
      /// @param major_version the HTTP major version.
      void set_major_version(char major_version)
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
      bool valid_;              ///< true if the response is valid

    public:

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit rx_response() :
        response_line(),
        headers_(),
        valid_(false)
      {}

      virtual ~rx_response() {}

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
      /// I.e. if the response is HTTP 1.1 and there is not a connection: close
      /// header field.
      /// @return true if it should be kept alive, false otherwise.
      bool keep_alive() const
      {
        return !is_http_1_0_or_earlier() &&
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
      /// @see http::response_status::code
      /// @param status_code the response status code
      /// @param header_string default blank
      explicit tx_response(response_status::code::status status_code,
                           std::string header_string = "") :
        response_line(status_code),
        header_string_(header_string)
      {}

      /// Constructor for creating a non-standard response.
      /// @param reason_phrase the reason phrase for the response status.
      /// @param status the response status
      /// @param header_string default blank
      explicit tx_response(const std::string& reason_phrase,
                           int status,
                           std::string header_string = "") :
        response_line(status, reason_phrase),
        header_string_(header_string)
      {}

      virtual ~tx_response()
      {}

      /// Set the header_string_ to the value given.
      /// Note: will overwrite any other headers, so must be called before
      /// the following add_header fucntions.
      /// @param header_string the new header string
      /// @return true if the header string has been set, false if the header
      /// string is invalid.
      bool set_header_string(std::string const& header_string)
      {
        header_string_ = header_string;
        return !are_headers_split(header_string_);
      }

      /// Add a standard header to the response.
      /// @see http::header_field::field_id
      /// @param id the header field id
      /// @param value the header field value
      void add_header(header_field::id::field field_id, const std::string& value)
      { header_string_ += header_field::to_header(field_id, value);  }

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

      /// Add a http content header to the response.
      void add_content_http_header()
      { header_string_ += header_field::content_http_header(); }

      /// Add an http content length header line for the given size.
      void add_content_length_header(size_t size)
      { header_string_ += header_field::content_length(size); }

      /// Determine whether the response is valid.
      /// @return true if the response does not contain "split headers".
      bool is_valid() const
      { return !are_headers_split(header_string_); }

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
              (header_field::standard_name(header_field::id::CONTENT_LENGTH)));
        bool no_transfer_encoding(std::string::npos == header_string_.find
              (header_field::standard_name(header_field::id::TRANSFER_ENCODING)));
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
      Rx receive(ForwardIterator& iter, ForwardIterator end)
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
          size_t content_length(response_.content_length());
          if (content_length == ULONG_MAX)
          {
            clear();
            return RX_INVALID;
          }

          // if there's a message body then insist on a content length header
          ptrdiff_t rx_size(std::distance(iter, end));
          if ((rx_size > 0) && (content_length == 0) &&
              response_.headers().find(header_field::id::CONTENT_LENGTH).empty())
          {
            clear();
            return RX_LENGTH_REQUIRED;
          }

          // received buffer contains more than the required data
          ptrdiff_t required(static_cast<ptrdiff_t>(content_length) -
                             static_cast<ptrdiff_t>(body_.size()));
          if (rx_size > required)
          {
              ForwardIterator next(iter + required);
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
