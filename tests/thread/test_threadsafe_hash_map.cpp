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
  BOOST_CHECK_EQUAL(19u, test_hash_map.bucket_count());

  typedef threadsafe_hash_map<int, int>::value_type vt;

  // Test an empty map
  BOOST_CHECK(test_hash_map.empty());
  auto bucket_data(test_hash_map.data());
  BOOST_CHECK_EQUAL(0u, bucket_data.size());

  auto test_value(test_hash_map.find(1));
  BOOST_CHECK_EQUAL(0, test_value.first);
  BOOST_CHECK_EQUAL(0, test_value.second);

  test_value = test_hash_map.find(1, vt(-2, -1));
  BOOST_CHECK_EQUAL(-2, test_value.first);
  BOOST_CHECK_EQUAL(-1, test_value.second);

  // Add a key, value pair
  test_hash_map.insert(vt(1, 10));
  BOOST_CHECK(!test_hash_map.empty());
  bucket_data = test_hash_map.data();
  BOOST_CHECK_EQUAL(1u, bucket_data.size());

  BOOST_CHECK_EQUAL(10, test_hash_map.find(1).second);
  BOOST_CHECK_EQUAL(0, test_hash_map.find(2).second);
  BOOST_CHECK_EQUAL(-1, test_hash_map.find(2, vt(-2, -1)).second);

  // Add another key, value pair
  test_hash_map.insert(vt(2, 20));
  BOOST_CHECK(!test_hash_map.empty());
  bucket_data = test_hash_map.data();
  BOOST_CHECK_EQUAL(2u, bucket_data.size());

  BOOST_CHECK_EQUAL(10, test_hash_map.find(1).second);
  BOOST_CHECK_EQUAL(20, test_hash_map.find(2).second);
  BOOST_CHECK_EQUAL(0, test_hash_map.find(21).second);

  // Add another key, value pair at the same hash as 2
  test_hash_map.emplace(21, 210);
  BOOST_CHECK(!test_hash_map.empty());
  bucket_data = test_hash_map.data();
  BOOST_CHECK_EQUAL(3u, bucket_data.size());

  BOOST_CHECK_EQUAL(10, test_hash_map.find(1).second);
  BOOST_CHECK_EQUAL(20, test_hash_map.find(2).second);
  BOOST_CHECK_EQUAL(210, test_hash_map.find(21).second);
  BOOST_CHECK_EQUAL(-1, test_hash_map.find(31, vt(-2,-1)).second);

  // Change a value
  test_hash_map.insert(vt(2, 200));
  BOOST_CHECK_EQUAL(200, test_hash_map.find(2).second);
  bucket_data = test_hash_map.data();
  BOOST_CHECK_EQUAL(3u, bucket_data.size());

  // Remove a value
  test_hash_map.erase(2);
  BOOST_CHECK_EQUAL(0, test_hash_map.find(2).second);
  bucket_data = test_hash_map.data();
  BOOST_CHECK_EQUAL(2u, bucket_data.size());

  // Clear the map
  test_hash_map.clear();
  BOOST_CHECK(test_hash_map.empty());
  bucket_data = test_hash_map.data();
  BOOST_CHECK_EQUAL(0u, bucket_data.size());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
