//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/character.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestHexString)

BOOST_AUTO_TEST_CASE(ValidDigits1)
{
  std::string hex_string("10");
  auto value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(16, value);
}

BOOST_AUTO_TEST_CASE(ValidHex1)
{
  std::string hex_string("abcdef");
  auto value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(0xABCDEF, value);
}

BOOST_AUTO_TEST_CASE(InvalidHex1)
{
  std::string hex_string("0xabcdef");
  auto value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(size_t(ULONG_MAX), value);
}

BOOST_AUTO_TEST_CASE(InvalidHex2)
{
  std::string hex_string("abcdefx");
  auto value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(size_t(ULONG_MAX), value);
}

BOOST_AUTO_TEST_CASE(InvalidHex3)
{
  std::string hex_string("ffffffffff");
  auto value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(size_t(ULONG_MAX), value);
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestDecString)

BOOST_AUTO_TEST_CASE(ValidDigits1)
{
  std::string dec_string("10");
  auto value(from_dec_string(dec_string));

  BOOST_CHECK_EQUAL(10, value);
}

BOOST_AUTO_TEST_CASE(ValidDec1)
{
  std::string dec_string("0");
  auto value(from_dec_string(dec_string));

  BOOST_CHECK_EQUAL(0, value);
}

BOOST_AUTO_TEST_CASE(InvalidDec1)
{
  std::string dec_string("123456x");
  auto value(from_dec_string(dec_string));

  BOOST_CHECK_EQUAL(size_t(ULONG_MAX), value);
}

BOOST_AUTO_TEST_CASE(InvalidDec2)
{
  std::string dec_string("10000000000000000");
  auto value(from_dec_string(dec_string));

  BOOST_CHECK_EQUAL(size_t(ULONG_MAX), value);
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
