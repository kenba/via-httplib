#pragma once

#ifndef BUFFERED_CONNECTION_HPP_VIA_HTTPLIB_
#define BUFFERED_CONNECTION_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "connection.hpp"
#include <deque>
#include <cassert>

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class buffered_connection
    /// This class adds queues to the connection class to manage the
    /// read and write buffers.
    //////////////////////////////////////////////////////////////////////////
    template <typename Container>
    class buffered_connection : public connection
    {
      std::deque<Container> rx_queue_;
      std::deque<Container> tx_queue_;
      size_t buffer_size_;
      bool is_writing_;

      static const size_t DEFAULT_BUFFER_SIZE = 8192;

    protected:

      ///
      virtual void read_handler(const boost::system::error_code& error,
                                size_t bytes_transferred)
      {
        if (error)
          signal_error(error);
        else
        {
          rx_queue_.back().resize(bytes_transferred);
          enable_reception();
          signal_received();
        }
      }

      ///
      virtual void write_handler(const boost::system::error_code& error,
                                size_t) // bytes_transferred
      {
        tx_queue_.pop_front();
        is_writing_ = false;

        if (error)
          signal_error(error);
        else
        {
          if (!tx_queue_.empty())
          {
            is_writing_ = true;
            write(&tx_queue_.front()[0], tx_queue_.front().size());
          }
          else
            signal_sent();
        }
      }

      ///
      explicit buffered_connection(size_t buffer_size = DEFAULT_BUFFER_SIZE) :
        connection(),
        rx_queue_(),
        tx_queue_(),
        buffer_size_(buffer_size),
        is_writing_(false)
      {}

      void set_buffer_size(size_t buffer_size)
      { buffer_size_ = buffer_size; }

    public:

      ///
      virtual void enable_reception()
      {
        // Note: this could be a move in C++11
        rx_queue_.push_back(Container(buffer_size_, 0));
        size_t size(buffer_size_);
        read(&rx_queue_.back()[0], size);
      }

      ///
      bool read_pending() const
      { return rx_queue_.size() > 1; }

      ///
      Container read_data()
      {
        assert(read_pending());
        Container data(rx_queue_.front());
        rx_queue_.pop_front();
        return data;
      }

      // Note: provide move sematics for C++11
      void send_data(Container const& packet)
      {
        tx_queue_.push_back(packet);
        if (!is_writing_)
        {
          is_writing_ = true;
          write(&tx_queue_.front()[0], tx_queue_.front().size());
        }
      }

      ///
      template<typename ForwardIterator1, typename ForwardIterator2>
      void send_data(ForwardIterator1 begin, ForwardIterator2 end)
      {
        Container buffer(begin, end);
        send_data(buffer);
      }


    };

  }
}
#endif
