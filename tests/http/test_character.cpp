//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "../../via/http/character.hpp"
#include <vector>
#include <iostream>

#ifdef _MSC_VER
// cpputest\MemoryLeakWarningPlugin.h: C++11 exception specification deprecated
#pragma warning (disable:4290)
#endif
#include "CppUTest/TestHarness.h"

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestHexString)
{
};

TEST(TestHexString, ValidDigits1)
{
  std::string hex_string("10");
  auto value(from_hex_string(hex_string));

  CHECK_EQUAL(16, value);
}

TEST(TestHexString, ValidHex1)
{
  std::string hex_string("abcdef");
  auto value(from_hex_string(hex_string));

  CHECK_EQUAL(0xABCDEF, value);
}

TEST(TestHexString, InvalidHex1)
{
  std::string hex_string("0xabcdef");
  auto value(from_hex_string(hex_string));

  CHECK_EQUAL(size_t(ULONG_MAX), value);
}

TEST(TestHexString, InvalidHex2)
{
  std::string hex_string("abcdefx");
  auto value(from_hex_string(hex_string));

  CHECK_EQUAL(size_t(ULONG_MAX), value);
}

TEST(TestHexString, InvalidHex3)
{
  std::string hex_string("ffffffffff");
  auto value(from_hex_string(hex_string));

  CHECK_EQUAL(size_t(ULONG_MAX), value);
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestDecString)
{
};

TEST(TestDecString, ValidDigits1)
{
  std::string dec_string("10");
  auto value(from_dec_string(dec_string));

  CHECK_EQUAL(10, value);
}

TEST(TestDecString, ValidDec1)
{
  std::string dec_string("0");
  auto value(from_dec_string(dec_string));

  CHECK_EQUAL(0, value);
}

TEST(TestDecString, InvalidDec1)
{
  std::string dec_string("123456x");
  auto value(from_dec_string(dec_string));

  CHECK_EQUAL(size_t(ULONG_MAX), value);
}

TEST(TestDecString, InvalidDec2)
{
  std::string dec_string("10000000000000000");
  auto value(from_dec_string(dec_string));

  CHECK_EQUAL(size_t(ULONG_MAX), value);
}
