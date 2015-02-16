//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/header_field.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <iostream>
#include <cstring>

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestHeaderFields)

BOOST_AUTO_TEST_CASE(NamesEqual)
{
  for (int i(static_cast<int>(header_field::id::CACHE_CONTROL));
       i <= static_cast<int>(header_field::id::EXTENSION_HEADER); ++i)
  {
    header_field::id field_id(static_cast<header_field::id>(i));
    std::string lowerName(boost::algorithm::to_lower_copy(standard_name(field_id)));

    BOOST_CHECK_EQUAL(lowercase_name(field_id).c_str(), lowerName.c_str());
  }
}

BOOST_AUTO_TEST_CASE(ToHeaderString)
{
  std::string name("Accept-Charset");
  std::string value("ISO-8859-1");
  std::string line("Accept-Charset: ISO-8859-1\r\n");

  std::string result(header_field::to_header(name, value));

  BOOST_CHECK_EQUAL(line.c_str(), result.c_str());
}

BOOST_AUTO_TEST_CASE(ToHeaderEnum)
{
  std::string value("ISO-8859-1");
  std::string line("Accept-Charset: ISO-8859-1\r\n");

  std::string result(header_field::to_header(header_field::id::ACCEPT_CHARSET, value));

  BOOST_CHECK_EQUAL(line.c_str(), result.c_str());
}

BOOST_AUTO_TEST_CASE(ToHeaderDate)
{
  std::string start("Date: ");
  std::string end(" GMT\r\n");

  std::string result(header_field::date_header());

  BOOST_CHECK(!memcmp(start.c_str(), result.c_str(), start.size()));
  BOOST_CHECK(!memcmp(end.c_str(), result.c_str() + 31, end.size()));
}

BOOST_AUTO_TEST_CASE(ServerHeader)
{
  std::string line("Server: Via-httplib/1.1.2\r\n");

  std::string result(header_field::server_header());
  BOOST_CHECK_EQUAL(line.c_str(), result.c_str());
}

BOOST_AUTO_TEST_CASE(ContentHttpHeader)
{
  std::string line("Content-Type: message/http\r\n");

  std::string result(header_field::content_http_header());
  BOOST_CHECK_EQUAL(line.c_str(), result.c_str());
}

BOOST_AUTO_TEST_CASE(ContentLengthHeader)
{
  size_t size(1234);
  std::string line("Content-Length: 1234\r\n");

  std::string result(header_field::content_length(size));
  BOOST_CHECK_EQUAL(line.c_str(), result.c_str());
}

BOOST_AUTO_TEST_CASE(ChunkedHeader)
{
  std::string line("Transfer-Encoding: Chunked\r\n");

  std::string result(header_field::chunked_encoding());
  BOOST_CHECK_EQUAL(line.c_str(), result.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
