#pragma once

#ifndef RESPONSE_HPP_VIA_HTTPLIB_
#define RESPONSE_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "response_status.hpp"
#include "headers.hpp"
#include <algorithm>

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class response_line
    //////////////////////////////////////////////////////////////////////////
    class response_line
    {
      int major_version_;
      int minor_version_;
      int status_;
      std::string reason_phrase_;

      enum parsing_state
      {
        RESP_HTTP,
        RESP_HTTP_T1,
        RESP_HTTP_T2,
        RESP_HTTP_P,
        RESP_HTTP_SLASH,
        RESP_HTTP_MAJOR,
        RESP_HTTP_MINOR,
        RESP_HTTP_STATUS,
        RESP_HTTP_REASON,
        RESP_HTTP_LF,
        RESP_HTTP_END
      };

      /// Parse an individual character.
      /// @param c the character to be parsed.
      /// @retval state the current state of the parser.
      /// @return true if valid, false otherwise.
      bool parse_char(char c,  parsing_state& state);

    public:

      ////////////////////////////////////////////////////////////////////////
      // Parsing interface.

      /// Default constructor
      explicit response_line()
        : major_version_(0)
        , minor_version_(0)
        , status_(0)
        , reason_phrase_("")
      {}

      /// Virtual destructor.
      /// Since the class is inherited...
      virtual ~response_line()
      {}

      /// Parse the line as an HTTP response.
      /// @retval next reference to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator1, typename ForwardIterator2>
      bool parse(ForwardIterator1& next, ForwardIterator2 end)
      {
        ForwardIterator1 iter(next);
        parsing_state state(RESP_HTTP);
        while ((iter != end) && (RESP_HTTP_END != state))
        {
          char c(static_cast<char>(*iter++));
          if (!parse_char(c, state))
            return false;
        }

        if (RESP_HTTP_END != state)
          return false;

        next = iter;
        return true;
      }

      // Accessors
      int major_version() const
      { return major_version_; }

      int minor_version() const
      { return minor_version_; }

      int status() const
      { return status_; }

      const std::string& reason_phrase() const
      { return reason_phrase_; }

      ////////////////////////////////////////////////////////////////////////
      // Encoding interface.

      /// Status constructor for standard responses.
      /// This is the usual contructor to use when creating an http response.
      /// @param status
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit response_line(response_status::status_code status,
                             int minor_version = 1,
                             int major_version = 1)
        : major_version_(major_version)
        , minor_version_(minor_version)
        , status_(status)
        , reason_phrase_(response_status::reason_phrase(status))
      {}

      /// Free form constructor.
      /// This contructor should only be used for non-standard http responses.
      /// @param status
      /// @param reason_phrase default blank
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit response_line(int status,
                             std::string reason_phrase = "",
                             int minor_version = 1,
                             int major_version = 1)
        : major_version_(major_version)
        , minor_version_(minor_version)
        , status_(status)
        , reason_phrase_(!reason_phrase.empty() ? reason_phrase :
               response_status::reason_phrase
                    (static_cast<response_status::status_code>(status)))                 
      {}

      // Setters
      void set_major_version(int major_version)
      { major_version_ = major_version; }

      void set_minor_version(int minor_version)
      { minor_version_ = minor_version; }

      void set_status(int status)
      { status_ = status; }

      void set_reason_phrase(const std::string& reason_phrase)
      { reason_phrase_ = reason_phrase; }

      /// Output as a string.
      /// @return a string containing the response line.
      std::string to_string() const;

    }; // class response_line

    //////////////////////////////////////////////////////////////////////////
    /// @class rx_response
    //////////////////////////////////////////////////////////////////////////
    class rx_response : public response_line
    {
      headers headers_;

    public:

      /// Default constructor
      explicit rx_response()
        : response_line()
        , headers_()
      {}

      /// Parse an HTTP response.
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
        if (!response_line::parse(iter, end))
          return false;

        if (!headers_.parse(iter, end))
          return false;

        next = iter;
        return true;
      }

      /// Parsing contructor.
      /// @retval next reference to an iterator to the start of the data.
      /// If the response is not valid it will not be changed,
      /// otherwise it will refer to:
      ///   - the start of the response body if content_length > 0,
      ///   - the start of the first data chunk if is_chunked(),
      ///   - the start of the next http response, or
      ///   - the end of the data buffer.
      /// @param end the end of the data buffer.
      template<typename ForwardIterator1, typename ForwardIterator2>
      explicit rx_response(ForwardIterator1& next, ForwardIterator2 end)
        : response_line()
        , headers_()
      { parse(next, end); }

      // Accessors
      const headers& header() const
      { return headers_; }

      size_t content_length() const
      { return headers_.content_length(); }

      bool is_chunked() const
      { return headers_.is_chunked(); }
    }; // class rx_response

    //////////////////////////////////////////////////////////////////////////
    /// @class tx_response
    //////////////////////////////////////////////////////////////////////////
    class tx_response : public response_line
    {
      size_t      content_length_;
      std::string header_string_;
      bool        is_chunked_;

    public:

      /// Constructor for a standard response.
      /// @param status
      /// @param content_length
      /// @param header_string default blank
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit tx_response(response_status::status_code status,
                           size_t content_length = 0,
                           std::string header_string = "",
                           bool is_chunked = false,
                           int minor_version = 1,
                           int major_version = 1)
        : response_line(status, minor_version, major_version)
        , content_length_(content_length)
        , header_string_(header_string)
        , is_chunked_(is_chunked)
      {}

      /// Constructor for non-standard responses.
      /// @param status
      /// @param content_length
      /// @param header_string default blank
      /// @param minor_version default 1
      /// @param major_version default 1
      explicit tx_response(int status,
                           std::string reason_phrase,
                           size_t content_length = 0,
                           std::string header_string = "",
                           bool is_chunked = false,
                           int minor_version = 1,
                           int major_version = 1)
        : response_line(status, reason_phrase, minor_version, major_version)
        , content_length_(content_length)
        , header_string_(header_string)
        , is_chunked_(is_chunked)
      {}

      /// Add a free form header to the response.
      void add_header(std::string const& field, const std::string& value)
      { header_string_ += header_field::to_header(field, value);  }

      /// Add a standard header to the response.
      void add_header(header_field::field_id id, const std::string& value)
      { header_string_ += header_field::to_header(id, value);  }

      /// The http message header string.
      std::string message() const
      {
        std::string output(response_line::to_string());
        output += header_string_;

        if (is_chunked_)
          output += header_field::chunked_encoding();
        else // always send the content length...
          output += header_field::content_length(content_length_);
        output += CRLF;

        return output;
      }
    };

  }
}

#endif
