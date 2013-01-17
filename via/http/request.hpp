#pragma once

#ifndef REQUEST_HPP_VIA_HTTPLIB_
#define REQUEST_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "request_method.hpp"
#include "headers.hpp"
#include <algorithm>

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class request_line
    //////////////////////////////////////////////////////////////////////////
    class request_line
    {
      std::string method_;
      std::string uri_;
      int major_version_;
      int minor_version_;

      enum parsing_state
      {
        REQ_METHOD,
        REQ_URI,
        REQ_HTTP,
        REQ_HTTP_T1,
        REQ_HTTP_T2,
        REQ_HTTP_P,
        REQ_HTTP_SLASH,
        REQ_HTTP_MAJOR,
        REQ_HTTP_MINOR,
        REQ_HTTP_LF,
        REQ_HTTP_END
      };

      /// Parse an individual character.
      /// @param c the character to be parsed.
      /// @retval state the current state of the parser.
      /// @return true if valid, false otherwise.
      bool parse_char(char c, parsing_state& state);

    public:

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default constructor
      explicit request_line()
        : method_()
        , uri_()
        , major_version_(0)
        , minor_version_(0)
      {}

      /// Virtual destructor.
      /// Since the class is inherited...
      virtual ~request_line()
      {}

      /// Parse the line as an HTTP request.
      /// @retval next reference to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator1, typename ForwardIterator2>
      bool parse(ForwardIterator1& next, ForwardIterator2 end)
      {
        ForwardIterator1 iter(next);
        parsing_state state(REQ_METHOD);
        while ((iter != end) && (REQ_HTTP_END != state))
        {
          char c(static_cast<char>(*iter++));
          if (!parse_char(c, state))
            return false;
        }

        if (REQ_HTTP_END != state)
          return false;

        next = iter;
        return true;
      }

      // Accessors
      const std::string& method() const
      { return method_; }

      const std::string& uri() const
      { return uri_; }

      int major_version() const
      { return major_version_; }

      int minor_version() const
      { return minor_version_; }

      ////////////////////////////////////////////////////////////////////////
      // Encoding interface.

      /// Id constructor
      /// @param method
      /// @param uri
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit request_line(request_method::method_id id,
                            std::string uri = "",
                            int minor_version = 1,
                            int major_version = 1)
        : method_(request_method::name(id))
        , uri_(uri)
        , major_version_(major_version)
        , minor_version_(minor_version)
      {}

      /// Free form constructor
      /// @param method
      /// @param uri
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit request_line(const std::string& method,
                            std::string uri = "",
                            int minor_version = 1,
                            int major_version = 1)
        : method_(method)
        , uri_(uri)
        , major_version_(major_version)
        , minor_version_(minor_version)
      {}

      // Setters
      void set_method(const std::string& method)
      { method_ = method; }

      void set_uri(const std::string& uri)
      { uri_ = uri; }

      void set_major_version(int major_version)
      { major_version_ = major_version; }

      void set_minor_version(int minor_version)
      { minor_version_ = minor_version; }

      /// Output as a string.
      /// @return a string containing the request line.
      std::string to_string() const;

    }; // class request_line

    //////////////////////////////////////////////////////////////////////////
    /// @class rx_request
    //////////////////////////////////////////////////////////////////////////
    class rx_request : public request_line
    {
      headers   headers_;

    public:

      /// Default constructor
      explicit rx_request()
        : request_line()
        , headers_()
      {}

      /// Parse an HTTP request.
      /// @retval next reference to an iterator to the start of the data.
      /// If the response is not valid it will not be changed,
      /// otherwise it will refer to:
      ///   - the start of the response body if content_length > 0,
      ///   - the start of the first data chunk if is_chunked(),
      ///   - the start of the next http response, or
      ///   - the end of the data buffer.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator1, typename ForwardIterator2>
      bool parse(ForwardIterator1& next, ForwardIterator2 end)
      {
        ForwardIterator1 iter(next);
        if (!request_line::parse(iter, end))
          return false;

        if (!headers_.parse(iter, end))
          return false;

        next = iter;
        return true;
      }

      /// Parsing contructor.
      /// @retval next reference to an iterator to the start of the data.
      /// If the request is not valid it will not be changed,
      /// otherwise it will refer to:
      ///   - the start of the request body if content_length > 0,
      ///   - the start of the first data chunk if is_chunked(),
      ///   - the start of the next http request, or
      ///   - the end of the data buffer.
      /// @param end the end of the data buffer.
      template<typename ForwardIterator1, typename ForwardIterator2>
      explicit rx_request(ForwardIterator1& next, ForwardIterator2 end)
        : request_line()
        , headers_()
      { parse(next, end); }

      // Accessors
      const headers& header() const
      { return headers_; }

      size_t content_length() const
      { return headers_.content_length(); }

      bool is_chunked() const
      { return headers_.is_chunked(); }
    }; // class rx_request

    //////////////////////////////////////////////////////////////////////////
    /// @class tx_request
    //////////////////////////////////////////////////////////////////////////
    class tx_request : public request_line
    {
      size_t      content_length_;
      std::string header_string_;
      bool        is_chunked_;

    public:

      /// Constructor for a standard request.
      /// @param method
      /// @param uri
      /// @param content_length
      /// @param header_string
      /// @param is_chunked
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit tx_request(request_method::method_id id,
                          std::string uri,
                          size_t content_length = 0,
                          std::string header_string = "",
                          bool is_chunked = false,
                          int minor_version = 1,
                          int major_version = 1)
        : request_line(id, uri, minor_version, major_version)
        , content_length_(content_length)
        , header_string_(header_string)
        , is_chunked_(is_chunked)
      {}

      /// Constructor for non-standard requests.
      /// @param method
      /// @param uri
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit tx_request(const std::string& method,
                          std::string uri,
                          size_t content_length = 0,
                          std::string header_string = "",
                          bool is_chunked = false,
                          int minor_version = 1,
                          int major_version = 1)
        : request_line(method, uri, minor_version, major_version)
        , content_length_(content_length)
        , header_string_(header_string)
        , is_chunked_(is_chunked)
      {}

      /// Add a free form header to the request.
      void add_header(std::string const& field, const std::string& value)
      { header_string_ += header_field::to_header(field, value);  }

      /// Add a standard header to the request.
      void add_header(header_field::field_id id, const std::string& value)
      { header_string_ += header_field::to_header(id, value);  }

      /// The http message header string.
      std::string message() const
      {
        std::string output(request_line::to_string());
        output += header_string_;

        if (is_chunked_)
          output += header_field::chunked_encoding();
        else // always send the content length...
          output += header_field::content_length(content_length_);
        output += CRLF;

        return output;
      }
    }; // class tx_request

  }
}

#endif
