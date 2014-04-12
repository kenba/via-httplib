//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "header_field.hpp"
#include "character.hpp"

#ifdef _MSC_VER // MSVC doesn't like gmtime...
#pragma warning( disable : 4996 )
#endif

namespace
{
  /// The server name for the server header.
  const std::string SERVER_NAME  ("Via-httplib/1.1.0");

  /// The message/http content for a TRACE response
  const std::string MESSAGE_HTTP ("message/http");

  /// The following strings are for the standard header field names.
  const std::string HEADERS[] =
  {
    // General Header Fields, RFC2616 sec 4.5 pg 33
    {"Cache-Control"},
    {"Connection"},
    {"Date"},
    {"Pragma"},
    {"Trailer"},
    {"Transfer-Encoding"},
    {"Upgrade"},
    {"Via"},
    {"Warning"},

    // Request Header Fields, RFC2616 sec 5.3, pg 38
    {"Accept"},
    {"Accept-Charset"},
    {"Accept-Encoding"},
    {"Accept-Language"},
    {"Authorization"},
    {"Expect"},
    {"From"},
    {"Host"},
    {"If-Match"},
    {"If-Modified-Since"},
    {"If-None-Match"},
    {"If-Range"},
    {"If-Unmodified-Since"},
    {"Max-Forwards"},
    {"Proxy-Authorization"},
    {"Range"},
    {"Referer"},
    {"TE"},
    {"User-Agent"},

    // Response Header Fields, RFC2616 sec 6.2, pg 40
    {"Accept-Ranges"},
    {"Age"},
    {"ETag"},
    {"Location"},
    {"Proxy-Authenticate"},
    {"Retry-After"},
    {"Server"},
    {"Vary"},
    {"WWW-Authenticate"},

    // Entity Header Fields, RFC2616 sec 7.1, pg 41
    {"Allow"},
    {"Content-Encoding"},
    {"Content-Language"},
    {"Content-Length"},
    {"Content-Location"},
    {"Content-MD5"},
    {"Content-Range"},
    {"Content-Type"},
    {"Expires"},
    {"Last-Modified"},
    {"extension-header"}
  };

  // The following strings are for the lower case header field names.
  const std::string LC_HEADERS[] =
  {
    // General Header Fields, RFC2616 sec 4.5 pg 33
    {"cache-control"},
    {"connection"},
    {"date"},
    {"pragma"},
    {"trailer"},
    {"transfer-encoding"},
    {"upgrade"},
    {"via"},
    {"warning"},

    // Request Header Fields, RFC2616 sec 5.3, pg 38
    {"accept"},
    {"accept-charset"},
    {"accept-encoding"},
    {"accept-language"},
    {"authorization"},
    {"expect"},
    {"from"},
    {"host"},
    {"if-match"},
    {"if-modified-since"},
    {"if-none-match"},
    {"if-range"},
    {"if-unmodified-since"},
    {"max-forwards"},
    {"proxy-authorization"},
    {"range"},
    {"referer"},
    {"te"},
    {"user-agent"},

    // Response Header Fields, RFC2616 sec 6.2, pg 40
    {"accept-ranges"},
    {"age"},
    {"etag"},
    {"location"},
    {"proxy-authenticate"},
    {"retry-after"},
    {"server"},
    {"vary"},
    {"www-authenticate"},

    // Entity Header Fields, RFC2616 sec 7.1, pg 41
    {"allow"},
    {"content-encoding"},
    {"content-language"},
    {"content-length"},
    {"content-location"},
    {"content-md5"},
    {"content-range"},
    {"content-type"},
    {"expires"},
    {"last-modified"},
    {"extension-header"}
  };

  const std::string EMPTY_STRING{""};

  /// The header field seperator, colon space.
  const std::string SEPARATOR {": "};

  /// The value to use for chunked tranfer encoding.
  const std::string CHUNKED  {"Chunked"};

  /// The value to use to format an HTTP date into RFC1123 format.
  const std::string DATE_FORMAT = {"%a, %d %b %Y %H:%M:%S GMT"};
}

namespace via
{
  namespace http
  {
    namespace header_field
    {
      ////////////////////////////////////////////////////////////////////////
      const std::string& standard_name(id field_id)
      { return HEADERS[static_cast<int>(field_id)]; }
       ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      const std::string& lowercase_name(id field_id)
      { return LC_HEADERS[static_cast<int>(field_id)]; }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string to_header(std::string const& name,
                            std::string const& value)
      {  return name + SEPARATOR + value + CRLF; }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string to_header(id field_id, std::string const& value)
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
      { return to_header(header_field::id::CONTENT_LENGTH, to_dec_string(size)); }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      std::string chunked_encoding()
      { return to_header(header_field::id::TRANSFER_ENCODING, CHUNKED); }
      ////////////////////////////////////////////////////////////////////////
    }
  }
}
