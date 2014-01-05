//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file test_response.cpp
/// @brief Unit tests for the classes in response.hpp.
//////////////////////////////////////////////////////////////////////////////
#include "../../via/http/response.hpp"
#include <vector>
#include <iostream>

#ifdef _MSC_VER
// cpputest\MemoryLeakWarningPlugin.h: C++11 exception specification deprecated
#pragma warning (disable:4290)
#endif
#include "CppUTest/TestHarness.h"

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestResponseStatus)
{
};

TEST(TestResponseStatus, ResponseStatus1)
{
  // Informational 1xx
  STRCMP_EQUAL("Continue", 
    response_status::reason_phrase(response_status::CONTINUE).c_str());

  // Successful 2xx
  STRCMP_EQUAL("OK", 
    response_status::reason_phrase(response_status::OK).c_str());
  STRCMP_EQUAL("Created", 
    response_status::reason_phrase(response_status::CREATED).c_str());

  // Redirection 3xx
  STRCMP_EQUAL("Moved Permanently", 
    response_status::reason_phrase(response_status::MOVED_PERMANENTLY).c_str());

  // Client Error 4xx
  STRCMP_EQUAL("Bad Request", 
    response_status::reason_phrase(response_status::BAD_REQUEST).c_str());
  STRCMP_EQUAL("Unauthorized", 
    response_status::reason_phrase(response_status::UNAUTHORISED).c_str());
  STRCMP_EQUAL("Not Found", 
    response_status::reason_phrase(response_status::NOT_FOUND).c_str());
  STRCMP_EQUAL("Method Not Allowed", 
    response_status::reason_phrase(response_status::METHOD_NOT_ALLOWED).c_str());

  // Server Error 5xx
  STRCMP_EQUAL("Internal Server Error", 
    response_status::reason_phrase(response_status::INTERNAL_SERVER_ERROR).c_str());
  STRCMP_EQUAL("Service Unavailable", 
    response_status::reason_phrase(response_status::SERVICE_UNAVAILABLE).c_str());
  STRCMP_EQUAL("HTTP Version not supported", 
    response_status::reason_phrase(response_status::HTTP_VERSION_NOT_SUPPORTED).c_str());
}
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestResponseLineParser)
{
};

// An http response line in a vector of chars.
TEST(TestResponseLineParser, ValidOkVectorChar1)
{
  std::string RESPONSE_LINE("HTTP/1.0 200 OK\r\n");
  std::vector<char> response_data(RESPONSE_LINE.begin(), RESPONSE_LINE.end());
  std::vector<char>::const_iterator next(response_data.begin());

  response_line the_response;
  CHECK(the_response.parse(next, response_data.end()));
  CHECK(response_data.end() == next);
  CHECK_EQUAL(200, the_response.status());
  STRCMP_EQUAL("OK", the_response.reason_phrase().c_str());
  CHECK_EQUAL(1, the_response.major_version());
  CHECK_EQUAL(0, the_response.minor_version());
}

// An http response line in a vector of unsigned chars.
TEST(TestResponseLineParser, ValidOkVectorUnsignedChar1)
{
  std::string RESPONSE_LINE("HTTP/1.0 200 OK\r\n");
  std::vector<unsigned char> response_data(RESPONSE_LINE.begin(), RESPONSE_LINE.end());
  std::vector<unsigned char>::const_iterator next(response_data.begin());

  response_line the_response;
  CHECK(the_response.parse(next, response_data.end()));
  CHECK(response_data.end() == next);
  CHECK_EQUAL(200, the_response.status());
  STRCMP_EQUAL("OK", the_response.reason_phrase().c_str());
  CHECK_EQUAL(1, the_response.major_version());
  CHECK_EQUAL(0, the_response.minor_version());
}

// An http response line in a string.
TEST(TestResponseLineParser, ValidOk1)
{
  std::string response_data("HTTP/1.0 200 OK\r\n");
  std::string::const_iterator next(response_data.begin());

  response_line the_response;
  CHECK(the_response.parse(next, response_data.end()));
  CHECK(response_data.end() == next);
  CHECK_EQUAL(200, the_response.status());
  STRCMP_EQUAL("OK", the_response.reason_phrase().c_str());
  CHECK_EQUAL(1, the_response.major_version());
  CHECK_EQUAL(0, the_response.minor_version());
}

// An http response line in a string without an \r.
TEST(TestResponseLineParser, ValidOk2)
{
  std::string response_data("HTTP/1.0 200 OK\n");
  std::string::const_iterator next(response_data.begin());

  response_line the_response;
  CHECK(the_response.parse(next, response_data.end()));
  CHECK(response_data.end() == next);
  CHECK_EQUAL(200, the_response.status());
  STRCMP_EQUAL("OK", the_response.reason_phrase().c_str());
  CHECK_EQUAL(1, the_response.major_version());
  CHECK_EQUAL(0, the_response.minor_version());
}

// An http response line in a string without an \r but with extra whitespace
TEST(TestResponseLineParser, ValidOk3)
{
  std::string response_data("HTTP/1.0\t200\t OK\n ");
  std::string::const_iterator next(response_data.begin());

  response_line the_response;
  CHECK(the_response.parse(next, response_data.end()));
  BYTES_EQUAL(' ', *next);
  CHECK_EQUAL(200, the_response.status());
  STRCMP_EQUAL("OK", the_response.reason_phrase().c_str());
  CHECK_EQUAL(1, the_response.major_version());
  CHECK_EQUAL(0, the_response.minor_version());
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestResponseLineEncoder)
{
};

TEST(TestResponseLineEncoder, ValidOkString1)
{
  response_line the_response(response_status::OK);
  std::string response_string(the_response.to_string());
  STRCMP_EQUAL("HTTP/1.1 200 OK\r\n", response_string.c_str());
}

TEST(TestResponseLineEncoder, ValidOkString2)
{
  response_line the_response(200);
  std::string response_string(the_response.to_string());
  STRCMP_EQUAL("HTTP/1.1 200 OK\r\n", response_string.c_str());
}

TEST(TestResponseLineEncoder, ValidNonstandardString1)
{
  response_line the_response(199, "Some rubbish");
  std::string response_string(the_response.to_string());
  STRCMP_EQUAL("HTTP/1.1 199 Some rubbish\r\n", response_string.c_str());
}

TEST(TestResponseLineEncoder, ValidNonstandardString2)
{
  response_line the_response(199);
  std::string response_string(the_response.to_string());
  STRCMP_EQUAL("HTTP/1.1 199 \r\n", response_string.c_str());
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestResponseParser)
{
};

TEST(TestResponseParser, ValidOK1)
{
  std::string response_data("HTTP/1.0 200 OK\r\nContent: text\r\n\r\n");
  std::string::const_iterator next(response_data.begin());

  rx_response the_response;
  CHECK(the_response.parse(next, response_data.end()));
  CHECK(response_data.end() == next);
  CHECK_EQUAL(200, the_response.status());
  STRCMP_EQUAL("OK", the_response.reason_phrase().c_str());
  CHECK_EQUAL(1, the_response.major_version());
  CHECK_EQUAL(0, the_response.minor_version());

  STRCMP_EQUAL("text", the_response.headers().find("content").c_str());
  CHECK_EQUAL(0, the_response.content_length());
  CHECK(!the_response.is_chunked());
  STRCMP_EQUAL("text", the_response.headers().find("content").c_str());
}

TEST(TestResponseParser, ValidOK2)
{
  std::string response_data("HTTP/1.0 200 OK\r\nContent-Length: 4\r\n\r\nabcd");
  std::string::const_iterator next(response_data.begin());

  rx_response the_response;
  CHECK(the_response.parse(next, response_data.end()));
  CHECK(response_data.begin() != next);
  CHECK_EQUAL(200, the_response.status());
  STRCMP_EQUAL("OK", the_response.reason_phrase().c_str());
  CHECK_EQUAL(1, the_response.major_version());
  CHECK_EQUAL(0, the_response.minor_version());

  CHECK(!the_response.is_chunked());
  CHECK_EQUAL(4, the_response.content_length());
  std::string body(next, next + the_response.content_length());
  STRCMP_EQUAL("abcd", body.c_str());
}

// Memory leaks according to Qt/MinGw
#ifdef _MSC_VER
TEST(TestResponseParser, ValidOKChunked1)
{
  std::string response_data
    ("HTTP/1.0 200 OK\r\nTransfer-Encoding: Chunked\r\n\r\n4\r\n\r\n\r\n\r\n");
  std::string::const_iterator next(response_data.begin());

  rx_response the_response;
  CHECK(the_response.parse(next, response_data.end()));
  CHECK_EQUAL(200, the_response.status());
  STRCMP_EQUAL("OK", the_response.reason_phrase().c_str());
  CHECK_EQUAL(1, the_response.major_version());
  CHECK_EQUAL(0, the_response.minor_version());

  CHECK(the_response.is_chunked());
  CHECK_EQUAL(9, response_data.end() - next);

    // TODO parse chunk...
}
#endif

TEST(TestResponseParser, ValidUnauthorised1)
{
  std::string RESPONSE_LINE
    ("HTTP/1.0\t401\t Unauthorized\r\nWWW-Authenticate: Challenge\r\n\r\n");
  std::vector<char> response_data(RESPONSE_LINE.begin(), RESPONSE_LINE.end());
  std::vector<char>::const_iterator next(response_data.begin());

  rx_response the_response;
  CHECK(the_response.parse(next, response_data.end()));
  CHECK_EQUAL(401, the_response.status());
  STRCMP_EQUAL("Unauthorized", the_response.reason_phrase().c_str());
  CHECK_EQUAL(1, the_response.major_version());
  CHECK_EQUAL(0, the_response.minor_version());

  STRCMP_EQUAL("Challenge",
    the_response.headers().find(header_field::WWW_AUTHENTICATE).c_str());
  CHECK_EQUAL(0, the_response.content_length());
  CHECK(!the_response.is_chunked());
}

TEST(TestResponseParser, ValidOKMultiLine1)
{
  std::string response_data("HTTP/1.0 200 OK\r\nC");
  std::string::const_iterator next(response_data.begin());

  rx_response the_response;
  CHECK(!the_response.parse(next, response_data.end()));
  CHECK(response_data.end() == next);
  CHECK_EQUAL(200, the_response.status());
  STRCMP_EQUAL("OK", the_response.reason_phrase().c_str());
  CHECK_EQUAL(1, the_response.major_version());
  CHECK_EQUAL(0, the_response.minor_version());

  std::string response_data2("ontent-Length: 4\r\n\r\nabcd");
  next = response_data2.begin();
  CHECK(the_response.parse(next, response_data2.end()));

  CHECK(!the_response.is_chunked());
  CHECK_EQUAL(4, the_response.content_length());
  std::string body(next, next + the_response.content_length());
  STRCMP_EQUAL("abcd", body.c_str());
}
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestResponseEncode)
{
};

TEST(TestResponseEncode, ResponseEncode1)
{
  std::string correct_response ("HTTP/1.1 200 OK\r\n");
  correct_response += "Content-Length: 0\r\n\r\n";

  tx_response the_response(response_status::OK);
  std::string resp_text(the_response.message());
//  std::cout << resp_text << std::endl;
  STRCMP_EQUAL(correct_response.c_str(), resp_text.c_str());
}

TEST(TestResponseEncode, ResponseEncode2)
{
  const std::string text("123456789abcdef");
  std::string correct_response ("HTTP/1.1 200 OK\r\n");
  correct_response += via::http::header_field::server_header();
  correct_response += "Content-Length: 15\r\n\r\n";

  tx_response the_response(response_status::OK);
  the_response.add_server_header();
  std::string resp_text(the_response.message(text.size()));
//  std::cout << resp_text << std::endl;
  STRCMP_EQUAL(correct_response.c_str(), resp_text.c_str());
}

TEST(TestResponseEncode, ResponseEncode3)
{
  const std::string text("123456789abcdef");
  std::string correct_response ("HTTP/1.1 200 OK\r\n");
  correct_response += via::http::header_field::server_header();
  correct_response += "Content-Length: 15\r\n\r\n";

  tx_response the_response(response_status::OK);
  the_response.add_server_header();
  the_response.add_content_length_header(text.size());
  std::string resp_text(the_response.message());
//  std::cout << resp_text << std::endl;
  STRCMP_EQUAL(correct_response.c_str(), resp_text.c_str());
}

TEST(TestResponseEncode, ResponseEncode4)
{
  const std::string text("123456789abcdef");
  std::string correct_response ("HTTP/1.1 200 OK\r\n");
//  correct_response += via::http::header_field::server_header();
  correct_response += "Transfer-Encoding: Chunked\r\n\r\n";

  tx_response the_response(response_status::OK);
  the_response.add_header(header_field::TRANSFER_ENCODING, "Chunked");
  std::string resp_text(the_response.message());
//  std::string resp_text(resp_data.begin(), resp_data.end());
  STRCMP_EQUAL(correct_response.c_str(), resp_text.c_str());
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestResponseReceiver)
{
};

TEST(TestResponseReceiver, ValidOK1)
{
  std::string response_data("HTTP/1.0 200 OK\r\nC");
  std::string::const_iterator next(response_data.begin());

  response_receiver<std::string> the_response_receiver;
  receiver_parsing_state rx_state
      (the_response_receiver.receive(next, response_data.end()));
  bool ok (rx_state == RX_INCOMPLETE);
  CHECK(ok);

  std::string response_data2("ontent-Length: 4\r\n\r\nabcd");
  next = response_data2.begin();
  rx_state = the_response_receiver.receive(next, response_data2.end());
  bool complete (rx_state == RX_VALID);
  CHECK(complete);

  CHECK_EQUAL(200, the_response_receiver.response().status());
  STRCMP_EQUAL("OK", the_response_receiver.response().reason_phrase().c_str());
  CHECK_EQUAL(1, the_response_receiver.response().major_version());
  CHECK_EQUAL(0, the_response_receiver.response().minor_version());
  CHECK(!the_response_receiver.response().is_chunked());
  CHECK_EQUAL(4, the_response_receiver.response().content_length());
  std::string body(the_response_receiver.body());
  STRCMP_EQUAL("abcd", body.c_str());
}

TEST(TestResponseReceiver, InValidOK1)
{
  std::string response_data("P");
  std::string::const_iterator next(response_data.begin());

  response_receiver<std::string> the_response_receiver;
  receiver_parsing_state rx_state
      (the_response_receiver.receive(next, response_data.end()));
  CHECK (rx_state == RX_INVALID);
}

#ifdef _MSC_VER
TEST(TestResponseReceiver, ValidOKChunked1)
{
  std::string response_data1("H");
  std::string::const_iterator next(response_data1.begin());

  response_receiver<std::string> the_response_receiver;
  receiver_parsing_state rx_state
      (the_response_receiver.receive(next, response_data1.end()));
  CHECK(rx_state == RX_INCOMPLETE);

  std::string response_data("TTP/1.0 200 OK\r\n");
  response_data += "Content-Type: application/json\r\n";
  response_data += "Transfer-Encoding: Chunked\r\n";
  response_data += "Connection: Keep-Alive\r\n";
  response_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = response_data.begin();

  rx_state = the_response_receiver.receive(next, response_data.end());
  CHECK(rx_state == RX_VALID);
  CHECK_EQUAL(200, the_response_receiver.response().status());
  STRCMP_EQUAL("OK", the_response_receiver.response().reason_phrase().c_str());
  CHECK_EQUAL(1, the_response_receiver.response().major_version());
  CHECK_EQUAL(0, the_response_receiver.response().minor_version());
  CHECK(the_response_receiver.response().is_chunked());
  CHECK(the_response_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\n");
  next = body_data.begin();
  rx_state = the_response_receiver.receive(next, body_data.end());
  bool complete (rx_state == RX_CHUNK);
  CHECK(complete);

  std::string body_data2("24\r\n0123456789abcdefghijkl");
  next = body_data2.begin();
  rx_state = the_response_receiver.receive(next, body_data2.end());
  CHECK (rx_state == RX_INCOMPLETE);

  std::string body_data3("mnopqrstuvwxyz\r\n");
  next = body_data3.begin();
  rx_state = the_response_receiver.receive(next, body_data3.end());
  CHECK (rx_state == RX_CHUNK);
}

TEST(TestResponseReceiver, ValidOKChunked2)
{
  std::string response_data1("HTTP/1.1 200 OK\r\n");
  response_data1 += "Server: Via-httplib/0.14\r\n";
  response_data1 += "Transfer-Encoding: Chunked\r\n";

  std::string::const_iterator next(response_data1.begin());

  response_receiver<std::string> the_response_receiver;
  receiver_parsing_state rx_state
      (the_response_receiver.receive(next, response_data1.end()));
  CHECK(rx_state == RX_INCOMPLETE);

  std::string response_data("\r\n15");
  next = response_data.begin();

  rx_state = the_response_receiver.receive(next, response_data.end());
  CHECK(rx_state == RX_VALID);
  CHECK_EQUAL(200, the_response_receiver.response().status());
  STRCMP_EQUAL("OK", the_response_receiver.response().reason_phrase().c_str());
  CHECK_EQUAL(1, the_response_receiver.response().major_version());
  CHECK_EQUAL(1, the_response_receiver.response().minor_version());
  CHECK(the_response_receiver.response().is_chunked());
  CHECK(the_response_receiver.body().empty());

  std::string body_data("\r\nHTTP chunk number: 1\n\r\n");
  next = body_data.begin();
  rx_state = the_response_receiver.receive(next, body_data.end());
  bool complete (rx_state == RX_CHUNK);
  CHECK(complete);
  CHECK (!the_response_receiver.chunk().is_last());

  CHECK_EQUAL(the_response_receiver.chunk().size(),
              the_response_receiver.chunk().data().size());
    
  std::string body_data2("16\r\nHTTP chunk ");
  next = body_data2.begin();
  rx_state = the_response_receiver.receive(next, body_data2.end());
  CHECK (rx_state == RX_INCOMPLETE);

  std::string body_data3("number: 21\n\r\n");
  next = body_data3.begin();
  rx_state = the_response_receiver.receive(next, body_data3.end());
  CHECK (rx_state == RX_CHUNK);
  CHECK (!the_response_receiver.chunk().is_last());
  CHECK_EQUAL(the_response_receiver.chunk().size(),
              the_response_receiver.chunk().data().size());

  std::string body_data4("0\r\n\r\n");
  next = body_data4.begin();
  rx_state = the_response_receiver.receive(next, body_data4.end());
  CHECK (rx_state == RX_CHUNK);
  CHECK (the_response_receiver.chunk().is_last());
}
#endif
//////////////////////////////////////////////////////////////////////////////
