//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file test_request.cpp
/// @brief Unit tests for the classes in requests.hpp.
//////////////////////////////////////////////////////////////////////////////
#include "via/http/request.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>
#include <limits>

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestRequestMethod)

BOOST_AUTO_TEST_CASE(RequestMethod1)
{
  BOOST_CHECK_EQUAL("OPTIONS", request_method::name(request_method::id::OPTIONS));
  BOOST_CHECK_EQUAL("GET",     request_method::name(request_method::id::GET));
  BOOST_CHECK_EQUAL("HEAD",    request_method::name(request_method::id::HEAD));
  BOOST_CHECK_EQUAL("POST",    request_method::name(request_method::id::POST));
  BOOST_CHECK_EQUAL("PUT",     request_method::name(request_method::id::PUT));
  BOOST_CHECK_EQUAL("DELETE",  request_method::name(request_method::id::DELETE));
  BOOST_CHECK_EQUAL("TRACE",   request_method::name(request_method::id::TRACE));
  BOOST_CHECK_EQUAL("CONNECT", request_method::name(request_method::id::CONNECT));
  BOOST_CHECK_EQUAL("OPTIONS", request_method::name(request_method::id::OPTIONS));
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestRequestLineParser)

// An http request line in a vector of chars.
BOOST_AUTO_TEST_CASE(ValidGetVectorChar1)
{
  std::string REQUEST_LINE("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n");
  std::vector<char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<char>::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
}

// An http request line in a vector of unsigned chars.
BOOST_AUTO_TEST_CASE(ValidGetVectorUnsignedChar1)
{
  std::string REQUEST_LINE("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n");
  std::vector<unsigned char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<unsigned char>::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
}

// An http request line in a string.
BOOST_AUTO_TEST_CASE(ValidGet1)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
}

// An http request line in a string without an \r
BOOST_AUTO_TEST_CASE(ValidGet2)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\n");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, false> the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
}

// An http request line in a string without an \r but with extra whitespace
// Also test swap
BOOST_AUTO_TEST_CASE(ValidGet3)
{
  std::string request_data("GET\tabcdefghijklmnopqrstuvwxyz \t HTTP/1.0\nA");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, false> a_request;
  BOOST_CHECK(a_request.parse(next, request_data.end()));
  request_line<1024, 8, 8, false> the_request;
  the_request.swap(a_request);

  BOOST_CHECK_EQUAL('A', *next);
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
}

// An http request line with an invalid method name (not all upper case)
BOOST_AUTO_TEST_CASE(InValidMethod1)
{
  std::string request_data("GeT abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n ");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("G", the_request.method().c_str());
  BOOST_CHECK_EQUAL("", the_request.uri().c_str());
  BOOST_CHECK_EQUAL(0, the_request.major_version());
  BOOST_CHECK_EQUAL(0, the_request.minor_version());
}

// An http request line with an invalid method name (not all upper case)
BOOST_AUTO_TEST_CASE(InValidMethod2)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n ");
  std::string::iterator next(request_data.begin());

  // set max_method_length to 2
  request_line<1024, 2, 1, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

// An http request line with an invalid uri (contains whitespace)
BOOST_AUTO_TEST_CASE(InValidUri1)
{
  std::string request_data("GET abcdefghijklm\tnopqrstuvwxyz HTTP/1.0\r\n ");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklm", the_request.uri().c_str());
  BOOST_CHECK_EQUAL(0, the_request.major_version());
  BOOST_CHECK_EQUAL(0, the_request.minor_version());
}

// An http request line with an invalid uri (contains end of line)
BOOST_AUTO_TEST_CASE(InValidUri2)
{
  std::string request_data("GET abcdefghijklm\nopqrstuvwxyz HTTP/1.0\r\n ");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklm", the_request.uri().c_str());
  BOOST_CHECK_EQUAL(0, the_request.major_version());
  BOOST_CHECK_EQUAL(0, the_request.minor_version());
}

// An http request line with an invalid uri (whitespace before too long)
BOOST_AUTO_TEST_CASE(InValidUri3)
{
  std::string request_data("GET          abcdefghi HTTP/1.0\r\n ");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
}

// An http request line with an invalid uri (uri too long)
BOOST_AUTO_TEST_CASE(InValidUri4)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n ");
  std::string::iterator next(request_data.begin());

  request_line<24, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());

//  BOOST_CHECK(via::http::request_line::Request::ERROR_URI_LENGTH ==
//              the_request.state());
}

// An http request line with an invalid uri (whitespace after too long)
BOOST_AUTO_TEST_CASE(InValidUri5)
{
  std::string request_data("GET abcdefghi              HTTP/1.0\r\n ");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
}

// An incomplete http request line in a string.
BOOST_AUTO_TEST_CASE(ValidGet4)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HT");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  BOOST_CHECK(!the_request.valid());

  std::string request_data2("TP/2.0\r\n");
  next = request_data2.begin();
  BOOST_CHECK(the_request.parse(next, request_data2.end()));
  BOOST_CHECK(request_data2.end() == next);
  BOOST_CHECK_EQUAL('2', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
}

BOOST_AUTO_TEST_CASE(InValidGetLength1)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line<25, 8, 1, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP1)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HXTP/1.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP2)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTXP/1.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP3)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTX/1.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP4)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTPX1.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP5)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/X.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP6)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1x0\r\n");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP7)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.Z\r\n");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP8)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0Z\r\n");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP9)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\r");
  std::string::iterator next(request_data.begin());

  request_line<1024, 8, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestRequestLineEncoder)

BOOST_AUTO_TEST_CASE(ValidGetString1)
{
  request_line<1024, 8, 8, true> the_request("GET", "/hello/world");
  std::string request_string(the_request.to_string());
  BOOST_CHECK_EQUAL("GET /hello/world HTTP/1.1\r\n", request_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidGetId1)
{
  request_line<1024, 8, 8, true> the_request(request_method::id::GET, "/hello/world");
  std::string request_string(the_request.to_string());
  BOOST_CHECK_EQUAL("GET /hello/world HTTP/1.1\r\n", request_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidPostId1)
{
  request_line<1024, 8, 8, true> the_request(request_method::id::POST, "/hello/world", '2', '0');
  std::string request_string(the_request.to_string());
  BOOST_CHECK_EQUAL("POST /hello/world HTTP/2.0\r\n", request_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidGetId2)
{
  request_line<1024, 8, 8, true> the_request(request_method::id::POST, "/hello", '2', '0');
  the_request.set_method("GET");
  the_request.set_uri("/hello/world");
  the_request.set_major_version('1');
  the_request.set_minor_version('1');
  std::string request_string(the_request.to_string());
  BOOST_CHECK_EQUAL("GET /hello/world HTTP/1.1\r\n", request_string.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestRequestParser)

BOOST_AUTO_TEST_CASE(ValidGetVectorChar1)
{
  std::string REQUEST_LINE("GET abcde HTTP/1.0\r\nContent: text\r\n\r\n");
  std::vector<char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<char>::iterator next(request_data.begin());

  rx_request the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcde", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());

  BOOST_CHECK_EQUAL("text", the_request.headers().find("content").data());
  BOOST_CHECK_EQUAL(0, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
}

BOOST_AUTO_TEST_CASE(ValidGetVectorUnsignedChar1)
{
  std::string REQUEST_LINE("GET abcde HTTP/1.0\r\nContent: text\r\n\r\n");
  std::vector<unsigned char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<unsigned char>::iterator next(request_data.begin());

  rx_request<1024, 8, 100, 8190, 1024, 8, true> the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcde", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());

  BOOST_CHECK_EQUAL("text", the_request.headers().find("content").data());
  BOOST_CHECK_EQUAL(0, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
  BOOST_CHECK(!the_request.missing_host_header());
  BOOST_CHECK(!the_request.keep_alive());
}

BOOST_AUTO_TEST_CASE(ValidGet1)
{
  std::string request_data("GET abcde HTTP/1.1\r\nContent: text\r\n\r\n");
  std::string::iterator next(request_data.begin());

  rx_request<1024, 8, 100, 8190, 1024, 8, true> a_request;
  BOOST_CHECK(a_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);

  rx_request<1024, 8, 100, 8190, 1024, 8, true> the_request;
  the_request.swap(a_request);

  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcde", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('1', the_request.minor_version());

  BOOST_CHECK_EQUAL("text", the_request.headers().find("content").data());
  BOOST_CHECK_EQUAL(0, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
  BOOST_CHECK(!the_request.is_trace());
  BOOST_CHECK(the_request.keep_alive());
}

BOOST_AUTO_TEST_CASE(ValidPost1)
{
  std::string request_data("POST abcde HTTP/1.0\r\nContent-Length: 4\r\n\r\nabcd");
  std::string::iterator next(request_data.begin());

  rx_request<1024, 8, 100, 8190, 1024, 8, true> the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcde", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());

  BOOST_CHECK_EQUAL(4, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
}

// Memory leaks in call to is_chunked according to Qt/MinGw
BOOST_AUTO_TEST_CASE(ValidChunked1)
{
  std::string request_data
    ("POST abc HTTP/1.1\r\nTransfer-Encoding: Chunked\r\n\r\n4\r\n\r\n\r\n\r\n");
  std::string::iterator next(request_data.begin());

  rx_request<1024, 8, 100, 8190, 1024, 8, true> the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abc", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('1', the_request.minor_version());

  BOOST_CHECK_EQUAL(0, the_request.content_length());
  BOOST_CHECK(the_request.is_chunked());
  BOOST_CHECK_EQUAL(9, request_data.end() - next);

  // TODO parse chunk...
}

// Memory leaks according to Qt/MinGw
BOOST_AUTO_TEST_CASE(ValidChunked2)
{
  std::string REQUEST_LINE("POST abc HTTP/1.1\r\n");
  REQUEST_LINE += "Transfer-Encoding: Chunked\r\n\r\n";
  std::vector<char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<char>::iterator next(request_data.begin());

  rx_request<1024, 8, 100, 8190, 1024, 8, true> the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abc", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('1', the_request.minor_version());

  BOOST_CHECK(the_request.valid());
  BOOST_CHECK(the_request.is_chunked());
}

BOOST_AUTO_TEST_CASE(ValidPostQt1)
{
  std::string request_data
      ("POST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Content-Length: 82\r\n";
  request_data += "Connection: Keep-Alive\r\n";
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  std::string::iterator next(request_data.begin());

  rx_request<1024, 8, 100, 8190, 1024, 8, true> the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  BOOST_CHECK_EQUAL(82, the_request.content_length());
}

BOOST_AUTO_TEST_CASE(ValidPostMultiLine1)
{
  std::string request_data("POST abc");
  std::string::iterator next(request_data.begin());

  rx_request<1024, 8, 100, 8190, 1024, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);

  std::string request_data2("de HTTP/1.0\r\nContent-Length: 4\r\n\r\n");
  next = request_data2.begin();
  BOOST_CHECK(the_request.parse(next, request_data2.end()));
  BOOST_CHECK(request_data2.end() == next);

  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcde", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
  BOOST_CHECK_EQUAL(4, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
}

BOOST_AUTO_TEST_CASE(ValidPostMultiLine2)
{
  std::string request_data("POST abcde HTTP/1.0\r\nContent-Le");
  std::string::iterator next(request_data.begin());

  rx_request<1024, 8, 100, 8190, 1024, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcde", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());

  std::string request_data2("ngth: 4\r\n\r\n");
  next = request_data2.begin();
  BOOST_CHECK(the_request.parse(next, request_data2.end()));
  BOOST_CHECK(request_data2.end() == next);

  BOOST_CHECK_EQUAL(4, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
}

BOOST_AUTO_TEST_CASE(InValidPostLength1)
{
  std::string request_data
      ("POST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Content-Length: 26\r\n";
  request_data += "Connection: Keep-Alive\r\n";
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  std::string::iterator next(request_data.begin());

  // set max message header line length to 25
  rx_request<1024, 8, 100, 8190, 25, 8, true> the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestRequestEncode)

BOOST_AUTO_TEST_CASE(RequestEncode1)
{
  std::string correct_request("GET /uri HTTP/1.1\r\n");
  correct_request += "Content-Length: 0\r\n\r\n";
  tx_request the_request(request_method::id::GET, "/uri");

  std::string req_text(the_request.message());
  BOOST_CHECK_EQUAL(correct_request.c_str(), req_text.c_str());
}

BOOST_AUTO_TEST_CASE(RequestEncode2)
{
  std::string text("123456789abcdef");
  std::string correct_request("POST /uri HTTP/1.1\r\n");
  correct_request += "Content-Length: 15\r\n\r\n";
 // correct_request += text;

  tx_request the_request("POST", "/uri");
  std::string req_text(the_request.message(text.size()));
  BOOST_CHECK_EQUAL(correct_request.c_str(), req_text.c_str());
}

BOOST_AUTO_TEST_CASE(RequestEncode3)
{
  std::string text("123456789abcdef");
  std::string correct_request("POST /uri HTTP/1.1\r\n");
  correct_request += "Content-Length: 15\r\n\r\n";
 // correct_request += text;

  tx_request the_request(request_method::id::POST, "/uri");
  the_request.add_content_length_header(text.size());
  std::string req_text(the_request.message());
  BOOST_CHECK_EQUAL(correct_request.c_str(), req_text.c_str());
}

BOOST_AUTO_TEST_CASE(RequestEncode4)
{
  std::string correct_request("POST /uri HTTP/1.1\r\n");
  correct_request += "Transfer-Encoding: Chunked\r\n";
  correct_request += "Expect: 100-Continue\r\n\r\n";

  tx_request the_request(request_method::id::POST, "/uri");
  the_request.add_header(header_field::HEADER_TRANSFER_ENCODING, "Chunked");
  the_request.add_header("Expect", "100-Continue");
  std::string req_text(the_request.message());
  BOOST_CHECK_EQUAL(correct_request.c_str(), req_text.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestRequestReceiver)

BOOST_AUTO_TEST_CASE(ValidGet1)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n\r\n");
  std::string::iterator next(request_data.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data.end()));
  bool complete (rx_state == Rx::VALID);
  BOOST_CHECK(complete);

  auto const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
}

BOOST_AUTO_TEST_CASE(ValidGet2)
{
  std::string request_data1("G");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data2("ET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n\r\n");
  next = request_data2.begin();
  rx_state = the_request_receiver.receive(next, request_data2.end());
  bool complete (rx_state == Rx::VALID);
  BOOST_CHECK(complete);

  auto const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
}

BOOST_AUTO_TEST_CASE(InValidGet1)
{
  std::string request_data1("g");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  BOOST_CHECK(rx_state == Rx::INVALID);
}

BOOST_AUTO_TEST_CASE(ValidPostQt1)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Content-Length: 26\r\n";
  request_data += "Connection: Keep-Alive\r\n";
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  ok = (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string body_data("abcdefghijklmnopqrstuvwxyz");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  bool complete (rx_state == Rx::VALID);
  BOOST_CHECK(complete);

  auto const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  BOOST_CHECK_EQUAL(26, the_request.content_length());
  BOOST_CHECK_EQUAL(body_data.c_str(), the_request_receiver.body().c_str());
}

BOOST_AUTO_TEST_CASE(ValidPostChunk1)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  // Receiver concatenates chunks
  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Transfer-Encoding: Chunked\r\n";
  request_data += "Connection: Keep-Alive\r\n";
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  ok = (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  auto const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  BOOST_CHECK(the_request_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\n");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  bool complete (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(complete);

  std::string body_data2("24\r\n0123456789abcdefghijkl");
  next = body_data2.begin();
  rx_state = the_request_receiver.receive(next, body_data2.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  std::string body_data3("mnopqrstuvwxyz\r\n");
  next = body_data3.begin();
  rx_state = the_request_receiver.receive(next, body_data3.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  std::string body_data4("0\r\n\r\n");
  next = body_data4.begin();
  rx_state = the_request_receiver.receive(next, body_data4.end());
  BOOST_CHECK(rx_state == Rx::VALID);
}

BOOST_AUTO_TEST_CASE(ValidPostChunk2)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  // Receiver does NOT concatenate_chunks
  request_receiver<std::string> the_request_receiver;
  the_request_receiver.set_concatenate_chunks(false);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Transfer-Encoding: Chunked\r\n";
  request_data += "Connection: Keep-Alive\r\n";
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  ok = (rx_state == Rx::VALID);
  BOOST_CHECK(ok);

  auto const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  BOOST_CHECK(the_request_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\n");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  bool complete (rx_state == Rx::CHUNK);
  BOOST_CHECK(complete);

  std::string body_data2("24\r\n0123456789abcdefghijkl");
  next = body_data2.begin();
  rx_state = the_request_receiver.receive(next, body_data2.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  std::string body_data3("mnopqrstuvwxyz\r\n");
  next = body_data3.begin();
  rx_state = the_request_receiver.receive(next, body_data3.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  std::string body_data4("0\r\n\r\n");
  next = body_data4.begin();
  rx_state = the_request_receiver.receive(next, body_data4.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);
}

BOOST_AUTO_TEST_CASE(ValidPostChunk3)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  // Receiver does NOT concatenate_chunks
  request_receiver<std::string> the_request_receiver;
  the_request_receiver.set_concatenate_chunks(false);
  Rx rx_state
      (the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Transfer-Encoding: Chunked\r\n";
  request_data += "Connection: Keep-Alive\r\n";
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Expect: 100-Continue\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  ok = (rx_state == Rx::EXPECT_CONTINUE);
  BOOST_CHECK(ok);
  BOOST_CHECK(!the_request_receiver.is_head());

  the_request_receiver.set_continue_sent();

  std::string chunk_tx_data("abcdefghijklmnopqrstuvwxyz");
  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\n");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  auto chunk(the_request_receiver.chunk());
  BOOST_CHECK_EQUAL(chunk_tx_data, chunk.data());
}

BOOST_AUTO_TEST_CASE(InvalidPostHeader1)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Length: 4z\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  ok = (rx_state == Rx::INVALID);
  BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(InvalidPostHeader2)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n\r\n"; // Note: extra CRLF
  request_data += "Content-Length: 26\r\n";
  request_data += "Connection: Keep-Alive\r\n";
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  ok = (rx_state == Rx::INVALID);
  BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(InvalidPostHeader3)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Content-Length: 26\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n";
  request_data += "Connection: Keep-Alive\r\n\r\n"; // Note: extra CRLF
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  ok = (rx_state == Rx::VALID);
  BOOST_CHECK(ok);

  BOOST_CHECK_EQUAL(the_request_receiver.body().size(),
         static_cast<size_t>(the_request_receiver.request().content_length()));

  // std::cout << "Body size: " << the_request_receiver.body().size();

  the_request_receiver.clear();
  std::string body_data("abcdefghijklmnopqrstuvwxyz");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  // std::cout << "rx_state: " << rx_state;
  ok = (rx_state == Rx::INVALID);
  BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(InValidPostBodyLength1)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  // invalid character in Content-Length
  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Content-Length: 2z6\r\n";
  request_data += "Connection: Keep-Alive\r\n";
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  BOOST_CHECK(rx_state == Rx::INVALID);
}

BOOST_AUTO_TEST_CASE(InValidPostBodyLength2)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  // set request_receiver max_content_length to 25 to fail
  request_receiver<std::string, 25> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Content-Length: 26\r\n";
  request_data += "Connection: Keep-Alive\r\n";
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  BOOST_CHECK(rx_state == Rx::INVALID);
}

BOOST_AUTO_TEST_CASE(InValidPostChunk1)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  // Receiver does NOT concatenate_chunks
  request_receiver<std::string> the_request_receiver;
  the_request_receiver.set_concatenate_chunks(false);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Transfer-Encoding: Chunked\r\n";
  request_data += "Connection: Keep-Alive\r\n";
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  BOOST_CHECK(rx_state == Rx::VALID);

  auto const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  BOOST_CHECK(the_request_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\r");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  BOOST_CHECK(rx_state == Rx::INVALID);
}

BOOST_AUTO_TEST_CASE(InValidPostChunk2)
{
  // A POST requests with two bodies in chunked bodies all in one buffer
  tx_request client_request(request_method::id::POST, "/hello");
  client_request.add_header(header_field::HEADER_HOST, "localhost");
  client_request.add_header(header_field::HEADER_TRANSFER_ENCODING, "Chunked");
  std::string request_data1(client_request.message());

  std::string  chunk_body1("abcdefghijklmnopqrstuvwxyz0123456789");
  chunk_header chunk_header1(chunk_body1.size());
  std::string  http_chunk_1(chunk_header1.to_string());
  chunk_body1 += CRLF;

  std::string chunk_body2("9876543210abcdefghijklmnopqrstuvwxyz");
  chunk_header chunk_header2(chunk_body2.size());
  std::string  http_chunk_2(chunk_header2.to_string());
  chunk_body2 += CRLF;

  std::string chunk_ext("chunk extension");
  std::string chunk_trailer("chunk: trailer");
  last_chunk  last_header(chunk_ext, chunk_trailer);
  std::string http_chunk_3(last_header.to_string());
  http_chunk_3 += CRLF;

  std::string request_buffer(request_data1 +
                             http_chunk_1 + chunk_body1 +
                             http_chunk_2 + chunk_body2 +
                             http_chunk_3 +
                             request_data1);
  std::string::iterator iter(request_buffer.begin());

  request_receiver<std::string, 40> the_request_receiver;
  the_request_receiver.set_concatenate_chunks(true);
  Rx rx_state(the_request_receiver.receive(iter, request_buffer.end()));
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  rx_state = the_request_receiver.receive(iter, request_buffer.end());
  BOOST_CHECK(iter != request_buffer.end());
  BOOST_CHECK(rx_state == Rx::INVALID);
  BOOST_CHECK(the_request_receiver.response_code() ==
              via::http::response_status::code::PAYLOAD_TOO_LARGE);
}

BOOST_AUTO_TEST_CASE(ValidHeadRequest1)
{
  std::string request_data("HEAD /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Host: 172.16.0.126:3456\r\n";
  request_data += "Content-Length: 0\r\n\r\n";
  std::string::iterator next(request_data.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data.end()));
  BOOST_CHECK(rx_state == Rx::VALID);

  BOOST_CHECK_EQUAL("GET", the_request_receiver.request().method());
  BOOST_CHECK(the_request_receiver.is_head());
}

BOOST_AUTO_TEST_CASE(ValidHeadRequest2)
{
  std::string request_data("HEAD /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Host: 172.16.0.126:3456\r\n";
  request_data += "Content-Length: 0\r\n\r\n";
  std::string::iterator next(request_data.begin());

  request_receiver<std::string> the_request_receiver;
  the_request_receiver.set_translate_head(false);
  Rx rx_state(the_request_receiver.receive(next, request_data.end()));
  BOOST_CHECK(rx_state == Rx::VALID);

  BOOST_CHECK_EQUAL("HEAD", the_request_receiver.request().method());
  BOOST_CHECK(the_request_receiver.is_head());
}

BOOST_AUTO_TEST_CASE(InValidUriLength1)
{
  std::string request_data("POST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Host: 172.16.0.126:3456\r\n";
  request_data += "Content-Length: 0\r\n\r\n";
  std::string::iterator next(request_data.begin());

  request_receiver<std::string, 1024, 1024, 16> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data.end()));
  BOOST_CHECK(rx_state == Rx::INVALID);

  BOOST_CHECK(the_request_receiver.response_code() ==
              via::http::response_status::code::REQUEST_URI_TOO_LONG);
}

BOOST_AUTO_TEST_CASE(InValidContentLength1)
{
  std::string request_data("POST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  request_data += "Body without a Content-Length header";
  std::string::iterator next(request_data.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data.end()));
  BOOST_CHECK(rx_state == Rx::INVALID);

  BOOST_CHECK(the_request_receiver.response_code() ==
              via::http::response_status::code::LENGTH_REQUIRED);
}

BOOST_AUTO_TEST_CASE(ValidTrace1)
{
  std::string request_data("TRACE / HTTP/1.1\r\n");
  request_data += "Host: 172.16.0.126:3456\r\n";
  request_data += "Content-Length: 0\r\n\r\n";
  std::string::iterator next(request_data.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data.end()));
  BOOST_CHECK(rx_state == Rx::VALID);

  BOOST_CHECK(the_request_receiver.response_code() ==
              via::http::response_status::code::METHOD_NOT_ALLOWED);
}

BOOST_AUTO_TEST_CASE(ValidTrace2)
{
  std::string request_data("TRACE / HTTP/1.1\r\n");
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  std::string::iterator next(request_data.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data.end()));
  BOOST_CHECK(rx_state == Rx::VALID);

  BOOST_CHECK(the_request_receiver.response_code() ==
              via::http::response_status::code::METHOD_NOT_ALLOWED);
}

BOOST_AUTO_TEST_CASE(InValidTrace1)
{
  std::string request_data("TRACE / HTTP/1.1\r\n");
  request_data += "Host: 172.16.0.126:3456\r\n";
  request_data += "Content-Length: 1\r\n\r\n";
  std::string::iterator next(request_data.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(next, request_data.end()));
  BOOST_CHECK(rx_state == Rx::INVALID);

  BOOST_CHECK(the_request_receiver.response_code() ==
              via::http::response_status::code::BAD_REQUEST);
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestRequestLoopback)

BOOST_AUTO_TEST_CASE(LoopbackGet1)
{
  tx_request client_request(request_method::GET, "/hello");
  client_request.add_header(header_field::HEADER_HOST, "localhost");
  std::string request_data1(client_request.message());
  std::string::iterator iter(request_data1.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(iter, request_data1.end()));
  BOOST_CHECK(iter == request_data1.end());

  bool ok (rx_state == Rx::VALID);
  BOOST_CHECK(ok);

  auto const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/hello", the_request.uri().c_str());
  BOOST_CHECK_EQUAL(0, the_request.content_length());
}

BOOST_AUTO_TEST_CASE(LoopbackPut1)
{
  // Two PUT requests with bodies all in separate buffers
  std::string request_body1("abcdefghijklmnopqrstuvwxyz0123456789");

  tx_request client_request(request_method::PUT, "/hello");
  client_request.add_header(header_field::HEADER_HOST, "localhost");
  std::string request_data1(client_request.message(request_body1.size()));
  std::string::iterator iter(request_data1.begin());

  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(iter, request_data1.end()));
  BOOST_CHECK(iter == request_data1.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  iter = request_body1.begin();
  rx_state = the_request_receiver.receive(iter, request_body1.end());
  BOOST_CHECK(iter == request_body1.end());
  BOOST_CHECK(rx_state == Rx::VALID);

  // The second request
  std::string request_body2("9876543210abcdefghijklmnopqrstuvwxyz0123456789");

  client_request.clear();
  client_request = tx_request(request_method::PUT, "/goodbye");
  client_request.add_header(header_field::HEADER_HOST, "localhost");
  std::string request_data2(client_request.message(request_body2.size()));
  iter = request_data2.begin();

  // reset the receiver
  the_request_receiver.clear();
  rx_state = the_request_receiver.receive(iter, request_data2.end());
  BOOST_CHECK(iter == request_data2.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  iter = request_body2.begin();
  rx_state = the_request_receiver.receive(iter, request_body2.end());
  BOOST_CHECK(iter == request_body2.end());
  BOOST_CHECK(rx_state == Rx::VALID);
}

BOOST_AUTO_TEST_CASE(LoopbackPut2)
{
  // Two PUT requests with bodies all in one buffer
  std::string request_body1("abcdefghijklmnopqrstuvwxyz0123456789");

  tx_request client_request(request_method::POST, "/hello");
  client_request.add_header(header_field::HEADER_HOST, "localhost");
  std::string request_data1(client_request.message(request_body1.size()));

  // The second request
  std::string request_body2("9876543210abcdefghijklmnopqrstuvwxyz0123456789");

  client_request.clear();
  client_request = tx_request(request_method::PUT, "/goodbye");
  client_request.add_header(header_field::HEADER_HOST, "localhost");
  std::string request_data2(client_request.message(request_body2.size()));

  std::string request_buffer(request_data1 + request_body1 +
                             request_data2 + request_body2);

  std::string::iterator iter(request_buffer.begin());
  request_receiver<std::string> the_request_receiver;
  Rx rx_state(the_request_receiver.receive(iter, request_buffer.end()));
  BOOST_CHECK(iter != request_buffer.end());
  BOOST_CHECK(rx_state == Rx::VALID);

  // reset the receiver
  the_request_receiver.clear();
  rx_state = the_request_receiver.receive(iter, request_buffer.end());
  BOOST_CHECK(iter == request_buffer.end());
  BOOST_CHECK(rx_state == Rx::VALID);
}

BOOST_AUTO_TEST_CASE(LoopbackPost1)
{
  // A POST requests with two bodies in chunked buffers.
  tx_request client_request(request_method::POST, "/hello");
  client_request.add_header(header_field::HEADER_HOST, "localhost");
  client_request.add_header(header_field::HEADER_TRANSFER_ENCODING, "Chunked");
  std::string request_data1(client_request.message());
  std::string::iterator iter(request_data1.begin());

//  std::cout << "request_data1: " << request_data1 << std::endl;

  request_receiver<std::string> the_request_receiver;
  the_request_receiver.set_concatenate_chunks(false);
  Rx rx_state(the_request_receiver.receive(iter, request_data1.end()));
  BOOST_CHECK(iter == request_data1.end());
  BOOST_CHECK(rx_state == Rx::VALID);

  std::string  chunk_body1("abcdefghijklmnopqrstuvwxyz0123456789");
  chunk_header chunk_header1(chunk_body1.size());
  std::string  http_chunk_1(chunk_header1.to_string());
  chunk_body1 += CRLF;

  iter = http_chunk_1.begin();
  rx_state = the_request_receiver.receive(iter, http_chunk_1.end());
  BOOST_CHECK(iter == http_chunk_1.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  iter = chunk_body1.begin();
  rx_state = the_request_receiver.receive(iter, chunk_body1.end());
  BOOST_CHECK(iter == chunk_body1.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  std::string chunk_body2("9876543210abcdefghijklmnopqrstuvwxyz");
  chunk_header chunk_header2(chunk_body2.size());
  std::string  http_chunk_2(chunk_header2.to_string());
  chunk_body2 += CRLF;

  iter = http_chunk_2.begin();
  rx_state = the_request_receiver.receive(iter, http_chunk_2.end());
  BOOST_CHECK(iter == http_chunk_2.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  iter = chunk_body2.begin();
  rx_state = the_request_receiver.receive(iter, chunk_body2.end());
  BOOST_CHECK(iter == chunk_body2.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  std::string chunk_ext("chunk extension");
  std::string chunk_trailer("chunk: trailer");
  last_chunk  last_header(chunk_ext, chunk_trailer);
  std::string http_chunk_3(last_header.to_string());
  http_chunk_3 += CRLF;

  iter = http_chunk_3.begin();
  rx_state = the_request_receiver.receive(iter, http_chunk_3.end());
  BOOST_CHECK(iter == http_chunk_3.end());
//  std::cout << "rx_state; " << rx_state << std::endl;
  BOOST_CHECK(rx_state == Rx::CHUNK);
}

BOOST_AUTO_TEST_CASE(LoopbackPost2)
{
  // A POST requests with two bodies in chunked bodies all in one buffer
  tx_request client_request(request_method::POST, "/hello");
  client_request.add_header(header_field::HEADER_HOST, "localhost");
  client_request.add_header(header_field::HEADER_TRANSFER_ENCODING, "Chunked");
  std::string request_data1(client_request.message());

  std::string  chunk_body1("abcdefghijklmnopqrstuvwxyz0123456789");
  chunk_header chunk_header1(chunk_body1.size());
  std::string  http_chunk_1(chunk_header1.to_string());
  chunk_body1 += CRLF;

  std::string chunk_body2("9876543210abcdefghijklmnopqrstuvwxyz");
  chunk_header chunk_header2(chunk_body2.size());
  std::string  http_chunk_2(chunk_header2.to_string());
  chunk_body2 += CRLF;

  std::string chunk_ext("chunk extension");
  std::string chunk_trailer("chunk: trailer");
  last_chunk  last_header(chunk_ext, chunk_trailer);
  std::string http_chunk_3(last_header.to_string());
  http_chunk_3 += CRLF;

  std::string request_buffer(request_data1 +
                             http_chunk_1 + chunk_body1 +
                             http_chunk_2 + chunk_body2 +
                             http_chunk_3 +
                             request_data1);

  std::string::iterator iter(request_buffer.begin());
  request_receiver<std::string> the_request_receiver;
  the_request_receiver.set_concatenate_chunks(false);
  Rx rx_state(the_request_receiver.receive(iter, request_buffer.end()));
  BOOST_CHECK(iter != request_buffer.end());
  BOOST_CHECK(rx_state == Rx::VALID);

  rx_state = the_request_receiver.receive(iter, request_buffer.end());
  BOOST_CHECK(iter != request_buffer.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  rx_state = the_request_receiver.receive(iter, request_buffer.end());
  BOOST_CHECK(iter != request_buffer.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  rx_state = the_request_receiver.receive(iter, request_buffer.end());
  BOOST_CHECK(iter != request_buffer.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  the_request_receiver.clear();
  rx_state = the_request_receiver.receive(iter, request_buffer.end());
  BOOST_CHECK(iter == request_buffer.end());
  BOOST_CHECK(rx_state == Rx::VALID);
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
