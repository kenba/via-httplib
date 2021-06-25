//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Via Technology Ltd. All Rights Reserved.
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
BOOST_AUTO_TEST_SUITE(TestSeparators)

BOOST_AUTO_TEST_CASE(EndOfLine)
{
  BOOST_CHECK(is_end_of_line('\r'));
  BOOST_CHECK(is_end_of_line('\n'));
  BOOST_CHECK(!is_end_of_line('\t'));
}

BOOST_AUTO_TEST_CASE(ValidSeparators1)
{
  BOOST_CHECK(is_separator('('));
  BOOST_CHECK(is_separator(')'));
  BOOST_CHECK(is_separator('<'));
  BOOST_CHECK(is_separator('>'));
  BOOST_CHECK(is_separator('@'));

  BOOST_CHECK(is_separator(','));
  BOOST_CHECK(is_separator(';'));
  BOOST_CHECK(is_separator(':'));
  BOOST_CHECK(is_separator('\\'));
  BOOST_CHECK(is_separator('"'));

  BOOST_CHECK(is_separator('/'));
  BOOST_CHECK(is_separator('['));
  BOOST_CHECK(is_separator(']'));
  BOOST_CHECK(is_separator('?'));
  BOOST_CHECK(is_separator('='));

  BOOST_CHECK(is_separator('{'));
  BOOST_CHECK(is_separator('}'));
  BOOST_CHECK(is_separator(' '));
  BOOST_CHECK(is_separator('\t'));

  BOOST_CHECK(!is_separator('1'));
  BOOST_CHECK(!is_separator('a'));
}

BOOST_AUTO_TEST_CASE(ValidPctEncoded1)
{
  BOOST_CHECK(is_pct_encoded("%AF"));
  BOOST_CHECK(is_pct_encoded("%af"));
  BOOST_CHECK(!is_pct_encoded("zxf"));
  BOOST_CHECK(!is_pct_encoded("123"));
}

BOOST_AUTO_TEST_CASE(ValidGenDelimitor1)
{
  BOOST_CHECK(is_gen_delim(':'));
  BOOST_CHECK(is_gen_delim('/'));
  BOOST_CHECK(is_gen_delim('?'));
  BOOST_CHECK(is_gen_delim('#'));
  BOOST_CHECK(is_gen_delim('['));
  BOOST_CHECK(is_gen_delim(']'));
  BOOST_CHECK(is_gen_delim('@'));

  BOOST_CHECK(!is_gen_delim('{'));
  BOOST_CHECK(!is_gen_delim('}'));
  BOOST_CHECK(!is_gen_delim(' '));
  BOOST_CHECK(!is_gen_delim('\t'));

  BOOST_CHECK(!is_gen_delim('1'));
  BOOST_CHECK(!is_gen_delim('a'));
}

BOOST_AUTO_TEST_CASE(ValidSubDelimitor1)
{
  BOOST_CHECK(is_sub_delim('!'));
  BOOST_CHECK(is_sub_delim('$'));
  BOOST_CHECK(is_sub_delim('&'));
  BOOST_CHECK(is_sub_delim('\''));
  BOOST_CHECK(is_sub_delim('('));
  BOOST_CHECK(is_sub_delim(')'));
  BOOST_CHECK(is_sub_delim('*'));
  BOOST_CHECK(is_sub_delim('+'));
  BOOST_CHECK(is_sub_delim(','));
  BOOST_CHECK(is_sub_delim(';'));
  BOOST_CHECK(is_sub_delim('='));

  BOOST_CHECK(!is_sub_delim('['));
  BOOST_CHECK(!is_sub_delim(']'));
  BOOST_CHECK(!is_sub_delim(' '));
  BOOST_CHECK(!is_sub_delim('\t'));

  BOOST_CHECK(!is_sub_delim('1'));
  BOOST_CHECK(!is_sub_delim('a'));
}

BOOST_AUTO_TEST_CASE(ValidReserved1)
{
  BOOST_CHECK(is_reserved(':'));
  BOOST_CHECK(is_reserved('/'));
  BOOST_CHECK(is_reserved('?'));
  BOOST_CHECK(is_reserved('#'));
  BOOST_CHECK(is_reserved('['));
  BOOST_CHECK(is_reserved(']'));
  BOOST_CHECK(is_reserved('@'));

  BOOST_CHECK(is_reserved('!'));
  BOOST_CHECK(is_reserved('$'));
  BOOST_CHECK(is_reserved('&'));
  BOOST_CHECK(is_reserved('\''));
  BOOST_CHECK(is_reserved('('));
  BOOST_CHECK(is_reserved(')'));
  BOOST_CHECK(is_reserved('*'));
  BOOST_CHECK(is_reserved('+'));
  BOOST_CHECK(is_reserved(','));
  BOOST_CHECK(is_reserved(';'));
  BOOST_CHECK(is_reserved('='));

  BOOST_CHECK(!is_reserved(' '));
  BOOST_CHECK(!is_reserved('\t'));

  BOOST_CHECK(!is_reserved('1'));
  BOOST_CHECK(!is_reserved('a'));
}

BOOST_AUTO_TEST_CASE(ValidUnreserved1)
{
  BOOST_CHECK(is_unreserved('-'));
  BOOST_CHECK(is_unreserved('.'));
  BOOST_CHECK(is_unreserved('_'));
  BOOST_CHECK(is_unreserved('~'));
  BOOST_CHECK(is_unreserved('a'));
  BOOST_CHECK(is_unreserved('Z'));
  BOOST_CHECK(is_unreserved('0'));

  BOOST_CHECK(!is_unreserved(' '));
  BOOST_CHECK(!is_unreserved('\t'));

  BOOST_CHECK(!is_unreserved('!'));
  BOOST_CHECK(!is_unreserved('$'));
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestHexString)

BOOST_AUTO_TEST_CASE(ValidDigits1)
{
  std::string hex_string("10");
  size_t value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(16U, value);
}

BOOST_AUTO_TEST_CASE(ValidHex1)
{
  std::string hex_string("abcdef");
  size_t value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(0xABCDEFU, value);
}

BOOST_AUTO_TEST_CASE(InvalidHex1)
{
  std::string hex_string("0xabcdef");
  auto value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(-1, value);
}

BOOST_AUTO_TEST_CASE(InvalidHex2)
{
  std::string hex_string("abcdefx");
  auto value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(-1, value);
}

BOOST_AUTO_TEST_CASE(InvalidHex3)
{
  std::string hex_string("fffffffffffffffff");
  auto value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(-1, value);
}

BOOST_AUTO_TEST_CASE(InvalidHex4)
{
  std::string hex_string("");
  size_t value(from_hex_string(hex_string));

  BOOST_CHECK_EQUAL(std::numeric_limits<size_t>::max(), value);
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestDecString)

BOOST_AUTO_TEST_CASE(ValidDigits1)
{
  std::string dec_string("10");
  size_t value(from_dec_string(dec_string));

  BOOST_CHECK_EQUAL(10U, value);
}

BOOST_AUTO_TEST_CASE(ValidDec1)
{
  std::string dec_string("0");
  size_t value(from_dec_string(dec_string));

  BOOST_CHECK_EQUAL(0U, value);
}

BOOST_AUTO_TEST_CASE(InvalidDec1)
{
  std::string dec_string("123456x");
  auto value(from_dec_string(dec_string));

  BOOST_CHECK_EQUAL(-1, value);
}

BOOST_AUTO_TEST_CASE(InvalidDec2)
{
  std::string dec_string("10000000000000000000");
  auto value(from_dec_string(dec_string));

  BOOST_CHECK_EQUAL(-1, value);
}

BOOST_AUTO_TEST_CASE(InvalidDec3)
{
  std::string dec_string("");
  auto value(from_dec_string(dec_string));

  BOOST_CHECK_EQUAL(-1, value);
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
