//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "../../via/http/header_field.hpp"
#include <boost/algorithm/string.hpp>
#include <vector>
#include <iostream>
#include <cstring>

#ifdef _MSC_VER
// cpputest\MemoryLeakWarningPlugin.h: C++11 exception specification deprecated
#pragma warning (disable:4290)
#endif
#include "CppUTest/TestHarness.h"

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestHeaderFields)
{
};

TEST(TestHeaderFields, NamesEqual)
{
  for (int i(header_field::CACHE_CONTROL);
       i <= header_field::EXTENSION_HEADER; ++i)
  {
    header_field::field_id id(static_cast<header_field::field_id>(i));
    std::string lowerName(boost::algorithm::to_lower_copy(standard_name(id)));

    STRCMP_EQUAL(lowercase_name(id).c_str(), lowerName.c_str());
  }
}

TEST(TestHeaderFields, ToHeaderString)
{
  std::string name("Accept-Charset");
  std::string value("ISO-8859-1");
  std::string line("Accept-Charset: ISO-8859-1\r\n");

  std::string result(header_field::to_header(name, value));

  STRCMP_EQUAL(line.c_str(), result.c_str());
}

TEST(TestHeaderFields, ToHeaderEnum)
{
  std::string value("ISO-8859-1");
  std::string line("Accept-Charset: ISO-8859-1\r\n");

  std::string result(header_field::to_header(header_field::ACCEPT_CHARSET, value));

  STRCMP_EQUAL(line.c_str(), result.c_str());
}

TEST(TestHeaderFields, ToHeaderDate)
{
  std::string start("Date: ");
  std::string end(" GMT\r\n");

  std::string result(header_field::date_header());

  CHECK(!memcmp(start.c_str(), result.c_str(), start.size()));
  CHECK(!memcmp(end.c_str(), result.c_str() + 31, end.size()));
}

TEST(TestHeaderFields, ContentLengthHeader)
{
  size_t size(1234);
  std::string line("Content-Length: 1234\r\n");

  std::string result(header_field::content_length(size));
  STRCMP_EQUAL(line.c_str(), result.c_str());
}

TEST(TestHeaderFields, ChunkedHeader)
{
  std::string line("Transfer-Encoding: Chunked\r\n");

  std::string result(header_field::chunked_encoding());
  STRCMP_EQUAL(line.c_str(), result.c_str());
}

//////////////////////////////////////////////////////////////////////////////
