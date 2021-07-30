//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Ken Barker
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

typedef response_receiver<std::string, 65534, 65534, 65534, LONG_MAX, 65534, 254, false>
    http_response_receiver;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseStatus)

BOOST_AUTO_TEST_CASE(ResponseStatus1)
{
  // Informational 1xx
  BOOST_CHECK_EQUAL("Continue",
    response_status::reason_phrase(response_status::code::CONTINUE).data());
  BOOST_CHECK_EQUAL("Switching Protocols",
    response_status::reason_phrase(response_status::code::SWITCHING_PROTOCOLS).data());

  // Successful 2xx
  BOOST_CHECK_EQUAL("OK",
    response_status::reason_phrase(response_status::code::OK).data());
  BOOST_CHECK_EQUAL("Created",
    response_status::reason_phrase(response_status::code::CREATED).data());
  BOOST_CHECK_EQUAL("Accepted",
    response_status::reason_phrase(response_status::code::ACCEPTED).data());
  BOOST_CHECK_EQUAL("Non-Authoritative Information",
    response_status::reason_phrase(response_status::code::NON_AUTHORITATIVE).data());
  BOOST_CHECK_EQUAL("No Content",
    response_status::reason_phrase(response_status::code::NO_CONTENT).data());
  BOOST_CHECK_EQUAL("Reset Content",
    response_status::reason_phrase(response_status::code::RESET_CONTENT).data());
  BOOST_CHECK_EQUAL("Partial Content",
    response_status::reason_phrase(response_status::code::PARTIAL_CONTENT).data());

  // Redirection 3xx
  BOOST_CHECK_EQUAL("Multiple Choices",
    response_status::reason_phrase(response_status::code::MULTIPLE_CHOICES).data());
  BOOST_CHECK_EQUAL("Moved Permanently",
    response_status::reason_phrase(response_status::code::MOVED_PERMANENTLY).data());
  BOOST_CHECK_EQUAL("Found",
    response_status::reason_phrase(response_status::code::FOUND).data());
  BOOST_CHECK_EQUAL("See Other",
    response_status::reason_phrase(response_status::code::SEE_OTHER).data());
  BOOST_CHECK_EQUAL("Not Modified",
    response_status::reason_phrase(response_status::code::NOT_MODIFIED).data());
  BOOST_CHECK_EQUAL("Use Proxy",
    response_status::reason_phrase(response_status::code::USE_PROXY).data());
  BOOST_CHECK_EQUAL("Temporary Redirect",
    response_status::reason_phrase(response_status::code::TEMPORARY_REDIRECT).data());
  BOOST_CHECK_EQUAL("Permanent Redirect",
    response_status::reason_phrase(response_status::code::PERMANENT_REDIRECT).data());

  // Client Error 4xx
  BOOST_CHECK_EQUAL("Bad Request",
    response_status::reason_phrase(response_status::code::BAD_REQUEST).data());
  BOOST_CHECK_EQUAL("Unauthorized",
    response_status::reason_phrase(response_status::code::UNAUTHORISED).data());
  BOOST_CHECK_EQUAL("Payment Required",
    response_status::reason_phrase(response_status::code::PAYMENT_REQUIRED).data());
  BOOST_CHECK_EQUAL("Forbidden",
    response_status::reason_phrase(response_status::code::FORBIDDEN).data());
  BOOST_CHECK_EQUAL("Not Found",
    response_status::reason_phrase(response_status::code::NOT_FOUND).data());
  BOOST_CHECK_EQUAL("Method Not Allowed",
    response_status::reason_phrase(response_status::code::METHOD_NOT_ALLOWED).data());
  BOOST_CHECK_EQUAL("Not Acceptable",
    response_status::reason_phrase(response_status::code::NOT_ACCEPTABLE).data());
  BOOST_CHECK_EQUAL("Proxy Authentication Required",
    response_status::reason_phrase(response_status::code::PROXY_AUTHENTICATION_REQUIRED).data());
  BOOST_CHECK_EQUAL("Request Time-out",
    response_status::reason_phrase(response_status::code::REQUEST_TIMEOUT).data());
  BOOST_CHECK_EQUAL("Conflict",
    response_status::reason_phrase(response_status::code::CONFLICT).data());
  BOOST_CHECK_EQUAL("Gone",
    response_status::reason_phrase(response_status::code::GONE).data());
  BOOST_CHECK_EQUAL("Length Required",
    response_status::reason_phrase(response_status::code::LENGTH_REQUIRED).data());
  BOOST_CHECK_EQUAL("Precondition Failed",
    response_status::reason_phrase(response_status::code::PRECONDITION_FAILED).data());
  BOOST_CHECK_EQUAL("Payload Too Large",
    response_status::reason_phrase(response_status::code::PAYLOAD_TOO_LARGE).data());
  BOOST_CHECK_EQUAL("Request-URI Too Long",
    response_status::reason_phrase(response_status::code::REQUEST_URI_TOO_LONG).data());
  BOOST_CHECK_EQUAL("Unsupported Media Type",
    response_status::reason_phrase(response_status::code::UNSUPPORTED_MEDIA_TYPE).data());
  BOOST_CHECK_EQUAL("Requested range not satisfiable",
    response_status::reason_phrase(response_status::code::REQUEST_RANGE_NOT_SATISFIABLE).data());
  BOOST_CHECK_EQUAL("Expectation Failed",
    response_status::reason_phrase(response_status::code::EXPECTATION_FAILED).data());
  BOOST_CHECK_EQUAL("Upgrade Required",
    response_status::reason_phrase(response_status::code::UPGRADE_REQUIRED).data());
  BOOST_CHECK_EQUAL("Precondition Required",
    response_status::reason_phrase(response_status::code::PRECONDITION_REQUIRED).data());
  BOOST_CHECK_EQUAL("Too Many Requests",
    response_status::reason_phrase(response_status::code::TOO_MANY_REQUESTS).data());
  BOOST_CHECK_EQUAL("Request Header Fields Too Large",
    response_status::reason_phrase(response_status::code::REQUEST_HEADER_FIELDS_TOO_LARGE).data());

  // Server Error 5xx
  BOOST_CHECK_EQUAL("Internal Server Error",
    response_status::reason_phrase(response_status::code::INTERNAL_SERVER_ERROR).data());
  BOOST_CHECK_EQUAL("Not Implemented",
    response_status::reason_phrase(response_status::code::NOT_IMPLEMENTED).data());
  BOOST_CHECK_EQUAL("Bad Gateway",
    response_status::reason_phrase(response_status::code::BAD_GATEWAY).data());
  BOOST_CHECK_EQUAL("Service Unavailable",
    response_status::reason_phrase(response_status::code::SERVICE_UNAVAILABLE).data());
  BOOST_CHECK_EQUAL("Gateway Time-out",
    response_status::reason_phrase(response_status::code::GATEWAY_TIMEOUT).data());
  BOOST_CHECK_EQUAL("HTTP Version not supported",
    response_status::reason_phrase(response_status::code::HTTP_VERSION_NOT_SUPPORTED).data());
  BOOST_CHECK_EQUAL("Network Authentication Required",
    response_status::reason_phrase(response_status::code::NETWORK_AUTHENTICATION_REQUIRED).data());
}

BOOST_AUTO_TEST_CASE(ResponseStatus2)
{
  using namespace via::http::response_status;

  // Informational 1xx
  BOOST_CHECK(!content_permitted(static_cast<int>(code::CONTINUE)));
  BOOST_CHECK(!content_permitted(static_cast<int>(code::SWITCHING_PROTOCOLS)));

  // Successful 2xx
  BOOST_CHECK(content_permitted(static_cast<int>(code::OK)));
  BOOST_CHECK(content_permitted(static_cast<int>(code::CREATED)));
  BOOST_CHECK(content_permitted(static_cast<int>(code::ACCEPTED)));
  BOOST_CHECK(content_permitted(static_cast<int>(code::NON_AUTHORITATIVE)));
  BOOST_CHECK(!content_permitted(static_cast<int>(code::NO_CONTENT)));
  BOOST_CHECK(content_permitted(static_cast<int>(code::RESET_CONTENT)));
  BOOST_CHECK(content_permitted(static_cast<int>(code::PARTIAL_CONTENT)));

  // Redirection 3xx
  BOOST_CHECK(content_permitted(static_cast<int>(code::MULTIPLE_CHOICES)));
  BOOST_CHECK(content_permitted(static_cast<int>(code::MOVED_PERMANENTLY)));
  BOOST_CHECK(content_permitted(static_cast<int>(code::FOUND)));
  BOOST_CHECK(content_permitted(static_cast<int>(code::SEE_OTHER)));
  BOOST_CHECK(!content_permitted(static_cast<int>(code::NOT_MODIFIED)));
  BOOST_CHECK(content_permitted(static_cast<int>(code::USE_PROXY)));
  BOOST_CHECK(content_permitted(static_cast<int>(code::TEMPORARY_REDIRECT)));

  // Client Error 4xx
  BOOST_CHECK(content_permitted(static_cast<int>(code::BAD_REQUEST)));
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

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());
}

// An http response line in a vector of unsigned chars.
BOOST_AUTO_TEST_CASE(ValidOkVectorUnsignedChar1)
{
  std::string RESPONSE_LINE("HTTP/1.0 200 OK\r\n");
  std::vector<unsigned char> response_data(RESPONSE_LINE.begin(), RESPONSE_LINE.end());
  std::vector<unsigned char>::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());
}

// An http response line in a string.
BOOST_AUTO_TEST_CASE(ValidOk1)
{
  std::string response_data("HTTP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());
}

// An http response line in a string without an \r.
BOOST_AUTO_TEST_CASE(ValidOk2)
{
  std::string response_data("HTTP/1.0 200 OK\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());
  BOOST_CHECK(!the_response.is_continue());
}

// An http response line in a string without an \r but with extra whitespace
BOOST_AUTO_TEST_CASE(ValidOk3)
{
  std::string response_data("HTTP/1.0\t200\t OK\n ");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK_EQUAL(' ', *next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());
  BOOST_CHECK(!the_response.is_continue());
}

BOOST_AUTO_TEST_CASE(ValidContinue1)
{
  std::string response_data("HTTP/1.1 100 Continue\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(100, the_response.status());
  BOOST_CHECK_EQUAL("Continue", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('1', the_response.minor_version());
  BOOST_CHECK(the_response.is_continue());
}

BOOST_AUTO_TEST_CASE(InValidOk0)
{
  // To much whitespace
  std::string response_data("          HTTP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk1)
{
  std::string response_data("XTTP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk2)
{
  std::string response_data("HXTP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk3)
{
  std::string response_data("HTXP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk4)
{
  std::string response_data("HTTX/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk5)
{
  std::string response_data("HTTPX1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<8, 1024, 254, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk6)
{
  std::string response_data("HTTP/X.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk7)
{
  std::string response_data("HTTP/1x0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk8)
{
  std::string response_data("HTTP/1.X 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}


BOOST_AUTO_TEST_CASE(InValidOk9)
{
  std::string response_data("HTTP/1.01 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk10)
{
  // To much whitespace
  std::string response_data("HTTP/1.0           200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk11)
{
  std::string response_data("HTTP/1.0 X00 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk12)
{
  std::string response_data("HTTP/1.0 2X0 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk13)
{
  std::string response_data("HTTP/1.0 200 OK \r\r");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk14)
{
  // To much whitespace
  std::string response_data("HTTP/1.0 200 OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<199, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk15)
{
  // To much whitespace
  std::string response_data("HTTP/1.0 200              OK\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk16)
{
  // To much whitespace
  std::string response_data("HTTP/1.0 200 BAD\r\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 2, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidOk17)
{
  // To much whitespace
  std::string response_data("HTTP/1.0 200 OK\n");
  std::string::iterator next(response_data.begin());

  response_line<1024, 254, 8, true> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseLineEncoder)

BOOST_AUTO_TEST_CASE(ValidOkString1)
{
  response_line<65534, 65534, 254, false> the_response(response_status::code::OK);
  std::string response_string(the_response.to_string());
  BOOST_CHECK_EQUAL("HTTP/1.1 200 OK\r\n", response_string.data());
}

BOOST_AUTO_TEST_CASE(ValidOkString2)
{
  response_line<65534, 65534, 254, false> the_response(200, "OK");
  std::string response_string(the_response.to_string());
  BOOST_CHECK_EQUAL("HTTP/1.1 200 OK\r\n", response_string.data());
}

BOOST_AUTO_TEST_CASE(ValidNonstandardString1)
{
  response_line<65534, 65534, 254, false> the_response(199, "Some rubbish");
  std::string response_string(the_response.to_string());
  BOOST_CHECK_EQUAL("HTTP/1.1 199 Some rubbish\r\n", response_string.data());
}

BOOST_AUTO_TEST_CASE(ValidNonstandardString2)
{
  response_line<65534, 65534, 254, false> the_response(response_status::code::OK);
  the_response.set_status_and_reason(199, "");
  std::string response_string(the_response.to_string());
  BOOST_CHECK_EQUAL("HTTP/1.1 199 \r\n", response_string.data());
}

BOOST_AUTO_TEST_CASE(ValidNonstandardString3)
{
  response_line<65534, 65534, 254, false> the_response(199, "", '0', '0');
  the_response.set_status(response_status::code::OK);
  the_response.set_major_version('1');
  the_response.set_minor_version('1');
  std::string response_string(the_response.to_string());
  BOOST_CHECK_EQUAL("HTTP/1.1 200 OK\r\n", response_string.data());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseParser)

BOOST_AUTO_TEST_CASE(ValidOK1)
{
  std::string response_data("HTTP/1.0 200 OK\r\nContent: text\r\n\r\n");
  std::string::iterator next(response_data.begin());

  rx_response<1024, 1024, 100, 8190, 1024, 8, false> the_response{};
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());

  BOOST_CHECK_EQUAL("text", the_response.headers().find("content").data());
  BOOST_CHECK_EQUAL(0, the_response.content_length());
  BOOST_CHECK(!the_response.is_continue());
  BOOST_CHECK(!the_response.is_chunked());
  BOOST_CHECK(!the_response.keep_alive());
  BOOST_CHECK_EQUAL("text", the_response.headers().find("content").data());
}

BOOST_AUTO_TEST_CASE(ValidOK2)
{
  std::string response_data("HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nabcd");
  std::string::iterator next(response_data.begin());

  rx_response<1024, 1024, 100, 8190, 1024, 8, false> a_response{};
  BOOST_CHECK(a_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.begin() != next);

  rx_response<1024, 1024, 100, 8190, 1024, 8, false> the_response{};
  the_response.swap(a_response);

  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('1', the_response.minor_version());

  BOOST_CHECK(!the_response.is_chunked());
  BOOST_CHECK(the_response.keep_alive());
  BOOST_CHECK(!the_response.is_continue());
  BOOST_CHECK_EQUAL(4, the_response.content_length());
  std::string body(next, next + the_response.content_length());
  BOOST_CHECK_EQUAL("abcd", body.data());
}

// Memory leaks according to Qt/MinGw
BOOST_AUTO_TEST_CASE(ValidOKChunked1)
{
  std::string response_data
    ("HTTP/1.0 200 OK\r\nTransfer-Encoding: Chunked\r\n\r\n4\r\n\r\n\r\n\r\n");
  std::string::iterator next(response_data.begin());

  rx_response<1024, 1024, 100, 8190, 1024, 8, false> the_response{};
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
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

  rx_response<1024, 1024, 100, 8190, 1024, 8, false> the_response{};
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(100, the_response.status());
  BOOST_CHECK_EQUAL("Continue", the_response.reason_phrase().data());
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

  rx_response<1024, 1024, 100, 8190, 1024, 8, false> the_response{};
  BOOST_CHECK(the_response.parse(next, response_data.end()));
  BOOST_CHECK_EQUAL(401, the_response.status());
  BOOST_CHECK_EQUAL("Unauthorized", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());

  BOOST_CHECK_EQUAL("Challenge",
    the_response.headers().find(header_field::id::WWW_AUTHENTICATE).data());
  BOOST_CHECK_EQUAL(0, the_response.content_length());
  BOOST_CHECK(!the_response.is_chunked());
}

BOOST_AUTO_TEST_CASE(ValidOKMultiLine1)
{
  std::string response_data("HTTP/1.0 200 OK\r\nC");
  std::string::iterator next(response_data.begin());

  rx_response<1024, 1024, 100, 8190, 1024, 8, false> the_response{};
  BOOST_CHECK(!the_response.parse(next, response_data.end()));
  BOOST_CHECK(response_data.end() == next);
  BOOST_CHECK_EQUAL(200, the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response.major_version());
  BOOST_CHECK_EQUAL('0', the_response.minor_version());

  std::string response_data2("ontent-Length: 4\r\n\r\nabcd");
  next = response_data2.begin();
  BOOST_CHECK(the_response.parse(next, response_data2.end()));

  BOOST_CHECK(!the_response.is_chunked());
  BOOST_CHECK_EQUAL(4, the_response.content_length());
  std::string body(next, next + the_response.content_length());
  BOOST_CHECK_EQUAL("abcd", body.data());
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
  BOOST_CHECK_EQUAL(correct_response.data(), resp_text.data());
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
  BOOST_CHECK_EQUAL(correct_response.data(), resp_text.data());
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
  BOOST_CHECK_EQUAL(correct_response.data(), resp_text.data());
}

BOOST_AUTO_TEST_CASE(ResponseEncode4)
{
  std::string correct_response ("HTTP/1.1 200 OK\r\n");
//  correct_response += via::http::header_field::server_header();
  correct_response += "Transfer-Encoding: Chunked\r\n\r\n";

  tx_response the_response(response_status::code::OK);
  the_response.add_header(header_field::HEADER_TRANSFER_ENCODING, "Chunked");
  std::string resp_text(the_response.message());
//  std::string resp_text(resp_data.begin(), resp_data.end());
  BOOST_CHECK_EQUAL(correct_response.data(), resp_text.data());
}

BOOST_AUTO_TEST_CASE(ResponseEncode5)
{
  std::string correct_response ("HTTP/1.1 204 No Content\r\n\r\n");

  tx_response the_response(response_status::code::NO_CONTENT);
  std::string resp_text(the_response.message());
//  std::cout << resp_text << std::endl;
  BOOST_CHECK_EQUAL(correct_response.data(), resp_text.data());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseReceiver)

BOOST_AUTO_TEST_CASE(ValidOK1)
{
  std::string response_data("HTTP/1.0 200 OK\r\nC");
  std::string::iterator next(response_data.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);

  std::string response_data2("ontent-Length: 4\r\n\r\nabcd");
  next = response_data2.begin();
  rx_state = the_response_receiver.receive(next, response_data2.end());
  bool complete (rx_state == Rx::VALID);
  BOOST_CHECK(complete);

  BOOST_CHECK_EQUAL(200, the_response_receiver.response().status());
  BOOST_CHECK_EQUAL("OK", the_response_receiver.response().reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response_receiver.response().major_version());
  BOOST_CHECK_EQUAL('0', the_response_receiver.response().minor_version());
  BOOST_CHECK(!the_response_receiver.response().is_chunked());
  BOOST_CHECK_EQUAL(4, the_response_receiver.response().content_length());
  std::string body(the_response_receiver.body());
  BOOST_CHECK_EQUAL("abcd", body.data());
}

BOOST_AUTO_TEST_CASE(ValidOK2)
{
  std::string response_data("HTTP/1.0 200 OK\r\n");
  response_data += "Server: Via-httplib/0.14\r\n";
  response_data += "Content-Length: 4\r\n";
  response_data += "\r\nabcd\r\n"; // extra chars at end of body
  std::string::iterator next(response_data.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data.end()));
  bool ok (rx_state == Rx::VALID);
  BOOST_CHECK(ok);

  std::string body(the_response_receiver.body());
  BOOST_CHECK_EQUAL("abcd", body.data());
}

BOOST_AUTO_TEST_CASE(InValidOK1)
{
  std::string response_data("P");
  std::string::iterator next(response_data.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data.end()));
  BOOST_CHECK(rx_state == Rx::INVALID);
}

BOOST_AUTO_TEST_CASE(ValidOKChunked1)
{
  std::string response_data1("H");
  std::string::iterator next(response_data1.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data1.end()));
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  std::string response_data("TTP/1.0 200 OK\r\n");
  response_data += "Content-Type: application/json\r\n";
  response_data += "Transfer-Encoding: Chunked\r\n";
  response_data += "Connection: Keep-Alive\r\n";
  response_data += "Host: 172.16.0.126:3456\r\n\r\n";
  next = response_data.begin();

  rx_state = the_response_receiver.receive(next, response_data.end());
  BOOST_CHECK(rx_state == Rx::VALID);
  BOOST_CHECK_EQUAL(200, the_response_receiver.response().status());
  BOOST_CHECK_EQUAL("OK", the_response_receiver.response().reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response_receiver.response().major_version());
  BOOST_CHECK_EQUAL('0', the_response_receiver.response().minor_version());
  BOOST_CHECK(the_response_receiver.response().is_chunked());
  BOOST_CHECK(the_response_receiver.body().empty());

  std::string body_data("1a\r\nabcdefghijklmnopqrstuvwxyz\r\n");
  next = body_data.begin();
  rx_state = the_response_receiver.receive(next, body_data.end());
  bool complete (rx_state == Rx::CHUNK);
  BOOST_CHECK(complete);

  std::string body_data2("24\r\n0123456789abcdefghijkl");
  next = body_data2.begin();
  rx_state = the_response_receiver.receive(next, body_data2.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  std::string body_data3("mnopqrstuvwxyz\r\n");
  next = body_data3.begin();
  rx_state = the_response_receiver.receive(next, body_data3.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);
}

BOOST_AUTO_TEST_CASE(ValidOKChunked2)
{
  std::string response_data1("HTTP/1.1 200 OK\r\n");
  response_data1 += "Server: Via-httplib/0.14\r\n";
  response_data1 += "Transfer-Encoding: Chunked\r\n";

  std::string::iterator next(response_data1.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data1.end()));
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  std::string response_data("\r\n");
  next = response_data.begin();

  rx_state = the_response_receiver.receive(next, response_data.end());
  BOOST_CHECK(rx_state == Rx::VALID);
  BOOST_CHECK_EQUAL(200, the_response_receiver.response().status());
  BOOST_CHECK_EQUAL("OK", the_response_receiver.response().reason_phrase().data());
  BOOST_CHECK_EQUAL('1', the_response_receiver.response().major_version());
  BOOST_CHECK_EQUAL('1', the_response_receiver.response().minor_version());
  BOOST_CHECK(the_response_receiver.response().is_chunked());
  BOOST_CHECK(the_response_receiver.body().empty());

  std::string body_data("15\r\nHTTP chunk number: 1\n\r\n");
  next = body_data.begin();
  rx_state = the_response_receiver.receive(next, body_data.end());
  bool complete (rx_state == Rx::CHUNK);
  BOOST_CHECK(complete);
  BOOST_CHECK(!the_response_receiver.chunk().is_last());

  BOOST_CHECK_EQUAL(the_response_receiver.chunk().size(),
                    the_response_receiver.chunk().data().size());

  std::string body_data2("16\r\nHTTP chunk ");
  next = body_data2.begin();
  rx_state = the_response_receiver.receive(next, body_data2.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  std::string body_data3("number: 21\n\r\n");
  next = body_data3.begin();
  rx_state = the_response_receiver.receive(next, body_data3.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);
  BOOST_CHECK(!the_response_receiver.chunk().is_last());
  BOOST_CHECK_EQUAL(the_response_receiver.chunk().size(),
                    the_response_receiver.chunk().data().size());

  std::string body_data4("0\r\n\r\n");
  next = body_data4.begin();
  rx_state = the_response_receiver.receive(next, body_data4.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);
  BOOST_CHECK(the_response_receiver.chunk().is_last());
}

BOOST_AUTO_TEST_CASE(InvalidOK2)
{
  std::string response_data("HTTP/1.0 200 OK\r\n");
  response_data += "Content-Length: 4z\r\n";
  response_data += "\r\nabcd";
  std::string::iterator next(response_data.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data.end()));
  bool ok (rx_state == Rx::INVALID);
  BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(InvalidOK3)
{
  // Note: a response without a content length is valid...
  std::string response_data("HTTP/1.0 200 OK\r\n");
  response_data += "Server: Via-httplib/1.1.0\r\n";
  response_data += "\r\nabcd";
  std::string::iterator next(response_data.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data.end()));
  bool ok (rx_state == Rx::INCOMPLETE);
  BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(InValidOKChunked4)
{
  std::string response_data1("HTTP/1.1 200 OK\r\n");
  response_data1 += "Server: Via-httplib/0.14\r\n";
  response_data1 += "Transfer-Encoding: Chunked\r\n";

  std::string::iterator next(response_data1.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(next, response_data1.end()));
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  std::string response_data("\r\n15");
  next = response_data.begin();

  rx_state = the_response_receiver.receive(next, response_data.end());
  BOOST_CHECK(rx_state == Rx::VALID);

  std::string body_data("\r\nHTTP chunk number: 1\n\r\r");
  next = body_data.begin();
  rx_state = the_response_receiver.receive(next, body_data.end());
  BOOST_CHECK(rx_state == Rx::INVALID);
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestResponseLoopback)

BOOST_AUTO_TEST_CASE(LoopbackOk1)
{
  tx_response server_response(response_status::code::OK);
  std::string response_data1(server_response.message());
  std::string::iterator iter(response_data1.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(iter, response_data1.end()));
  BOOST_CHECK(rx_state == Rx::VALID);

  auto const& the_response(the_response_receiver.response());
  BOOST_CHECK_EQUAL(static_cast<int>(response_status::code::OK),
                    the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL(0, the_response.content_length());
}

BOOST_AUTO_TEST_CASE(LoopbackOk2)
{
  // Two OK responses with bodies all in separate buffers
  std::string response_body1("abcdefghijklmnopqrstuvwxyz0123456789");

  tx_response server_response1(response_status::code::OK);
  std::string response_data1(server_response1.message(response_body1.size()));
  std::string::iterator iter(response_data1.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(iter, response_data1.end()));
  BOOST_CHECK(iter == response_data1.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  auto const& the_response(the_response_receiver.response());
  BOOST_CHECK_EQUAL(static_cast<int>(response_status::code::OK),
                    the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL(36, the_response.content_length());

  iter = response_body1.begin();
  rx_state = the_response_receiver.receive(iter, response_body1.end());
  BOOST_CHECK(iter == response_body1.end());
  BOOST_CHECK(rx_state == Rx::VALID);

  // The second response
  std::string response_body2("9876543210abcdefghijklmnopqrstuvwxyz0123456789");

  tx_response server_response2(response_status::code::OK);
  std::string response_data2(server_response2.message(response_body2.size()));
  iter = response_data2.begin();

  // reset the receiver
  the_response_receiver.clear();
  rx_state = the_response_receiver.receive(iter, response_data2.end());
  BOOST_CHECK(iter == response_data2.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  iter = response_body2.begin();
  rx_state = the_response_receiver.receive(iter, response_body2.end());
  BOOST_CHECK(iter == response_body2.end());
  BOOST_CHECK(rx_state == Rx::VALID);
}

BOOST_AUTO_TEST_CASE(LoopbackOk3)
{
  // Two OK responses with bodies all in one buffer
  std::string response_body1("abcdefghijklmnopqrstuvwxyz0123456789");

  tx_response server_response1(response_status::code::OK);
  std::string response_data1(server_response1.message(response_body1.size()));

  // The second response
  std::string response_body2("9876543210abcdefghijklmnopqrstuvwxyz0123456789");

  tx_response server_response2(response_status::code::OK);
  std::string response_data2(server_response2.message(response_body2.size()));

  std::string response_buffer(response_data1 + response_body1 +
                              response_data2 + response_body2);

  std::string::iterator iter(response_buffer.begin());
  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(iter, response_buffer.end()));
  BOOST_CHECK(iter != response_buffer.end());
  BOOST_CHECK(rx_state == Rx::VALID);

  auto const& the_response(the_response_receiver.response());
  BOOST_CHECK_EQUAL(static_cast<int>(response_status::code::OK),
                    the_response.status());
  BOOST_CHECK_EQUAL("OK", the_response.reason_phrase().data());
  BOOST_CHECK_EQUAL(36, the_response.content_length());

  // reset the receiver
  the_response_receiver.clear();
  rx_state = the_response_receiver.receive(iter, response_buffer.end());
  BOOST_CHECK(iter == response_buffer.end());
  BOOST_CHECK(rx_state == Rx::VALID);
}

BOOST_AUTO_TEST_CASE(LoopbackOkChunked1)
{
  // OK response with two bodies in chunked buffers.
  tx_response server_response1(response_status::code::OK);
  server_response1.add_header(header_field::HEADER_TRANSFER_ENCODING, "Chunked");
  std::string response_data1(server_response1.message());
  std::string::iterator iter(response_data1.begin());

  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(iter, response_data1.end()));
  BOOST_CHECK(rx_state == Rx::VALID);

  std::string  chunk_body1("abcdefghijklmnopqrstuvwxyz0123456789");
  chunk_header<1024, 8, false> chunk_header1(chunk_body1.size());
  std::string  http_chunk_1(chunk_header1.to_string());
  chunk_body1 += CRLF;

  iter = http_chunk_1.begin();
  rx_state = the_response_receiver.receive(iter, http_chunk_1.end());
  BOOST_CHECK(iter == http_chunk_1.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  iter = chunk_body1.begin();
  rx_state = the_response_receiver.receive(iter, chunk_body1.end());
  BOOST_CHECK(iter == chunk_body1.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  std::string chunk_body2("9876543210abcdefghijklmnopqrstuvwxyz");
  chunk_header<1024, 8, false> chunk_header2(chunk_body2.size());
  std::string  http_chunk_2(chunk_header2.to_string());
  chunk_body2 += CRLF;

  iter = http_chunk_2.begin();
  rx_state = the_response_receiver.receive(iter, http_chunk_2.end());
  BOOST_CHECK(iter == http_chunk_2.end());
  BOOST_CHECK(rx_state == Rx::INCOMPLETE);

  iter = chunk_body2.begin();
  rx_state = the_response_receiver.receive(iter, chunk_body2.end());
  BOOST_CHECK(iter == chunk_body2.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  std::string chunk_ext("chunk extension");
  std::string chunk_trailer("chunk: trailer");
  last_chunk  last_header(chunk_ext, chunk_trailer);
  std::string http_chunk_3(last_header.to_string());
  http_chunk_3 += CRLF;

  iter = http_chunk_3.begin();
  rx_state = the_response_receiver.receive(iter, http_chunk_3.end());
  BOOST_CHECK(iter == http_chunk_3.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);
}

BOOST_AUTO_TEST_CASE(LoopbackOkChunked2)
{
  // OK response with two bodies in chunked bodies all in one buffer
  tx_response server_response1(response_status::code::OK);
  server_response1.add_header(header_field::HEADER_TRANSFER_ENCODING, "Chunked");
  std::string response_data1(server_response1.message());

  std::string  chunk_body1("abcdefghijklmnopqrstuvwxyz0123456789");
  chunk_header<1024, 8, false> chunk_header1(chunk_body1.size());
  std::string  http_chunk_1(chunk_header1.to_string());
  chunk_body1 += CRLF;

  std::string chunk_body2("9876543210abcdefghijklmnopqrstuvwxyz");
  chunk_header<1024, 8, false> chunk_header2(chunk_body2.size());
  std::string  http_chunk_2(chunk_header2.to_string());
  chunk_body2 += CRLF;

  std::string chunk_ext("chunk extension");
  std::string chunk_trailer("chunk: trailer");
  last_chunk  last_header(chunk_ext, chunk_trailer);
  std::string http_chunk_3(last_header.to_string());
  http_chunk_3 += CRLF;

  std::string response_buffer(response_data1 +
                              http_chunk_1 + chunk_body1 +
                              http_chunk_2 + chunk_body2 +
                              http_chunk_3 +
                              response_data1);

  std::string::iterator iter(response_buffer.begin());
  http_response_receiver the_response_receiver;
  Rx rx_state(the_response_receiver.receive(iter, response_buffer.end()));
  BOOST_CHECK(iter != response_buffer.end());
  BOOST_CHECK(rx_state == Rx::VALID);

  rx_state = the_response_receiver.receive(iter, response_buffer.end());
  BOOST_CHECK(iter != response_buffer.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  rx_state = the_response_receiver.receive(iter, response_buffer.end());
  BOOST_CHECK(iter != response_buffer.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  rx_state = the_response_receiver.receive(iter, response_buffer.end());
  BOOST_CHECK(iter != response_buffer.end());
  BOOST_CHECK(rx_state == Rx::CHUNK);

  the_response_receiver.clear();
  rx_state = the_response_receiver.receive(iter, response_buffer.end());
  BOOST_CHECK(iter == response_buffer.end());
  BOOST_CHECK(rx_state == Rx::VALID);
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
