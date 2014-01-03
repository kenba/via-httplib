//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file test_request.cpp
/// @brief Unit tests for the classes in requests.hpp.
//////////////////////////////////////////////////////////////////////////////
#include "../../via/http/request.hpp"
#include <vector>
#include <iostream>

#ifdef _MSC_VER
// cpputest\MemoryLeakWarningPlugin.h: C++11 exception specification deprecated
#pragma warning (disable:4290)
#endif
#include "CppUTest/TestHarness.h"

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestRequestLineParser)
{
};

// An http request line in a vector of chars.
TEST(TestRequestLineParser, ValidGetVectorChar1)
{
  std::string REQUEST_LINE("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n");
  std::vector<char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<char>::const_iterator next(request_data.begin());

  request_line the_request;
  CHECK(the_request.parse(next, request_data.end()));
  CHECK(request_data.end() == next);
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());
}

// An http request line in a vector of unsigned chars.
TEST(TestRequestLineParser, ValidGetVectorUnsignedChar1)
{
  std::string REQUEST_LINE("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n");
  std::vector<unsigned char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<unsigned char>::const_iterator next(request_data.begin());

  request_line the_request;
  CHECK(the_request.parse(next, request_data.end()));
  CHECK(request_data.end() == next);
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());
}

// An http request line in a string.
TEST(TestRequestLineParser, ValidGet1)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n");
  std::string::const_iterator next(request_data.begin());

  request_line the_request;
  CHECK(the_request.parse(next, request_data.end()));
  CHECK(request_data.end() == next);
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());
}

// An http request line in a string without an \r
TEST(TestRequestLineParser, ValidGet2)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\n");
  std::string::const_iterator next(request_data.begin());

  request_line the_request;
  CHECK(the_request.parse(next, request_data.end()));
  CHECK(request_data.end() == next);
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());
}

// An http request line in a string without an \r but with extra whitespace
TEST(TestRequestLineParser, ValidGet3)
{
  std::string request_data("GET\tabcdefghijklmnopqrstuvwxyz \t HTTP/1.0\nA");
  std::string::const_iterator next(request_data.begin());

  request_line the_request;
  CHECK(the_request.parse(next, request_data.end()));
  BYTES_EQUAL('A', *next);
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());
}

// An http request line with an invalid method name (not all upper case)
TEST(TestRequestLineParser, InValidMethod1)
{
  std::string request_data("GeT abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n ");
  std::string::const_iterator next(request_data.begin());

  request_line the_request;
  CHECK(!the_request.parse(next, request_data.end()));
  STRCMP_EQUAL("G", the_request.method().c_str());
  STRCMP_EQUAL("", the_request.uri().c_str());
  CHECK_EQUAL(0, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());
}

// An http request line with an invalid uri (contains whitespace)
TEST(TestRequestLineParser, InValidUri1)
{
  std::string request_data("GET abcdefghijklm\tnopqrstuvwxyz HTTP/1.0\r\n ");
  std::string::const_iterator next(request_data.begin());

  request_line the_request;
  CHECK(!the_request.parse(next, request_data.end()));
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcdefghijklm", the_request.uri().c_str());
  CHECK_EQUAL(0, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());
}

// An incomplete http request line in a string.
TEST(TestRequestLineParser, ValidGet4)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HT");
  std::string::const_iterator next(request_data.begin());

  request_line the_request;
  CHECK(!the_request.parse(next, request_data.end()));
  CHECK(request_data.end() == next);
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  CHECK(!the_request.valid());

  std::string request_data2("TP/1.2\r\n");
  next = request_data2.begin();
  CHECK(the_request.parse(next, request_data2.end()));
  CHECK(request_data2.end() == next);
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(2, the_request.minor_version());
}
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestRequestLineEncoder)
{
};

TEST(TestRequestLineEncoder, ValidGetString1)
{
  request_line the_request("GET", "/hello/world");
  std::string request_string(the_request.to_string());
  STRCMP_EQUAL("GET /hello/world HTTP/1.1\r\n", request_string.c_str());
}

TEST(TestRequestLineEncoder, ValidGetId1)
{
  request_line the_request(request_method::GET, "/hello/world");
  std::string request_string(the_request.to_string());
  STRCMP_EQUAL("GET /hello/world HTTP/1.1\r\n", request_string.c_str());
}
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestRequestParser)
{
};

TEST(TestRequestParser, ValidGetVectorChar1)
{
  std::string REQUEST_LINE("GET abcde HTTP/1.0\r\nContent: text\r\n\r\n");
  std::vector<char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<char>::const_iterator next(request_data.begin());

  rx_request the_request;
  CHECK(the_request.parse(next, request_data.end()));
  CHECK(request_data.end() == next);
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcde", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());

  STRCMP_EQUAL("text", the_request.headers().find("content").c_str());
  CHECK_EQUAL(0, the_request.content_length());
  CHECK(!the_request.is_chunked());
}


TEST(TestRequestParser, ValidGetVectorUnsignedChar1)
{
  std::string REQUEST_LINE("GET abcde HTTP/1.0\r\nContent: text\r\n\r\n");
  std::vector<unsigned char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<unsigned char>::const_iterator next(request_data.begin());

  rx_request the_request;
  CHECK(the_request.parse(next, request_data.end()));
  CHECK(request_data.end() == next);
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcde", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());

  STRCMP_EQUAL("text", the_request.headers().find("content").c_str());
  CHECK_EQUAL(0, the_request.content_length());
  CHECK(!the_request.is_chunked());
}

TEST(TestRequestParser, ValidGet1)
{
  std::string request_data("GET abcde HTTP/1.0\r\nContent: text\r\n\r\n");
  std::string::const_iterator next(request_data.begin());

  rx_request the_request;
  CHECK(the_request.parse(next, request_data.end()));
  CHECK(request_data.end() == next);
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcde", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());

  STRCMP_EQUAL("text", the_request.headers().find("content").c_str());
  CHECK_EQUAL(0, the_request.content_length());
  CHECK(!the_request.is_chunked());
}

TEST(TestRequestParser, ValidPost1)
{
  std::string request_data("POST abcde HTTP/1.0\r\nContent-Length: 4\r\n\r\nabcd");
  std::string::const_iterator next(request_data.begin());

  rx_request the_request;
  CHECK(the_request.parse(next, request_data.end()));
  STRCMP_EQUAL("POST", the_request.method().c_str());
  STRCMP_EQUAL("abcde", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());

  CHECK_EQUAL(4, the_request.content_length());
  CHECK(!the_request.is_chunked());
}

// Memory leaks in call to is_chunked according to Qt/MinGw
// probably an issue with boost::regex
#ifdef _MSC_VER
TEST(TestRequestParser, ValidChunked1)
{
  std::string request_data
    ("POST abc HTTP/1.0\r\nTransfer-Encoding: Chunked\r\n\r\n4\r\n\r\n\r\n\r\n");
  std::string::const_iterator next(request_data.begin());

  rx_request the_request;
  CHECK(the_request.parse(next, request_data.end()));
  STRCMP_EQUAL("POST", the_request.method().c_str());
  STRCMP_EQUAL("abc", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());

  CHECK_EQUAL(0, the_request.content_length());
  CHECK(the_request.is_chunked());
  CHECK_EQUAL(9, request_data.end() - next);

  // TODO parse chunk...
}
#endif

// Memory leaks according to Qt/MinGw
#ifdef _MSC_VER
TEST(TestRequestParser, ValidChunked2)
{
  std::string REQUEST_LINE
    ("POST abc HTTP/1.0\r\nTransfer-Encoding: Chunked\r\n\r\n4\r\n\r\n\r\n\r\n");
  std::vector<char> request_data(REQUEST_LINE.begin(), REQUEST_LINE.end());
  std::vector<char>::const_iterator next(request_data.begin());
  rx_request the_request;

  CHECK(the_request.parse(next, request_data.end()));
  STRCMP_EQUAL("POST", the_request.method().c_str());
  STRCMP_EQUAL("abc", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());

  CHECK(the_request.is_chunked());
  CHECK_EQUAL(9, request_data.end() - next);

  // TODO parse chunk...
}
#endif

TEST(TestRequestParser, ValidPostQt1)
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
  std::string::const_iterator next(request_data.begin());

  rx_request the_request;
  CHECK(the_request.parse(next, request_data.end()));
  CHECK(request_data.end() == next);
  STRCMP_EQUAL("POST", the_request.method().c_str());
  STRCMP_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  CHECK_EQUAL(82, the_request.content_length());
}

TEST(TestRequestParser, ValidPostMultiLine1)
{
  std::string request_data("POST abc");
  std::string::const_iterator next(request_data.begin());

  rx_request the_request;
  CHECK(!the_request.parse(next, request_data.end()));
  CHECK(request_data.end() == next);

  std::string request_data2("de HTTP/1.0\r\nContent-Length: 4\r\n\r\n");
  next = request_data2.begin();
  CHECK(the_request.parse(next, request_data2.end()));
  CHECK(request_data2.end() == next);

  STRCMP_EQUAL("POST", the_request.method().c_str());
  STRCMP_EQUAL("abcde", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());
  CHECK_EQUAL(4, the_request.content_length());
  CHECK(!the_request.is_chunked());
}

TEST(TestRequestParser, ValidPostMultiLine2)
{
  std::string request_data("POST abcde HTTP/1.0\r\nContent-Le");
  std::string::const_iterator next(request_data.begin());

  rx_request the_request;
  CHECK(!the_request.parse(next, request_data.end()));
  STRCMP_EQUAL("POST", the_request.method().c_str());
  STRCMP_EQUAL("abcde", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());

  std::string request_data2("ngth: 4\r\n\r\n");
  next = request_data2.begin();
  CHECK(the_request.parse(next, request_data2.end()));
  CHECK(request_data2.end() == next);

  CHECK_EQUAL(4, the_request.content_length());
  CHECK(!the_request.is_chunked());
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestRequestEncode)
{
};

TEST(TestRequestEncode, RequestEncode1)
{
  std::string correct_request("GET /uri HTTP/1.1\r\n");
  correct_request += "Content-Length: 0\r\n\r\n";
  tx_request the_request(request_method::GET, "/uri");

  std::string req_text(the_request.message());
  STRCMP_EQUAL(correct_request.c_str(), req_text.c_str());
}

TEST(TestRequestEncode, RequestEncode2)
{
  std::string text("123456789abcdef");
  std::string correct_request("POST /uri HTTP/1.1\r\n");
  correct_request += "Content-Length: 15\r\n\r\n";
 // correct_request += text;

  tx_request the_request(request_method::POST, "/uri");
  std::string req_text(the_request.message(text.size()));
  STRCMP_EQUAL(correct_request.c_str(), req_text.c_str());
}

TEST(TestRequestEncode, RequestEncode3)
{
  std::string text("123456789abcdef");
  std::string correct_request("POST /uri HTTP/1.1\r\n");
  correct_request += "Content-Length: 15\r\n\r\n";
 // correct_request += text;

  tx_request the_request(request_method::POST, "/uri");
  the_request.add_content_length_header(text.size());
  std::string req_text(the_request.message());
  STRCMP_EQUAL(correct_request.c_str(), req_text.c_str());
}

TEST(TestRequestEncode, RequestEncode4)
{
  std::string correct_request("POST /uri HTTP/1.1\r\n");
  correct_request += "Transfer-Encoding: Chunked\r\n\r\n";

  tx_request the_request(request_method::POST, "/uri");
  the_request.add_header(header_field::TRANSFER_ENCODING, "Chunked");
  std::string req_text(the_request.message());
  STRCMP_EQUAL(correct_request.c_str(), req_text.c_str());
}
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestRequestReceiver)
{
};

TEST(TestRequestReceiver, ValidGet1)
{
  std::string request_data("GET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n\r\n");
  std::string::const_iterator next(request_data.begin());

  request_receiver<std::string, false> the_request_receiver(true);
  receiver_parsing_state rx_state
      (the_request_receiver.receive(next, request_data.end()));
  bool complete (rx_state == RX_VALID);
  CHECK(complete);

  rx_request const& the_request(the_request_receiver.request());
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());
}

TEST(TestRequestReceiver, ValidGet2)
{
  std::string request_data1("G");
  std::string::const_iterator next(request_data1.begin());

  request_receiver<std::string, false> the_request_receiver(true);
  receiver_parsing_state rx_state
      (the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
  CHECK(ok);

  std::string request_data2("ET abcdefghijklmnopqrstuvwxyz HTTP/1.0\r\n\r\n");
  next = request_data2.begin();
  rx_state = the_request_receiver.receive(next, request_data2.end());
  bool complete (rx_state == RX_VALID);
  CHECK(complete);

  rx_request const& the_request(the_request_receiver.request());
  STRCMP_EQUAL("GET", the_request.method().c_str());
  STRCMP_EQUAL("abcdefghijklmnopqrstuvwxyz", the_request.uri().c_str());
  CHECK_EQUAL(1, the_request.major_version());
  CHECK_EQUAL(0, the_request.minor_version());
}

TEST(TestRequestReceiver, InValidGet1)
{
  std::string request_data1("g");
  std::string::const_iterator next(request_data1.begin());

  request_receiver<std::string, false> the_request_receiver(true);
  receiver_parsing_state rx_state
      (the_request_receiver.receive(next, request_data1.end()));
  CHECK(rx_state == RX_INVALID);
}

TEST(TestRequestReceiver, ValidPostQt1)
{
  std::string request_data1("P");
  std::string::const_iterator next(request_data1.begin());

  request_receiver<std::string, false> the_request_receiver(true);
  receiver_parsing_state rx_state
      (the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
  CHECK(ok);

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
  CHECK(ok);

  std::string body_data("abcdefghijklmnopqrstuvwxyz");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  bool complete (rx_state == RX_VALID);
  CHECK(complete);

  rx_request const& the_request(the_request_receiver.request());
  STRCMP_EQUAL("POST", the_request.method().c_str());
  STRCMP_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  CHECK_EQUAL(26, the_request.content_length());
  STRCMP_EQUAL(body_data.c_str(), the_request_receiver.body().c_str());
}

#ifdef _MSC_VER
TEST(TestRequestReceiver, ValidPostChunk1)
{
  std::string request_data1("P");
  std::string::const_iterator next(request_data1.begin());

  // Receiver concatenates chunks
  request_receiver<std::string, false> the_request_receiver(true);
  receiver_parsing_state rx_state
      (the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
  CHECK(ok);

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
  CHECK(ok);

  rx_request const& the_request(the_request_receiver.request());
  STRCMP_EQUAL("POST", the_request.method().c_str());
  STRCMP_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  CHECK(the_request_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  bool complete (rx_state == RX_INCOMPLETE);
  CHECK(complete);

  std::string body_data2("24\r\n0123456789abcdefghijkl");
  next = body_data2.begin();
  rx_state = the_request_receiver.receive(next, body_data2.end());
  CHECK (rx_state == RX_INCOMPLETE);

  std::string body_data3("mnopqrstuvwxyz");
  next = body_data3.begin();
  rx_state = the_request_receiver.receive(next, body_data3.end());
  CHECK (rx_state == RX_INCOMPLETE);

  std::string body_data4("0\r\n");
  next = body_data4.begin();
  rx_state = the_request_receiver.receive(next, body_data4.end());
  CHECK (rx_state == RX_VALID);
}
#endif

#ifdef _MSC_VER
TEST(TestRequestReceiver, ValidPostChunk2)
{
  std::string request_data1("P");
  std::string::const_iterator next(request_data1.begin());

  // Receiver does NOT concatenate_chunks
  request_receiver<std::string, false> the_request_receiver(false);
  receiver_parsing_state rx_state
      (the_request_receiver.receive(next, request_data1.end()));
  bool ok (rx_state == RX_INCOMPLETE);
  CHECK(ok);

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
  CHECK(ok);

  rx_request const& the_request(the_request_receiver.request());
  STRCMP_EQUAL("POST", the_request.method().c_str());
  STRCMP_EQUAL("/dhcp/blocked_addresses", the_request.uri().c_str());
  CHECK(the_request_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz");
  next = body_data.begin();
  rx_state = the_request_receiver.receive(next, body_data.end());
  bool complete (rx_state == RX_CHUNK);
  CHECK(complete);

  std::string body_data2("24\r\n0123456789abcdefghijkl");
  next = body_data2.begin();
  rx_state = the_request_receiver.receive(next, body_data2.end());
  CHECK (rx_state == RX_INCOMPLETE);

  std::string body_data3("mnopqrstuvwxyz");
  next = body_data3.begin();
  rx_state = the_request_receiver.receive(next, body_data3.end());
  CHECK (rx_state == RX_CHUNK);

  std::string body_data4("0\r\n");
  next = body_data4.begin();
  rx_state = the_request_receiver.receive(next, body_data4.end());
  CHECK (rx_state == RX_CHUNK);
}
#endif

//////////////////////////////////////////////////////////////////////////////
