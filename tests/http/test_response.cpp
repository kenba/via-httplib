//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file test_response.cpp
/// @brief Unit tests for the classes in response.hpp.
//////////////////////////////////////////////////////////////////////////////
#include "via/http/response.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseStatus)

BOOST_AUTO_TEST_CASE(ResponseStatus1)
{
  // Informational 1xx
  BOOST_CHECK_EQUAL("Continue",
    response_status::reason_phrase(response_status::code::CONTINUE).c_str());
  BOOST_CHECK_EQUAL("Switching Protocols",
    response_status::reason_phrase(response_status::code::SWITCHING_PROTOCOLS).c_str());

  // Successful 2xx
  BOOST_CHECK_EQUAL("OK",
    response_status::reason_phrase(response_status::code::OK).c_str());
  BOOST_CHECK_EQUAL("Created",
    response_status::reason_phrase(response_status::code::CREATED).c_str());
  BOOST_CHECK_EQUAL("Accepted",
    response_status::reason_phrase(response_status::code::ACCEPTED).c_str());
  BOOST_CHECK_EQUAL("Non-Authoritative Information",
    response_status::reason_phrase(response_status::code::NON_AUTHORITATIVE).c_str());
  BOOST_CHECK_EQUAL("No Content",
    response_status::reason_phrase(response_status::code::NO_CONTENT).c_str());
  BOOST_CHECK_EQUAL("Reset Content",
    response_status::reason_phrase(response_status::code::RESET_CONTENT).c_str());
  BOOST_CHECK_EQUAL("Partial Content",
    response_status::reason_phrase(response_status::code::PARTIAL_CONTENT).c_str());

  // Redirection 3xx
  BOOST_CHECK_EQUAL("Multiple Choices",
    response_status::reason_phrase(response_status::code::MULTIPLE_CHOICES).c_str());
  BOOST_CHECK_EQUAL("Moved Permanently",
    response_status::reason_phrase(response_status::code::MOVED_PERMANENTLY).c_str());
  BOOST_CHECK_EQUAL("Found",
    response_status::reason_phrase(response_status::code::FOUND).c_str());
  BOOST_CHECK_EQUAL("See Other",
    response_status::reason_phrase(response_status::code::SEE_OTHER).c_str());
  BOOST_CHECK_EQUAL("Not Modified",
    response_status::reason_phrase(response_status::code::NOT_MODIFIED).c_str());
  BOOST_CHECK_EQUAL("Use Proxy",
    response_status::reason_phrase(response_status::code::USE_PROXY).c_str());
  BOOST_CHECK_EQUAL("Temporary Redirect",
    response_status::reason_phrase(response_status::code::TEMPORARY_REDIRECT).c_str());

  // Client Error 4xx
  BOOST_CHECK_EQUAL("Bad Request",
    response_status::reason_phrase(response_status::code::BAD_REQUEST).c_str());
  BOOST_CHECK_EQUAL("Unauthorized",
    response_status::reason_phrase(response_status::code::UNAUTHORISED).c_str());
  BOOST_CHECK_EQUAL("Payment Required",
    response_status::reason_phrase(response_status::code::PAYMENT_REQUIRED).c_str());
  BOOST_CHECK_EQUAL("Forbidden",
    response_status::reason_phrase(response_status::code::FORBIDDEN).c_str());
  BOOST_CHECK_EQUAL("Not Found",
    response_status::reason_phrase(response_status::code::NOT_FOUND).c_str());
  BOOST_CHECK_EQUAL("Method Not Allowed",
    response_status::reason_phrase(response_status::code::METHOD_NOT_ALLOWED).c_str());
  BOOST_CHECK_EQUAL("Not Acceptable",
    response_status::reason_phrase(response_status::code::NOT_ACCEPTABLE).c_str());
  BOOST_CHECK_EQUAL("Proxy Authentication Required",
    response_status::reason_phrase(response_status::code::PROXY_AUTHENTICATION_REQUIRED).c_str());
  BOOST_CHECK_EQUAL("Request Time-out",
    response_status::reason_phrase(response_status::code::REQUEST_TIMEOUT).c_str());
  BOOST_CHECK_EQUAL("Conflict",
    response_status::reason_phrase(response_status::code::CONFLICT).c_str());
  BOOST_CHECK_EQUAL("Gone",
    response_status::reason_phrase(response_status::code::GONE).c_str());
  BOOST_CHECK_EQUAL("Length Required",
    response_status::reason_phrase(response_status::code::LENGTH_REQUIRED).c_str());
  BOOST_CHECK_EQUAL("Precondition Failed",
    response_status::reason_phrase(response_status::code::PRECONDITION_FAILED).c_str());
  BOOST_CHECK_EQUAL("Request Entity Too Large",
    response_status::reason_phrase(response_status::code::REQUEST_ENTITY_TOO_LARGE).c_str());
  BOOST_CHECK_EQUAL("Request-URI Too Long",
    response_status::reason_phrase(response_status::code::REQUEST_URI_TOO_LONG).c_str());
  BOOST_CHECK_EQUAL("Unsupported Media Type",
    response_status::reason_phrase(response_status::code::UNSUPPORTED_MEDIA_TYPE).c_str());
  BOOST_CHECK_EQUAL("Requested range not satisfiable",
    response_status::reason_phrase(response_status::code::REQUEST_RANGE_NOT_SATISFIABLE).c_str());
  BOOST_CHECK_EQUAL("Expectation Failed",
    response_status::reason_phrase(response_status::code::EXPECTATION_FAILED).c_str());

  // Server Error 5xx
  BOOST_CHECK_EQUAL("Internal Server Error",
    response_status::reason_phrase(response_status::code::INTERNAL_SERVER_ERROR).c_str());
  BOOST_CHECK_EQUAL("Not Implemented",
    response_status::reason_phrase(response_status::code::NOT_IMPLEMENTED).c_str());
  BOOST_CHECK_EQUAL("Bad Gateway",
    response_status::reason_phrase(response_status::code::BAD_GATEWAY).c_str());
  BOOST_CHECK_EQUAL("Service Unavailable",
    response_status::reason_phrase(response_status::code::SERVICE_UNAVAILABLE).c_str());
  BOOST_CHECK_EQUAL("Gateway Time-out",
    response_status::reason_phrase(response_status::code::GATEWAY_TIMEOUT).c_str());
  BOOST_CHECK_EQUAL("HTTP Version not supported",
    response_status::reason_phrase(response_status::code::HTTP_VERSION_NOT_SUPPORTED).c_str());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseLineParser)

// An http response line in a vector of chars.
BOOST_AUTO_TEST_CASE(ValidOkVectorChar1)
{
  std::string RESPONSE_LINE("HTTP/1.0 200 OK\r\n");
  std::vector<char> response_data(RESPONSE_LINE.begin(), RESPONSE_LINE.end());
  std::vector<char>::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());
}

// An http response line in a vector of unsigned chars.
BOOST_AUTO_TEST_CASE(ValidOkVectorUnsignedChar1)
{
  std::string RESPONSE_LINE("HTTP/1.0 200 OK\r\n");
  std::vector<unsigned char> response_data(RESPONSE_LINE.begin(), RESPONSE_LINE.end());
  std::vector<unsigned char>::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());
}

// An http response line in a string.
BOOST_AUTO_TEST_CASE(ValidOk1)
{
  std::string response_data("HTTP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());
}

// An http response line in a string without an \r.
BOOST_AUTO_TEST_CASE(ValidOk2)
{
  std::string response_data("HTTP/1.0 200 OK\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());
  BOOST_CHECK(!the_response.is_continue());
}

// An http response line in a string without an \r but with extra whitespace
BOOST_AUTO_TEST_CASE(ValidOk3)
{
  std::string response_data("HTTP/1.0\t200\t OK\n ");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK_EQUAL(' ', *next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());
  BOOST_CHECK(!the_response.is_continue());
}

BOOST_AUTO_TEST_CASE(ValidContinue1)
{
  std::string response_data("HTTP/1.1 100 Continue\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(100, the_response.status());
  BOOST_CHECK_EQUAL("Continue", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('1', the_response.minor_version());
  BOOST_CHECK(the_response.is_continue());
}

BOOST_AUTO_TEST_CASE(InValidOk0)
{
  // To much whitespace
  std::string response_data("          HTTP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  size_t length(response_line::max_ws_s);
  response_line::max_ws_s = 8;
  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
  response_line::max_ws_s = length;
}

BOOST_AUTO_TEST_CASE(InValidOk1)
{
  std::string response_data("XTTP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk2)
{
  std::string response_data("HXTP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk3)
{
  std::string response_data("HTXP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk4)
{
  std::string response_data("HTTX/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk5)
{
  std::string response_data("HTTPX1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk6)
{
  std::string response_data("HTTP/X.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk7)
{
  std::string response_data("HTTP/1x0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk8)
{
  std::string response_data("HTTP/1.X 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}


BOOST_AUTO_TEST_CASE(InValidOk9)
{
  std::string response_data("HTTP/1.01 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk10)
{
  // To much whitespace
  std::string response_data("HTTP/1.0           200 OK\r\n");
  std::string::iterator next(response_data.begin());

  size_t length(response_line::max_ws_s);
  response_line::max_ws_s = 8;
  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
  response_line::max_ws_s = length;
}

BOOST_AUTO_TEST_CASE(InValidOk11)
{
  std::string response_data("HTTP/1.0 X00 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk12)
{
  std::string response_data("HTTP/1.0 2X0 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk13)
{
  std::string response_data("HTTP/1.0 200 OK \r\r");
  std::string::iterator next(response_data.begin());

  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk14)
{
  // To much whitespace
  std::string response_data("HTTP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  int max_status(response_line::max_status_s);
  response_line::max_status_s = 199;
  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
  response_line::max_status_s = max_status;
}

BOOST_AUTO_TEST_CASE(InValidOk15)
{
  // To much whitespace
  std::string response_data("HTTP/1.0 200              OK\r\n");
  std::string::iterator next(response_data.begin());

  size_t length(response_line::max_ws_s);
  response_line::max_ws_s = 8;
  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
  response_line::max_ws_s = length;
}

BOOST_AUTO_TEST_CASE(InValidOk16)
{
  // To much whitespace
  std::string response_data("HTTP/1.0 200 BAD\r\n");
  std::string::iterator next(response_data.begin());

  size_t length(response_line::max_reason_length_s);
  response_line::max_reason_length_s = 2;
  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
  response_line::max_reason_length_s = length;
}

BOOST_AUTO_TEST_CASE(InValidOk17)
{
  // To much whitespace
  std::string response_data("HTTP/1.0 200 OK\n");
  std::string::iterator next(response_data.begin());

  response_line::strict_crlf_s = true;
  response_line the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
  response_line::strict_crlf_s = false;
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseLineEncoder)

BOOST_AUTO_TEST_CASE(ValidOkString1)
{
  response_line the_response(response_status::code::OK);
  std::string response_string(the_response.to_string());
  BOOST_CHECK_EQUAL("HTTP/1.1 200 OK\r\n", response_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidOkString2)
{
  response_line the_response(200, "OK");
  std::string response_string(the_response.to_string());
  BOOST_CHECK_EQUAL("HTTP/1.1 200 OK\r\n", response_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidNonstandardString1)
{
  response_line the_response(199, "Some rubbish");
  std::string response_string(the_response.to_string());
  BOOST_CHECK_EQUAL("HTTP/1.1 199 Some rubbish\r\n", response_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidNonstandardString2)
{
  response_line the_response(response_status::code::OK);
  the_response.set_status_and_reason(199, "");
  std::string response_string(the_response.to_string());
  BOOST_CHECK_EQUAL("HTTP/1.1 199 \r\n", response_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidNonstandardString3)
{
  response_line the_response(199, "", '0', '0');
  the_response.set_status(response_status::code::OK);
  the_response.set_major_version('1');
  the_response.set_minor_version('1');
  std::string response_string(the_response.to_string());
  BOOST_CHECK_EQUAL("HTTP/1.1 200 OK\r\n", response_string.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseParser)

BOOST_AUTO_TEST_CASE(ValidOK1)
{
  std::string response_data("HTTP/1.0 200 OK\r\nContent: text\r\n\r\n");
  std::string::iterator next(response_data.begin());

  rx_response the_response;
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());

  BOOST_CHECK_EQUAL("text", the_response.headers().find("content").c_str());
  BOOST_CHECK_EQUAL(0U, the_response.content_length());
  BOOST_CHECK(!the_response.is_continue());
  BOOST_CHECK(!the_response.is_chunked());
  BOOST_CHECK(!the_response.keep_alive());
  BOOST_CHECK_EQUAL("text", the_response.headers().find("content").c_str());
}

BOOST_AUTO_TEST_CASE(ValidOK2)
{
  std::string response_data("HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nabcd");
  std::string::iterator next(response_data.begin());

  rx_response a_response;
  BOOST_CHECK(a_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.begin() != next);

  rx_response the_response;
  the_response.swap(a_response);

  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('1', the_response.minor_version());

  BOOST_CHECK(!the_response.is_chunked());
  BOOST_CHECK(the_response.keep_alive());
  BOOST_CHECK(!the_response.is_continue());
  BOOST_CHECK_EQUAL(4U, the_response.content_length());
  std::string body(next, next + the_response.content_length());
  BOOST_CHECK_EQUAL("abcd", body.c_str());
}

// Memory leaks according to Qt/MinGw
BOOST_AUTO_TEST_CASE(ValidOKChunked1)
{
  std::string response_data
    ("HTTP/1.0 200 OK\r\nTransfer-Encoding: Chunked\r\n\r\n4\r\n\r\n\r\n\r\n");
  std::string::iterator next(response_data.begin());

  rx_response the_response;
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());

  BOOST_CHECK(the_response.is_chunked());
  BOOST_CHECK_EQUAL(9, response_data.end() - next);

    // TODO parse chunk...
}

BOOST_AUTO_TEST_CASE(ValidContinue1)
{
  std::string response_data("HTTP/1.1 100 Continue\r\n\r\n");
  std::string::iterator next(response_data.begin());

  rx_response the_response;
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(100, the_response.status());
  BOOST_CHECK_EQUAL("Continue", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('1', the_response.minor_version());
  BOOST_CHECK(the_response.is_continue());
}

BOOST_AUTO_TEST_CASE(ValidUnauthorised1)
{
  std::string RESPONSE_LINE
    ("HTTP/1.0\t401\t Unauthorized\r\nWWW-Authenticate: Challenge\r\n\r\n");
  std::vector<char> response_data(RESPONSE_LINE.begin(), RESPONSE_LINE.end());
  std::vector<char>::iterator next(response_data.begin());

  rx_response the_response;
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK_EQUAL(401, the_response.status());
  BOOST_CHECK_EQUAL("Unauthorized", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());

  BOOST_CHECK_EQUAL("Challenge",
    the_response.headers().find(header_field::id::WWW_AUTHENTICATE).c_str());
  BOOST_CHECK_EQUAL(0U, the_response.content_length());
  BOOST_CHECK(!the_response.is_chunked());
}

BOOST_AUTO_TEST_CASE(ValidOKMultiLine1)
{
  std::string response_data("HTTP/1.0 200 OK\r\nC");
  std::string::iterator next(response_data.begin());

  rx_response the_response;
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());

  std::string response_data2("ontent-Length: 4\r\n\r\nabcd");
  next = response_data2.begin();
  BOOST_CHECK(the_response.parse(next, response_data2.end()));

  BOOST_CHECK(!the_response.is_chunked());
  BOOST_CHECK_EQUAL(4U, the_response.content_length());
  std::string body(next, next + the_response.content_length());
  BOOST_CHECK_EQUAL("abcd", body.c_str());
}
BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseEncode)

BOOST_AUTO_TEST_CASE(ResponseEncode1)
{
  std::string correct_response ("HTTP/1.1 200 OK\r\n");
  correct_response += "Content-Length: 0\r\n\r\n";

  tx_response the_response(response_status::code::OK);
  std::string resp_text(the_response.message());
//  std::cout << resp_text << std::endl;
  BOOST_CHECK_EQUAL(correct_response.c_str(), resp_text.c_str());
}

BOOST_AUTO_TEST_CASE(ResponseEncode2)
{
  const std::string text("123456789abcdef");
  std::string correct_response ("HTTP/1.1 200 OK\r\n");
  correct_response += via::http::header_field::server_header();
  correct_response += "Content-Length: 15\r\n\r\n";

  tx_response the_response(response_status::code::OK);
  the_response.add_server_header();
  std::string resp_text(the_response.message(text.size()));
//  std::cout << resp_text << std::endl;
  BOOST_CHECK_EQUAL(correct_response.c_str(), resp_text.c_str());
}

BOOST_AUTO_TEST_CASE(ResponseEncode3)
{
  const std::string text("123456789abcdef");
  std::string correct_response ("HTTP/1.1 200 OK\r\n");
  correct_response += via::http::header_field::server_header();
  correct_response += "Content-Length: 15\r\n\r\n";

  tx_response the_response(response_status::code::OK);
  the_response.add_server_header();
  the_response.add_content_length_header(text.size());
  std::string resp_text(the_response.message());
//  std::cout << resp_text << std::endl;
  BOOST_CHECK_EQUAL(correct_response.c_str(), resp_text.c_str());
}

BOOST_AUTO_TEST_CASE(ResponseEncode4)
{
  const std::string text("123456789abcdef");
  std::string correct_response ("HTTP/1.1 200 OK\r\n");
//  correct_response += via::http::header_field::server_header();
  correct_response += "Transfer-Encoding: Chunked\r\n\r\n";

  tx_response the_response(response_status::code::OK);
  the_response.add_header(header_field::id::TRANSFER_ENCODING, "Chunked");
  std::string resp_text(the_response.message());
//  std::string resp_text(resp_data.begin(), resp_data.end());
  BOOST_CHECK_EQUAL(correct_response.c_str(), resp_text.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseReceiver)

BOOST_AUTO_TEST_CASE(ValidOK1)
{
  std::string response_data("HTTP/1.0 200 OK\r\nC");
  std::string::iterator next(response_data.begin());

  response_receiver<std::string> the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data.end()));
  bool ok (rx_state == RX_INCOMPLETE);
  BOOST_CHECK(ok);

  std::string response_data2("ontent-Length: 4\r\n\r\nabcd");
  next = response_data2.begin();
  rx_state = the_response_receiver.receive(next, response_data2.end());
  bool complete (rx_state == RX_VALID);
  BOOST_CHECK(complete);

  BOOST_CHECK_EQUAL(200, the_response_receiver.response().status());
  BOOST_CHECK_EQUAL("OK", the_response_receiver.response().reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response_receiver.response().major_version());
  BOOST_CHECK_EQUAL('0', the_response_receiver.response().minor_version());
  BOOST_CHECK(!the_response_receiver.response().is_chunked());
  BOOST_CHECK_EQUAL(4U, the_response_receiver.response().content_length());
  std::string body(the_response_receiver.body());
  BOOST_CHECK_EQUAL("abcd", body.c_str());
}

BOOST_AUTO_TEST_CASE(ValidOK2)
{
  std::string response_data("HTTP/1.0 200 OK\r\n");
  response_data += "Server: Via-httplib/0.14\r\n";
  response_data += "Content-Length: 4\r\n";
  response_data += "\r\nabcd\r\n"; // extra chars at end of body
  std::string::iterator next(response_data.begin());

  response_receiver<std::string> the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data.end()));
  bool ok (rx_state == RX_VALID);
  BOOST_CHECK(ok);

  std::string body(the_response_receiver.body());
  BOOST_CHECK_EQUAL("abcd", body.c_str());
}

BOOST_AUTO_TEST_CASE(InValidOK1)
{
  std::string response_data("P");
  std::string::iterator next(response_data.begin());

  response_receiver<std::string> the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data.end()));
  BOOST_CHECK(rx_state == RX_INVALID);
}

BOOST_AUTO_TEST_CASE(ValidOKChunked1)
{
  std::string response_data1("H");
  std::string::iterator next(response_data1.begin());

  response_receiver<std::string> the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data1.end()));
  BOOST_CHECK(rx_state == RX_INCOMPLETE);

  std::string response_data("TTP/1.0 200 OK\r\n");
  response_data += "Content-Type: application/json\r\n";
  response_data += "Transfer-Encoding: Chunked\r\n";
  response_data += "Connection: Keep-Alive\r\n";
  response_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = response_data.begin();

  rx_state = the_response_receiver.receive(next, response_data.end());
  BOOST_CHECK(rx_state == RX_VALID);
  BOOST_CHECK_EQUAL(200, the_response_receiver.response().status());
  BOOST_CHECK_EQUAL("OK", the_response_receiver.response().reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response_receiver.response().major_version());
  BOOST_CHECK_EQUAL('0', the_response_receiver.response().minor_version());
  BOOST_CHECK(the_response_receiver.response().is_chunked());
  BOOST_CHECK(the_response_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\n");
  next = body_data.begin();
  rx_state = the_response_receiver.receive(next, body_data.end());
  bool complete (rx_state == RX_CHUNK);
  BOOST_CHECK(complete);

  std::string body_data2("24\r\n0123456789abcdefghijkl");
  next = body_data2.begin();
  rx_state = the_response_receiver.receive(next, body_data2.end());
  BOOST_CHECK(rx_state == RX_INCOMPLETE);

  std::string body_data3("mnopqrstuvwxyz\r\n");
  next = body_data3.begin();
  rx_state = the_response_receiver.receive(next, body_data3.end());
  BOOST_CHECK(rx_state == RX_CHUNK);
}

BOOST_AUTO_TEST_CASE(ValidOKChunked2)
{
  std::string response_data1("HTTP/1.1 200 OK\r\n");
  response_data1 += "Server: Via-httplib/0.14\r\n";
  response_data1 += "Transfer-Encoding: Chunked\r\n";

  std::string::iterator next(response_data1.begin());

  response_receiver<std::string> the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data1.end()));
  BOOST_CHECK(rx_state == RX_INCOMPLETE);

  std::string response_data("\r\n15");
  next = response_data.begin();

  rx_state = the_response_receiver.receive(next, response_data.end());
  BOOST_CHECK(rx_state == RX_VALID);
  BOOST_CHECK_EQUAL(200, the_response_receiver.response().status());
  BOOST_CHECK_EQUAL("OK", the_response_receiver.response().reason_phrase().c_str());
  BOOST_CHECK_EQUAL('1', the_response_receiver.response().major_version());
  BOOST_CHECK_EQUAL('1', the_response_receiver.response().minor_version());
  BOOST_CHECK(the_response_receiver.response().is_chunked());
  BOOST_CHECK(the_response_receiver.body().empty());

  std::string body_data("\r\nHTTP chunk number: 1\n\r\n");
  next = body_data.begin();
  rx_state = the_response_receiver.receive(next, body_data.end());
  bool complete (rx_state == RX_CHUNK);
  BOOST_CHECK(complete);
  BOOST_CHECK(!the_response_receiver.chunk().is_last());

  BOOST_CHECK_EQUAL(the_response_receiver.chunk().size(),
                    the_response_receiver.chunk().data().size());

  std::string body_data2("16\r\nHTTP chunk ");
  next = body_data2.begin();
  rx_state = the_response_receiver.receive(next, body_data2.end());
  BOOST_CHECK(rx_state == RX_INCOMPLETE);

  std::string body_data3("number: 21\n\r\n");
  next = body_data3.begin();
  rx_state = the_response_receiver.receive(next, body_data3.end());
  BOOST_CHECK(rx_state == RX_CHUNK);
  BOOST_CHECK(!the_response_receiver.chunk().is_last());
  BOOST_CHECK_EQUAL(the_response_receiver.chunk().size(),
                    the_response_receiver.chunk().data().size());

  std::string body_data4("0\r\n\r\n");
  next = body_data4.begin();
  rx_state = the_response_receiver.receive(next, body_data4.end());
  BOOST_CHECK(rx_state == RX_CHUNK);
  BOOST_CHECK(the_response_receiver.chunk().is_last());
}

BOOST_AUTO_TEST_CASE(InvalidOK2)
{
  std::string response_data("HTTP/1.0 200 OK\r\n");
  response_data += "Content-Length: 4z\r\n";
  response_data += "\r\nabcd";
  std::string::iterator next(response_data.begin());

  response_receiver<std::string> the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data.end()));
  bool ok (rx_state == RX_INVALID);
  BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(InvalidOK3)
{
  std::string response_data("HTTP/1.0 200 OK\r\n");
  response_data += "Server: Via-httplib/1.1.0\r\n";
  response_data += "\r\nabcd";
  std::string::iterator next(response_data.begin());

  response_receiver<std::string> the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data.end()));
  bool ok (rx_state == RX_LENGTH_REQUIRED);
  BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(InValidOKChunked4)
{
  std::string response_data1("HTTP/1.1 200 OK\r\n");
  response_data1 += "Server: Via-httplib/0.14\r\n";
  response_data1 += "Transfer-Encoding: Chunked\r\n";

  std::string::iterator next(response_data1.begin());

  response_receiver<std::string> the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data1.end()));
  BOOST_CHECK(rx_state == RX_INCOMPLETE);

  std::string response_data("\r\n15");
  next = response_data.begin();

  rx_state = the_response_receiver.receive(next, response_data.end());
  BOOST_CHECK(rx_state == RX_VALID);

  std::string body_data("\r\nHTTP chunk number: 1\n\r\r");
  next = body_data.begin();
  rx_state = the_response_receiver.receive(next, body_data.end());
  BOOST_CHECK(rx_state == RX_INVALID);
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
