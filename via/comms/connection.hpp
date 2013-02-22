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
/// @file connection.hpp
/// @brief The connection template class.
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
    /// @class connection
    /// A template class providing buffering to the socket adaptors.
    //////////////////////////////////////////////////////////////////////////
    template <typename SocketAdaptor, typename Container = std::vector<char> >
    class connection : public SocketAdaptor,
        public boost::enable_shared_from_this
            <connection<SocketAdaptor, Container> >
    {
    public:

      /// @typedef a weak pointer to a connection.
      typedef typename boost::weak_ptr<connection<SocketAdaptor, Container> >
         weak_pointer;

      /// @typedef a shared pointer to a connection.
      typedef typename boost::shared_ptr<connection<SocketAdaptor, Container> >
         shared_pointer;

      /// @typedef the enable_shared_from_this type of this class.
      typedef typename boost::enable_shared_from_this
                              <connection<SocketAdaptor, Container> > enable;

      /// @typedef the boost signal to indicate that an event occured.
      typedef boost::signal<void (int event, weak_pointer)> event_signal_type;

      /// @typedef the boost slot associated with the event_signal_type.
      typedef typename event_signal_type::slot_type event_slot_type;

      /// @typedef the boost signal to indicate that an error occured.
      typedef boost::signal<void (const boost::system::error_code&, weak_pointer)>
                                  error_signal_type;

      /// @typedef the boost slot associated with the error_signal_type.
      typedef typename error_signal_type::slot_type error_slot_type;

    private:

      std::deque<Container> rx_queue_; ///< The receive buffer.
      std::deque<Container> tx_queue_; ///< The transmit buffer.
      bool is_writing_;                ///< A flag for the transmit buffer.

      /// The default receive buffer size.
      static const size_t DEFAULT_BUFFER_SIZE = 8192;

      event_signal_type signal_event_; ///< The event signal.
      error_signal_type signal_error_; ///< The error signal.

      /// @fn signal_event
      /// This function is called whenever a comms event occurs.
      /// It sends the event signal and in the case of the connection
      /// becoming connected, it enables the recevier on the connection.
      /// @param event the event, @see event_type.
      void signal_event(int event)
      {
        if (event == CONNECTED)
          enable_reception();

        signal_event_(event, enable::shared_from_this());
      }

      /// @fn signal_event
      /// This function is called whenever an error event occurs.
      /// It sends the error signal, unless the socket adaptor determines
      /// that the error is a disconnect, in which case it sends a
      /// DISCONNECTED event instead.
      /// @param error the boost asio error.
      void signal_error(const boost::system::error_code& error)
      {
        if ((error == boost::asio::error::eof) ||
             SocketAdaptor::is_disconnect(error))
          signal_event_(DISCONNECTED, enable::shared_from_this());
        else
          signal_error_(error, enable::shared_from_this());
      }

      /// @fn read_handler
      /// The callback function called whenever a socket adaptor receives
      /// a data packet.
      /// If there was no error, it resizes the buffer to the size of the
      /// received packet and signals that a packet has been recieved.
      /// Otherwise it calls signal_error to determine whether the socket has
      /// been disconnected or to pass on the error.
      /// @param error the boost asio error (if any).
      /// @param bytes_transferred the size of the received data packet.
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

      /// @fn write_handler
      /// The callback function called whenever a socket adaptor sends
      /// a data packet.
      /// If there was no error, it removes the packet at the front of the
      /// tx queue and attempts to send the next packet. If there are no
      /// more packets to send, it signals that the packets were sent.
      /// Otherwise it calls signal_error to determine whether the socket has
      /// been disconnected or to pass on the error.
      /// received packet and signals that a packet has been recieved.
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

      /// @fn Constructor
      /// The constructor is private to ensure that it instances of the class
      /// can only be created as shared pointers by calling the create
      /// function below.
      /// @param io_service the boost asio io_service used by the underlying
      /// socket adaptor.
      explicit connection(boost::asio::io_service& io_service,
                          unsigned short port_number) :
        SocketAdaptor(io_service,
                      boost::bind(&connection::read_handler, this,
                                  boost::asio::placeholders::error,
                                  boost::asio::placeholders::bytes_transferred),
                      boost::bind(&connection::write_handler, this,
                                  boost::asio::placeholders::error,
                                  boost::asio::placeholders::bytes_transferred),
                      boost::bind(&connection::signal_event, this, _1),
                      boost::bind(&connection::signal_error, this, _1),
                      port_number),
        rx_queue_(),
        tx_queue_(),
        is_writing_(false),
        signal_event_(),
        signal_error_()
      {}

    public:

      /// @fn buffer_size
      /// Get/set the buffer size.
      /// Can be called with a size to set the buffer_size. Otherwise,
      /// used to get the buffer_size.
      /// Note: the call to set the buffer_size MUST be the first call of
      /// the function, otherwise the DEFAULT_BUFFER_SIZE will be used.
      /// @param size the desired buffer_size.
      /// @return the buffer_size.
      static size_t buffer_size(size_t size = DEFAULT_BUFFER_SIZE)
      {
        static size_t buffer_size_(size);
        return buffer_size_;
      }

      /// @fn create
      /// The factory function to create connections.
      /// @param io_service the boost asio io_service used by the underlying
      /// socket adaptor.
      static shared_pointer create(boost::asio::io_service& io_service,
                                   unsigned short port_number = 0)
      { return shared_pointer(new connection(io_service, port_number)); }

      /// @fn get_event_signal
      /// A function to connect a slot to the event signal.
      /// @param slot the slot to connect.
      void get_event_signal(const event_slot_type& slot)
      { signal_event_.connect(slot); }

      /// @fn get_error_signal
      /// A function to connect a slot to the error signal.
      /// @param slot the slot to connect.
      void get_error_signal(const error_slot_type& slot)
      { signal_error_.connect(slot); }

      /// @fn destructor.
      /// Disconnects the socket.
      ~connection()
      { disconnect(); }

      /// @fn connect
      /// Connect the underlying socket adaptor to the given host name and
      /// port.
      /// @pre To be called by "client" connections only.
      /// Server connections are accepted by the server instead.
      /// @param host_name the host to connect to.
      /// @param port_name the port to connect to.
      bool connect(const char *host_name, const char *port_name)
      { return SocketAdaptor::connect(host_name, port_name); }

      /// @fn disconnect
      /// Disconnect the underlying socket adaptor.
      void disconnect()
      { SocketAdaptor::stop(); }

      /// @fn enable_reception
      /// This function primes the receive buffer and calls the socket adaptor
      /// to read the next received data packet.
      void enable_reception()
      {
        rx_queue_.push_back(Container(buffer_size(), 0));
        size_t size(buffer_size());
        SocketAdaptor::read(&rx_queue_.back()[0], size);
      }

      /// @fn read_pending
      /// This function returns true if there is at least one received data
      /// packet ready to be read.
      /// @return true if there is one (or more) received data packets ready,
      /// false otherwise.
      bool read_pending() const
      { return rx_queue_.size() > 1; }

      /// @fn read_data
      /// This function returns the data packet at the head of the queue
      /// an "pops" it off the head of the queue.
      /// @pre There must be at least one data packet in the queue, i.e.
      /// read_pending() == true.
      /// Otherwise the function will assert an error.
      /// @return the data packet at the head of the receive queue.
      Container read_data()
      {
        assert(read_pending());
        Container data(rx_queue_.front());
        rx_queue_.pop_front();
        return data;
      }

      /// @fn send_data
      /// Send a packet of data.
      /// The data is put on the back of the queue, whilst the function
      /// ensures that the data at the front of the queue is being written.
      /// @param packet the data packet to write.
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

#if defined(BOOST_ASIO_HAS_MOVE)
      /// @fn send_data
      /// Send a packet of data, move version for C++11.
      /// The data is put on the back of the queue, whilst the function
      /// ensures that the data at the front of the queue is being written.
      /// @param packet the data packet to write.
      void send_data(Container&& packet)
      {
        tx_queue_.push_back(packet);
        if (!is_writing_)
        {
          is_writing_ = true;
          SocketAdaptor::write(&tx_queue_.front()[0],
                                tx_queue_.front().size());
        }
      }
#endif

      /// @fn send_data
      /// Send a packet of data.
      /// As above, but this function takes a pair of iterators, so the data
      /// doesn't have to be held in the same type of container as the
      /// connection has been instantiated with.
      /// @param begin iterator to the beginning of the data to write.
      /// @param end iterator to the end of the data to write.
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
