#ifndef HEADER_FIELD_HPP_VIA_HTTPLIB_
#define HEADER_FIELD_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file header_field.hpp
/// @brief Enumerations and functions to handle HTTP header fields.
//////////////////////////////////////////////////////////////////////////////
#include <string>
#include <ctime>

namespace via
{
  namespace http
  {
    namespace header_field
    {
      /// Ids for the standard headers defined in RFC2616.
      enum field_id
      {
        // General Header Fields
        CACHE_CONTROL,
        CONNECTION,
        DATE,
        PRAGMA,
        TRAILER,
        TRANSFER_ENCODING,
        UPGRADE,
        VIA,
        WARNING,

        // Request Header Fields
        ACCEPT,
        ACCEPT_CHARSET,
        ACCEPT_ENCODING,
        ACCEPT_LANGUAGE,
        AUTHORIZATION,
        EXPECT,
        FROM,
        HOST,
        IF_MATCH,
        IF_MODIFIED_SINCE,
        IF_NONE_MATCH,
        IF_RANGE,
        IF_UNMODIFIED_SINCE,
        MAX_FORWARDS,
        PROXY_AUTHORIZATION,
        RANGE,
        REFERER,
        TE,
        USER_AGENT,

        // Response Header Fields
        ACCEPT_RANGES,
        AGE,
        ETAG,
        LOCATION,
        PROXY_AUTHENTICATE,
        RETRY_AFTER,
        SERVER,
        VARY,
        WWW_AUTHENTICATE,

        // Entity Header Fields
        ALLOW,
        CONTENT_ENCODING,
        CONTENT_LANGUAGE,
        CONTENT_LENGTH,
        CONTENT_LOCATION,
        CONTENT_MD5,
        CONTENT_RANGE,
        CONTENT_TYPE,
        EXPIRES,
        LAST_MODIFIED,
        EXTENSION_HEADER
      };

      /// Lookup the RFC2616 standard name for the given header field.
      /// @param id the header field id.
      /// @return the field name from RFC2616.
      const std::string& standard_name(field_id id);

      /// Lookup the lowercase name for the given header field.
      /// @param id the header field id.
      /// @return the field name from RFC2616 converted to lowercase.
      const std::string& lowercase_name(field_id id);

      /// Format the field name and value into an http header line.
      /// @param name header field name.
      /// @param value header field value.
      std::string to_header(std::string const& name,
                            std::string const& value);

      /// Format the field id and value into an http header line.
      /// @param id header field id.
      /// @param value header field value.
      std::string to_header(field_id id, std::string const& value);

      /// Create an http header line for the current date and time.
      std::string date_header();

      /// Create an http header line for this server.
      std::string server_header();

      /// Create a Content-Type: message/http header for a TRACE response
      std::string content_http_header();

      /// An http content length header line for the given size.
      /// @param size
      /// @return http content length header line for the size.
      std::string content_length(size_t size);

      /// An http transfer encoding header line containing "Chunked".
      std::string chunked_encoding();

    }
  }
}

#endif
