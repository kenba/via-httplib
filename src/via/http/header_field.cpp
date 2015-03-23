//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/header_field.hpp"
#include "via/http/character.hpp"

#ifdef _MSC_VER // MSVC doesn't like gmtime...
#pragma warning( disable : 4996 )
#endif

namespace
{
  /// The server name for the server header.
  const std::string	SERVER_NAME             ("Via-httplib");

  /// The message/http content for a TRACE response
  const std::string	MESSAGE_HTTP            ("message/http");

  // The following strings are for the standard header field names.

  // General Header Fields, RFC2616 sec 4.5 pg 33
  const std::string	HEADER_CACHE_CONTROL    ("Cache-Control");
  const std::string	HEADER_CONNECTION       ("Connection");
  const std::string	HEADER_DATE             ("Date");
  const std::string	HEADER_PRAGMA           ("Pragma");
  const std::string	HEADER_TRAILER          ("Trailer");
  const std::string	HEADER_TRANSFER_ENCODING("Transfer-Encoding");
  const std::string	HEADER_UPGRADE          ("Upgrade");
  const std::string	HEADER_VIA              ("Via");
  const std::string	HEADER_WARNING          ("Warning");

  // Request Header Fields, RFC2616 sec 5.3, pg 38
  const std::string	HEADER_ACCEPT           ("Accept");
  const std::string	HEADER_ACCEPT_CHARSET   ("Accept-Charset");
  const std::string	HEADER_ACCEPT_ENCODING  ("Accept-Encoding");
  const std::string	HEADER_ACCEPT_LANGUAGE  ("Accept-Language");
  const std::string	HEADER_AUTHORIZATION    ("Authorization");
  const std::string	HEADER_EXPECT           ("Expect");
  const std::string	HEADER_FROM             ("From");
  const std::string	HEADER_HOST             ("Host");
  const std::string	HEADER_IF_MATCH         ("If-Match");
  const std::string	HEADER_IF_MODIFIED_SINCE("If-Modified-Since");
  const std::string	HEADER_IF_NONE_MATCH    ("If-None-Match");
  const std::string	HEADER_IF_RANGE         ("If-Range");
  const std::string	HEADER_IF_UNMODIFIED_SINCE
                                            ("If-Unmodified-Since");
  const std::string	HEADER_MAX_FORWARDS     ("Max-Forwards");
  const std::string	HEADER_PROXY_AUTHORIZATION
                                            ("Proxy-Authorization");
  const std::string	HEADER_RANGE            ("Range");
  const std::string	HEADER_REFERER          ("Referer");
  const std::string	HEADER_TE               ("TE");
  const std::string	HEADER_USER_AGENT       ("User-Agent");

  // Response Header Fields, RFC2616 sec 6.2, pg 40
  const std::string	HEADER_ACCEPT_RANGES    ("Accept-Ranges");
  const std::string	HEADER_AGE              ("Age");
  const std::string	HEADER_ETAG             ("ETag");
  const std::string	HEADER_LOCATION         ("Location");
  const std::string	HEADER_PROXY_AUTHENTICATE("Proxy-Authenticate");
  const std::string	HEADER_RETRY_AFTER      ("Retry-After");
  const std::string	HEADER_SERVER           ("Server");
  const std::string	HEADER_VARY             ("Vary");
  const std::string	HEADER_WWW_AUTHENTICATE ("WWW-Authenticate");

  // Entity Header Fields, RFC2616 sec 7.1, pg 41
  const std::string	HEADER_ALLOW            ("Allow");
  const std::string	HEADER_CONTENT_ENCODING ("Content-Encoding");
  const std::string	HEADER_CONTENT_LANGUAGE ("Content-Language");
  const std::string	HEADER_CONTENT_LENGTH   ("Content-Length");
  const std::string	HEADER_CONTENT_LOCATION ("Content-Location");
  const std::string	HEADER_CONTENT_MD5      ("Content-MD5");
  const std::string	HEADER_CONTENT_RANGE    ("Content-Range");
  const std::string	HEADER_CONTENT_TYPE     ("Content-Type");
  const std::string	HEADER_EXPIRES          ("Expires");
  const std::string	HEADER_LAST_MODIFIED    ("Last-Modified");
  const std::string	HEADER_EXTENSION_HEADER ("extension-header");

  // The following strings are for the lower case header field names.

  // General Header Fields, RFC2616 sec 4.5 pg 33
  const std::string	LC_CACHE_CONTROL        ("cache-control");
  const std::string	LC_CONNECTION           ("connection");
  const std::string	LC_DATE                 ("date");
  const std::string	LC_PRAGMA               ("pragma");
  const std::string	LC_TRAILER              ("trailer");
  const std::string	LC_TRANSFER_ENCODING    ("transfer-encoding");
  const std::string	LC_UPGRADE              ("upgrade");
  const std::string	LC_VIA                  ("via");
  const std::string	LC_WARNING              ("warning");

  // Request Header Fields, RFC2616 sec 5.3, pg 38
  const std::string	LC_ACCEPT               ("accept");
  const std::string	LC_ACCEPT_CHARSET       ("accept-charset");
  const std::string	LC_ACCEPT_ENCODING      ("accept-encoding");
  const std::string	LC_ACCEPT_LANGUAGE      ("accept-language");
  const std::string	LC_AUTHORIZATION        ("authorization");
  const std::string	LC_EXPECT               ("expect");
  const std::string	LC_FROM                 ("from");
  const std::string	LC_HOST                 ("host");
  const std::string	LC_IF_MATCH             ("if-match");
  const std::string	LC_IF_MODIFIED_SINCE    ("if-modified-since");
  const std::string	LC_IF_NONE_MATCH        ("if-none-match");
  const std::string	LC_IF_RANGE             ("if-range");
  const std::string	LC_IF_UNMODIFIED_SINCE  ("if-unmodified-since");
  const std::string	LC_MAX_FORWARDS         ("max-forwards");
  const std::string	LC_PROXY_AUTHORIZATION  ("proxy-authorization");
  const std::string	LC_RANGE                ("range");
  const std::string	LC_REFERER              ("referer");
  const std::string	LC_TE                   ("te");
  const std::string	LC_USER_AGENT           ("user-agent");

  // Response Header Fields, RFC2616 sec 6.2, pg 40
  const std::string	LC_ACCEPT_RANGES        ("accept-ranges");
  const std::string	LC_AGE                  ("age");
  const std::string	LC_ETAG                 ("etag");
  const std::string	LC_LOCATION             ("location");
  const std::string	LC_PROXY_AUTHENTICATE   ("proxy-authenticate");
  const std::string	LC_RETRY_AFTER          ("retry-after");
  const std::string	LC_SERVER               ("server");
  const std::string	LC_VARY                 ("vary");
  const std::string	LC_WWW_AUTHENTICATE     ("www-authenticate");

  // Entity Header Fields, RFC2616 sec 7.1, pg 41
  const std::string	LC_ALLOW                ("allow");
  const std::string	LC_CONTENT_ENCODING     ("content-encoding");
  const std::string	LC_CONTENT_LANGUAGE     ("content-language");
  const std::string	LC_CONTENT_LENGTH       ("content-length");
  const std::string	LC_CONTENT_LOCATION     ("content-location");
  const std::string	LC_CONTENT_MD5          ("content-md5");
  const std::string	LC_CONTENT_RANGE        ("content-range");
  const std::string	LC_CONTENT_TYPE         ("content-type");
  const std::string	LC_EXPIRES              ("expires");
  const std::string	LC_LAST_MODIFIED        ("last-modified");
  const std::string	LC_EXTENSION_HEADER     ("extension-header");

  const std::string EMPTY_STRING("");

  /// The header field seperator, colon space.
  const std::string SEPARATOR (": ");

  /// The value to use for chunked tranfer encoding.
  const std::string CHUNKED  ("Chunked");

  /// The value to use to format an HTTP date into RFC1123 format.
  const std::string DATE_FORMAT = ("%a, %d %b %Y %H:%M:%S GMT");
}

namespace via
{
  namespace http
  {
    namespace header_field
    {
      ////////////////////////////////////////////////////////////////////////
      const std::string& standard_name(id::field field_id) NOEXCEPT
      {
        switch(field_id)
        {
        // General Header Fields
        case id::CACHE_CONTROL:                 return HEADER_CACHE_CONTROL;
        case id::CONNECTION:                    return HEADER_CONNECTION;
        case id::DATE:                          return HEADER_DATE;
        case id::PRAGMA:                        return HEADER_PRAGMA;
        case id::TRAILER:                       return HEADER_TRAILER;
        case id::TRANSFER_ENCODING:             return HEADER_TRANSFER_ENCODING;
        case id::UPGRADE:                       return HEADER_UPGRADE;
        case id::VIA:                           return HEADER_VIA;
        case id::WARNING:                       return HEADER_WARNING;

        // Request Header Fields
        case id::ACCEPT:                        return HEADER_ACCEPT;
        case id::ACCEPT_CHARSET:                return HEADER_ACCEPT_CHARSET;
        case id::ACCEPT_ENCODING:               return HEADER_ACCEPT_ENCODING;
        case id::ACCEPT_LANGUAGE:               return HEADER_ACCEPT_LANGUAGE;
        case id::AUTHORIZATION:                 return HEADER_AUTHORIZATION;
        case id::EXPECT:                        return HEADER_EXPECT;
        case id::FROM:                          return HEADER_FROM;
        case id::HOST:                          return HEADER_HOST;
        case id::IF_MATCH:                      return HEADER_IF_MATCH;
        case id::IF_MODIFIED_SINCE:             return HEADER_IF_MODIFIED_SINCE;
        case id::IF_NONE_MATCH:                 return HEADER_IF_NONE_MATCH;
        case id::IF_RANGE:                      return HEADER_IF_RANGE;
        case id::IF_UNMODIFIED_SINCE:           return HEADER_IF_UNMODIFIED_SINCE;
        case id::MAX_FORWARDS:                  return HEADER_MAX_FORWARDS;
        case id::PROXY_AUTHORIZATION:           return HEADER_PROXY_AUTHORIZATION;
        case id::RANGE:                         return HEADER_RANGE;
        case id::REFERER:                       return HEADER_REFERER;
        case id::TE:                            return HEADER_TE;
        case id::USER_AGENT:                    return HEADER_USER_AGENT;

        // Response Header Fields
        case id::ACCEPT_RANGES:                 return HEADER_ACCEPT_RANGES;
        case id::AGE:                           return HEADER_AGE;
        case id::ETAG:                          return HEADER_ETAG;
        case id::LOCATION:                      return HEADER_LOCATION;
        case id::PROXY_AUTHENTICATE:            return HEADER_PROXY_AUTHENTICATE;
        case id::RETRY_AFTER:                   return HEADER_RETRY_AFTER;
        case id::SERVER:                        return HEADER_SERVER;
        case id::VARY:                          return HEADER_VARY;
        case id::WWW_AUTHENTICATE:              return HEADER_WWW_AUTHENTICATE;

        // Entity Header Fields
        case id::ALLOW:                         return HEADER_ALLOW;
        case id::CONTENT_ENCODING:              return HEADER_CONTENT_ENCODING;
        case id::CONTENT_LANGUAGE:              return HEADER_CONTENT_LANGUAGE;
        case id::CONTENT_LENGTH:                return HEADER_CONTENT_LENGTH;
        case id::CONTENT_LOCATION:              return HEADER_CONTENT_LOCATION;
        case id::CONTENT_MD5:                   return HEADER_CONTENT_MD5;
        case id::CONTENT_RANGE:                 return HEADER_CONTENT_RANGE;
        case id::CONTENT_TYPE:                  return HEADER_CONTENT_TYPE;
        case id::EXPIRES:                       return HEADER_EXPIRES;
        case id::LAST_MODIFIED:                 return HEADER_LAST_MODIFIED;
        case id::EXTENSION_HEADER:              return HEADER_EXTENSION_HEADER;

        default:                                return EMPTY_STRING;
        }
      }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      const std::string& lowercase_name(id::field field_id) NOEXCEPT
      {
        switch(field_id)
        {
        // General Header Fields
        case id::CACHE_CONTROL:                 return LC_CACHE_CONTROL;
        case id::CONNECTION:                    return LC_CONNECTION;
        case id::DATE:                          return LC_DATE;
        case id::PRAGMA:                        return LC_PRAGMA;
        case id::TRAILER:                       return LC_TRAILER;
        case id::TRANSFER_ENCODING:             return LC_TRANSFER_ENCODING;
        case id::UPGRADE:                       return LC_UPGRADE;
        case id::VIA:                           return LC_VIA;
        case id::WARNING:                       return LC_WARNING;

        // Request Header Fields
        case id::ACCEPT:                        return LC_ACCEPT;
        case id::ACCEPT_CHARSET:                return LC_ACCEPT_CHARSET;
        case id::ACCEPT_ENCODING:               return LC_ACCEPT_ENCODING;
        case id::ACCEPT_LANGUAGE:               return LC_ACCEPT_LANGUAGE;
        case id::AUTHORIZATION:                 return LC_AUTHORIZATION;
        case id::EXPECT:                        return LC_EXPECT;
        case id::FROM:                          return LC_FROM;
        case id::HOST:                          return LC_HOST;
        case id::IF_MATCH:                      return LC_IF_MATCH;
        case id::IF_MODIFIED_SINCE:             return LC_IF_MODIFIED_SINCE;
        case id::IF_NONE_MATCH:                 return LC_IF_NONE_MATCH;
        case id::IF_RANGE:                      return LC_IF_RANGE;
        case id::IF_UNMODIFIED_SINCE:           return LC_IF_UNMODIFIED_SINCE;
        case id::MAX_FORWARDS:                  return LC_MAX_FORWARDS;
        case id::PROXY_AUTHORIZATION:           return LC_PROXY_AUTHORIZATION;
        case id::RANGE:                         return LC_RANGE;
        case id::REFERER:                       return LC_REFERER;
        case id::TE:                            return LC_TE;
        case id::USER_AGENT:                    return LC_USER_AGENT;

        // Response Header Fields
        case id::ACCEPT_RANGES:                 return LC_ACCEPT_RANGES;
        case id::AGE:                           return LC_AGE;
        case id::ETAG:                          return LC_ETAG;
        case id::LOCATION:                      return LC_LOCATION;
        case id::PROXY_AUTHENTICATE:            return LC_PROXY_AUTHENTICATE;
        case id::RETRY_AFTER:                   return LC_RETRY_AFTER;
        case id::SERVER:                        return LC_SERVER;
        case id::VARY:                          return LC_VARY;
        case id::WWW_AUTHENTICATE:              return LC_WWW_AUTHENTICATE;

        // Entity Header Fields
        case id::ALLOW:                         return LC_ALLOW;
        case id::CONTENT_ENCODING:              return LC_CONTENT_ENCODING;
        case id::CONTENT_LANGUAGE:              return LC_CONTENT_LANGUAGE;
        case id::CONTENT_LENGTH:                return LC_CONTENT_LENGTH;
        case id::CONTENT_LOCATION:              return LC_CONTENT_LOCATION;
        case id::CONTENT_MD5:                   return LC_CONTENT_MD5;
        case id::CONTENT_RANGE:                 return LC_CONTENT_RANGE;
        case id::CONTENT_TYPE:                  return LC_CONTENT_TYPE;
        case id::EXPIRES:                       return LC_EXPIRES;
        case id::LAST_MODIFIED:                 return LC_LAST_MODIFIED;
        case id::EXTENSION_HEADER:              return LC_EXTENSION_HEADER;

        default:                                return EMPTY_STRING;
        }
      }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string to_header(std::string const& name,
                            std::string const& value)
      {  return name + SEPARATOR + value + CRLF; }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string to_header(id::field field_id, std::string const& value)
      { return to_header(standard_name(field_id), value); }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string date_header()
      {
        char dateBuffer[30] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        time_t uTime;
        time(&uTime);
        strftime(dateBuffer, 30, DATE_FORMAT.c_str(), std::gmtime(&uTime));
        return to_header(header_field::id::DATE, dateBuffer);
      }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string server_header()
      { return to_header(header_field::id::SERVER, SERVER_NAME); }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string content_http_header()
      { return to_header(header_field::id::CONTENT_TYPE, MESSAGE_HTTP); }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string content_length(size_t size)
      {
        std::string output(header_field::standard_name
                                             (header_field::id::CONTENT_LENGTH));
        output += SEPARATOR + to_dec_string(size) + CRLF;
        return output;
      }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string chunked_encoding()
      {
        return (HEADER_TRANSFER_ENCODING + SEPARATOR + CHUNKED + CRLF);
      }
      ////////////////////////////////////////////////////////////////////////
    }
  }
}
