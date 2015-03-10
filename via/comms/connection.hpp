#ifndef CONNECTION_HPP_VIA_HTTPLIB_
#define CONNECTION_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
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

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class connection
    /// A template class that buffers tcp or ssl comms sockets.
    /// The class can be configured to use either tcp or ssl sockets depending
    /// upon which class is provided as the SocketAdaptor: tcp_adaptor or
    /// ssl::ssl_tcp_adaptor respectively.

    /// @see tcp_adaptor
    /// @see ssl::ssl_tcp_adaptor
    /// @param SocketAdaptor the type of socket, use: tcp_adaptor or
    /// ssl::ssl_tcp_adaptor
    /// @param Container the container to use for the rx & tx buffers, default
    /// std::vector<char> or std::string.
    /// It must contain a contiguous array of bytes. E.g. std::string or
    /// std::array<char, size>
    /// @param use_strand if true use an asio::strand to wrap the handlers,
    /// default false.
    //////////////////////////////////////////////////////////////////////////
    template <typename SocketAdaptor, typename Container = std::vector<char>,
              bool use_strand = false>
    class connection : public SocketAdaptor,
        public boost::enable_shared_from_this
            <connection<SocketAdaptor, Container, use_strand> >
    {
    public:


      /// A weak pointer to a connection.
      typedef typename boost::weak_ptr<connection<SocketAdaptor, Container,
                                                  use_strand> >
         weak_pointer;

      /// A shared pointer to a connection.
      typedef typename boost::shared_ptr<connection<SocketAdaptor, Container,
                                                    use_strand> >
         shared_pointer;

      /// The enable_shared_from_this type of this class.
      typedef typename boost::enable_shared_from_this
                  <connection<SocketAdaptor, Container, use_strand> > enable;

      /// The resolver_iterator type of the SocketAdaptor
      typedef typename boost::asio::ip::tcp::resolver::iterator resolver_iterator;

      /// Event callback function type.
      typedef TR1::function<void (int, weak_pointer)> event_callback_type;

      /// Error callback function type.
      typedef TR1::function<void (boost::system::error_code const&,
                                  weak_pointer)> error_callback_type;

    private:

      /// Strand to ensure the connection's handlers are not called concurrently.
      boost::asio::io_service::strand strand_;
      size_t rx_buffer_size_;              ///< The recieve buffer size.
      boost::shared_ptr<Container> rx_buffer_; ///< The receive buffer.
      boost::shared_ptr<std::deque<Container> > tx_queue_; ///< The transmit buffers.
      event_callback_type event_callback_; ///< The event callback function.
      error_callback_type error_callback_; ///< The error callback function.
      /// The send and receive timeouts, in milliseconds, zero is disabled.
      int timeout_;
      bool transmitting_;                  ///< Whether a write's in progress
      bool no_delay_;                      ///< The tcp no delay status.
      bool keep_alive_;                    ///< The tcp keep alive status.
      bool connected_;                     ///< If the socket is connected.

      /// @fn weak_from_this
      /// Get a weak_pointer to this instance.
      /// @return a weak_pointer to this connection.
      weak_pointer weak_from_this()
      { return weak_pointer(enable::shared_from_this()); }

      /// @fn write_data
      /// Write data via the socket adaptor.
      /// @param buffers the buffer(s) containing the message.
      /// @return true if connected, false otherwise.
      bool write_data(ConstBuffers const& buffers)
      {
        if (connected_)
        {
          if (use_strand)
            SocketAdaptor::write(buffers,
               strand_.wrap(
               boost::bind(&connection::write_callback,
                           weak_from_this(),
                           boost::asio::placeholders::error,
                           boost::asio::placeholders::bytes_transferred,
                           tx_queue_)));
          else
            SocketAdaptor::write(buffers,
               boost::bind(&connection::write_callback,
                           weak_from_this(),
                           boost::asio::placeholders::error,
                           boost::asio::placeholders::bytes_transferred,
                           tx_queue_));
        }

        return connected_;
      }


      /// @fn read_data
      /// Read data via the socket adaptor.
      void read_data()
      {
        if (use_strand)
          SocketAdaptor::read(&(*rx_buffer_)[0], rx_buffer_->size(),
              strand_.wrap(
              boost::bind(&connection::read_callback,
                          weak_from_this(),
                          boost::asio::placeholders::error,
                          boost::asio::placeholders::bytes_transferred,
                          rx_buffer_)));
        else
          SocketAdaptor::read(&(*rx_buffer_)[0], rx_buffer_->size(),
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
      /// @param rx_buffer a shared pointer to the receive buffer to control
      /// object lifetime.
      static void read_callback(weak_pointer ptr,
                                boost::system::error_code const& error,
                                size_t bytes_transferred,
                                boost::shared_ptr<Container>) // rx_buffer)
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
      /// @param tx_queue a shared pointer to the transmit buffers to control
      /// object lifetime.
      static void write_callback(weak_pointer ptr,
                                 boost::system::error_code const& error,
                                 size_t bytes_transferred,
                                 boost::shared_ptr<std::deque<Container> >) // tx_queue)
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
        if (!transmitting_ && !tx_queue_->empty())
          tx_queue_->pop_front();

        transmitting_ = false;

        if (!tx_queue_->empty())
          write_data(ConstBuffers(1, boost::asio::buffer(tx_queue_->front())));

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
            pointer->set_socket_options();
            if (!pointer->tx_queue_->empty())
              pointer->write_data
          (ConstBuffers(1, boost::asio::buffer(pointer->tx_queue_->front())));
            pointer->enable_reception();
            pointer->event_callback_(CONNECTED, ptr);
          }
          else
          {
            pointer->close();
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
              pointer->close();
              pointer->signal_error(error);
            }
          }
        }
      }

      /// @fn shutdown_callback
      /// The function called whenever a socket adaptor attempts to disconnect.
      /// It is required so that HTTPS clients can shutdown gracefully.
      /// @param ptr a weak pointer to the connection
      /// @param error the boost asio error (if any).
      static void shutdown_callback(weak_pointer ptr,
                                    boost::system::error_code const& error)
      {
        shared_pointer pointer(ptr.lock());
        if (pointer && (boost::asio::error::operation_aborted != error))
          pointer->close();
      }

      /// Constructor for server connections.
      /// The constructor is private to ensure that it instances of the class
      /// can only be created as shared pointers by calling the create
      /// function below.
      /// @param io_service the boost asio io_service used by the underlying
      /// socket adaptor.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      /// @param rx_buffer_size the size of the receive_buffer.
      explicit connection(boost::asio::io_service& io_service,
                          event_callback_type event_callback,
                          error_callback_type error_callback,
                          size_t rx_buffer_size) :
        SocketAdaptor(io_service),
        strand_(io_service),
        rx_buffer_size_(rx_buffer_size),
        rx_buffer_(new Container(rx_buffer_size_, 0)),
        tx_queue_(new std::deque<Container>()),
        event_callback_(event_callback),
        error_callback_(error_callback),
        timeout_(0),
        transmitting_(false),
        no_delay_(false),
        keep_alive_(false),
        connected_(false)
      {}

      /// Constructor for client connections.
      /// The constructor is private to ensure that it instances of the class
      /// can only be created as shared pointers by calling the create
      /// function below.
      /// @param io_service the boost asio io_service used by the underlying
      /// socket adaptor.
      /// @param rx_buffer_size the size of the receive_buffer.
      explicit connection(boost::asio::io_service& io_service,
                          size_t rx_buffer_size) :
        SocketAdaptor(io_service),
        strand_(io_service),
        rx_buffer_size_(rx_buffer_size),
        rx_buffer_(new Container(rx_buffer_size_, 0)),
        tx_queue_(new std::deque<Container>()),
        event_callback_(),
        error_callback_(),
        timeout_(0),
        transmitting_(false),
        no_delay_(false),
        keep_alive_(false),
        connected_(false)
      {}

      /// Set the socket's tcp no delay status.
      /// If no_delay_ is set it disables the Nagle algorithm on the socket.
      void no_delay()
      {
          SocketAdaptor::socket().set_option
              (boost::asio::ip::tcp::no_delay(no_delay_));
      }

      /// Set the socket's tcp keep alive status.
      void keep_alive()
      {
          SocketAdaptor::socket().set_option
              (boost::asio::socket_base::keep_alive(keep_alive_));
      }

      /// Set the socket's tcp send and receive timeouts.
      /// Note: asio does not directly support timeouts at the moment.
      /// This code is an amalgamation of two different answers on
      /// StackOverflow, see:
      /// http://stackoverflow.com/questions/20188718/configuring-tcp-keep-alive-with-boostasio
      /// and the 2nd answer to the question it's a duplicate of.
      void tcp_timeouts()
      {
#if defined _WIN32 || defined WIN32 || defined _WIN64 || defined WIN64 \
  || defined  WINNT || defined OS_WIN64
        // use windows-specific time
        SocketAdaptor::socket().set_option(boost::asio::detail::
                   socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>(timeout_));
        SocketAdaptor::socket().set_option(boost::asio::detail::
                   socket_option::integer<SOL_SOCKET, SO_SNDTIMEO>(timeout_));
#else
        // assume everything else is posix
        struct timeval tv;
        tv.tv_sec  = timeout_ / 1000;
        tv.tv_usec = 1000 * (timeout_ % 1000);
        setsockopt(SocketAdaptor::socket().native(), SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&tv), sizeof(tv));
        setsockopt(SocketAdaptor::socket().native(), SOL_SOCKET, SO_SNDTIMEO,
                   reinterpret_cast<const char*>(&tv), sizeof(tv));
#endif
      }

      /// @fn set_socket_options
      /// Disable the nagle algorithm (no delay) on the socket and
      /// (optionally) enable keep alive on the socket and set the tcp
      /// send and receive timeouts.
      void set_socket_options()
      {
        if (no_delay_)
          no_delay();

        if (keep_alive_)
          keep_alive();

        if (timeout_ > 0)
          tcp_timeouts();
      }

    public:

      /// The destructor calls close to ensure that all of the socket's
      /// callback functions are cancelled so that the object can (eventually)
      /// be destroyed.
      ///
      /// Note: the constructors are declared as private to ensure that the
      /// class may only be constructed as a shared pointer via the create
      /// static functions.
      /// @see create
      ~connection()
      { close(); }

      /// The factory function to create server connections.
      /// @pre the event_callback and error_callback functions must exist.
      /// E.g. if either of them are class member functions then the class
      /// MUST have been constructed BEFORE this function is called.
      /// @param io_service the boost asio io_service for the socket adaptor.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      /// @param rx_buffer_size the size of the receive_buffer,
      /// default SocketAdaptor::DEFAULT_RX_BUFFER_SIZE.
      static shared_pointer create(boost::asio::io_service& io_service,
                                   event_callback_type event_callback,
                                   error_callback_type error_callback,
               size_t rx_buffer_size = SocketAdaptor::DEFAULT_RX_BUFFER_SIZE)
      {
        return shared_pointer(new connection(io_service, event_callback,
                                             error_callback, rx_buffer_size));
      }

      /// The factory function to create client connections.
      /// @param io_service the boost asio io_service for the socket adaptor.
      /// @param rx_buffer_size the size of the receive_buffer,
      /// default SocketAdaptor::DEFAULT_RX_BUFFER_SIZE.
      static shared_pointer create(boost::asio::io_service& io_service,
               size_t rx_buffer_size = SocketAdaptor::DEFAULT_RX_BUFFER_SIZE)
      { return shared_pointer(new connection(io_service, rx_buffer_size)); }

      /// @fn set_event_callback
      /// Function to set the event callback function.
      /// For use with the client connection factory function.
      /// @see create(boost::asio::io_service& io_service)
      /// @param event_callback the event callback function.
      void set_event_callback(event_callback_type event_callback)
      { event_callback_ = event_callback; }

      /// @fn set_error_callback
      /// Function to set the error callback function.
      /// For use with the client connection factory function.
      /// @see create(boost::asio::io_service& io_service)
      /// @param error_callback the error callback function.
      void set_error_callback(error_callback_type error_callback)
      { error_callback_ = error_callback; }

      /// @fn connect
      /// Connect the underlying socket adaptor to the given host name and
      /// port.
      /// @pre To be called by "client" connections only after the event
      /// callbacks have been set.
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
      /// @pre To be called by "server" connections only after its accepted
      /// the connection.
      /// @param no_delay whether to enable tcp no delay
      /// @param keep_alive whether to enable tcp keep alive
      /// @param timeout the send and receive timeouts, in milliseconds,
      /// zero is disabled
      void start(bool no_delay, bool keep_alive, int timeout)
      {
        no_delay_   = no_delay;
        keep_alive_ = keep_alive;
        timeout_    = timeout;
        SocketAdaptor::start(boost::bind(&connection::handshake_callback,
                                         weak_from_this(),
                                         boost::asio::placeholders::error));
      }

      /// @fn shutdown
      /// Shutdown the underlying socket adaptor.
      void shutdown()
      {
        SocketAdaptor::shutdown(boost::bind(&connection::shutdown_callback,
                                            weak_from_this(),
                                            boost::asio::placeholders::error));
      }

      /// @fn close
      /// Close the underlying socket adaptor.
      /// Cancels all of the socket's callback functions.
      void close()
      { SocketAdaptor::close(); }

      /// @fn enable_reception
      /// This function prepares the receive buffer and calls the
      /// socket adaptor read function to listen for the next data packet.
      void enable_reception()
      {
        rx_buffer_->resize(rx_buffer_size_);
        read_data();
      }

      /// @fn rx_buffer
      /// Accessor for the receive buffer.
      /// @pre Only valid within the receive event callback function.
      /// @return the receive buffer.
      Container const& rx_buffer() const
      { return *rx_buffer_; }

      /// @fn connected
      /// Accessor for the connected_ flag.
      bool connected() const
      { return connected_; }

      /// Accessor to set the connected_ flag.
      void set_connected(bool enable)
      { connected_ = enable; }

      /// @fn send_data(Container const& packet)
      /// Send a packet of data.
      /// The packet is added to the back of the transmit queue and sent if
      /// the queue was empty.
      /// @param packet the data packet to write.
      void send_data(Container const& packet)
      {
        bool was_empty(tx_queue_->empty());
        tx_queue_->push_back(packet);

        if (!transmitting_ && was_empty)
          write_data(ConstBuffers(1, boost::asio::buffer(tx_queue_->front())));
      }

      /// Send the data in the buffers.
      /// @param buffers the data to write.
      /// @return true if the buffers are being sent, false otherwise.
      bool send_data(ConstBuffers const& buffers)
      {
        if (!transmitting_ && tx_queue_->empty())
        {
          transmitting_ = write_data(buffers);
          return transmitting_;
        }
        else
          return false;
      }

      /// @fn send_data(ForwardIterator1 begin, ForwardIterator2 end)
      /// The packet is added to the back of the transmit queue and sent if
      /// the queue was empty.
      /// This function takes a pair of iterators, so the data doesn't have
      /// to be held in the same type of container as the connection has been
      /// instantiated with.
      /// @param begin iterator to the beginning of the data to write.
      /// @param end iterator to the end of the data to write.
      template<typename ForwardIterator1, typename ForwardIterator2>
      void send_data(ForwardIterator1 begin, ForwardIterator2 end)
      {
        Container buffer(begin, end);
        send_data(buffer);
      }

      /// @fn set_no_delay
      /// Set the tcp no delay status.
      /// @param enable enable/disable tcp no delay.
      void set_no_delay(bool enable)
      {
        no_delay_ = enable;
        if (connected_)
          no_delay();
      }

      /// @fn set_keep_alive
      /// Set the tcp keep alive status.
      /// @param enable enable/disable tcp keep alive.
      void set_keep_alive(bool enable)
      {
        keep_alive_ = enable;
        if (connected_)
          keep_alive();
      }

      /// @fn set_timeout
      /// Set the tcp send and receive timeouts.
      /// @param timeout the tcp send and receive timeout in milliseconds.
      void set_timeout(int timeout)
      {
        timeout_ = timeout;
        if (connected_)
          tcp_timeouts();
      }
    };

  }
}

#endif
