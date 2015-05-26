//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/authentication/basic.hpp"
#include "via/http/authentication/base64.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace via::http::authentication;
using namespace via::http;

namespace
{
  // A boost test fixture for this test suite.
  struct BasicAuthFixture
  {
    basic basic_authentication1;
    basic basic_authentication2;
    basic basic_authentication3;

    std::string failure_response;
    std::string request_header;
    std::string basic_auth;

    std::string user1;
    std::string pw1;
    std::string credentials1;

    std::string user2;
    std::string pw2;
    std::string credentials2;

    std::string user3;
    std::string pw3;
    std::string credentials3;

    std::string user4;
    std::string pw4;
    std::string credentials4;

    BasicAuthFixture()
      : basic_authentication1("realm1")
      , basic_authentication2("realm2")
      , basic_authentication3()
      , failure_response("Basic realm=\"" + std::string("realm1") + "\"")
      , request_header("GET abcde HTTP/1.1\r\nContent: text\r\n")
      , basic_auth("Authorization: Basic ")
      , user1("Homer")
      , pw1("AaaaaH")
      , credentials1(base64::encode(user1 + ":" + pw1))
      , user2("Lisa")
      , pw2("Clarinet")
      , credentials2(base64::encode(user2 + ":" + pw2))
      , user3("Bart")
      , pw3("Cowabunga")
      , credentials3(base64::encode(user3 + ":" + pw3))
      , user4("Marge")
      , pw4("Really!")
      , credentials4(base64::encode(user4 + ":" + pw4))
    {
      basic_authentication1.add_user(user1, pw1);
      basic_authentication1.add_user(user2, pw2);

      basic_authentication2.add_user(user3, pw3);
      basic_authentication2.add_user(user4, pw4);

      basic_authentication3.add_user(user1, pw1);
      basic_authentication3.add_user(user2, pw2);
      basic_authentication3.add_user(user3, pw3);
      basic_authentication3.add_user(user4, pw4);
    }

    ~BasicAuthFixture() = default;
  };
}

//////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE(TestBasicAuthentication, BasicAuthFixture)

BOOST_AUTO_TEST_CASE(FailAuthentication1)
{
  // A request without an Authentication header
  std::string request_data(request_header);
  request_data += CRLF;
  std::string::iterator next(request_data.begin());
  rx_request request(false, 8, 8, 1024, 1024, 100, 8190);
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string response(basic_authentication1.authenticate(request));
  BOOST_CHECK(!response.empty());
  BOOST_CHECK_EQUAL(failure_response, response);
}

BOOST_AUTO_TEST_CASE(FailAuthentication2)
{
  // A request just the authentication text
  std::string request_data(request_header);
  request_data += basic_auth + CRLF + CRLF;
  std::string::iterator next(request_data.begin());
  rx_request request(false, 8, 8, 1024, 1024, 100, 8190);
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string response(basic_authentication1.authenticate(request));
  BOOST_CHECK(!response.empty());
  BOOST_CHECK_EQUAL(failure_response, response);
}

BOOST_AUTO_TEST_CASE(FailAuthentication3)
{
  // A request for a user from a different realm
  std::string request_data(request_header);
  request_data += basic_auth + credentials3 + CRLF + CRLF;
  std::string::iterator next(request_data.begin());
  rx_request request(false, 8, 8, 1024, 1024, 100, 8190);
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string response(basic_authentication1.authenticate(request));
  BOOST_CHECK(!response.empty());
  BOOST_CHECK_EQUAL(failure_response, response);
}

BOOST_AUTO_TEST_CASE(FailAuthentication4)
{
  // A request for a valid user with the wrong password
  std::string credentials_bad(base64::encode(user1 + ":" + pw2));
  std::string request_data(request_header);
  request_data += basic_auth + credentials_bad + CRLF + CRLF;
  std::string::iterator next(request_data.begin());
  rx_request request(false, 8, 8, 1024, 1024, 100, 8190);
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string response(basic_authentication1.authenticate(request));
  BOOST_CHECK(!response.empty());
  BOOST_CHECK_EQUAL(failure_response, response);
}

BOOST_AUTO_TEST_CASE(FailAuthentication5)
{
  // A request without an Authentication header to a server without a realm
  std::string request_data(request_header);
  request_data += CRLF;
  std::string::iterator next(request_data.begin());
  rx_request request(false, 8, 8, 1024, 1024, 100, 8190);
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string response(basic_authentication3.authenticate(request));
  BOOST_CHECK(!response.empty());
  BOOST_CHECK_EQUAL("Basic", response);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(PassAuthentication1)
{
  // Simple user name password test
  std::string request_data(request_header);
  request_data += basic_auth + credentials1 + CRLF + CRLF;
  std::string::iterator next(request_data.begin());
  rx_request request(false, 8, 8, 1024, 1024, 100, 8190);
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string response(basic_authentication1.authenticate(request));
  BOOST_CHECK(response.empty());
}

BOOST_AUTO_TEST_CASE(PassAuthentication2)
{
  // Simple user name password test, with an extra header
  std::string request_data(request_header);
  request_data += basic_auth + credentials1 + CRLF;
  request_data += "TE: Chunked" + CRLF + CRLF;
  std::string::iterator next(request_data.begin());
  rx_request request(false, 8, 8, 1024, 1024, 100, 8190);
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string response(basic_authentication1.authenticate(request));
  BOOST_CHECK(response.empty());
}

BOOST_AUTO_TEST_CASE(PassAuthentication3)
{
  // Simple user name password test, different realm
  std::string request_data(request_header);
  request_data += basic_auth + credentials3 + CRLF + CRLF;
  std::string::iterator next(request_data.begin());
  rx_request request(false, 8, 8, 1024, 1024, 100, 8190);
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string response(basic_authentication2.authenticate(request));
  BOOST_CHECK(response.empty());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
