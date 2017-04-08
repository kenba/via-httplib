//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/thread/threadsafe_hash_map.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace via::thread;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(Test_Threadsafe_Hash_Map)

BOOST_AUTO_TEST_CASE(Default_Single_Threaded_1)
{
  threadsafe_hash_map<int, int> test_hash_map;

  // Test an empty map
  BOOST_CHECK(test_hash_map.empty());
  BOOST_CHECK_EQUAL(0, test_hash_map.value_for(1));
  BOOST_CHECK_EQUAL(-1, test_hash_map.value_for(1, -1));

  // Add a key, value pair
  test_hash_map.add_or_update_mapping(1, 10);
  BOOST_CHECK(!test_hash_map.empty());

  BOOST_CHECK_EQUAL(10, test_hash_map.value_for(1));
  BOOST_CHECK_EQUAL(0, test_hash_map.value_for(2));
  BOOST_CHECK_EQUAL(-1, test_hash_map.value_for(2, -1));

  // Add another key, value pair
  test_hash_map.add_or_update_mapping(2, 20);
  BOOST_CHECK(!test_hash_map.empty());

  BOOST_CHECK_EQUAL(10, test_hash_map.value_for(1));
  BOOST_CHECK_EQUAL(20, test_hash_map.value_for(2));
  BOOST_CHECK_EQUAL(0, test_hash_map.value_for(21));

  // Add another key, value pair at the same hash as 2
  test_hash_map.add_or_update_mapping(21, 210);
  BOOST_CHECK(!test_hash_map.empty());

  BOOST_CHECK_EQUAL(10, test_hash_map.value_for(1));
  BOOST_CHECK_EQUAL(20, test_hash_map.value_for(2));
  BOOST_CHECK_EQUAL(210, test_hash_map.value_for(21));
  BOOST_CHECK_EQUAL(-1, test_hash_map.value_for(31, -1));

  // Change a value
  test_hash_map.add_or_update_mapping(2, 200);
  BOOST_CHECK_EQUAL(200, test_hash_map.value_for(2));

  // Remove a value
  test_hash_map.remove_mapping(2);
  BOOST_CHECK_EQUAL(0, test_hash_map.value_for(2));

  // Clear the map
  test_hash_map.clear();
  BOOST_CHECK(test_hash_map.empty());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
