//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
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
BOOST_AUTO_TEST_SUITE(TestRequestLineParser)

// An http request line in a vector of chars.
BOOST_AUTO_TEST_CASE(ValidGetVectorChar1)
{
  std::string REQUEST_LINE("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n");
  std::vector<char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<char>::iterator next(request_data.begin());

  request_line the_request;
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

  request_line the_request;
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

  request_line the_request;
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

  request_line the_request;
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

  request_line a_request;
  BOOST_CHECK(a_request.parse(next, request_data.end()));
  request_line the_request;
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

  request_line the_request;
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

  size_t length(request_line::max_method_length_s);
  request_line::max_method_length_s = 2;
  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  request_line::max_method_length_s = length;
}

// An http request line with an invalid uri (contains whitespace)
BOOST_AUTO_TEST_CASE(InValidUri1)
{
  std::string request_data("GET abcdefghijklm\tnopqrstuvwxyz HTTP/1.0\r\n ");
  std::string::iterator next(request_data.begin());

  request_line the_request;
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

  request_line the_request;
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

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
}

// An http request line with an invalid uri (whitespace after too long)
BOOST_AUTO_TEST_CASE(InValidUri4)
{
  std::string request_data("GET abcdefghi              HTTP/1.0\r\n ");
  std::string::iterator next(request_data.begin());

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
}

// An incomplete http request line in a string.
BOOST_AUTO_TEST_CASE(ValidGet4)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HT");
  std::string::iterator next(request_data.begin());

  request_line the_request;
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

  // Save the previous max_uri_length, before setting it to a value to fail
  size_t max_uri_length(request_line::max_uri_length_s);
  request_line::max_uri_length_s = 25;
  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));

  request_line::max_uri_length_s = max_uri_length;
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP1)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HXTP/1.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP2)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTXP/1.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP3)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTX/1.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP4)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTPX1.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP5)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/X.0\r\n");
  std::string::iterator next(request_data.begin());

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP6)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1x0\r\n");
  std::string::iterator next(request_data.begin());

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP7)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.Z\r\n");
  std::string::iterator next(request_data.begin());

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP8)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0Z\r\n");
  std::string::iterator next(request_data.begin());

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidGetHTTP9)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\r");
  std::string::iterator next(request_data.begin());

  request_line the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestRequestLineEncoder)

BOOST_AUTO_TEST_CASE(ValidGetString1)
{
  request_line the_request("GET", "/hello/world");
  std::string request_string(the_request.to_string());
  BOOST_CHECK_EQUAL("GET /hello/world HTTP/1.1\r\n", request_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidGetId1)
{
  request_line the_request(request_method::id::GET, "/hello/world");
  std::string request_string(the_request.to_string());
  BOOST_CHECK_EQUAL("GET /hello/world HTTP/1.1\r\n", request_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidPostId1)
{
  request_line the_request(request_method::id::POST, "/hello/world", '2', '0');
  std::string request_string(the_request.to_string());
  BOOST_CHECK_EQUAL("POST /hello/world HTTP/2.0\r\n", request_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidGetId2)
{
  request_line the_request(request_method::id::POST, "/hello", '2', '0');
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

  BOOST_CHECK_EQUAL("text", the_request.headers().find("content").c_str());
  BOOST_CHECK_EQUAL(0U, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
}

BOOST_AUTO_TEST_CASE(ValidGetVectorUnsignedChar1)
{
  std::string REQUEST_LINE("GET abcde HTTP/1.0\r\nContent: text\r\n\r\n");
  std::vector<unsigned char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<unsigned char>::iterator next(request_data.begin());

  rx_request the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcde", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());

  BOOST_CHECK_EQUAL("text", the_request.headers().find("content").c_str());
  BOOST_CHECK_EQUAL(0U, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
  BOOST_CHECK(!the_request.missing_host_header());
  BOOST_CHECK(!the_request.keep_alive());
}

BOOST_AUTO_TEST_CASE(ValidGet1)
{
  std::string request_data("GET abcde HTTP/1.1\r\nContent: text\r\n\r\n");
  std::string::iterator next(request_data.begin());

  rx_request a_request;
  BOOST_CHECK(a_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);

  rx_request the_request;
  the_request.swap(a_request);

  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcde", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('1', the_request.minor_version());

  BOOST_CHECK_EQUAL("text", the_request.headers().find("content").c_str());
  BOOST_CHECK_EQUAL(0U, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
  BOOST_CHECK(!the_request.is_trace());
  BOOST_CHECK(the_request.keep_alive());
}

BOOST_AUTO_TEST_CASE(ValidPost1)
{
  std::string request_data("POST abcde HTTP/1.0\r\nContent-Length: 4\r\n\r\nabcd");
  std::string::iterator next(request_data.begin());

  rx_request the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcde", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());

  BOOST_CHECK_EQUAL(4U, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
}

// Memory leaks in call to is_chunked according to Qt/MinGw
BOOST_AUTO_TEST_CASE(ValidChunked1)
{
  std::string request_data
    ("POST abc HTTP/1.1\r\nTransfer-Encoding: Chunked\r\n\r\n4\r\n\r\n\r\n\r\n");
  std::string::iterator next(request_data.begin());

  rx_request the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abc", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('1', the_request.minor_version());

  BOOST_CHECK_EQUAL(0U, the_request.content_length());
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
  rx_request the_request;

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

  rx_request the_request;
  BOOST_CHECK(the_request.parse(next, request_data.end()));
  BOOST_CHECK(request_data.end() == next);
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  BOOST_CHECK_EQUAL(82U, the_request.content_length());
}

BOOST_AUTO_TEST_CASE(ValidPostMultiLine1)
{
  std::string request_data("POST abc");
  std::string::iterator next(request_data.begin());

  rx_request the_request;
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
  BOOST_CHECK_EQUAL(4U, the_request.content_length());
  BOOST_CHECK(!the_request.is_chunked());
}

BOOST_AUTO_TEST_CASE(ValidPostMultiLine2)
{
  std::string request_data("POST abcde HTTP/1.0\r\nContent-Le");
  std::string::iterator next(request_data.begin());

  rx_request the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcde", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());

  std::string request_data2("ngth: 4\r\n\r\n");
  next = request_data2.begin();
  BOOST_CHECK(the_request.parse(next, request_data2.end()));
  BOOST_CHECK(request_data2.end() == next);

  BOOST_CHECK_EQUAL(4U, the_request.content_length());
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

  // Save the previous max_uri_length, before setting it to a value to fail
  size_t max_headers_length(message_headers::max_length_s);
  message_headers::max_length_s = 25;
  rx_request the_request;
  BOOST_CHECK(!the_request.parse(next, request_data.end()));
  message_headers::max_length_s = max_headers_length;
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
  the_request.add_header(header_field::id::TRANSFER_ENCODING, "Chunked");
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

  request_receiver<std::string> the_request_receiver(true, false, true);
  Rx rx_state(the_request_receiver.receive(next, request_data.end()));
  bool complete (rx_state == RX_VALID);
  BOOST_CHECK(complete);

  rx_request const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
}

BOOST_AUTO_TEST_CASE(ValidGet2)
{
  std::string request_data1("G");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver(true, false, true);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data2("ET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n\r\n");
  next = request_data2.begin();
  rx_state = the_request_receiver.receive(next, request_data2.end());
  bool complete (rx_state == RX_VALID);
  BOOST_CHECK(complete);

  rx_request const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("GET", the_request.method().c_str());
  BOOST_CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  BOOST_CHECK_EQUAL('1', the_request.major_version());
  BOOST_CHECK_EQUAL('0', the_request.minor_version());
}

BOOST_AUTO_TEST_CASE(InValidGet1)
{
  std::string request_data1("g");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver(true, false, true);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  BOOST_CHECK(rx_state == RX_INVALID);
}

BOOST_AUTO_TEST_CASE(ValidPostQt1)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver(true, false, true);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
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
  ok = (rx_state == RX_INCOMPLETE);
  BOOST_CHECK(ok);

  std::string body_data("abcdefghijklmnopqrstuvwxyz");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  bool complete (rx_state == RX_VALID);
  BOOST_CHECK(complete);

  rx_request const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  BOOST_CHECK_EQUAL(26U, the_request.content_length());
  BOOST_CHECK_EQUAL(body_data.c_str(), the_request_receiver.body().c_str());
}

BOOST_AUTO_TEST_CASE(ValidPostChunk1)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  // Receiver concatenates chunks
  request_receiver<std::string> the_request_receiver(true, false, true);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
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
  ok = (rx_state == RX_INCOMPLETE);
  BOOST_CHECK(ok);

  rx_request const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  BOOST_CHECK(the_request_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\n");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  bool complete (rx_state == RX_INCOMPLETE);
  BOOST_CHECK(complete);

  std::string body_data2("24\r\n0123456789abcdefghijkl");
  next = body_data2.begin();
  rx_state = the_request_receiver.receive(next, body_data2.end());
  BOOST_CHECK(rx_state == RX_INCOMPLETE);

  std::string body_data3("mnopqrstuvwxyz\r\n");
  next = body_data3.begin();
  rx_state = the_request_receiver.receive(next, body_data3.end());
  BOOST_CHECK(rx_state == RX_INCOMPLETE);

  std::string body_data4("0\r\n\r\n");
  next = body_data4.begin();
  rx_state = the_request_receiver.receive(next, body_data4.end());
  BOOST_CHECK(rx_state == RX_VALID);
}

BOOST_AUTO_TEST_CASE(ValidPostChunk2)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  // Receiver does NOT concatenate_chunks
  request_receiver<std::string> the_request_receiver(false, false, true);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
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
  ok = (rx_state == RX_VALID);
  BOOST_CHECK(ok);

  rx_request const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  BOOST_CHECK(the_request_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\n");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  bool complete (rx_state == RX_CHUNK);
  BOOST_CHECK(complete);

  std::string body_data2("24\r\n0123456789abcdefghijkl");
  next = body_data2.begin();
  rx_state = the_request_receiver.receive(next, body_data2.end());
  BOOST_CHECK(rx_state == RX_INCOMPLETE);

  std::string body_data3("mnopqrstuvwxyz\r\n");
  next = body_data3.begin();
  rx_state = the_request_receiver.receive(next, body_data3.end());
  BOOST_CHECK(rx_state == RX_CHUNK);

  std::string body_data4("0\r\n\r\n");
  next = body_data4.begin();
  rx_state = the_request_receiver.receive(next, body_data4.end());
  BOOST_CHECK(rx_state == RX_CHUNK);
}

BOOST_AUTO_TEST_CASE(ValidPostChunk3)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  // Receiver does NOT concatenate_chunks
  request_receiver<std::string> the_request_receiver(false, false, true);
  Rx rx_state
      (the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
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
  ok = (rx_state == RX_EXPECT_CONTINUE);
  BOOST_CHECK(ok);
  BOOST_CHECK(!the_request_receiver.is_head());

  the_request_receiver.set_continue_sent();

  std::string chunk_tx_data("abcdefghijklmnopqrstuvwxyz");
  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\n");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  BOOST_CHECK(rx_state == RX_CHUNK);

  rx_chunk<std::string> chunk(the_request_receiver.chunk());
  BOOST_CHECK_EQUAL(chunk_tx_data, chunk.data());
}

BOOST_AUTO_TEST_CASE(InvalidPostHeader1)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver(true, false, true);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Length: 4z\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  ok = (rx_state == RX_INVALID);
  BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(InvalidPostHeader2)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver(true, false, true);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
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
  ok = (rx_state == RX_INVALID);
  BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(InvalidPostHeader3)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  request_receiver<std::string> the_request_receiver(true, false, false);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
  BOOST_CHECK(ok);

  std::string request_data
      ("OST /dhcp/blocked_addresses HTTP/1.1\r\n");
  request_data += "Content-Type: application/json\r\n";
  request_data += "Content-Length: 26\r\n";
  request_data += "Connection: Keep-Alive\r\n\r\n"; // Note: extra CRLF
  request_data += "Accept-Encoding: gzip";
  request_data += "Accept-Language: en-GB,*\r\n";
  request_data += "User-Agent: Mozilla/5.0\r\n";
  request_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = request_data.begin();
  rx_state = the_request_receiver.receive(next, request_data.end());
  ok = (rx_state == RX_VALID);
  BOOST_CHECK(ok);

  BOOST_CHECK(the_request_receiver.body().size() ==
              the_request_receiver.request().content_length());

  // std::cout << "Body size: " << the_request_receiver.body().size();

  the_request_receiver.clear();
  std::string body_data("abcdefghijklmnopqrstuvwxyz");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  // std::cout << "rx_state: " << rx_state;
  ok = (rx_state == RX_INVALID);
  BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(InValidPostBodyLength1)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  size_t previous_max_length(message_headers::max_content_length_s);
  message_headers::max_content_length_s = 25;
  request_receiver<std::string> the_request_receiver(true, false, true);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
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
  BOOST_CHECK(rx_state == RX_INVALID);

  message_headers::max_content_length_s = previous_max_length;
}

BOOST_AUTO_TEST_CASE(InValidPostChunk1)
{
  std::string request_data1("P");
  std::string::iterator next(request_data1.begin());

  // Receiver does NOT concatenate_chunks
  request_receiver<std::string> the_request_receiver(false, false, true);
  Rx rx_state(the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
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
  BOOST_CHECK(rx_state == RX_VALID);

  rx_request const& the_request(the_request_receiver.request());
  BOOST_CHECK_EQUAL("POST", the_request.method().c_str());
  BOOST_CHECK_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  BOOST_CHECK(the_request_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\r");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  BOOST_CHECK(rx_state == RX_INVALID);
}

BOOST_AUTO_TEST_SUITE_END()

//////////////////////////////////////////////////////////////////////////////
