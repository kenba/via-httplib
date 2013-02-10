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
#include <boost/logic/tribool.hpp>
#include <algorithm>
#include <cassert>

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class request_line
    //////////////////////////////////////////////////////////////////////////
    class request_line
    {
    public:

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

    private:

      std::string method_;
      std::string uri_;
      int major_version_;
      int minor_version_;
      parsing_state state_;
      bool major_read_;
      bool minor_read_;
      bool valid_;

      /// Parse an individual character.
      /// @param c the character to be parsed.
      /// @return true if valid, false otherwise.
      bool parse_char(char c);

    public:

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default constructor
      explicit request_line() :
        method_(),
        uri_(),
        major_version_(0),
        minor_version_(0),
        state_(REQ_METHOD),
        major_read_(false),
        minor_read_(false),
        valid_(false)
      {}

      void clear()
      {
        method_.clear();
        uri_.clear();
        major_version_ = 0;
        minor_version_ = 0;
        state_ = REQ_METHOD;
        major_read_ =  false;
        minor_read_ =  false;
        valid_ =  false;
      }

      void swap(request_line& other)
      {
        method_.swap(other.method_);
        uri_.swap(other.uri_);
        std::swap(major_version_, other.major_version_);
        std::swap(minor_version_, other.minor_version_);
        std::swap(state_, other.state_);
        std::swap(major_read_, other.major_read_);
        std::swap(minor_read_, other.minor_read_);
        std::swap(valid_, other.valid_);
      }

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
      bool parse(ForwardIterator1& iter, ForwardIterator2 end)
      {
        while ((iter != end) && (REQ_HTTP_END != state_))
        {
          char c(static_cast<char>(*iter++));
          if (!parse_char(c))
            return false;
        }
        valid_ = (REQ_HTTP_END == state_);
        return valid_;
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

      bool valid() const
      { return valid_; }

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
                            int major_version = 1) :
        method_(request_method::name(id)),
        uri_(uri),
        major_version_(major_version),
        minor_version_(minor_version),
        state_(REQ_HTTP_END),
        major_read_(true),
        minor_read_(true),
        valid_(true)
      {}

      /// Free form constructor
      /// @param method
      /// @param uri
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit request_line(const std::string& method,
                            std::string uri = "",
                            int minor_version = 1,
                            int major_version = 1) :
        method_(method),
        uri_(uri),
        major_version_(major_version),
        minor_version_(minor_version),
        state_(REQ_HTTP_END),
        major_read_(true),
        minor_read_(true),
        valid_(true)
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
      message_headers headers_;
      bool valid_;

    public:

      /// Default constructor
      explicit rx_request() :
        request_line(),
        headers_(),
        valid_(false)
      {}

      void reset()
      {
        request_line::clear();
        headers_.clear();
        valid_ =  false;
      }

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
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator1, typename ForwardIterator2>
      bool parse(ForwardIterator1& iter, ForwardIterator2 end)
      {
        if (!request_line::valid() && !request_line::parse(iter, end))
          return false;

        if (!headers_.valid() && !headers_.parse(iter, end))
          return false;

        valid_ = true;
        return valid_;
      }

      // Accessors
      const message_headers& header() const
      { return headers_; }

      size_t content_length() const
      { return headers_.content_length(); }

      bool is_chunked() const
      { return headers_.is_chunked(); }

      bool valid() const
      { return valid_; }

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
                          int major_version = 1) :
        request_line(id, uri, minor_version, major_version),
        content_length_(content_length),
        header_string_(header_string),
        is_chunked_(is_chunked)
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
                          int major_version = 1) :
        request_line(method, uri, minor_version, major_version),
        content_length_(content_length),
        header_string_(header_string),
        is_chunked_(is_chunked)
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


    //////////////////////////////////////////////////////////////////////////
    /// @class request_receiver
    //////////////////////////////////////////////////////////////////////////
    template <typename Container>
    class request_receiver
    {
      typedef typename Container::const_iterator Container_const_iterator;
      rx_request request_;
      Container  body_;

    public:

      explicit request_receiver() :
        request_(),
        body_()
      {}

      void clear()
      {
        request_.reset();
        body_.clear();
      }

      rx_request const& request() const
      { return request_; }

      Container const& body() const
      { return body_; }

      boost::logic::tribool receive(Container_const_iterator iter,
                                    Container_const_iterator end)
      {
        // building a request
        if (!request_.valid())
        {
          // failed to parse request
          if (!request_.parse(iter, end))
          {
            // if a parsing error (not run out of data)
            if (iter != end)
            {
              request_.reset();
              return false;
            }
            else
              return boost::logic::tribool::indeterminate_value;
          }
        }

        // build a request body
        assert(request_.valid());
        if (end > iter)
          body_.insert(body_.end(), iter, end);

        // return whether the body is complete
        if (body_.size() >= request_.content_length())
          return true;

        return boost::logic::tribool::indeterminate_value;
      }

    };

  }
}

#endif
