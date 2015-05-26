//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/authentication/base64.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace via::http::authentication;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestEncodeBase64)

BOOST_AUTO_TEST_CASE(Encode1)
{
  std::string input("ABCDEFGHIJKLMNOPQRSTUVWXYZ:0123456789");
  std::string output("QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo6MDEyMzQ1Njc4OQ==");
  std::string result(base64::encode(input));

  BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_CASE(Encode2)
{
  std::string input("Ken:ABCD");
  std::string output("S2VuOkFCQ0Q=");
  std::string result(base64::encode(input));

  BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestDecodeBase64)

BOOST_AUTO_TEST_CASE(Decode1)
{
  std::string input("QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo6MDEyMzQ1Njc4OQ==");
  std::string output("ABCDEFGHIJKLMNOPQRSTUVWXYZ:0123456789");
  std::string result(base64::decode(input));

  BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_CASE(Decode2)
{
  std::string input("QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo6MDEyMzQ1Njc4OQ");
  std::string output("ABCDEFGHIJKLMNOPQRSTUVWXYZ:0123456789");
  std::string result(base64::decode(input));

  BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_CASE(Decode3)
{
  std::string input("QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo6MDEyMzQ1Njc4OQ=");
  std::string output("ABCDEFGHIJKLMNOPQRSTUVWXYZ:0123456789");
  std::string result(base64::decode(input));

  BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_CASE(Decode4)
{
  std::string input("QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo6MDEyMzQ1Njc4O");
  std::string output("ABCDEFGHIJKLMNOPQRSTUVWXYZ:012345678");
  std::string result(base64::decode(input));

  BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_CASE(Decode5)
{
  std::string input("S2VuOkFCQ0Q");
  std::string output("Ken:ABCD");
  std::string result(base64::decode(input));

  BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_CASE(Decode6)
{
  // The input contains an invalid Base64 character
  std::string input("Ken:ABCD");
  std::string result(base64::decode(input));

  BOOST_CHECK(result.empty());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
