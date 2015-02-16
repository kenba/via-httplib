#ifndef REQUEST_HPP_VIA_HTTPLIB_
#define REQUEST_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file request.hpp
/// @brief Classes to parse and encode HTTP requests.
//////////////////////////////////////////////////////////////////////////////
#include "request_method.hpp"
#include "headers.hpp"
#include "chunk.hpp"
#include <algorithm>
#include <cassert>

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class request_line
    /// The HTTP request start line.
    //////////////////////////////////////////////////////////////////////////
    class request_line
    {
    public:
      /// @enum Request the state of the request line parser.
      enum Request : char
      {
        METHOD,              ///< request method
        URI,                 ///< request uri
        HTTP_H,              ///< HTTP/ H
        HTTP_T1,             ///< HTTP/ first T
        HTTP_T2,             ///< HTTP/ second T
        HTTP_P,              ///< HTTP/ P
        HTTP_SLASH,          ///< HTTP/ slash
        HTTP_MAJOR,          ///< HTTP major version number
        HTTP_DOT,            ///< HTTP . between major and minor versions
        HTTP_MINOR,          ///< HTTP minor version number
        CR,                  ///< the carriage return (if any)
        LF,                  ///< the line feed
        VALID,               ///< the request line is valid
        ERROR_CRLF,          ///< strict_crlf_s is true and LF was received without CR
        ERROR_WS,            ///< the whitespace is longer than max_ws_s
        ERROR_METHOD_LENGTH, ///< the method name is longer than max_method_length_s
        ERROR_URI_LENGTH     ///< then uri is longer than max_uri_length_s
      };

    private:

      std::string method_;   ///< the request method
      std::string uri_;      ///< the request uri
      size_t      ws_count_; ///< the current whitespace count
      char major_version_;   ///< the HTTP major version character
      char minor_version_;   ///< the HTTP minor version character
      Request state_;        ///< the current parsing state
      bool valid_;           ///< true if the request line is valid
      bool fail_;            ///< true if the request line failed validation

      /// Parse an individual character.
      /// @param c the character to be parsed.
      /// @return true if valid, false otherwise.
      bool parse_char(char c);

    public:

      /// Delete copy construction and assignment
      request_line(request_line const&) = delete;
      request_line& operator=(request_line const&) = delete;

      /// whether to enforce strict parsing of CRLF
      static bool strict_crlf_s;

      /// the maximum number of consectutive whitespace characters.
      static size_t max_ws_s;

      /// the maximum length of a request method, default 16 chars
      static size_t max_method_length_s;

      /// the maximum length of a uri.
      static size_t max_uri_length_s;

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit request_line() :
        method_(),
        uri_(),
        ws_count_(0),
        major_version_(0),
        minor_version_(0),
        state_(Request::METHOD),
        valid_(false),
        fail_(false)
      {}

      /// Clear the request_line.
      /// Sets all member variables to their initial state.
      void clear()
      {
        method_.clear();
        uri_.clear();
        ws_count_ = 0;
        major_version_ = 0;
        minor_version_ = 0;
        state_ = Request::METHOD;
        valid_ =  false;
        fail_ = false;
      }

      /// Swap member variables with another request_line.
      /// @param other the other request_line
      void swap(request_line& other)
      {
        method_.swap(other.method_);
        uri_.swap(other.uri_);
        std::swap(ws_count_, other.ws_count_);
        std::swap(major_version_, other.major_version_);
        std::swap(minor_version_, other.minor_version_);
        std::swap(state_, other.state_);
        std::swap(valid_, other.valid_);
        std::swap(fail_, other.fail_);
      }

      /// Virtual destructor.
      /// Since the class is inherited...
      virtual ~request_line() = default;

      /// Parse the line as an HTTP request.
      /// @retval iter reference to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        while ((iter != end) && (Request::VALID != state_))
        {
          auto c(static_cast<char>(*iter++));
          if ((fail_ = !parse_char(c))) // Note: deliberate assignment
            return false;
        }
        valid_ = (Request::VALID == state_);
        return valid_;
      }

      /// Accessor for the HTTP minor version number.
      /// @return the minor version number.
      const std::string& method() const
      { return method_; }

      /// Accessor for the request uri.
      /// @return the request uri string.
      const std::string& uri() const
      { return uri_; }

      /// Accessor for the HTTP major version number.
      /// @return the major version number.
      char major_version() const
      { return major_version_; }

      /// Accessor for the HTTP minor version number.
      /// @return the minor version number.
      char minor_version() const
      { return minor_version_; }

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

      /// Constructor for creating a request for one of the standard methods
      /// defined in RFC2616.
      /// @see http::request_method::id
      /// @param method_id the HTTP request method id
      /// @param uri the HTTP uri, default blank
      /// @param major_version default '1'
      /// @param minor_version default '1'
      explicit request_line(request_method::id method_id,
                            std::string uri,
                            char major_version = '1',
                            char minor_version = '1') :
        method_(request_method::name(method_id)),
        uri_(uri),
        major_version_(major_version),
        minor_version_(minor_version),
        state_(Request::VALID),
        valid_(true),
        fail_(false)
      {}

      /// Constructor for creating a request with a non-standard method.
      /// @param method the HTTP request method name
      /// @param uri the HTTP uri, default blank
      /// @param major_version default '1'
      /// @param minor_version default '1'
      explicit request_line(std::string const& method,
                            std::string const& uri,
                            char major_version = '1',
                            char minor_version = '1') :
        method_(method),
        uri_(uri),
        major_version_(major_version),
        minor_version_(minor_version),
        state_(Request::VALID),
        valid_(true),
        fail_(false)
      {}

      /// Set the request method.
      /// @param method the HTTP request method.
      void set_method(const std::string& method)
      { method_ = method; }

      /// Set the request uri.
      /// @param uri the HTTP request uri.
      void set_uri(const std::string& uri)
      { uri_ = uri; }

      /// Set the HTTP major version.
      /// @param major_version the HTTP major version.
      void set_major_version(char major_version)
      { major_version_ = major_version; }

      /// Set the HTTP minor version.
      /// @param minor_version the HTTP minor version.
      void set_minor_version(char minor_version)
      { minor_version_ = minor_version; }

      /// Output as a string.
      /// @return a string containing the request line.
      std::string to_string() const;
    }; // class request_line

    //////////////////////////////////////////////////////////////////////////
    /// @class rx_request
    /// A class to receive an HTTP request.
    //////////////////////////////////////////////////////////////////////////
    class rx_request : public request_line
    {
      message_headers headers_; ///< the HTTP headers for the request
      bool valid_;              ///< true if the request is valid

    public:

      /// Delete copy construction and assignment
      rx_request(rx_request const&) = delete;
      rx_request& operator=(rx_request const&) = delete;

      /// Default constructor.
      /// Sets all member variables to their initial state.
      explicit rx_request() :
        request_line(),
        headers_(),
        valid_(false)
      {}

      virtual ~rx_request() = default;

      /// Clear the rx_request.
      /// Sets all member variables to their initial state.
      void clear()
      {
        request_line::clear();
        headers_.clear();
        valid_ =  false;
      }

      /// Swap member variables with another rx_request.
      /// @param other the other rx_request
      void swap(rx_request& other)
      {
        request_line::swap(other);
        headers_.swap(other.headers_);
        std::swap(valid_, other.valid_);
      }

      /// Parse an HTTP request.
      /// @retval iter reference to an iterator to the start of the data.
      /// If the response is valid it will refer to:
      ///   - the start of the response body if content_length > 0,
      ///   - the start of the first data chunk if is_chunked(),
      ///   - the start of the next http response, or
      ///   - the end of the data buffer.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok, false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        if (!request_line::valid() && !request_line::parse(iter, end))
          return false;

        if (!headers_.valid() && !headers_.parse(iter, end))
          return false;

        valid_ = true;
        return valid_;
      }

      /// Accessor for the request message headers.
      /// @return a constant reference to the message_headers
      const message_headers& headers() const
      { return headers_; }

      /// The size in the content_length header (if there is one)
      /// @return the content_length header value.
      size_t content_length() const
      { return headers_.content_length(); }

      /// Whether the request is "HEAD"
      /// @return true if the request is "HEAD"
      bool is_head() const
      { return request_method::name(request_method::id::HEAD) == method(); }

      /// Whether the request is "TRACE"
      /// @return true if the request is "TRACE"
      bool is_trace() const
      { return request_method::name(request_method::id::TRACE) == method(); }

      /// Whether chunked transfer encoding is enabled.
      /// @return true if chunked transfer encoding is enabled.
      bool is_chunked() const
      { return headers_.is_chunked(); }

      /// Whether the client expects a "100-continue" response.
      /// @return true if the server should send a 100-Continue header, false
      /// otherwise
      bool expect_continue() const
      {
        return !is_http_1_0_or_earlier() &&
               headers_.expect_continue();
      }

      /// Whether a request is missing a Host: header.
      /// I.e. if the request is HTTP 1.1 then it should contain a host
      /// header field.
      /// @return true if the request should have a host header, false
      /// otherwise
      bool missing_host_header() const
      {
        return major_version() == '1' &&
               minor_version() == '1' &&
               headers_.find(header_field::id::HOST).empty();
      }

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
        return !is_http_1_0_or_earlier() &&
               !headers_.close_connection();
      }
    }; // class rx_request

    //////////////////////////////////////////////////////////////////////////
    /// @class tx_request
    /// A class to encode an HTTP request.
    //////////////////////////////////////////////////////////////////////////
    class tx_request : public request_line
    {
      std::string header_string_; ///< The headers as a string.

    public:

      /// Delete copy construction and assignment
      tx_request(tx_request const&) = delete;
      tx_request& operator=(tx_request const&) = delete;

      /// Constructor for creating a request for one of the standard methods
      /// defined in RFC2616.
      /// @see http::request_method::id
      /// @param method_id the HTTP request method id
      /// @param uri the HTTP uri
      /// @param header_string default blank
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit tx_request(request_method::id method_id,
                          std::string uri,
                          std::string header_string = "",
                          char minor_version = '1',
                          char major_version = '1') :
        request_line(method_id, uri, minor_version, major_version),
        header_string_(header_string)
      {}

      /// Constructor for creating a request with a non-standard method.
      /// @param method the HTTP request method name
      /// @param uri the HTTP uri
      /// @param header_string default blank
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit tx_request(const std::string& method,
                          std::string uri,
                          std::string header_string = "",
                          char minor_version = '1',
                          char major_version = '1') :
        request_line(method, uri, minor_version, major_version),
        header_string_(header_string)
      {}

      virtual ~tx_request() = default;

      /// Add a free form header to the request.
      /// @param field the header field name
      /// @param value the header field value
      void add_header(std::string const& field, const std::string& value)
      { header_string_ += header_field::to_header(field, value);  }

      /// Add a standard header to the request.
      /// @see http::header_field::field_id
      /// @param id the header field id
      /// @param value the header field value
      void add_header(header_field::id field_id, const std::string& value)
      { header_string_ += header_field::to_header(field_id, value);  }

      /// Add an http content length header line for the given size.
      /// @param size the size of the message body.
      void add_content_length_header(size_t size)
      { header_string_ += header_field::content_length(size); }

      /// The http message header string.
      /// @param content_length the size of the message body for the
      /// content_length header.
      /// @return The http message header as a std:string.
      std::string message(size_t content_length = 0) const
      {
        std::string output(request_line::to_string());
        output += header_string_;

        // Ensure that it's got a content length header unless
        // a tranfer encoding is being applied.
        bool no_content_length{std::string::npos == header_string_.find
              (header_field::standard_name(header_field::id::CONTENT_LENGTH))};
        bool no_transfer_encoding{std::string::npos == header_string_.find
              (header_field::standard_name(header_field::id::TRANSFER_ENCODING))};
        if (no_content_length && no_transfer_encoding)
          output += header_field::content_length(content_length);
        output += CRLF;

        return output;
      }
    }; // class tx_request

    //////////////////////////////////////////////////////////////////////////
    /// @class request_receiver
    /// A template class to receive HTTP requests and any associated data.
    /// @param Container the type of container in which the request is held.
    /// @param translate_head whether the receiver passes a HEAD request to
    /// the application as a GET request.
    //////////////////////////////////////////////////////////////////////////
    template <typename Container, bool translate_head>
    class request_receiver
    {
      rx_request request_; ///< the received request
      rx_chunk<Container> chunk_;   ///< the received chunk
      Container  body_;    ///< the request body or data for the last chunk
      bool       continue_sent_;   ///< a 100 Continue response has been sent
      bool       is_head_;         ///< whether it's a HEAD request
      bool       concatenate_chunks_; ///< concatenate chunk data into the body

    public:

      /// Delete copy construction and assignment
      request_receiver(request_receiver const&) = delete;
      request_receiver& operator=(request_receiver const&) = delete;

      /// Default constructor.
      /// Sets all member variables to their initial state.
      /// @param concatenate_chunks if true concatenate chunk data into the body
      /// otherwise the body just contains the data for each chunk.
      explicit request_receiver(bool concatenate_chunks) :
        request_(),
        chunk_(),
        body_(),
        continue_sent_(false),
        is_head_(false),
        concatenate_chunks_(concatenate_chunks)
      {}

      /// clear the request_receiver.
      /// Sets all member variables to their initial state.
      void clear()
      {
        request_.clear();
        chunk_.clear();
        body_.clear();
        continue_sent_ = false;
        is_head_ = false;
      }

      /// set the continue_sent_ flag
      void set_continue_sent()
      { continue_sent_ = true; }

      /// Accessor for the is_head flag.
      bool is_head() const
      { return is_head_; }

      /// Accessor for the HTTP request header.
      /// @return a constant reference to an rx_request.
      rx_request const& request() const
      { return request_; }

      /// Accessor for the received chunk.
      /// @return a constant reference to the received chunk.
      rx_chunk<Container> const& chunk() const
      { return chunk_; }

      /// Accessor for the request body / last chunk data.
      /// @return a constant reference to the data.
      Container const& body() const
      { return body_; }

      /// Receive data for an HTTP request, body or data chunk.
      /// @param iter an iterator to the beginning of the received data.
      /// @param end an iterator to the end of the received data.
      template<typename ForwardIterator>
      Rx receive(ForwardIterator& iter, ForwardIterator end)
      {
        // building a request
        auto request_parsed(!request_.valid());
        if (request_parsed)
        {
          // failed to parse request
          if (!request_.parse(iter, end))
          {
            // if a parsing error (not run out of data)
            if ((iter != end) || request_.fail())
            {
              clear();
              return Rx::INVALID;
            }
            else
              return Rx::INCOMPLETE;
          }
        }

        // build a request body or receive a chunk
        assert(request_.valid());
        if (!request_.is_chunked())
        {
          // if there is a content length header, ensure it's valid
          auto content_length(request_.content_length());
          if ((content_length == std::numeric_limits<size_t>::max()) ||
              (content_length > message_headers::max_content_length_s))
          {
            clear();
            return Rx::INVALID;
          }

          // if there's a message body then insist on a content length header
          auto rx_size(std::distance(iter, end));
          if ((rx_size > 0) && (content_length == 0) &&
              request_.headers().find(header_field::id::CONTENT_LENGTH).empty())
          {
            clear();
            return Rx::LENGTH_REQUIRED;
          }

          // received buffer contains more than the required data
          auto required(static_cast<long>(content_length - body_.size()));
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

          // determine whether the body is complete
          if (body_.size() == request_.content_length())
          {
            is_head_ = request_.is_head();
            // If enabled, translate a HEAD request to a GET request
            if (is_head_ && translate_head)
              request_.set_method(request_method::name(request_method::id::GET));

            return Rx::VALID;
          }
        }
        else // request_.is_chunked()
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
              return Rx::INVALID;
            }
          }

          // If parsed the request header, respond if necessary
          if (request_parsed)
          {
            if (request_.expect_continue() && !continue_sent_)
              return Rx::EXPECT_CONTINUE;
            else
            {
              if (!concatenate_chunks_)
                return Rx::VALID;
            }
          }

          // A complete chunk has been parsed..
          if (chunk_.valid())
          {
            if (concatenate_chunks_)
            {
              if (chunk_.is_last())
                return Rx::VALID;
              else
                body_.insert(body_.end(),
                             chunk_.data().begin(), chunk_.data().end());
            }
            else
              return Rx::CHUNK;
          }
        }

        return Rx::INCOMPLETE;
      }

    };

  }
}

#endif
