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

//////////////////////////////////////////////////////////////////////////////
