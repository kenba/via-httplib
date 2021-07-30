#ifndef REQUEST_HPP_VIA_HTTPLIB_
#define REQUEST_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Ken Barker
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
#include "response_status.hpp"
#include "headers.hpp"
#include "chunk.hpp"
#include <algorithm>

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class request_line
    /// The HTTP request start line.
    /// @tparam MAX_URI_LENGTH the maximum length of an HTTP request uri:
    /// min 1, max 4 billion.
    /// @tparam MAX_METHOD_LENGTH the maximum length of an HTTP request method:
    /// min 1, max 254.
    /// @tparam MAX_WHITESPACE_CHARS the maximum number of consectutive whitespace
    /// characters allowed in a request: min 1, max 254.
    /// @tparam STRICT_CRLF enforce strict parsing of CRLF.
    //////////////////////////////////////////////////////////////////////////
    template <size_t        MAX_URI_LENGTH,
              unsigned char MAX_METHOD_LENGTH,
              unsigned char MAX_WHITESPACE_CHARS,
              bool          STRICT_CRLF>
    class request_line
    {
    public:
      /// @enum Request the state of the request line parser.
      enum class Request
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
        ERROR_CRLF,          ///< STRICT_CRLF is true and LF was received without CR
        ERROR_WS,            ///< the whitespace is longer than MAX_WHITESPACE_CHARS
        ERROR_METHOD_LENGTH, ///< the method name is longer than MAX_METHOD_LENGTH
        ERROR_URI_LENGTH     ///< then uri is longer than MAX_URI_LENGTH
      };

    private:

      // Request information
      std::string method_ {};    ///< the request method
      std::string uri_ {};       ///< the request uri
      char major_version_ { 0 }; ///< the HTTP major version character
      char minor_version_ { 0 }; ///< the HTTP minor version character

      // Parser state
      Request state_ { Request::METHOD }; ///< the current parsing state
      unsigned short ws_count_ { 0u };    ///< the current whitespace count
      bool valid_ { false };              ///< true if the request line is valid
      bool fail_ { false };               ///< true if the request line failed validation

      /// Parse an individual character.
      /// @param c the character to be parsed.
      /// @return true if valid, false otherwise.
      bool parse_char(char c)
      {
        switch (state_)
        {
        case Request::METHOD:
          // Valid HTTP methods must be uppercase chars
          if (std::isupper(c))
          {
            method_.push_back(c);
            if (method_.size() > MAX_METHOD_LENGTH)
            {
              state_ = Request::ERROR_METHOD_LENGTH;
              return false;
            }
          }
          // If this char is whitespace and method has been read
          else if (std::isblank(c) && !method_.empty())
          {
            ws_count_ = 1;
            state_ = Request::URI;
          }
          else
            return false;
          break;

        case Request::URI:
          if (is_end_of_line(c))
            return false;
          else if (std::isblank(c))
          {
            // Ignore leading whitespace
            // but only upto to a limit!
            if (++ws_count_ > MAX_WHITESPACE_CHARS)
            {
              state_ = Request::ERROR_WS;
              return false;
            }

            if (!uri_.empty())
            {
              ws_count_ = 1;
              state_ = Request::HTTP_H;
            }
          }
          else
          {
            uri_.push_back(c);
            if (uri_.size() > MAX_URI_LENGTH)
            {
              state_ = Request::ERROR_URI_LENGTH;
              return false;
            }
          }
          break;

        case Request::HTTP_H:
          // Ignore leading whitespace
          if (std::isblank(c))
          {
            // but only upto to a limit!
            if (++ws_count_ > MAX_WHITESPACE_CHARS)
            {
              state_ = Request::ERROR_WS;
              return false;
            }
          }
          else
          {
            if ('H' == c)
              state_ = Request::HTTP_T1;
            else
              return false;
          }
          break;

        case Request::HTTP_T1:
          if ('T' == c)
            state_ = Request::HTTP_T2;
          else
            return false;
          break;

        case Request::HTTP_T2:
          if ('T' == c)
            state_ = Request::HTTP_P;
          else
            return false;
          break;

        case Request::HTTP_P:
          if ('P' == c)
            state_ = Request::HTTP_SLASH;
          else
            return false;
          break;

        case Request::HTTP_SLASH:
          if ('/' == c)
            state_ = Request::HTTP_MAJOR;
          else
            return false;
          break;

        case Request::HTTP_MAJOR:
          if (std::isdigit(c))
          {
            major_version_ = c;
            state_ = Request::HTTP_DOT;
          }
          else
            return false;
          break;

        case Request::HTTP_DOT:
          if ('.' == c)
            state_ = Request::HTTP_MINOR;
          else
            return false;
          break;

        case Request::HTTP_MINOR:
          if (std::isdigit(c))
          {
            minor_version_ = c;
            state_ = Request::CR;
          }
          else
            return false;
          break;

        case Request::CR:
          // The HTTP line should end with a \r\n...
          if ('\r' == c)
            state_ = Request::LF;
          else
          {
            // but (if not being strict) permit just \n
            if (!STRICT_CRLF && ('\n' == c))
              state_ = Request::VALID;
            else
            {
              state_ = Request::ERROR_CRLF;
              return false;
            }
          }
          break;

        case Request::LF:
          if ('\n' == c)
          {
            state_ = Request::VALID;
            break;
          }
          [[fallthrough]]; // intentional fall-through (for code coverage)

         default:
          return false;
        }

        return true;
      }

    public:

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default Constructor.
      request_line() = default;

      /// Clear the request_line.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        method_.clear();
        uri_.clear();
        major_version_ = 0;
        minor_version_ = 0;

        state_ = Request::METHOD;
        ws_count_ = 0;
        valid_ =  false;
        fail_ = false;
      }

      /// Swap member variables with another request_line.
      /// @param other the other request_line
      void swap(request_line& other) noexcept
      {
        method_.swap(other.method_);
        uri_.swap(other.uri_);
        std::swap(major_version_, other.major_version_);
        std::swap(minor_version_, other.minor_version_);

        std::swap(state_, other.state_);
        std::swap(ws_count_, other.ws_count_);
        std::swap(valid_, other.valid_);
        std::swap(fail_, other.fail_);
      }

      /// Virtual destructor.
      /// Since the class is inherited...
      virtual ~request_line() = default;

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4706 ) // assignment within conditional expression
#endif
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
          char c(*iter++);
          if ((fail_ = !parse_char(c))) // Note: deliberate assignment
            return false;
        }
        valid_ = (Request::VALID == state_);
        return valid_;
      }
#ifdef _MSC_VER
#pragma warning( pop )
#endif
      /// Accessor for the HTTP minor version number.
      /// @return the minor version number.
      const std::string& method() const noexcept
      { return method_; }

      /// Accessor for the request uri.
      /// @return the request uri string.
      const std::string& uri() const noexcept
      { return uri_; }

      /// Accessor for the HTTP major version number.
      /// @return the major version number.
      char major_version() const noexcept
      { return major_version_; }

      /// Accessor for the HTTP minor version number.
      /// @return the minor version number.
      char minor_version() const noexcept
      { return minor_version_; }

      /// Accessor for the parsing state.
      /// @return the parsing state.
      Request state() const noexcept
      { return state_; }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const noexcept
      { return valid_; }

      /// Accessor for the fail flag.
      /// @return the fail flag.
      bool fail() const noexcept
      { return fail_; }

      /// Test for early HTTP versions
      /// @return true if HTTP/1.0 or earlier.
      bool is_http_1_0_or_earlier() const noexcept
      {
        return (major_version_ == '0') ||
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
      request_line(request_method::id method_id,
                    std::string_view uri,
                    char major_version = '1',
                    char minor_version = '1') :
        method_(request_method::name(method_id)),
        uri_(uri),
        major_version_(major_version),
        minor_version_(minor_version),

        state_(Request::VALID),
        valid_(true)
      {}

      /// Constructor for creating a request with a non-standard method.
      /// @param method the HTTP request method name
      /// @param uri the HTTP uri, default blank
      /// @param major_version default '1'
      /// @param minor_version default '1'
       request_line(std::string_view method,
                     std::string_view uri,
                     char major_version = '1',
                     char minor_version = '1') :
        method_(method),
        uri_(uri),
        major_version_(major_version),
        minor_version_(minor_version),

        state_(Request::VALID),
        valid_(true)
      {}

      /// Set the request method.
      /// @param method the HTTP request method.
      void set_method(std::string_view method)
      { method_ = method; }

      /// Set the request uri.
      /// @param uri the HTTP request uri.
      void set_uri(std::string_view uri)
      { uri_ = uri; }

      /// Set the HTTP major version.
      /// @param major_version the HTTP major version.
      void set_major_version(char major_version) noexcept
      { major_version_ = major_version; }

      /// Set the HTTP minor version.
      /// @param minor_version the HTTP minor version.
      void set_minor_version(char minor_version) noexcept
      { minor_version_ = minor_version; }

      /// Output as a string.
      /// @return a string containing the request line.
      std::string to_string() const
      {
        std::string output(method_);
        output += ' ' + uri_ + ' '
               + http_version(major_version_, minor_version_)
               + CRLF;
        return output;
      }
    }; // class request_line

    //////////////////////////////////////////////////////////////////////////
    /// @class rx_request
    /// A class to receive an HTTP request.
    /// @tparam MAX_URI_LENGTH the maximum length of an HTTP request uri:
    /// min 1, max 4 billion.
    /// @tparam MAX_METHOD_LENGTH the maximum length of an HTTP request method:
    /// min 1, max 254.
    /// @tparam MAX_HEADER_NUMBER the maximum number of HTTP header field lines:
    /// max 65534.
    /// @tparam MAX_HEADER_LENGTH the maximum cumulative length the HTTP header
    /// fields: max 4 billion.
    /// @tparam MAX_LINE_LENGTH the maximum length of an HTTP header field line:
    /// min 1, max 65534.
    /// @tparam MAX_WHITESPACE_CHARS the maximum number of consectutive whitespace
    /// characters allowed in a request: min 1, max 254.
    /// @tparam STRICT_CRLF enforce strict parsing of CRLF.
    //////////////////////////////////////////////////////////////////////////
    template <size_t         MAX_URI_LENGTH,
              unsigned char  MAX_METHOD_LENGTH,
              unsigned short MAX_HEADER_NUMBER,
              size_t         MAX_HEADER_LENGTH,
              unsigned short MAX_LINE_LENGTH,
              unsigned char  MAX_WHITESPACE_CHARS,
              bool           STRICT_CRLF>
    class rx_request : public request_line<MAX_URI_LENGTH,
                                            MAX_METHOD_LENGTH,
                                            MAX_WHITESPACE_CHARS,
                                            STRICT_CRLF>
    {
      using MessageHeaders = message_headers<MAX_HEADER_NUMBER,
                                             MAX_HEADER_LENGTH,
                                             MAX_LINE_LENGTH,
                                             MAX_WHITESPACE_CHARS,
                                             STRICT_CRLF>;

      using request_ln = request_line<MAX_URI_LENGTH,
                                      MAX_METHOD_LENGTH,                                      
                                      MAX_WHITESPACE_CHARS,
                                      STRICT_CRLF>;

      MessageHeaders headers_ {}; ///< the HTTP headers for the request
      bool valid_ { false };       ///< true if the request is valid

    public:

      /// Default Constructor.
      rx_request() = default;

      virtual ~rx_request() {}

      /// Clear the rx_request.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        request_ln::clear();
        headers_.clear();
        valid_ =  false;
      }

      /// Swap member variables with another rx_request.
      /// @param other the other rx_request
      void swap(rx_request& other) noexcept
      {
        request_ln::swap(other);
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
        if (!request_ln::valid() && !request_ln::parse(iter, end))
          return false;

        if (!headers_.valid() && !headers_.parse(iter, end))
          return false;

        valid_ = true;
        return valid_;
      }

      /// Accessor for the request message headers.
      /// @return a constant reference to the message_headers
      const MessageHeaders& headers() const noexcept
      { return headers_; }

      /// The size in the content_length header (if there is one)
      /// @return the content_length header value.
      std::ptrdiff_t content_length() const noexcept
      { return headers_.content_length(); }

      /// Whether chunked transfer encoding is enabled.
      /// @return true if chunked transfer encoding is enabled.
      bool is_chunked() const noexcept
      { return headers_.is_chunked(); }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const noexcept
      { return valid_; }

      /// Whether the connection should be kept alive.
      /// I.e. if the request is HTTP 1.1 and there is not a connection: close
      /// header field.
      /// @return true if it should be kept alive, false otherwise.
      bool keep_alive() const noexcept
      {
        return !request_ln::is_http_1_0_or_earlier() &&
               !headers_.close_connection();
      }

      /// Whether a request is missing a Host: header.
      /// I.e. if the request is HTTP 1.1 then it should contain a host
      /// header field.
      /// @return true if the request should have a host header, false
      /// otherwise
      bool missing_host_header() const
      {
        return request_ln::major_version() == '1' &&
               request_ln::minor_version() == '1' &&
               headers_.find(header_field::LC_HOST).empty();
      }

      /// Whether the client expects a "100-continue" response.
      /// @return true if the server should send a 100-Continue header, false
      /// otherwise
      bool expect_continue() const
      {
        return !request_ln::is_http_1_0_or_earlier() &&
               headers_.expect_continue();
      }

      /// Whether the request is "HEAD"
      /// @return true if the request is "HEAD"
      bool is_head() const noexcept
      { return request_method::HEAD == request_ln::method(); }

      /// Whether the request is "TRACE"
      /// @return true if the request is "TRACE"
      bool is_trace() const noexcept
      { return request_method::TRACE == request_ln::method(); }
    }; // class rx_request

    //////////////////////////////////////////////////////////////////////////
    /// @class tx_request
    /// A class to encode an HTTP request.
    //////////////////////////////////////////////////////////////////////////
    class tx_request : public request_line<8190, 8, 8, true>
    {
      using request_ln = request_line<8190, 8, 8, true>;

      std::string header_string_ {}; ///< The headers as a string.

    public:

      /// Default Constructor.
      tx_request() = default;

      /// Constructor for creating a request for one of the standard methods
      /// defined in RFC2616.
      /// @see http::request_method::id
      /// @param method_id the HTTP request method id
      /// @param uri the HTTP uri
      /// @param header_string default blank
      /// @param major_version default 1
      /// @param minor_version default 1
      tx_request(request_method::id method_id,
                  std::string_view uri,
                  std::string_view header_string = std::string_view(),
                  char major_version = '1',
                  char minor_version = '1') :
        request_ln(method_id, uri, major_version, minor_version),
        header_string_(header_string)
      {}

      /// Constructor for creating a request with a non-standard method.
      /// @param method the HTTP request method name
      /// @param uri the HTTP uri
      /// @param header_string default blank
      /// @param major_version default 1
      /// @param minor_version default 1
      tx_request(std::string_view method,
                  std::string_view uri,
                  std::string_view header_string = std::string_view(),
                  char major_version = '1',
                  char minor_version = '1') :
        request_ln(method, uri, major_version, minor_version),
        header_string_(header_string)
      {}

      virtual ~tx_request() {}

      /// Set the header_string_ to the value given.
      /// Note: will overwrite any other headers, so must be called before
      /// the following add_header fucntions.
      /// @param header_string the new header string
      void set_header_string(std::string_view header_string)
      { header_string_ = header_string; }

      /// Add a standard header to the request.
      /// @see http::header_field::field_id
      /// @param field_id the header field id
      /// @param value the header field value
      void add_header(header_field::id field_id, std::string_view value)
      { header_string_ += header_field::to_header(field_id, value);  }

      /// Add a free form header to the request.
      /// @param field the header field name
      /// @param value the header field value
      void add_header(std::string_view field, std::string_view value)
      { header_string_ += header_field::to_header(field, value);  }

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
        std::string output(request_ln::to_string());
        output += header_string_;

        // Ensure that it's got a content length header unless
        // a tranfer encoding is being applied.
        bool no_content_length(std::string::npos == header_string_.find
              (header_field::HEADER_CONTENT_LENGTH));
        bool no_transfer_encoding(std::string::npos == header_string_.find
              (header_field::HEADER_TRANSFER_ENCODING));
        if (no_content_length && no_transfer_encoding)
          output += header_field::content_length(content_length);
        output += CRLF;

        return output;
      }
    }; // class tx_request

    //////////////////////////////////////////////////////////////////////////
    /// @class request_receiver
    /// A template class to receive HTTP requests and any associated data.
    /// @tparam Container the type of container in which the request is held.
    /// @tparam MAX_URI_LENGTH the maximum length of an HTTP request uri:
    /// min 1, max 4 billion.
    /// @tparam MAX_METHOD_LENGTH the maximum length of an HTTP request method:
    /// min 1, max 254.
    /// @tparam MAX_HEADER_NUMBER the maximum number of HTTP header field lines:
    /// max 65534.
    /// @tparam MAX_HEADER_LENGTH the maximum cumulative length the HTTP header
    /// fields: max 4 billion.
    /// @tparam MAX_LINE_LENGTH the maximum length of an HTTP header field line:
    /// min 1, max 65534.
    /// @tparam MAX_WHITESPACE_CHARS the maximum number of consectutive whitespace
    /// characters allowed in a request: min 1, max 254.
    /// @tparam STRICT_CRLF enforce strict parsing of CRLF.
    //////////////////////////////////////////////////////////////////////////
    template <typename Container,
              size_t         MAX_URI_LENGTH,
              unsigned char  MAX_METHOD_LENGTH,
              unsigned short MAX_HEADER_NUMBER,
              size_t         MAX_HEADER_LENGTH,
              unsigned short MAX_LINE_LENGTH,
              unsigned char  MAX_WHITESPACE_CHARS,
              bool           STRICT_CRLF>
    class request_receiver
    {
    public:
      /// The request_line for this request_receiver.
      using request_ln = request_line<MAX_URI_LENGTH,
                                      MAX_METHOD_LENGTH,
                                      MAX_WHITESPACE_CHARS,
                                      STRICT_CRLF>;

      /// The rx_request for this request_receiver.
      using Request = rx_request<MAX_URI_LENGTH,
                                 MAX_METHOD_LENGTH,
                                 MAX_HEADER_NUMBER,
                                 MAX_HEADER_LENGTH,
                                 MAX_LINE_LENGTH,
                                 MAX_WHITESPACE_CHARS,
                                 STRICT_CRLF>;

      /// The rx_chunk for this request_receiver.
      using Chunk = rx_chunk<Container,
                             MAX_HEADER_NUMBER,
                             MAX_HEADER_LENGTH,
                             MAX_LINE_LENGTH,
                             MAX_WHITESPACE_CHARS,
                             STRICT_CRLF>;

      /// The default maximum size of request content, 1M.
      static constexpr size_t DEFAULT_MAX_CONTENT_LENGTH = 1048576u;

    private:

      /// Behaviour
      /// the maximum request content size.
      size_t max_content_length_ { DEFAULT_MAX_CONTENT_LENGTH };
      bool translate_head_ { true };     ///< pass a HEAD request as a GET request.
      bool concatenate_chunks_ { true }; ///< concatenate chunk data into the body

      /// Request information
      Request   request_ {}; ///< the received request
      Chunk     chunk_ {};   ///< the received chunk
      Container body_ {};    ///< the request body or data for the last chunk
      /// the appropriate response to the request:
      /// either an error code or 100 Continue.
      response_status::code response_code_{ response_status::code::NO_CONTENT };
      bool       continue_sent_ { false };   ///< a 100 Continue response has been sent
      bool       is_head_ { false };         ///< whether it's a HEAD request

    public:

      /// Default Constructor
      request_receiver() = default;

      /// Constructor.
      /// Sets all member variables to their initial state.
      /// @param max_content_length the maximum size of HTTP request content:
      /// max 4 billion.
      /// @param max_chunk_size the maximum size of an HTTP request chunk:
      /// max 4 billion.
      request_receiver(size_t max_content_length, size_t max_chunk_size) :
        max_content_length_(max_content_length),
        translate_head_(true),
        concatenate_chunks_(true),
        request_(),
        chunk_(max_chunk_size)
      {}

      /// Enable whether HEAD requests are translated into GET
      /// requests for the application.
      /// @param enable enable the function.
      void set_translate_head(bool enable) noexcept
      { translate_head_ = enable; }

      /// Enable whether chunked requests will be concatenated.
      /// @param enable enable the function.
      void set_concatenate_chunks(bool enable) noexcept
      { concatenate_chunks_ = enable; }

      /// set the continue_sent_ flag
      void set_continue_sent() noexcept
      { continue_sent_ = true; }

      /// clear the request_receiver.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        request_.clear();
        chunk_.clear();
        body_.clear();
        // response_code_ is required for response so NOT cleared.
        continue_sent_ = false;
        is_head_ = false;
      }

      /// Accessor for the is_head flag.
      bool is_head() const noexcept
      { return is_head_; }

      /// Accessor for the HTTP request header.
      /// @return a constant reference to an Request.
      Request const& request() const noexcept
      { return request_; }

      /// Accessor for the received chunk.
      /// @return a constant reference to the received chunk.
      Chunk const& chunk() const noexcept
      { return chunk_; }

      /// Accessor for the request body / last chunk data.
      /// @return a constant reference to the data.
      Container const& body() const noexcept
      { return body_; }

      /// Accessor for the response code.
      response_status::code response_code() const noexcept
      { return response_code_; }

      /// Create the body of the TRACE response in the request body.
      /// @return the body for a TRACE response.
      Container trace_body()
      {
        std::string trace_buffer(request().to_string());
        trace_buffer += request().headers().to_string();
        return Container(trace_buffer.begin(), trace_buffer.end());
      }

      /// Receive data for an HTTP request, body or data chunk.
      /// @param iter an iterator to the beginning of the received data.
      /// @param end an iterator to the end of the received data.
      template<typename ForwardIterator>
      Rx receive(ForwardIterator& iter, ForwardIterator end)
      {
        // building a request
        bool request_parsed(!request_.valid());
        if (request_parsed)
        {
          // failed to parse request
          if (!request_.parse(iter, end))
          {
            // if a parsing error (not run out of data)
            if ((iter != end) || request_.fail())
            {
              switch (request_.state())
              {
              case request_ln::Request::ERROR_METHOD_LENGTH:
                response_code_ = response_status::code::NOT_IMPLEMENTED;
                break;
              case request_ln::Request::ERROR_URI_LENGTH:
                response_code_ = response_status::code::REQUEST_URI_TOO_LONG;
                break;
              default:
                response_code_ = response_status::code::BAD_REQUEST;
              }

              clear();
              return Rx::INVALID;
            }
            else
              return Rx::INCOMPLETE;
          }
        }

        // ensure that the request has a "host" header
        if (request_.missing_host_header())
        {
          response_code_ = response_status::code::BAD_REQUEST;
          return Rx::INVALID;
        }

        // build a response body or receive a chunk
        if (!request_.is_chunked())
        {
          // the size of the body received in this message
          std::ptrdiff_t rx_size(std::distance(iter, end));
          std::ptrdiff_t content_length(request_.content_length());

          // TRACE requests may not be allowed
          if (request_.is_trace())
          {
            if (content_length == 0)
              response_code_ = response_status::code::METHOD_NOT_ALLOWED;
            else
            {
              // TRACE requests are not permitted without a body
              response_code_ = response_status::code::BAD_REQUEST;
              clear();
              return Rx::INVALID;
            }
          }

          // test whether the content length header is valid
          if (content_length < 0)
          {
            response_code_ = response_status::code::BAD_REQUEST;
            clear();
            return Rx::INVALID;
          }
          else
          {
            // if theres a valid non-zero content length header
            if (content_length > 0)
            {
              // test the size
              if (content_length > static_cast<std::ptrdiff_t>(max_content_length_))
              {
                response_code_ = response_status::code::PAYLOAD_TOO_LARGE;
                clear();
                return Rx::INVALID;
              }
            } // if there's a body without a  content length header
            else if ((rx_size > 0) && request_.headers().
                                find(header_field::LC_CONTENT_LENGTH).empty())
            {
              response_code_ = response_status::code::LENGTH_REQUIRED;
              clear();
              return Rx::INVALID;
            }
          }

          // received buffer contains more than the required data
          std::ptrdiff_t required(content_length -
                                  static_cast<std::ptrdiff_t>(body_.size()));
          if (rx_size > required)
          {
              ForwardIterator next(iter + required);
              body_.insert(body_.end(), iter, next);
              iter = next;
          }
          else // received buffer <= required data
          {
            if (end > iter)
            {
              body_.insert(body_.end(), iter, end);
              iter = end;
            }
          }

          // determine whether the body is complete
          if (body_.size() == static_cast<size_t>(request_.content_length()))
          {
            is_head_ = request_.is_head();
            // If enabled, translate a HEAD request to a GET request
            if (is_head_ && translate_head_)
              request_.set_method(request_method::GET);
            return Rx::VALID;
          }
        }
        else // request_.is_chunked()
        {
          // If parsed a chunk and its data previously,
          // then clear it ready for the next chunk
          if (chunk_.valid())
            chunk_.clear();

          // If parsed the request header, respond if necessary
          if (request_parsed)
          {
            if (request_.expect_continue() && !continue_sent_)
            {
              response_code_ = response_status::code::CONTINUE;
              return Rx::EXPECT_CONTINUE;
            }
            else
            {
              if (!concatenate_chunks_)
                return Rx::VALID;
            }
          }

          // parse the chunk
          if (!chunk_.parse(iter, end))
          {
            // if a parsing error (not run out of data)
            if (iter != end)
            {
              response_code_ = response_status::code::BAD_REQUEST;
              clear();
              return Rx::INVALID;
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
              {
                // Determine whether the total size of the concatenated chunks
                // is within the maximum body size.
                if ((body_.size() + chunk_.data().size()) > max_content_length_)
                {
                  response_code_ = response_status::code::PAYLOAD_TOO_LARGE;
                  clear();
                  return Rx::INVALID;
                }
                else // concatenate the chunk into the message body
                  body_.insert(body_.end(),
                               chunk_.data().begin(), chunk_.data().end());
              }
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
