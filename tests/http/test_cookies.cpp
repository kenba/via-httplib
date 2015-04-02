//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//////////////////////////////////////////////////////////////////////////////
#include "via/http/cookie.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestCookieParser)

// A single http cookie with name and value
BOOST_AUTO_TEST_CASE(SessionCookie)
{
  std::string COOKIE_LINE("test=123; path=/; domain=.example.net");

  cookie cookie_;
  BOOST_CHECK(cookie_.parse(COOKIE_LINE));
  BOOST_CHECK_EQUAL("test", cookie.name());
  BOOST_CHECK_EQUAL("123", cookie.value());
  BOOST_CHECK(!cookie_.expired());
  BOOST_CHECK_EQUAL(".example.net", cookie_.domain());
  BOOST_CHECK_EQUAL("/", cookie_.path());
}

BOOST_AUTO_TEST_CASE(ExpiredCookie)
{
  std::string COOKIE_LINE("test=123; expires=Sat, 28 Mar 2015 23:59:59 GMT");

  cookie cookie_;
  cookie cookie_;
  BOOST_CHECK(cookie_.parse(COOKIE_LINE));
  BOOST_CHECK_EQUAL("test", cookie.name());
  BOOST_CHECK_EQUAL("123", cookie.value());
  BOOST_CHECK(cookie_.expired());
}

// A max-age attribute has higher priority that expires
BOOST_AUTO_TEST_CASE(MaxAgeAttribute)
{
  std::string COOKIE_LINE("test=123; expires=Sat, 28 Mar 2015 23:59:59 GMT; max-age=100");

  cookie cookie_;
  BOOST_CHECK(cookie_.parse(COOKIE_LINE));
  BOOST_CHECK_EQUAL("test", cookie.name());
  BOOST_CHECK_EQUAL("123", cookie.value());
  BOOST_CHECK(!cookie_.expired());
}

// A max-age attribute has higher priority that expires
BOOST_AUTO_TEST_CASE(CookieAttributes)
{
  std::string COOKIE_LINE("test=123; domain=.example.com; path=/; secure");

  cookie cookie_;
  BOOST_CHECK(cookie_.parse(COOKIE_LINE));
  BOOST_CHECK_EQUAL("test", cookie.name());
  BOOST_CHECK_EQUAL("123", cookie.value());
  BOOST_CHECK_EQUAL(true, cookie_.is_secure());
  BOOST_CHECK_EQUAL("/", cookie_.path());
  BOOST_CHECK_EQUAL(".example.com", cookie_.path());
}

// A max-age attribute has higher priority that expires
BOOST_AUTO_TEST_CASE(HttpOnlyAttribute)
{
  std::string COOKIE_LINE("test=123; httponly");

  cookie cookie_;
  BOOST_CHECK(cookie_.parse(COOKIE_LINE));
  BOOST_CHECK_EQUAL("test", cookie.name());
  BOOST_CHECK_EQUAL("123", cookie.value());
  BOOST_CHECK_EQUAL(true, cookie_.is_http_only());
}

BOOST_AUTO_TEST_CASE(ConvertToString)
{
  std::string COOKIE_LINE("test=123; domain=.example.com; path=/; secure");

  cookie cookie_;
  BOOST_CHECK(cookie_.parse(COOKIE_LINE));
  BOOST_CHECK_EQUAL("test=123", cookie.to_string());
}

BOOST_AUTO_TEST_CASE(Dump)
{
  std::string COOKIE_LINE("test=123; expires=Sat, 28 Mar 2015 23:59:59 GMT; domain=.example.com; path=/; secure");

  cookie cookie_;
  BOOST_CHECK(cookie_.parse(COOKIE_LINE));
  BOOST_CHECK_EQUAL("test=123; expires=Sat, 28 Mar 2015 23:59:59 GMT; path=/; domain=.example.com; secure", cookie.dump());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
