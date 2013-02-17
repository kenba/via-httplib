#pragma once

#ifndef CONNECTION_HPP_VIA_HTTPLIB_
#define CONNECTION_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "socket_adaptor.hpp"
#include <boost/system/error_code.hpp>
#include <boost/signal.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>
#include <deque>

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class
    /// This class
    //////////////////////////////////////////////////////////////////////////
    template <typename SocketAdaptor,
              typename Container = std::vector<char> >
    class connection : public SocketAdaptor,
        public boost::enable_shared_from_this
            <connection<SocketAdaptor, Container> >
    {
    public:

      typedef typename boost::weak_ptr<connection<SocketAdaptor, Container> >
         weak_pointer;

      typedef typename boost::shared_ptr<connection<SocketAdaptor, Container> >
         shared_pointer;

      typedef typename boost::enable_shared_from_this<connection
        <SocketAdaptor, Container> > enable;

      /// A signal to indicate that an event occured.
      typedef boost::signal<void (int event, weak_pointer)> event_signal_type;

      typedef typename event_signal_type::slot_type event_slot_type;

      /// A signal to indicate that an error occured.
      typedef boost::signal<void (const boost::system::error_code&, weak_pointer)>
                                  error_signal_type;

      typedef typename error_signal_type::slot_type error_slot_type;

    private:

      std::deque<Container> rx_queue_;
      std::deque<Container> tx_queue_;
      bool is_writing_;

      size_t buffer_size_;
      static const size_t DEFAULT_BUFFER_SIZE = 8192;

      event_signal_type signal_event_;
      error_signal_type signal_error_;

      void signal_event(int event)
      {
        if (event == CONNECTED)
          enable_reception();

        signal_event_(event, enable::shared_from_this());
      }

      void signal_error(const boost::system::error_code& error)
      {
        if (SocketAdaptor::is_disconnect(error))
          signal_event_(DISCONNECTED, enable::shared_from_this());
        else
          signal_error_(error, enable::shared_from_this());
      }

      void read_handler(const boost::system::error_code& error,
                        size_t bytes_transferred)
      {
        if (!error)
        {
          rx_queue_.back().resize(bytes_transferred);
          enable_reception();
          signal_event(RECEIVED);
        }
        else
          signal_error(error);
      }

      void write_handler(const boost::system::error_code& error,
                         size_t) // bytes_transferred
      {
        tx_queue_.pop_front();
        is_writing_ = false;

        if (!error)
        {
          if (!tx_queue_.empty())
          {
            is_writing_ = true;
            SocketAdaptor::write(&tx_queue_.front()[0],
                                  tx_queue_.front().size());
          }
          else
            signal_event(SENT);
        }
        else
          signal_error(error);
      }

      explicit connection(boost::asio::io_service& io_service) :
        SocketAdaptor(io_service,
                      boost::bind(&connection::read_handler, this,
                                  boost::asio::placeholders::error,
                                  boost::asio::placeholders::bytes_transferred),
                      boost::bind(&connection::write_handler, this,
                                  boost::asio::placeholders::error,
                                  boost::asio::placeholders::bytes_transferred),
                      boost::bind(&connection::signal_event, this, _1),
                      boost::bind(&connection::signal_error, this, _1)),
        rx_queue_(),
        tx_queue_(),
        is_writing_(false),
        buffer_size_(DEFAULT_BUFFER_SIZE),
        signal_event_(),
        signal_error_()
      {}

    public:

      void get_event_signal(const event_slot_type& slot)
      { signal_event_.connect(slot); }

      void get_error_signal(const error_slot_type& slot)
      { signal_error_.connect(slot); }

      static boost::shared_ptr<connection>
         create(boost::asio::io_service& io_service)
      {
        return boost::shared_ptr<connection>
            (new connection(io_service));
      }

      ~connection()
      { disconnect(); }

      bool connect(const char *host_name, const char *port_name)
      { return SocketAdaptor::connect(host_name, port_name); }

      void disconnect()
      { SocketAdaptor::stop(); }

      void enable_reception()
      {
        // Note: this could be a move in C++11
        rx_queue_.push_back(Container(buffer_size_, 0));
        size_t size(buffer_size_);
        SocketAdaptor::read(&rx_queue_.back()[0], size);
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
          SocketAdaptor::write(&tx_queue_.front()[0],
                                tx_queue_.front().size());
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
