//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2018 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/header_field.hpp"
#include "via/http/character.hpp"
#include <ctime>

#ifdef _MSC_VER // MSVC doesn't like gmtime...
#pragma warning( disable : 4996 )
#endif

namespace
{
  /// The server name for the server header.
  const char SERVER_NAME[] =                {"Via-httplib"};

  /// The message/http content for a TRACE response
  const char MESSAGE_HTTP[] =               {"message/http"};

  // The following strings are for the standard header field names.

  // General Header Fields, RFC2616 sec 4.5 pg 33
  const char HEADER_CACHE_CONTROL[] =       {"Cache-Control"};
  const char HEADER_CONNECTION[] =          {"Connection"};
  const char HEADER_DATE[] =                {"Date"};
  const char HEADER_PRAGMA[] =              {"Pragma"};
  const char HEADER_TRAILER[] =             {"Trailer"};
  const char HEADER_TRANSFER_ENCODING[] =   {"Transfer-Encoding"};
  const char HEADER_UPGRADE[] =             {"Upgrade"};
  const char HEADER_VIA[] =                 {"Via"};
  const char HEADER_WARNING[] =             {"Warning"};

  // Request Header Fields, RFC2616 sec 5.3, pg 38
  const char HEADER_ACCEPT[] =              {"Accept"};
  const char HEADER_ACCEPT_CHARSET[] =      {"Accept-Charset"};
  const char HEADER_ACCEPT_ENCODING[] =     {"Accept-Encoding"};
  const char HEADER_ACCEPT_LANGUAGE[] =     {"Accept-Language"};
  const char HEADER_AUTHORIZATION[] =       {"Authorization"};
  const char HEADER_EXPECT[] =              {"Expect"};
  const char HEADER_FROM[] =                {"From"};
  const char HEADER_HOST[] =                {"Host"};
  const char HEADER_IF_MATCH[] =            {"If-Match"};
  const char HEADER_IF_MODIFIED_SINCE [] =  {"If-Modified-Since"};
  const char HEADER_IF_NONE_MATCH[] =       {"If-None-Match"};
  const char HEADER_IF_RANGE[] =            {"If-Range"};
  const char HEADER_IF_UNMODIFIED_SINCE[] = {"If-Unmodified-Since"};
  const char HEADER_MAX_FORWARDS[] =        {"Max-Forwards"};
  const char HEADER_PROXY_AUTHORIZATION[] = {"Proxy-Authorization"};
  const char HEADER_RANGE[] =               {"Range"};
  const char HEADER_REFERER[] =             {"Referer"};
  const char HEADER_TE[] =                  {"TE"};
  const char HEADER_USER_AGENT[] =          {"User-Agent"};

  // Response Header Fields, RFC2616 sec 6.2, pg 40
  const char HEADER_ACCEPT_RANGES[] =       {"Accept-Ranges"};
  const char HEADER_AGE[] =                 {"Age"};
  const char HEADER_ETAG[] =                {"ETag"};
  const char HEADER_LOCATION[] =            {"Location"};
  const char HEADER_PROXY_AUTHENTICATE[] =  {"Proxy-Authenticate"};
  const char HEADER_RETRY_AFTER[] =         {"Retry-After"};
  const char HEADER_SERVER[] =              {"Server"};
  const char HEADER_VARY[] =                {"Vary"};
  const char HEADER_WWW_AUTHENTICATE[] =    {"WWW-Authenticate"};

  // Entity Header Fields, RFC2616 sec 7.1, pg 41
  const char HEADER_ALLOW[] =               {"Allow"};
  const char HEADER_CONTENT_ENCODING[] =    {"Content-Encoding"};
  const char HEADER_CONTENT_LANGUAGE[] =    {"Content-Language"};
  const char HEADER_CONTENT_LENGTH[] =      {"Content-Length"};
  const char HEADER_CONTENT_LOCATION[] =    {"Content-Location"};
  const char HEADER_CONTENT_MD5[] =         {"Content-MD5"};
  const char HEADER_CONTENT_RANGE[] =       {"Content-Range"};
  const char HEADER_CONTENT_TYPE[] =        {"Content-Type"};
  const char HEADER_EXPIRES[] =             {"Expires"};
  const char HEADER_LAST_MODIFIED[] =       {"Last-Modified"};
  const char HEADER_EXTENSION_HEADER[] =    {"extension-header"};

  // The following strings are for the lower case header field names.

  // General Header Fields, RFC2616 sec 4.5 pg 33
  const char LC_CACHE_CONTROL[] =           {"cache-control"};
  const char LC_CONNECTION[] =              {"connection"};
  const char LC_DATE[] =                    {"date"};
  const char LC_PRAGMA[] =                  {"pragma"};
  const char LC_TRAILER[] =                 {"trailer"};
  const char LC_TRANSFER_ENCODING[] =       {"transfer-encoding"};
  const char LC_UPGRADE[] =                 {"upgrade"};
  const char LC_VIA[] =                     {"via"};
  const char LC_WARNING[] =                 {"warning"};

  // Request Header Fields, RFC2616 sec 5.3, pg 38
  const char LC_ACCEPT[] =                  {"accept"};
  const char LC_ACCEPT_CHARSET[] =          {"accept-charset"};
  const char LC_ACCEPT_ENCODING[] =         {"accept-encoding"};
  const char LC_ACCEPT_LANGUAGE[] =         {"accept-language"};
  const char LC_AUTHORIZATION[] =           {"authorization"};
  const char LC_EXPECT[] =                  {"expect"};
  const char LC_FROM[] =                    {"from"};
  const char LC_HOST[] =                    {"host"};
  const char LC_IF_MATCH[] =                {"if-match"};
  const char LC_IF_MODIFIED_SINCE[] =       {"if-modified-since"};
  const char LC_IF_NONE_MATCH[] =           {"if-none-match"};
  const char LC_IF_RANGE[] =                {"if-range"};
  const char LC_IF_UNMODIFIED_SINCE[] =     {"if-unmodified-since"};
  const char LC_MAX_FORWARDS[] =            {"max-forwards"};
  const char LC_PROXY_AUTHORIZATION[] =     {"proxy-authorization"};
  const char LC_RANGE[] =                   {"range"};
  const char LC_REFERER[] =                 {"referer"};
  const char LC_TE[] =                      {"te"};
  const char LC_USER_AGENT[] =              {"user-agent"};

  // Response Header Fields, RFC2616 sec 6.2, pg 40
  const char LC_ACCEPT_RANGES[] =           {"accept-ranges"};
  const char LC_AGE[] =                     {"age"};
  const char LC_ETAG[] =                    {"etag"};
  const char LC_LOCATION[] =                {"location"};
  const char LC_PROXY_AUTHENTICATE[] =      {"proxy-authenticate"};
  const char LC_RETRY_AFTER[] =             {"retry-after"};
  const char LC_SERVER[] =                  {"server"};
  const char LC_VARY[] =                    {"vary"};
  const char LC_WWW_AUTHENTICATE[] =        {"www-authenticate"};

  // Entity Header Fields, RFC2616 sec 7.1, pg 41
  const char LC_ALLOW[] =                   {"allow"};
  const char LC_CONTENT_ENCODING[] =        {"content-encoding"};
  const char LC_CONTENT_LANGUAGE[] =        {"content-language"};
  const char LC_CONTENT_LENGTH[] =          {"content-length"};
  const char LC_CONTENT_LOCATION[] =        {"content-location"};
  const char LC_CONTENT_MD5[] =             {"content-md5"};
  const char LC_CONTENT_RANGE[] =           {"content-range"};
  const char LC_CONTENT_TYPE[] =            {"content-type"};
  const char LC_EXPIRES[] =                 {"expires"};
  const char LC_LAST_MODIFIED[] =           {"last-modified"};
  const char LC_EXTENSION_HEADER[] =        {"extension-header"};

  /// The header field seperator, colon space.
  const char SEPARATOR[] = {": "};

  /// The value to use for chunked tranfer encoding.
  const char CHUNKED[] = {"Chunked"};
}

namespace via
{
  namespace http
  {
    namespace header_field
    {
      ////////////////////////////////////////////////////////////////////////
      const std::string_view standard_name(id field_id) noexcept
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
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      const std::string_view lowercase_name(id field_id) noexcept
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
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string to_header(std::string_view name, std::string_view value)
      { return std::string(name) + SEPARATOR + std::string(value) + CRLF; }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string date_header()
      {
        /// The value to use to format an HTTP date into RFC1123 format.
        static const char DATE_FORMAT[] = {"%a, %d %b %Y %H:%M:%S GMT"};

        char dateBuffer[30] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        time_t uTime;
        time(&uTime);
        strftime(dateBuffer, 30, DATE_FORMAT, std::gmtime(&uTime));
        return to_header(id::DATE, dateBuffer);
      }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string server_header()
      { return to_header(id::SERVER, SERVER_NAME); }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string content_http_header()
      { return to_header(id::CONTENT_TYPE, MESSAGE_HTTP); }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string content_length(size_t size)
      {
        return std::string(standard_name(id::CONTENT_LENGTH)) + SEPARATOR
             + to_dec_string(size) + CRLF;
      }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string chunked_encoding()
      {
        return std::string(HEADER_TRANSFER_ENCODING) + SEPARATOR + CHUNKED + CRLF;
      }
      ////////////////////////////////////////////////////////////////////////
    }
  }
}
