//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/request_uri.hpp"
#include <boost/test/unit_test.hpp>

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestUri)

BOOST_AUTO_TEST_CASE(PathUri1)
{
  // Just a path
  std::string input("/docs/pdf");

  request_uri test_uri(input);

  BOOST_CHECK_EQUAL(input, test_uri.path());
  BOOST_CHECK(test_uri.query().empty());
  BOOST_CHECK(test_uri.fragment().empty());
}

BOOST_AUTO_TEST_CASE(PathQueryUri1)
{
  // A path and a query
  std::string input("/docs/pdf?rfc3986");
  std::string result_path("/docs/pdf");
  std::string result_query("rfc3986");

  request_uri test_uri(input);

  BOOST_CHECK_EQUAL(result_path, test_uri.path());
  BOOST_CHECK_EQUAL(result_query, test_uri.query());
  BOOST_CHECK(test_uri.fragment().empty());
}

BOOST_AUTO_TEST_CASE(PathFragmentUri1)
{
  // A path and a fragment
  std::string input("/docs/pdf#Section1");
  std::string result_path("/docs/pdf");
  std::string result_fragment("Section1");

  request_uri test_uri(input);

  BOOST_CHECK_EQUAL(result_path, test_uri.path());
  BOOST_CHECK(test_uri.query().empty());
  BOOST_CHECK_EQUAL(result_fragment, test_uri.fragment());
}

BOOST_AUTO_TEST_CASE(PathFragmentUri2)
{
  // A path and a fragment containing a question mark
  std::string input("/docs/pdf#Section1?rfc3986");
  std::string result_path("/docs/pdf");
  std::string result_fragment("Section1?rfc3986");

  request_uri test_uri(input);

  BOOST_CHECK_EQUAL(result_path, test_uri.path());
  BOOST_CHECK(test_uri.query().empty());
  BOOST_CHECK_EQUAL(result_fragment, test_uri.fragment());
}

BOOST_AUTO_TEST_CASE(PathQueryFragmentUri1)
{
  std::string input("/docs/pdf?rfc3986#Section1");
  std::string result_path("/docs/pdf");
  std::string result_query("rfc3986");
  std::string result_fragment("Section1");

  request_uri test_uri(input);

  BOOST_CHECK_EQUAL(result_path, test_uri.path());
  BOOST_CHECK_EQUAL(result_query, test_uri.query());
  BOOST_CHECK_EQUAL(result_fragment, test_uri.fragment());
}

BOOST_AUTO_TEST_CASE(PathQueryFragmentUri2)
{
  std::string input("/docs/pdf?rfc3986#Section1?Section2");
  std::string result_path("/docs/pdf");
  std::string result_query("rfc3986");
  std::string result_fragment("Section1?Section2");

  request_uri test_uri(input);

  BOOST_CHECK_EQUAL(result_path, test_uri.path());
  BOOST_CHECK_EQUAL(result_query, test_uri.query());
  BOOST_CHECK_EQUAL(result_fragment, test_uri.fragment());
}

BOOST_AUTO_TEST_CASE(PathQueryFragmentUri3)
{
  std::string input("/docs/pdf?rfc3986#Section1#Section2");
  std::string result_path("/docs/pdf");
  std::string result_query("rfc3986");
  std::string result_fragment("Section1#Section2");

  request_uri test_uri(input);

  BOOST_CHECK_EQUAL(result_path, test_uri.path());
  BOOST_CHECK_EQUAL(result_query, test_uri.query());
  BOOST_CHECK_EQUAL(result_fragment, test_uri.fragment());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
