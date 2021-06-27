#ifndef RESPONSE_HPP_VIA_HTTPLIB_
#define RESPONSE_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Ken Barker
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
#include <climits>

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class response_line
    /// The HTTP response start line.
    /// @tparam max_status_no the maximum number of an HTTP response status:
    /// max 65534.
    /// @tparam max_reason_length the maximum length of a response reason:
    /// max 65534.
    /// @tparam max_whitespace the maximum number of consectutive whitespace
    /// characters allowed in a request: min 1, max 254.
    /// @param strict_crlf enforce strict parsing of CRLF.
    //////////////////////////////////////////////////////////////////////////
    template <unsigned short MAX_STATUS_NUMBER,
              unsigned short MAX_REASON_LENGTH,
              unsigned char  MAX_WHITESPACE_CHARS,
              bool           STRICT_CRLF>
    class response_line
    {
    public:
      /// @enum Response the state of the response line parser.
      enum class Response
      {
        HTTP_H,             ///< HTTP/ H
        HTTP_T1,            ///< HTTP/ first T
        HTTP_T2,            ///< HTTP/ second T
        HTTP_P,             ///< HTTP/ P
        HTTP_SLASH,         ///< HTTP/ slash
        HTTP_MAJOR,         ///< HTTP major version number
        HTTP_DOT,           ///< HTTP . between major and minor versions
        HTTP_MINOR,         ///< HTTP minor version number
        HTTP_WS,            ///< HTTP space of tab before status
        STATUS,             ///< response status code
        REASON,             ///< response reason
        CR,                 ///< the carriage return (if any)
        LF,                 ///< the line feed
        VALID,              ///< the response line is valid
        ERROR_CRLF,         ///< STRICT_CRLF is true and LF was received without CR
        ERROR_WS,           ///< the whitespace is longer than MAX_WHITESPACE_CHARS
        ERROR_STATUS_VALUE, ///< the method name is longer than max_method_length_s
        ERROR_REASON_LENGTH ///< then uri is longer than max_uri_length_s
      };

    private:

      // Response information
      int status_ { 0 };             ///< the response status code
      std::string reason_phrase_ {}; ///< the response reason phrase
      char major_version_ { 0 };     ///< the HTTP major version number
      char minor_version_ { 0 };     ///< the HTTP minor version number

      // Parser state
      Response state_ { Response::HTTP_H }; ///< the current parsing state
      unsigned short ws_count_ { 0u };      ///< the current whitespace count
      bool status_read_ { false };          ///< true if status code was read
      bool valid_ { false };                ///< true if the response line is valid
      bool fail_ { false };                 ///< true if the response line failed validation

      /// Parse an individual character.
      /// @param c the character to be parsed.
      /// @return true if valid, false otherwise.
      bool parse_char(char c)
      {
        switch (state_)
        {
        case Response::HTTP_H:
          // Ignore leading whitespace
          if (std::isblank(c))
          {
            // but only upto to a limit!
            if (++ws_count_ > MAX_WHITESPACE_CHARS)
            {
              state_ = Response::ERROR_WS;
              return false;
            }
          }
          else
          {
            if ('H' == c)
              state_ = Response::HTTP_T1;
            else
              return false;
          }
          break;

        case Response::HTTP_T1:
          if ('T' == c)
            state_ = Response::HTTP_T2;
          else
            return false;
          break;

        case Response::HTTP_T2:
          if ('T' == c)
            state_ = Response::HTTP_P;
          else
            return false;
          break;

        case Response::HTTP_P:
          if ('P' == c)
            state_ = Response::HTTP_SLASH;
          else
            return false;
          break;

        case Response::HTTP_SLASH:
          if ('/' == c)
            state_ = Response::HTTP_MAJOR;
          else
            return false;
          break;

        case Response::HTTP_MAJOR:
          if (std::isdigit(c))
          {
            major_version_ = c;
            state_ = Response::HTTP_DOT;
          }
          else
            return false;
          break;

        case Response::HTTP_DOT:
          if ('.' == c)
            state_ = Response::HTTP_MINOR;
          else
            return false;
          break;

        case Response::HTTP_MINOR:
          if (std::isdigit(c))
          {
            minor_version_ = c;
            // must be at least one whitespace before status
            state_ = Response::HTTP_WS;
          }
          else
            return false;
          break;

        case Response::HTTP_WS:
          if (std::isblank(c))
          {
            ws_count_ = 1;
            state_ = Response::STATUS;
          }
          else
            return false;
          break;

        case Response::STATUS:
          if (std::isdigit(c))
          {
            status_read_ = true;
            status_ *= 10;
            status_ += read_digit(c);
            if (status_ > MAX_STATUS_NUMBER)
            {
              state_ = Response::ERROR_STATUS_VALUE;
              return false;
            }
          }
          else if (std::isblank(c))
          {
            if (status_read_)
            {
              ws_count_ = 1;
              state_ = Response::REASON;
            }
            else // Ignore extra leading whitespace
            {
              // but only upto to a limit!
              if (++ws_count_ > MAX_WHITESPACE_CHARS)
              {
                state_ = Response::ERROR_WS;
                return false;
              }
            }
          }
          else
            return false;
          break;

        case Response::REASON:
          if (!is_end_of_line(c))
          {
            // Ignore leading whitespace
            if (reason_phrase_.empty() && std::isblank(c))
            {
              // but only upto to a limit!
              if (++ws_count_ > MAX_WHITESPACE_CHARS)
              {
                state_ = Response::ERROR_WS;
                return false;
              }
            }
            else
            {
              reason_phrase_.push_back(c);
              if (reason_phrase_.size() > MAX_REASON_LENGTH)
              {
                state_ = Response::ERROR_REASON_LENGTH;
                return false;
              }
            }
            break;
          }
          [[fallthrough]]; // intentional fall-through

        case Response::CR:
          // The HTTP line should end with a \r\n...
          if ('\r' == c)
            state_ = Response::LF;
          else
          {
            // but (if not being strict) permit just \n
            if (!STRICT_CRLF && ('\n' == c))
              state_ = Response::VALID;
            else
            {
              state_ = Response::ERROR_CRLF;
              return false;
            }
          }
          break;

        case Response::LF:
          if ('\n' == c)
          {
            state_ = Response::VALID;
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
      response_line() = default;

      /// Clear the response_line.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        status_ = 0;
        reason_phrase_.clear();
        major_version_ = 0;
        minor_version_ = 0;

        state_ = Response::HTTP_H;
        ws_count_ = 0;
        status_read_ = false;
        valid_ =  false;
        fail_ = false;
      }

      /// Swap member variables with another response_line.
      /// @param other the other response_line
      void swap(response_line& other) noexcept
      {
        std::swap(status_, other.status_);
        reason_phrase_.swap(other.reason_phrase_);
        std::swap(major_version_, other.major_version_);
        std::swap(minor_version_, other.minor_version_);

        std::swap(state_, other.state_);
        std::swap(ws_count_, other.ws_count_);
        std::swap(status_read_, other.status_read_);
        std::swap(valid_, other.valid_);
        std::swap(fail_, other.fail_);
      }

      /// Virtual destructor.
      /// Since the class is inherited...
      virtual ~response_line() = default;

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4706 ) // assignment within conditional expression
#endif
      /// Parse the line as an HTTP response.
      /// @retval iter reference to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        while ((iter != end) && (Response::VALID != state_))
        {
          char c(*iter++);
          if ((fail_ = !parse_char(c))) // Note: deliberate assignment
            return false;
        }
        valid_ = (Response::VALID == state_);
        return valid_;
      }
#ifdef _MSC_VER
#pragma warning( pop )
#endif

      /// Accessor for the HTTP major version number.
      /// @return the major version number.
      char major_version() const noexcept
      { return major_version_; }

      /// Accessor for the HTTP minor version number.
      /// @return the minor version number.
      char minor_version() const noexcept
      { return minor_version_; }

      /// Accessor for the response status.
      /// @return the response status number.
      int status() const noexcept
      { return status_; }

      /// Whether this is a continue response.
      /// @return true if this is a continue response, false otherwise.
      bool is_continue() const noexcept
      { return status_ == static_cast<int>(response_status::code::CONTINUE); }

      /// Accessor for the response reason string.
      /// @return the response reason string.
      const std::string& reason_phrase() const noexcept
      { return reason_phrase_; }

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
      explicit response_line(response_status::code status_code,
                             char major_version = '1',
                             char minor_version = '1') :
        status_(static_cast<int>(status_code)),
        reason_phrase_(response_status::reason_phrase(status_code)),
        major_version_(major_version),
        minor_version_(minor_version),

        state_(Response::VALID),
        status_read_(true),
        valid_(true)
      {}

      /// Constructor for creating a non-standard response.
      /// @param status the response status code
      /// @param reason_phrase the reason phrase for the response status.
      /// @param major_version default '1'
      /// @param minor_version default '1'
      response_line(int status,
                     std::string_view reason_phrase,
                     char major_version = '1',
                     char minor_version = '1') :
        status_(status),
        reason_phrase_(!reason_phrase.empty() ? reason_phrase :
               response_status::reason_phrase
                    (static_cast<response_status::code>(status))),
        major_version_(major_version),
        minor_version_(minor_version),

        state_(Response::VALID),
        status_read_(true),
        valid_(true)
      {}

      /// Set the response status for standard responses.
      /// @param status_code the response status.
      void set_status(response_status::code status_code)
      {
        status_ = static_cast<int>(status_code);
        reason_phrase_ = response_status::reason_phrase(status_code);
      }

      /// Set the response status and reason phrase.
      /// @param status the response status.
      /// @param reason_phrase the the response reason phrase.
      void set_status_and_reason(int status, std::string_view reason_phrase)
      {
        status_ = status;
        reason_phrase_ = reason_phrase;
      }

      /// Set the HTTP minor version.
      /// @param minor_version the HTTP minor version.
      void set_minor_version(char minor_version) noexcept
      { minor_version_ = minor_version; }

      /// Set the HTTP major version.
      /// @param major_version the HTTP major version.
      void set_major_version(char major_version) noexcept
      { major_version_ = major_version; }

      /// Output as a string.
      /// @return a string containing the response line.
      std::string to_string() const
      {
        std::string output(http_version(major_version_, minor_version_));
        output += ' ' + std::to_string(status_)
                + ' ' + reason_phrase_
                + CRLF;
        return output;
      }
    }; // class response_line

    //////////////////////////////////////////////////////////////////////////
    /// @class rx_response
    /// A class to receive an HTTP response.
    /// @tparam max_status_no the maximum number of an HTTP response status:
    /// max 65534.
    /// @tparam max_reason_length the maximum length of a response reason:
    /// max 65534.
    /// @tparam max_header_number the maximum number of HTTP header field lines:
    /// max 65534.
    /// @tparam max_header_length the maximum cumulative length the HTTP header
    /// fields: max 4 billion.
    /// @tparam max_line_length the maximum length of an HTTP header field line:
    /// max 65534.
    /// @tparam max_whitespace the maximum number of consectutive whitespace
    /// characters allowed in a request: min 1, max 254.
    /// @param strict_crlf enforce strict parsing of CRLF.
    //////////////////////////////////////////////////////////////////////////
    template <unsigned short MAX_STATUS_NUMBER,
              unsigned short MAX_REASON_LENGTH,
              unsigned short MAX_HEADER_NUMBER,
              size_t         MAX_HEADER_LENGTH,
              unsigned short MAX_LINE_LENGTH,
              unsigned char  MAX_WHITESPACE_CHARS,
              bool           STRICT_CRLF>
    class rx_response : public response_line<MAX_STATUS_NUMBER,
                                              MAX_REASON_LENGTH,
                                              MAX_WHITESPACE_CHARS,
                                              STRICT_CRLF>
    {
      using response_ln = response_line<MAX_STATUS_NUMBER,
                                        MAX_REASON_LENGTH,
                                        MAX_WHITESPACE_CHARS,
                                        STRICT_CRLF>;

      using MessageHeaders = message_headers<MAX_HEADER_NUMBER,
                                             MAX_HEADER_LENGTH,
                                             MAX_LINE_LENGTH,
                                             MAX_WHITESPACE_CHARS,
                                             STRICT_CRLF>;

      MessageHeaders headers_ {}; ///< the HTTP headers for the response
      bool valid_ { false };      ///< true if the response is valid

    public:

      /// Default Constructor.
      rx_response() = default;

      virtual ~rx_response() {}

      /// Clear the rx_response.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        response_ln::clear();
        headers_.clear();
        valid_ =  false;
      }

      /// Swap member variables with another rx_response.
      /// @param other the other rx_response
      void swap(rx_response& other) noexcept
      {
        response_ln::swap(other);
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
        if (!response_ln::valid() && !response_ln::parse(iter, end))
          return false;

        if (!headers_.valid() && !headers_.parse(iter, end))
          return false;

        valid_ = true;
        return valid_;
      }

      /// Accessor for the response message headers.
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
      /// I.e. if the response is HTTP 1.1 and there is not a connection: close
      /// header field.
      /// @return true if it should be kept alive, false otherwise.
      bool keep_alive() const
      {
        return !response_ln::is_http_1_0_or_earlier() &&
               !headers_.close_connection();
      }
    }; // class rx_response

    //////////////////////////////////////////////////////////////////////////
    /// @class tx_response
    /// A class to encode an HTTP response.
    //////////////////////////////////////////////////////////////////////////
    class tx_response : public response_line<65534, 65534, 254, false>
    {
      using response_ln = response_line<65534, 65534, 254, false>;

      std::string header_string_ {}; ///< The headers as a string.

    public:

      /// Default Constructor.
      tx_response() = default;

      /// Constructor for creating a response for one of the standard
      /// responses defined in RFC2616.
      /// @see http::response_status::code
      /// @param status_code the response status code
      /// @param header_string default blank
      explicit tx_response(response_status::code status_code,
                           std::string_view header_string = std::string_view()) :
        response_ln(status_code),
        header_string_(header_string)
      {}

      /// Constructor for creating a non-standard response.
      /// @param reason_phrase the reason phrase for the response status.
      /// @param status the response status
      /// @param header_string default blank
      explicit tx_response(std::string_view reason_phrase,
                            int status,
                            std::string_view header_string = std::string_view()) :
        response_ln(status, reason_phrase),
        header_string_(header_string)
      {}

      virtual ~tx_response() {}

      /// Set the header_string_ to the value given.
      /// Note: will overwrite any other headers, so must be called before
      /// the following add_header fucntions.
      /// @param header_string the new header string
      /// @return true if the header string has been set, false if the header
      /// string is invalid.
      bool set_header_string(std::string_view header_string)
      {
        header_string_ = header_string;
        return !are_headers_split(header_string_);
      }

      /// Add a standard header to the response.
      /// @see http::header_field::field_id
      /// @param field_id the header field id
      /// @param value the header field value
      void add_header(header_field::id field_id, std::string_view value)
      { header_string_ += header_field::to_header(field_id, value);  }

      /// Add a free form header to the response.
      /// @param field the header field name
      /// @param value the header field value
      void add_header(std::string_view field, std::string_view value)
      { header_string_ += header_field::to_header(field, value);  }

      /// Add an http content length header line for the given size.
      void add_content_length_header(size_t size)
      { header_string_ += header_field::content_length(size); }

      /// Add a Date header to the response.
      void add_date_header()
      { header_string_ += header_field::date_header(); }

      /// Add a Server header to the response.
      void add_server_header()
      { header_string_ += header_field::server_header(); }

      /// Add a http content header to the response.
      void add_content_http_header()
      { header_string_ += header_field::content_http_header(); }

      /// Determine whether the response is valid.
      /// @return true if the response does not contain "split headers".
      bool is_valid() const noexcept
      { return !are_headers_split(header_string_); }

      /// The http message header string.
      /// @param content_length the size of the message body for the
      /// content_length header.
      /// @return The http message header as a std:string.
      std::string message(size_t content_length = 0) const
      {
        std::string output(response_ln::to_string());
        output += header_string_;

        // Ensure that it's got a content length header unless
        // a transfer encoding is being applied or content is not permitted
        bool no_content_length(std::string::npos == header_string_.find
              (header_field::HEADER_CONTENT_LENGTH));
        bool no_transfer_encoding(std::string::npos == header_string_.find
              (header_field::HEADER_TRANSFER_ENCODING));
        if (no_content_length && no_transfer_encoding &&
            response_status::content_permitted(status()))
          output += header_field::content_length(content_length);
        output += CRLF;

        return output;
      }
    }; // class tx_response

    //////////////////////////////////////////////////////////////////////////
    /// @class response_receiver
    /// A template class to receive HTTP responses and any associated data.
    /// @tparam Container std::string or std::vector<char>
    /// @tparam MAX_BODY_SIZE the maximum size of a response body:
    /// default LONG_MAX, max LONG_MAX.
    /// @tparam MAX_CHUNK_SIZE the maximum size of a response chunk:
    /// default LONG_MAX, max LONG_MAX.
    /// @tparam MAX_STATUS_NUMBER the maximum number of an HTTP response status:
    /// default 65534, max 65534.
    /// @tparam MAX_REASON_LENGTH the maximum length of a response reason string
    /// default 65534, max 65534.
    /// @tparam MAX_LINE_LENGTH the maximum length of an HTTP header field line:
    /// default 65534, min 1, max 65534.
    /// @tparam MAX_HEADER_NUMBER the maximum number of HTTP header field lines:
    /// default 65534, max 65534.
    /// @param MAX_HEADER_LENGTH the maximum cumulative length the HTTP header
    /// fields: default LONG_MAX, max LONG_MAX.

    /// @tparam MAX_WHITESPACE_CHARS the maximum number of consectutive
    /// whitespace characters allowed in a request: default 254, min 1, max 254.
    /// @tparam STRICT_CRLF enforce strict parsing of CRLF, default false.
    //////////////////////////////////////////////////////////////////////////
    template <typename Container,
              size_t         MAX_BODY_SIZE        = LONG_MAX,
              size_t         MAX_CHUNK_SIZE       = LONG_MAX,
              unsigned short MAX_STATUS_NUMBER    = 65534,
              unsigned short MAX_REASON_LENGTH    = 65534,
              unsigned short MAX_HEADER_NUMBER    = 65534,
              size_t         MAX_HEADER_LENGTH    = LONG_MAX,
              unsigned short MAX_LINE_LENGTH      = 65534,
              unsigned char  MAX_WHITESPACE_CHARS = 254,
              bool           STRICT_CRLF = false>
    class response_receiver
    {
      using Response = rx_response<MAX_STATUS_NUMBER,
                                   MAX_REASON_LENGTH,                           
                                   MAX_HEADER_NUMBER,
                                   MAX_HEADER_LENGTH,
                                   MAX_LINE_LENGTH,
                                   MAX_WHITESPACE_CHARS,
                                   STRICT_CRLF>;

      using MessageHeaders = message_headers<MAX_HEADER_NUMBER,
                                             MAX_HEADER_LENGTH,
                                             MAX_LINE_LENGTH,
                                             MAX_WHITESPACE_CHARS,
                                             STRICT_CRLF>;

      using Chunk = rx_chunk<Container,
                             MAX_CHUNK_SIZE,
                             MAX_HEADER_NUMBER,
                             MAX_HEADER_LENGTH,
                             MAX_LINE_LENGTH,
                             MAX_WHITESPACE_CHARS,
                             STRICT_CRLF>;

      /// Response information
      Response  response_ {}; ///< the received response
      Chunk     chunk_ {};    ///< the received chunk
      Container body_ {};     ///< the response body or data for the last chunk

    public:

      /// Default Constructor.
      response_receiver() = default;

      /// clear the response_receiver.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        response_.clear();
        chunk_.clear();
        body_.clear();
      }

      /// Accessor for the HTTP response header.
      /// @return a constant reference to an rx_response.
      Response const& response() const noexcept
      { return response_; }

      /// Accessor for the received chunk.
      /// @return a constant reference to the received chunk.
      Chunk const& chunk() const noexcept
      { return chunk_; }

      /// Accessor for the response body / last chunk data.
      /// @return a constant reference to the data.
      Container const& body() const noexcept
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
          // failed to parse response
          if (!response_.parse(iter, end))
          {
            // if a parsing error (not run out of data)
            if ((iter != end) || response_.fail())
            {
              clear();
              return Rx::INVALID;
            }
            else
              return Rx::INCOMPLETE;
          }
        }

        // build a response body or receive a chunk
        if (!response_.is_chunked())
        {
          // if there is a content length header, ensure it's valid
          std::ptrdiff_t content_length(response_.content_length());
          if (content_length < 0)
          {
            clear();
            return Rx::INVALID;
          }

          // if there's a message body without a content length header
          // then allow upto MAX_BODY_SIZE
          // The server can disconnect after it's finished sending the body
          std::ptrdiff_t rx_size(std::distance(iter, end));
          if ((rx_size > 0) && (content_length == 0) &&
              response_.headers().find(header_field::LC_CONTENT_LENGTH).empty())
            content_length = MAX_BODY_SIZE;

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

          // return whether the body is complete
          if (body_.size() == static_cast<size_t>(response_.content_length()))
            return Rx::VALID;
        }
        else // response_.is_chunked()
        {
          // If parsed a chunk and its data previously,
          // then clear it ready for the next chunk
          if (chunk_.valid())
            chunk_.clear();

          // If parsed the response header, pass it to the application
          if (response_parsed)
            return Rx::VALID;

          // parse the chunk
          if (!chunk_.parse(iter, end))
          {
            // if a parsing error (not run out of data)
            if (iter != end)
            {
              clear();
              return Rx::INVALID;
            }
          }

          // A complete chunk has been parsed..
          if (chunk_.valid())
            return Rx::CHUNK;
        }

        return Rx::INCOMPLETE;
      }

    };

  }
}

#endif
