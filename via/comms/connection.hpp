#pragma once

#ifndef CONNECTION_HPP_VIA_HTTPLIB_
#define CONNECTION_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
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
    /// @param SocketAdaptor the type of socket to use, tcp or ssl
    /// @param Container the type of container to use
    /// @param use_strand if true use an asio::strand to wrap the handlers
    //////////////////////////////////////////////////////////////////////////
    template <typename SocketAdaptor, typename Container = std::vector<char>,
              bool use_strand = false>
    class connection : public SocketAdaptor,
        public boost::enable_shared_from_this
            <connection<SocketAdaptor, Container, use_strand> >
    {
    public:

      /// a weak pointer to a connection.
      typedef typename boost::weak_ptr<connection<SocketAdaptor, Container, use_strand> >
         weak_pointer;

      /// a shared pointer to a connection.
      typedef typename boost::shared_ptr<connection<SocketAdaptor, Container, use_strand> >
         shared_pointer;

      /// the enable_shared_from_this type of this class.
      typedef typename boost::enable_shared_from_this
                  <connection<SocketAdaptor, Container, use_strand> > enable;

      /// the resolver_iterator type of the SocketAdaptor
      typedef typename SocketAdaptor::resolver_iterator resolver_iterator;

      /// Event callback function type.
#if ((__cplusplus >= 201103L) || (_MSC_VER >= 1600))
      typedef std::function<void (int, weak_pointer)> event_callback_type;
#else
      typedef std::tr1::function<void (int, weak_pointer)> event_callback_type;
#endif

      /// Error callback function type.
#if ((__cplusplus >= 201103L) || (_MSC_VER >= 1600))
      typedef std::function<void (boost::system::error_code const&,
                                  weak_pointer)> error_callback_type;
#else
      typedef std::tr1::function<void (boost::system::error_code const&,
                                       weak_pointer)> error_callback_type;
#endif

    private:

      /// Strand to ensure the connection's handlers are not called concurrently.
      boost::asio::io_service::strand strand_;
      boost::shared_ptr<Container> rx_buffer_;                 ///< The receive buffer.
      boost::shared_ptr<std::deque<Container> > tx_queue_;     ///< The transmit buffers.
      event_callback_type event_callback_; ///< The event callback function.
      error_callback_type error_callback_; ///< The error callback function.
      bool connected_;                     ///< If the socket is connected.

      /// The default receive buffer size.
      static const size_t DEFAULT_BUFFER_SIZE = 8192;

      /// @fn weak_from_this
      /// Get a weak_pointer to this instance.
      /// @return a weak_pointer to this connection.
      weak_pointer weak_from_this()
      { return weak_pointer(enable::shared_from_this()); }

      /// @fn write_data
      /// Write data via the socket adaptor.
      void write_data()
      {
        if (connected_ && !tx_queue_->empty())
        {
          if (use_strand)
            SocketAdaptor::write(&tx_queue_->front()[0],
                                  tx_queue_->front().size(),
               strand_.wrap(
               boost::bind(&connection::write_callback,
                           weak_from_this(),
                           boost::asio::placeholders::error,
                           boost::asio::placeholders::bytes_transferred,
                           tx_queue_)));
          else
            SocketAdaptor::write(&tx_queue_->front()[0],
                                  tx_queue_->front().size(),
               boost::bind(&connection::write_callback,
                           weak_from_this(),
                           boost::asio::placeholders::error,
                           boost::asio::placeholders::bytes_transferred,
                           tx_queue_));
        }
      }

      /// @fn read_data
      /// Read data via the socket adaptor.
      void read_data()
      {
        if (use_strand)
          SocketAdaptor::read(&(*rx_buffer_)[0], buffer_size(),
              strand_.wrap(
              boost::bind(&connection::read_callback,
                          weak_from_this(),
                          boost::asio::placeholders::error,
                          boost::asio::placeholders::bytes_transferred,
                          rx_buffer_)));
        else
          SocketAdaptor::read(&(*rx_buffer_)[0], buffer_size(),
              boost::bind(&connection::read_callback,
                          weak_from_this(),
                          boost::asio::placeholders::error,
                          boost::asio::placeholders::bytes_transferred,
                          rx_buffer_));
      }

      /// @fn signal_error
      /// This function is called whenever an error event occurs.
      /// It sends the error signal, unless the socket adaptor determines
      /// that the error is a disconnect, in which case it sends a
      /// DISCONNECTED event instead.
      /// @param error the boost asio error.
      void signal_error(boost::system::error_code const& error)
      {
          if ((error == boost::asio::error::eof) ||
                SocketAdaptor::is_disconnect(error))
          event_callback_(DISCONNECTED, weak_from_this());
        else
          error_callback_(error, weak_from_this());
      }

      /// @fn read_callback
      /// The function called whenever a socket adaptor receives a data packet.
      /// It ensures that the connection still exists and the event is valid.
      /// If there was an error it calls the connection's signal_error
      /// function, otherwise it calls the connection's read_handler.
      /// @param ptr a weak pointer to the connection
      /// @param error the boost asio error (if any).
      /// @param bytes_transferred the size of the received data packet.
      static void read_callback(weak_pointer ptr,
                                boost::system::error_code const& error,
                                size_t bytes_transferred,
                                boost::shared_ptr<Container> /*rx_buffer*/)
      {
        shared_pointer pointer(ptr.lock());
        if (pointer && (boost::asio::error::operation_aborted != error))
        {
          if (error)
            pointer->signal_error(error);
          else
            pointer->read_handler(bytes_transferred);
        }
      }

      /// @fn read_handler
      /// The function called whenever a data packet has been received.
      /// It resizes the receive buffer to the size of the received packet,
      /// signals that a packet has been received and then calls
      /// enable_reception to listen for the next packet.
      /// @param bytes_transferred the size of the received data packet.
      void read_handler(size_t bytes_transferred)
      {
        rx_buffer_->resize(bytes_transferred);
        event_callback_(RECEIVED, weak_from_this());
        enable_reception();
      }

      /// @fn write_callback
      /// The function called whenever a socket adaptor has sent a data packet.
      /// It ensures that the connection still exists and the event is valid.
      /// If there was an error it calls the connection's signal_error
      /// function, otherwise it calls the connection's write_handler.
      /// @param ptr a weak pointer to the connection
      /// @param error the boost asio error (if any).
      /// @param bytes_transferred the size of the sent data packet.
      static void write_callback(weak_pointer ptr,
                                 boost::system::error_code const& error,
                                 size_t bytes_transferred,
                                 boost::shared_ptr<std::deque<Container> > /* tx_queue */)
      {
        shared_pointer pointer(ptr.lock());
        if (pointer && (boost::asio::error::operation_aborted != error))
        {
          if (error)
          {
            pointer->tx_queue_->clear();
            pointer->signal_error(error);
          }
          else
            pointer->write_handler(bytes_transferred);
        }
      }

      /// @fn write_handler
      /// The function called whenever a data packet has been sent.
      /// It removes the data packet at the front of the transmit queue, sends
      /// the next packet in the queue (if any) and signals that that a packet
      /// has been sent.
      /// @param bytes_transferred the size of the sent data packet.
      void write_handler(size_t) // bytes_transferred
      {
        tx_queue_->pop_front();

        if (!tx_queue_->empty())
          write_data();

        event_callback_(SENT, weak_from_this());
      }
      
      /// @fn handshake_callback
      /// The function called whenever a socket adaptor receives a connection
      /// handshake.
      /// It ensures that the connection still exists and the event is valid.
      /// If there was an error, it shutdowns the connection and signals the
      /// error. Otherwise, it calls enable_reception to listen on the
      /// connection and signals that it's connected.
      /// @param ptr a weak pointer to the connection
      /// @param error the boost asio error (if any).
      static void handshake_callback(weak_pointer ptr,
                                     boost::system::error_code const& error)
      {
        shared_pointer pointer(ptr.lock());
        if (pointer && (boost::asio::error::operation_aborted != error))
        {
          if (!error)
          {
            pointer->connected_ = true;
            if (!pointer->tx_queue_->empty())
              pointer->write_data();
            pointer->enable_reception();
            pointer->event_callback_(CONNECTED, ptr);
          }
          else
          {
            pointer->shutdown();
            pointer->signal_error(error);
          }
        }
      }

      /// @fn connect_callback
      /// The function called whenever a socket adaptor attempts to connect.
      /// It ensures that the connection still exists and the event is valid.
      /// If there was no error, it attempts to handshake on the connection -
      /// this shall always be accepted for an unencypted conection.
      /// If the error was host not found and there are more host to try,
      /// it attempts to connect to the next host
      /// Otherwise it shuts down and signals an error
      /// @param ptr a weak pointer to the connection
      /// @param error the boost asio error (if any).
      /// @param host_iterator an iterator to the host to connect to.
      static void connect_callback(weak_pointer ptr,
                                   boost::system::error_code const& error,
                                   resolver_iterator host_iterator)
      {
        shared_pointer pointer(ptr.lock());
        if (pointer && (boost::asio::error::operation_aborted != error))
        {
          if (!error)
            pointer->handshake(boost::bind(&connection::handshake_callback, ptr, 
                               boost::asio::placeholders::error), false);
          else
          {
            if ((boost::asio::error::host_not_found == error) &&
                (resolver_iterator() != host_iterator))
              pointer->connect_socket
                  (boost::bind(&connection::connect_callback, ptr,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::iterator),
                   ++host_iterator);
            else
            {
              pointer->shutdown();
              pointer->signal_error(error);
            }
          }
        }
      }

      /// Constructor for server connections.
      /// The constructor is private to ensure that it instances of the class
      /// can only be created as shared pointers by calling the create
      /// function below.
      /// @param io_service the boost asio io_service used by the underlying
      /// socket adaptor.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      /// @param port_number the port number (only applies to UDP servers).
      explicit connection(boost::asio::io_service& io_service,
                          event_callback_type event_callback,
                          error_callback_type error_callback,
                          unsigned short port_number) :
        SocketAdaptor(io_service, port_number),
        strand_(io_service),
        rx_buffer_(new Container()),
        tx_queue_(new std::deque<Container>()),
        event_callback_(event_callback),
        error_callback_(error_callback),
        connected_(false)
      {}

      /// Constructor for client connections.
      /// The constructor is private to ensure that it instances of the class
      /// can only be created as shared pointers by calling the create
      /// function below.
      /// @param io_service the boost asio io_service used by the underlying
      /// socket adaptor.
      /// @param port_number required for UDP servers, default zero.
      explicit connection(boost::asio::io_service& io_service,
                          unsigned short port_number) :
        SocketAdaptor(io_service, port_number),
        strand_(io_service),
        rx_buffer_(new Container()),
        tx_queue_(new std::deque<Container>()),
        event_callback_(),
        error_callback_(),
        connected_(false)
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

      /// The destructor closes the connections to ensure that all of the
      /// callback functions are cancelled.
      ~connection()
      { close(); }

      /// @fn create
      /// The factory function to create server connections.
      /// @param io_service the boost asio io_service used by the underlying
      /// socket adaptor.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      /// @param port_number required for UDP servers, default zero.
      static shared_pointer create(boost::asio::io_service& io_service,
                                   event_callback_type event_callback,
                                   error_callback_type error_callback,
                                   unsigned short port_number = 0)
      {
        return shared_pointer(new connection(io_service, event_callback,
                                             error_callback, port_number));
      }

      /// @fn create
      /// The factory function to create client connections.
      /// @param io_service the boost asio io_service used by the underlying
      /// socket adaptor.
      /// @param port_number required for UDP servers, default zero.
      static shared_pointer create(boost::asio::io_service& io_service,
                                   unsigned short port_number = 0)
      { return shared_pointer(new connection(io_service, port_number)); }

      /// Function to set the event callback function.
      /// @param event_callback the event callback function.
      void set_event_callback(event_callback_type event_callback)
      { event_callback_ = event_callback; }

      /// Function to set the error callback function.
      /// @param error_callback the error callback function.
      void set_error_callback(error_callback_type error_callback)
      { error_callback_ = error_callback; }

      /// @fn connect
      /// Connect the underlying socket adaptor to the given host name and
      /// port.
      /// @pre To be called by "client" connections only.
      /// Server connections are accepted by the server instead.
      /// @param host_name the host to connect to.
      /// @param port_name the port to connect to.
      bool connect(const char *host_name, const char *port_name)
      {
        return SocketAdaptor::connect(host_name, port_name,
          boost::bind(&connection::connect_callback, weak_from_this(),
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::iterator));
      }

      /// @fn start
      /// Start the handshake for a server connection.
      void start()
      {
        SocketAdaptor::start(boost::bind(&connection::handshake_callback,
                                         weak_from_this(),
                                         boost::asio::placeholders::error));
      }

      /// @fn disconnect
      /// Shutdown the underlying socket adaptor.
      void disconnect()
      { SocketAdaptor::shutdown(); }

      /// @fn close
      /// Close the underlying socket adaptor.
      void close()
      { SocketAdaptor::close(); }

      /// @fn enable_reception
      /// This function prepares the receive buffer and calls the
      /// socket adaptor read function to listen for the next data packet.
      void enable_reception()
      {
        rx_buffer_->resize(buffer_size());
        read_data();
      }

      /// @fn rx_buffer
      /// Accessor for the receive buffer.
      /// @pre Only valid within the receive even callback function.
      /// @return the data packet at the head of the receive queue.
      Container const& rx_buffer()
      { return *rx_buffer_; }

      /// @fn send_data(Container const& packet)
      /// Send a packet of data.
      /// The data added to the back of the transmit queue and sends the
      /// packet if the queue was empty.
      /// @param packet the data packet to write.
      void send_data(Container const& packet)
      {
        bool notWriting(tx_queue_->empty());
        tx_queue_->push_back(packet);

        if (notWriting)
          write_data();
      }

#if defined(BOOST_ASIO_HAS_MOVE)
      /// @fn send_data(Container&& packet)
      /// Send a packet of data, move version for C++11.
      /// The data added to the back of the transmit queue and sends the
      /// packet if the queue was empty.
      /// @param packet the data packet to write.
      void send_data(Container&& packet)
      {
        bool notWriting(tx_queue_.empty());
        tx_queue_.push_back(packet);

        if (notWriting)
          write_data();
      }
#endif // BOOST_ASIO_HAS_MOVE

      /// @fn send_data(ForwardIterator1 begin, ForwardIterator2 end)
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
