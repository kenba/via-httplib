#ifndef VIA_THREADSAFE_HASH_MAP_HPP_
#define VIA_THREADSAFE_HASH_MAP_HPP_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file threadsafe_hash_map.hpp
/// @brief An implementation of a concurrent hash map for C++.
/// The code is a modified version of threadsafe_lookup_table from
/// Chapter 6 of C++ Concurrency In Action by Anthony Williams.
//////////////////////////////////////////////////////////////////////////////
#include <utility>
#include <vector>
#include <array>
#include <algorithm>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

namespace via
{
  namespace thread
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class threadsafe_hash_map
    ///
    /// A thread hash map which uses shared_mutex's for thread safety.
    /// It has a number of data buckets each protected by it's own shared_mutex.
    /// The number of data buckets can be set in the class constructor.
    /// The more buckets, the lower the probability of thread contention...
    ///
    /// @tparam Key the map key type.
    /// @tparam Value the map value type.
    /// @tparam Hash the hash function for the Key. Default std::hash<Key>.
    /// @tparam num_buckets the number of data buckets.
    /// It should be a prime number, default 19.
    /// @tparam Alloc the allocator for the bucket_data_type.
    /// Default std::allocator.
    /// @tparam cache_line_size the size of a cache line on the hardware.
    /// Default 64 bytes.
    //////////////////////////////////////////////////////////////////////////
    template<typename Key, typename Value,
             typename Hash = std::hash<Key>,
             unsigned num_buckets = 19,
             typename Alloc = std::allocator<std::pair<Key, Value>>,
             unsigned cache_line_size = 64u>
    class threadsafe_hash_map
    {
      /// @class bucket_type
      /// Each bucket is a std::vector of Key, Value std::pair's sorted by Key
      /// and protected by a shared_mutex.
      /// The shared_mutex enables mulutiple simultaneous readers per bucket.
      struct alignas(cache_line_size) bucket_type
      {
        /// The underlying data type: a key, value pair.
        typedef std::pair<Key, Value> value_type;

        /// The container for storing values.
        typedef std::vector<value_type, Alloc> bucket_data_type;

        /// An iterator into the bucket container.
        typedef typename bucket_data_type::iterator bucket_iterator;

        //////////////////////////////////////////////////////////////////////
        // Data

        /// A shared_mutex to protect the bucket data.
        mutable boost::shared_mutex mutex_;

        /// The bucket data.
        bucket_data_type data_;

        //////////////////////////////////////////////////////////////////////
        // Functions

        /// Find the position to insert / overwrite a key value pair.
        /// Calls lower_bound to find the current position of the key,
        /// or where a new key, value pair would be inserted.
        /// @param key the search key.
        /// @return an iterator to the key, value pair or where a new pair
        /// should be inserted.
        bucket_iterator find_position_for(Key const& key)
        {
          bucket_iterator iter(std::lower_bound(data_.begin(), data_.end(), key,
          [](value_type const& item, Key const& key){ return item.first < key; }));

          return iter;
        }

        /// Constructor, default.
        bucket_type() = default;

        /// Destructor, default.
        ~bucket_type() = default;

        /// Get the value for the given Key.
        /// @param key the search key.
        /// @param default_value the value to return if key wasn't found.
        /// @return the value if key is found, default_value otherwise.
        value_type value_for(Key const& key, value_type const& default_value) const
        {
          // Allow multiple readers.
          boost::shared_lock<boost::shared_mutex> guard(mutex_);

          // Note: can't use find_position_for becasue it isn't const
          auto iter(std::lower_bound(data_.cbegin(), data_.cend(), key,
          [](value_type const& item, Key const& key){ return item.first < key; }));

          return ((iter != data_.cend()) && (iter->first == key)) ?
                   *iter : default_value;
        }

        /// Add or update a key, value entry.
        /// @param key the search key.
        /// @param value the value for the key.
        void add_or_update_mapping(value_type value)
        {
          // Only allow one writer.
          std::lock_guard<boost::shared_mutex> guard(mutex_);

          auto iter(find_position_for(value.first));
          if((iter != data_.end()) && (iter->first == value.first))
            *iter = std::move(value);
          else
            data_.emplace(iter, std::move(value));
        }

        /// Remove the key entry.
        /// @param key the search key.
        void remove_mapping(Key key)
        {
          std::lock_guard<boost::shared_mutex> guard(mutex_);

          auto iter(find_position_for(key));
          if(iter != data_.end())
            data_.erase(iter);
        }
      };

      ////////////////////////////////////////////////////////////////////////
      // Data

      /// The hash function.
      Hash hasher_;

      /// The data buckets.
      std::array<bucket_type, num_buckets> buckets_;

      ////////////////////////////////////////////////////////////////////////
      // Functions

      /// Get the index of the data bucket for the key.
      /// @param key the search key.
      /// @return the index of the dat bucket in the array.
      std::size_t get_bucket_index(Key const& key) const
      { return hasher_(key) % buckets_.size(); }

    public:

      /// The underlying data type: a key, value pair.
      typedef typename bucket_type::value_type value_type;

      /// Constructor
      /// @param hasher the hash function, default the template hash function.
      threadsafe_hash_map(Hash const& hasher = Hash())
        : hasher_(hasher)
        , buckets_()
      {}

      /// Destructor, default.
      ~threadsafe_hash_map() = default;

      /// Disable copy construction.
      threadsafe_hash_map(threadsafe_hash_map const& other) = delete;

      /// Disable assignment.
      threadsafe_hash_map& operator=(threadsafe_hash_map const& other) = delete;

      /// The number of buckets.
      std::size_t bucket_count() const noexcept
      { return buckets_.size(); }

      /// Find the value for the given Key, returns default_value if not found.
      /// @param key the search key.
      /// @param default_value the value to return if key wasn't found,
      /// default, the default value for the Value type.
      /// @return the value if key is found, default_value otherwise.
      value_type find(Key const& key,
                      value_type const& default_value = value_type()) const
      {
        auto index(get_bucket_index(key));
        return buckets_[index].value_for(key, default_value);
      }

      /// Insert or update a key, value entry.
      /// @param key the search key.
      /// @param value the value for the key.
      void insert(value_type value)
      {
        auto index(get_bucket_index(value.first));
        buckets_[index].add_or_update_mapping(std::move(value));
      }

      /// Emplace or update a key, value entry.
      /// @param key the search key.
      /// @param value the value for the key.
      void emplace(Key key, Value value)
      { insert(value_type(key, value)); }

      /// Remove the key entry.
      /// @param key the search key.
      void erase(Key key)
      {
        auto index(get_bucket_index(key));
        buckets_[index].remove_mapping(std::move(key));
      }

      /// Determine whether the collection is empty.
      /// Note: it locks all of the buckets for reading before reading them.
      /// @return true if all of the buckets are empty, false otherwise.
      bool empty() const
      {
        // lock all of the buckets for reading
        std::vector<boost::shared_lock<boost::shared_mutex>> locks;
        locks.reserve(buckets_.size());
        for(auto& elem : buckets_)
          locks.push_back
              (boost::shared_lock<boost::shared_mutex>(elem.mutex_));

        for(auto const& elem : buckets_)
        {
          if (!elem.data_.empty())
            return false;
        }

        return true;
      }

      /// Take a snapshot of the collection.
      /// Note: it locks all of the buckets for reading before reading them.
      /// @return a copy of the bucket's contents.
      std::vector<value_type> data() const
      {
        // lock all of the buckets for reading
        std::vector<boost::shared_lock<boost::shared_mutex>> locks;
        locks.reserve(buckets_.size());
        for(auto& elem : buckets_)
          locks.push_back
              (boost::shared_lock<boost::shared_mutex>(elem.mutex_));

        std::vector<value_type> bucket_data;
        for(auto const& bucket : buckets_)
        {
          if (!bucket.data_.empty())
          {
            for (auto elem : bucket.data_)
              bucket_data.push_back(std::move(elem));
          }
        }

        return bucket_data;
      }

      /// Clear the collection.
      /// Note: it locks all of the buckets for writing before clearing them.
      void clear()
      {
        // lock all of the buckets for writing
        std::vector<boost::unique_lock<boost::shared_mutex>> locks;
        locks.reserve(buckets_.size());
        for(auto& elem : buckets_)
          locks.push_back(boost::unique_lock<boost::shared_mutex>(elem.mutex_));

        for(auto& elem : buckets_)
          elem.data_.clear();
      }

    };
  }
}

#endif
