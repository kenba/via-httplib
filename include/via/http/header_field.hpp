#ifndef HEADER_FIELD_HPP_VIA_HTTPLIB_
#define HEADER_FIELD_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file header_field.hpp
/// @brief Enumerations and functions to handle HTTP header fields.
//////////////////////////////////////////////////////////////////////////////
#include "character.hpp"
#include <string>
#include <string_view>
#include <ctime>

namespace via
{
  namespace http
  {
    namespace header_field
    {
      /// The server name for the server header.
      constexpr char SERVER_NAME[]                {"Via-httplib"};

      /// The message/http content for a TRACE response
      constexpr char MESSAGE_HTTP[]               {"message/http"};

      // The following strings are for the standard header field names.

      // General Header Fields, RFC2616 sec 4.5 pg 33
      constexpr char HEADER_CACHE_CONTROL[]       {"Cache-Control"};
      constexpr char HEADER_CONNECTION[]          {"Connection"};
      constexpr char HEADER_DATE[]                {"Date"};
      constexpr char HEADER_PRAGMA[]              {"Pragma"};
      constexpr char HEADER_TRAILER[]             {"Trailer"};
      constexpr char HEADER_TRANSFER_ENCODING[]   {"Transfer-Encoding"};
      constexpr char HEADER_UPGRADE[]             {"Upgrade"};
      constexpr char HEADER_VIA[]                 {"Via"};
      constexpr char HEADER_WARNING[]             {"Warning"};

      // Request Header Fields, RFC2616 sec 5.3, pg 38
      constexpr char HEADER_ACCEPT[]              {"Accept"};
      constexpr char HEADER_ACCEPT_CHARSET[]      {"Accept-Charset"};
      constexpr char HEADER_ACCEPT_ENCODING[]     {"Accept-Encoding"};
      constexpr char HEADER_ACCEPT_LANGUAGE[]     {"Accept-Language"};
      constexpr char HEADER_AUTHORIZATION[]       {"Authorization"};
      constexpr char HEADER_EXPECT[]              {"Expect"};
      constexpr char HEADER_FROM[]                {"From"};
      constexpr char HEADER_HOST[]                {"Host"};
      constexpr char HEADER_IF_MATCH[]            {"If-Match"};
      constexpr char HEADER_IF_MODIFIED_SINCE []  {"If-Modified-Since"};
      constexpr char HEADER_IF_NONE_MATCH[]       {"If-None-Match"};
      constexpr char HEADER_IF_RANGE[]            {"If-Range"};
      constexpr char HEADER_IF_UNMODIFIED_SINCE[] {"If-Unmodified-Since"};
      constexpr char HEADER_MAX_FORWARDS[]        {"Max-Forwards"};
      constexpr char HEADER_PROXY_AUTHORIZATION[] {"Proxy-Authorization"};
      constexpr char HEADER_RANGE[]               {"Range"};
      constexpr char HEADER_REFERER[]             {"Referer"};
      constexpr char HEADER_TE[]                  {"TE"};
      constexpr char HEADER_USER_AGENT[]          {"User-Agent"};

      // Response Header Fields, RFC2616 sec 6.2, pg 40
      constexpr char HEADER_ACCEPT_RANGES[]       {"Accept-Ranges"};
      constexpr char HEADER_AGE[]                 {"Age"};
      constexpr char HEADER_ETAG[]                {"ETag"};
      constexpr char HEADER_LOCATION[]            {"Location"};
      constexpr char HEADER_PROXY_AUTHENTICATE[]  {"Proxy-Authenticate"};
      constexpr char HEADER_RETRY_AFTER[]         {"Retry-After"};
      constexpr char HEADER_SERVER[]              {"Server"};
      constexpr char HEADER_VARY[]                {"Vary"};
      constexpr char HEADER_WWW_AUTHENTICATE[]    {"WWW-Authenticate"};

      // Entity Header Fields, RFC2616 sec 7.1, pg 41
      constexpr char HEADER_ALLOW[]               {"Allow"};
      constexpr char HEADER_CONTENT_ENCODING[]    {"Content-Encoding"};
      constexpr char HEADER_CONTENT_LANGUAGE[]    {"Content-Language"};
      constexpr char HEADER_CONTENT_LENGTH[]      {"Content-Length"};
      constexpr char HEADER_CONTENT_LOCATION[]    {"Content-Location"};
      constexpr char HEADER_CONTENT_MD5[]         {"Content-MD5"};
      constexpr char HEADER_CONTENT_RANGE[]       {"Content-Range"};
      constexpr char HEADER_CONTENT_TYPE[]        {"Content-Type"};
      constexpr char HEADER_EXPIRES[]             {"Expires"};
      constexpr char HEADER_LAST_MODIFIED[]       {"Last-Modified"};
      constexpr char HEADER_EXTENSION_HEADER[]    {"extension-header"};

      // The following strings are for the lower case header field names.

      // General Header Fields, RFC2616 sec 4.5 pg 33
      constexpr char LC_CACHE_CONTROL[]           {"cache-control"};
      constexpr char LC_CONNECTION[]              {"connection"};
      constexpr char LC_DATE[]                    {"date"};
      constexpr char LC_PRAGMA[]                  {"pragma"};
      constexpr char LC_TRAILER[]                 {"trailer"};
      constexpr char LC_TRANSFER_ENCODING[]       {"transfer-encoding"};
      constexpr char LC_UPGRADE[]                 {"upgrade"};
      constexpr char LC_VIA[]                     {"via"};
      constexpr char LC_WARNING[]                 {"warning"};

      // Request Header Fields, RFC2616 sec 5.3, pg 38
      constexpr char LC_ACCEPT[]                  {"accept"};
      constexpr char LC_ACCEPT_CHARSET[]          {"accept-charset"};
      constexpr char LC_ACCEPT_ENCODING[]         {"accept-encoding"};
      constexpr char LC_ACCEPT_LANGUAGE[]         {"accept-language"};
      constexpr char LC_AUTHORIZATION[]           {"authorization"};
      constexpr char LC_EXPECT[]                  {"expect"};
      constexpr char LC_FROM[]                    {"from"};
      constexpr char LC_HOST[]                    {"host"};
      constexpr char LC_IF_MATCH[]                {"if-match"};
      constexpr char LC_IF_MODIFIED_SINCE[]       {"if-modified-since"};
      constexpr char LC_IF_NONE_MATCH[]           {"if-none-match"};
      constexpr char LC_IF_RANGE[]                {"if-range"};
      constexpr char LC_IF_UNMODIFIED_SINCE[]     {"if-unmodified-since"};
      constexpr char LC_MAX_FORWARDS[]            {"max-forwards"};
      constexpr char LC_PROXY_AUTHORIZATION[]     {"proxy-authorization"};
      constexpr char LC_RANGE[]                   {"range"};
      constexpr char LC_REFERER[]                 {"referer"};
      constexpr char LC_TE[]                      {"te"};
      constexpr char LC_USER_AGENT[]              {"user-agent"};

      // Response Header Fields, RFC2616 sec 6.2, pg 40
      constexpr char LC_ACCEPT_RANGES[]           {"accept-ranges"};
      constexpr char LC_AGE[]                     {"age"};
      constexpr char LC_ETAG[]                    {"etag"};
      constexpr char LC_LOCATION[]                {"location"};
      constexpr char LC_PROXY_AUTHENTICATE[]      {"proxy-authenticate"};
      constexpr char LC_RETRY_AFTER[]             {"retry-after"};
      constexpr char LC_SERVER[]                  {"server"};
      constexpr char LC_VARY[]                    {"vary"};
      constexpr char LC_WWW_AUTHENTICATE[]        {"www-authenticate"};

      // Entity Header Fields, RFC2616 sec 7.1, pg 41
      constexpr char LC_ALLOW[]                   {"allow"};
      constexpr char LC_CONTENT_ENCODING[]        {"content-encoding"};
      constexpr char LC_CONTENT_LANGUAGE[]        {"content-language"};
      constexpr char LC_CONTENT_LENGTH[]          {"content-length"};
      constexpr char LC_CONTENT_LOCATION[]        {"content-location"};
      constexpr char LC_CONTENT_MD5[]             {"content-md5"};
      constexpr char LC_CONTENT_RANGE[]           {"content-range"};
      constexpr char LC_CONTENT_TYPE[]            {"content-type"};
      constexpr char LC_EXPIRES[]                 {"expires"};
      constexpr char LC_LAST_MODIFIED[]           {"last-modified"};
      constexpr char LC_EXTENSION_HEADER[]        {"extension-header"};

      /// The header field seperator, colon space.
      constexpr char SEPARATOR[] {": "};

      /// The value to use for chunked transfer encoding.
      constexpr char CHUNKED[] {"Chunked"};

      /// Ids for the standard headers defined in RFC2616.
      enum class id
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
      constexpr std::string_view standard_name(id field_id) noexcept
      {
        switch(field_id)
        {
        // General Header Fields
        case id::CACHE_CONTROL:       return HEADER_CACHE_CONTROL;
        case id::CONNECTION:          return HEADER_CONNECTION;
        case id::DATE:                return HEADER_DATE;
        case id::PRAGMA:              return HEADER_PRAGMA;
        case id::TRAILER:             return HEADER_TRAILER;
        case id::TRANSFER_ENCODING:   return HEADER_TRANSFER_ENCODING;
        case id::UPGRADE:             return HEADER_UPGRADE;
        case id::VIA:                 return HEADER_VIA;
        case id::WARNING:             return HEADER_WARNING;

        // Request Header Fields
        case id::ACCEPT:              return HEADER_ACCEPT;
        case id::ACCEPT_CHARSET:      return HEADER_ACCEPT_CHARSET;
        case id::ACCEPT_ENCODING:     return HEADER_ACCEPT_ENCODING;
        case id::ACCEPT_LANGUAGE:     return HEADER_ACCEPT_LANGUAGE;
        case id::AUTHORIZATION:       return HEADER_AUTHORIZATION;
        case id::EXPECT:              return HEADER_EXPECT;
        case id::FROM:                return HEADER_FROM;
        case id::HOST:                return HEADER_HOST;
        case id::IF_MATCH:            return HEADER_IF_MATCH;
        case id::IF_MODIFIED_SINCE:   return HEADER_IF_MODIFIED_SINCE;
        case id::IF_NONE_MATCH:       return HEADER_IF_NONE_MATCH;
        case id::IF_RANGE:            return HEADER_IF_RANGE;
        case id::IF_UNMODIFIED_SINCE: return HEADER_IF_UNMODIFIED_SINCE;
        case id::MAX_FORWARDS:        return HEADER_MAX_FORWARDS;
        case id::PROXY_AUTHORIZATION: return HEADER_PROXY_AUTHORIZATION;
        case id::RANGE:               return HEADER_RANGE;
        case id::REFERER:             return HEADER_REFERER;
        case id::TE:                  return HEADER_TE;
        case id::USER_AGENT:          return HEADER_USER_AGENT;

        // Response Header Fields
        case id::ACCEPT_RANGES:       return HEADER_ACCEPT_RANGES;
        case id::AGE:                 return HEADER_AGE;
        case id::ETAG:                return HEADER_ETAG;
        case id::LOCATION:            return HEADER_LOCATION;
        case id::PROXY_AUTHENTICATE:  return HEADER_PROXY_AUTHENTICATE;
        case id::RETRY_AFTER:         return HEADER_RETRY_AFTER;
        case id::SERVER:              return HEADER_SERVER;
        case id::VARY:                return HEADER_VARY;
        case id::WWW_AUTHENTICATE:    return HEADER_WWW_AUTHENTICATE;

        // Entity Header Fields
        case id::ALLOW:               return HEADER_ALLOW;
        case id::CONTENT_ENCODING:    return HEADER_CONTENT_ENCODING;
        case id::CONTENT_LANGUAGE:    return HEADER_CONTENT_LANGUAGE;
        case id::CONTENT_LENGTH:      return HEADER_CONTENT_LENGTH;
        case id::CONTENT_LOCATION:    return HEADER_CONTENT_LOCATION;
        case id::CONTENT_MD5:         return HEADER_CONTENT_MD5;
        case id::CONTENT_RANGE:       return HEADER_CONTENT_RANGE;
        case id::CONTENT_TYPE:        return HEADER_CONTENT_TYPE;
        case id::EXPIRES:             return HEADER_EXPIRES;
        case id::LAST_MODIFIED:       return HEADER_LAST_MODIFIED;
        case id::EXTENSION_HEADER:    return HEADER_EXTENSION_HEADER;

        default:                      return std::string_view();
        }
      }

      /// Lookup the lowercase name for the given header field.
      /// @param id the header field id.
      /// @return the field name from RFC2616 converted to lowercase.
      constexpr std::string_view lowercase_name(id field_id) noexcept
      {
        switch(field_id)
        {
        // General Header Fields
        case id::CACHE_CONTROL:       return LC_CACHE_CONTROL;
        case id::CONNECTION:          return LC_CONNECTION;
        case id::DATE:                return LC_DATE;
        case id::PRAGMA:              return LC_PRAGMA;
        case id::TRAILER:             return LC_TRAILER;
        case id::TRANSFER_ENCODING:   return LC_TRANSFER_ENCODING;
        case id::UPGRADE:             return LC_UPGRADE;
        case id::VIA:                 return LC_VIA;
        case id::WARNING:             return LC_WARNING;

        // Request Header Fields
        case id::ACCEPT:              return LC_ACCEPT;
        case id::ACCEPT_CHARSET:      return LC_ACCEPT_CHARSET;
        case id::ACCEPT_ENCODING:     return LC_ACCEPT_ENCODING;
        case id::ACCEPT_LANGUAGE:     return LC_ACCEPT_LANGUAGE;
        case id::AUTHORIZATION:       return LC_AUTHORIZATION;
        case id::EXPECT:              return LC_EXPECT;
        case id::FROM:                return LC_FROM;
        case id::HOST:                return LC_HOST;
        case id::IF_MATCH:            return LC_IF_MATCH;
        case id::IF_MODIFIED_SINCE:   return LC_IF_MODIFIED_SINCE;
        case id::IF_NONE_MATCH:       return LC_IF_NONE_MATCH;
        case id::IF_RANGE:            return LC_IF_RANGE;
        case id::IF_UNMODIFIED_SINCE: return LC_IF_UNMODIFIED_SINCE;
        case id::MAX_FORWARDS:        return LC_MAX_FORWARDS;
        case id::PROXY_AUTHORIZATION: return LC_PROXY_AUTHORIZATION;
        case id::RANGE:               return LC_RANGE;
        case id::REFERER:             return LC_REFERER;
        case id::TE:                  return LC_TE;
        case id::USER_AGENT:          return LC_USER_AGENT;

        // Response Header Fields
        case id::ACCEPT_RANGES:       return LC_ACCEPT_RANGES;
        case id::AGE:                 return LC_AGE;
        case id::ETAG:                return LC_ETAG;
        case id::LOCATION:            return LC_LOCATION;
        case id::PROXY_AUTHENTICATE:  return LC_PROXY_AUTHENTICATE;
        case id::RETRY_AFTER:         return LC_RETRY_AFTER;
        case id::SERVER:              return LC_SERVER;
        case id::VARY:                return LC_VARY;
        case id::WWW_AUTHENTICATE:    return LC_WWW_AUTHENTICATE;

        // Entity Header Fields
        case id::ALLOW:               return LC_ALLOW;
        case id::CONTENT_ENCODING:    return LC_CONTENT_ENCODING;
        case id::CONTENT_LANGUAGE:    return LC_CONTENT_LANGUAGE;
        case id::CONTENT_LENGTH:      return LC_CONTENT_LENGTH;
        case id::CONTENT_LOCATION:    return LC_CONTENT_LOCATION;
        case id::CONTENT_MD5:         return LC_CONTENT_MD5;
        case id::CONTENT_RANGE:       return LC_CONTENT_RANGE;
        case id::CONTENT_TYPE:        return LC_CONTENT_TYPE;
        case id::EXPIRES:             return LC_EXPIRES;
        case id::LAST_MODIFIED:       return LC_LAST_MODIFIED;
        case id::EXTENSION_HEADER:    return LC_EXTENSION_HEADER;

        default:                      return std::string_view();
        }
      }

      /// Format the field name and value into an http header line.
      /// @param name header field name.
      /// @param value header field value.
      inline std::string to_header(std::string_view name, std::string_view value)
      { return std::string(name) + SEPARATOR + std::string(value) + CRLF; }

      /// Format the field id and value into an http header line.
      /// @param id header field id.
      /// @param value header field value.
      inline std::string to_header(id field_id, std::string_view value)
      { return to_header(standard_name(field_id), value); }

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 ) // MSVC doesn't like gmtime...
#endif
      /// Create an http header line for the current date and time.
      inline std::string date_header()
      {
        /// The value to use to format an HTTP date into RFC1123 format.
        static constexpr char DATE_FORMAT[] {"%a, %d %b %Y %H:%M:%S GMT"};

        char dateBuffer[30] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        time_t uTime;
        time(&uTime);
        strftime(dateBuffer, 30, DATE_FORMAT, std::gmtime(&uTime));
        return to_header(HEADER_DATE, dateBuffer);
      }
#ifdef _MSC_VER
#pragma warning( pop )
#endif

      /// Create an http header line for this server.
      inline std::string server_header()
      { return to_header(HEADER_SERVER, SERVER_NAME); }

      /// Create a Content-Type: message/http header for a TRACE response
      inline std::string content_http_header()
      { return to_header(HEADER_CONTENT_TYPE, MESSAGE_HTTP); }

      /// An http content length header line for the given size.
      /// @param size
      /// @return http content length header line for the size.
      inline std::string content_length(size_t size)
      {
        return std::string(HEADER_CONTENT_LENGTH) + SEPARATOR
             + std::to_string(size) + CRLF;
      }

      /// An http transfer encoding header line containing "Chunked".
      inline std::string chunked_encoding()
      {
        return std::string(HEADER_TRANSFER_ENCODING) + SEPARATOR + CHUNKED + CRLF;
      }

    }
  }
}

#endif
