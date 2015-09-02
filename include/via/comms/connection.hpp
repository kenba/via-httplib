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
#include "via/no_except.hpp"
#include <boost/system/error_code.hpp>
#include <memory>
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
        public std::enable_shared_from_this
            <connection<SocketAdaptor, Container, use_strand> >
    {
    public:


      /// A weak pointer to a connection.
      typedef typename std::weak_ptr<connection<SocketAdaptor, Container,
                                                  use_strand> >
         weak_pointer;

      /// A shared pointer to a connection.
      typedef typename std::shared_ptr<connection<SocketAdaptor, Container,
                                                    use_strand> >
         shared_pointer;

      /// The enable_shared_from_this type of this class.
      typedef typename std::enable_shared_from_this
                  <connection<SocketAdaptor, Container, use_strand> > enable;

      /// The resolver_iterator type of the SocketAdaptor
      typedef typename boost::asio::ip::tcp::resolver::iterator resolver_iterator;

      /// Event callback function type.
      typedef std::function<void (int, weak_pointer)> event_callback_type;

      /// Error callback function type.
      typedef std::function<void (boost::system::error_code const&,
                                  weak_pointer)> error_callback_type;

    private:

      /// Strand to ensure the connection's handlers are not called concurrently.
      boost::asio::io_service::strand strand_;
      size_t rx_buffer_size_;              ///< The receive buffer size.
      std::shared_ptr<Container> rx_buffer_; ///< The receive buffer.
      std::shared_ptr<std::deque<Container> > tx_queue_; ///< The transmit queue.
      ConstBuffers tx_buffers_;            ///< The transmit buffers.
      event_callback_type event_callback_; ///< The event callback function.
      error_callback_type error_callback_; ///< The error callback function.
      /// The send and receive timeouts, in milliseconds, zero is disabled.
      int timeout_;
      int receive_buffer_size_; ///< The socket receive buffer size.
      int send_buffer_size_;    ///< The socket send buffer size.
      bool receiving_;          ///< Whether a read's in progress
      bool transmitting_;       ///< Whether a write's in progress
      bool no_delay_;           ///< The tcp no delay status.
      bool keep_alive_;         ///< The tcp keep alive status.
      bool connected_;          ///< If the socket is connected.
      bool disconnect_pending_; ///< Shutdown the socket after the next write.

      /// @fn weak_from_this
      /// Get a weak_pointer to this instance.
      /// @return a weak_pointer to this connection.
      weak_pointer weak_from_this()
      { return weak_pointer(enable::shared_from_this()); }

      /// @fn write_data
      /// Write data via the socket adaptor.
      /// @param buffers the buffer(s) containing the message.
      /// @return true if connected, false otherwise.
      bool write_data(ConstBuffers buffers)
      {
        tx_buffers_.swap(buffers);

        if (connected_)
        {
          // local copies for lambdas
          weak_pointer weak_ptr(weak_from_this());
          std::shared_ptr<std::deque<Container> > tx_queue(tx_queue_);
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4127 ) // conditional expression is constant
#endif
          if (use_strand)
#ifdef _MSC_VER
#pragma warning( pop )
#endif
            SocketAdaptor::write(tx_buffers_,
               strand_.wrap([weak_ptr, tx_queue]
                            (boost::system::error_code const& error,
                             size_t bytes_transferred)
            { write_callback(weak_ptr, error, bytes_transferred, tx_queue); }));
          else
            SocketAdaptor::write(tx_buffers_,
              [weak_ptr, tx_queue](boost::system::error_code const& error,
                                   size_t bytes_transferred)
            { write_callback(weak_ptr, error, bytes_transferred, tx_queue); });
        }

        return connected_;
      }

      /// @fn read_data
      /// Read data via the socket adaptor.
      void read_data()
      {
        // local copies for lambdas
        weak_pointer weak_ptr(weak_from_this());
        std::shared_ptr<Container> rx_buffer(rx_buffer_);
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4127 ) // conditional expression is constant
#endif
        if (use_strand)
#ifdef _MSC_VER
#pragma warning( pop )
#endif
          SocketAdaptor::read(&(*rx_buffer_)[0], rx_buffer_->size(),
              strand_.wrap([weak_ptr, rx_buffer]
                           (boost::system::error_code const& error,
                            size_t bytes_transferred)
           { read_callback(weak_ptr, error, bytes_transferred, rx_buffer); }));
        else
          SocketAdaptor::read(&(*rx_buffer_)[0], rx_buffer_->size(),
            [weak_ptr, rx_buffer](boost::system::error_code const& error,
                                  size_t bytes_transferred)
           { read_callback(weak_ptr, error, bytes_transferred, rx_buffer); });
      }

      /// This function determines whether the error is a socket disconnect.
      /// Common disconnection error codes are:
      ///  + connection_refused - server not available for a client connection.
      ///  + connection_reset - the other side closed the connection.
      ///  + connection_aborted - routing / firewall issue.
      ///  + bad_descriptor - socket is in the process of closing, see:
      /// http://sourceforge.net/p/asio/mailman/message/6493983/
      /// @return true if a disconnect error, false otherwise.
      bool is_error_a_disconnect(boost::system::error_code const& error)
      {
        switch(error.value())
        {
        case boost::asio::error::eof:
        case boost::asio::error::connection_refused:
        case boost::asio::error::connection_reset:
        case boost::asio::error::connection_aborted:
        case boost::asio::error::bad_descriptor:
          return true;
        default:
          {
          bool ssl_shutdown(false);
          bool is_a_disconnect(SocketAdaptor::is_disconnect(error, ssl_shutdown));
          if (ssl_shutdown)
            shutdown();

          return is_a_disconnect;
          }
        }
      }

      /// @fn signal_error
      /// This function is called whenever an error event occurs.
      /// It determines whether the error code is for a disconnect in which
      /// case it sends a DISCONNECTED signal otherwise it sends the
      /// error signal.
      void signal_error(boost::system::error_code const& error)
      {
        if (is_error_a_disconnect(error))
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
                                std::shared_ptr<Container>) // rx_buffer)
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
        receiving_ = false;
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
                                 std::shared_ptr<std::deque<Container> >) // tx_queue)
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
          {
            if (pointer->disconnect_pending_)
              pointer->shutdown();
            else
              pointer->write_handler(bytes_transferred);
          }
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
            pointer->receiving_ = false;
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
            pointer->handshake([ptr](boost::system::error_code const& error)
              { handshake_callback(ptr, error); }, false);
          else
          {
            if ((boost::asio::error::host_not_found == error) &&
                (resolver_iterator() != host_iterator))
              pointer->connect_socket([ptr]
                  (boost::system::error_code const& error,
                   resolver_iterator host_iterator)
              { connect_callback(ptr, error, host_iterator); }, ++host_iterator);
            else
            {
              pointer->close();
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
        tx_buffers_(),
        event_callback_(event_callback),
        error_callback_(error_callback),
        timeout_(0),
        receive_buffer_size_(0),
        send_buffer_size_(0),
        receiving_(false),
        transmitting_(false),
        no_delay_(false),
        keep_alive_(false),
        connected_(false),
        disconnect_pending_(false)
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
        tx_buffers_(),
        event_callback_(),
        error_callback_(),
        timeout_(0),
        receive_buffer_size_(0),
        send_buffer_size_(0),
        receiving_(false),
        transmitting_(false),
        no_delay_(false),
        keep_alive_(false),
        connected_(false),
        disconnect_pending_(false)
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
        setsockopt(SocketAdaptor::socket().native_handle(),
                   SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&tv), sizeof(tv));
        setsockopt(SocketAdaptor::socket().native_handle(),
                   SOL_SOCKET, SO_SNDTIMEO,
                   reinterpret_cast<const char*>(&tv), sizeof(tv));
#endif
      }

      /// Set the socket's receive buffer size.
      void resize_receive_buffer()
      {
        SocketAdaptor::socket().set_option
            (boost::asio::socket_base::receive_buffer_size(receive_buffer_size_));
      }

      /// Set the socket's send buffer size.
      void resize_send_buffer()
      {
        SocketAdaptor::socket().set_option
            (boost::asio::socket_base::send_buffer_size(send_buffer_size_));
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

        if (receive_buffer_size_ > 0)
          resize_receive_buffer();

        if (send_buffer_size_ > 0)
          resize_send_buffer();
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

      /// Set the connection's rx_buffer_size_.
      void set_rx_buffer_size(size_t rx_buffer_size)
      { rx_buffer_size_ = rx_buffer_size; }

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
        weak_pointer ptr(weak_from_this());
        return SocketAdaptor::connect(host_name, port_name,
          [ptr](boost::system::error_code const& error, resolver_iterator itr)
            { connect_callback(ptr, error, itr); });
      }

      /// @fn start
      /// Start the handshake for a server connection.
      /// @pre To be called by "server" connections only after its accepted
      /// the connection.
      /// @param no_delay whether to enable tcp no delay
      /// @param keep_alive whether to enable tcp keep alive
      /// @param timeout the send and receive timeouts, in milliseconds
      /// @param receive_buffer_size the size of the socket's receive buffer
      /// @param send_buffer_size the size of the socket's send buffer
      /// zero is disabled
      void start(bool no_delay, bool keep_alive, int timeout,
                 int receive_buffer_size, int send_buffer_size)
      {
        no_delay_            = no_delay;
        keep_alive_          = keep_alive;
        timeout_             = timeout;
        receive_buffer_size_ = receive_buffer_size;
        send_buffer_size_    = send_buffer_size;

        weak_pointer weak_ptr(weak_from_this());
        SocketAdaptor::start([weak_ptr](boost::system::error_code const& error)
          { handshake_callback(weak_ptr, error); });
      }

      /// @fn disconnect
      /// Shutdown the socket after the last message has been sent.
      void disconnect()
      {
        // If nothing is currently being sent
        if (!transmitting_ && tx_queue_->empty())
          shutdown();
        else // shutdown the socekt in the write callback
          disconnect_pending_ = true;
      }

      /// @fn shutdown
      /// Shutdown the socket now.
      void shutdown()
      {
        // local copies for the lambda
        weak_pointer weak_ptr(weak_from_this());
        std::shared_ptr<std::deque<Container>> tx_queue(tx_queue_);

        // Call shutdown with the callback
        SocketAdaptor::shutdown([weak_ptr, tx_queue]
                        (boost::system::error_code const& error, int bytes)
                        { write_callback(weak_ptr, error, bytes, tx_queue); });
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
        if (!receiving_)
        {
          receiving_ = true;
          rx_buffer_->resize(rx_buffer_size_);
          read_data();
        }
      }

      /// Accessor for the receive buffer.
      /// Swaps the contents of the receive buffer with the rx_buffer parameter
      /// and re-enables the receiver.
      /// This effectively double buffer's rx_buffer_, permitting the
      /// receiver to be re-enabled without corrupting the data.
      /// @pre Only valid within the receive event callback function.
      /// @post receive buffer is invalid to read again.
      /// @retval the receive buffer.
      void read_rx_buffer(Container& rx_buffer)
      {
        rx_buffer_->swap(rx_buffer);
        enable_reception();
      }

      /// @fn connected
      /// Accessor for the connected_ flag.
      bool connected() const NOEXCEPT
      { return connected_; }

      /// Accessor to set the connected_ flag.
      void set_connected(bool enable) NOEXCEPT
      { connected_ = enable; }

      /// @fn send_data(Container const& packet)
      /// Send a packet of data.
      /// The packet is added to the back of the transmit queue and sent if
      /// the queue was empty.
      /// @param packet the data packet to write.
      void send_data(Container packet)
      {
        bool was_empty(tx_queue_->empty());
        tx_queue_->push_back(std::move(packet));

        if (!transmitting_ && was_empty)
          write_data(ConstBuffers(1, boost::asio::buffer(tx_queue_->front())));
      }

      /// Send the data in the buffers.
      /// @param buffers the data to write.
      /// @return true if the buffers are being sent, false otherwise.
      bool send_data(ConstBuffers buffers)
      {
        if (!transmitting_ && tx_queue_->empty())
        {
          transmitting_ = write_data(std::move(buffers));
          return transmitting_;
        }
        else
          return false;
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

      /// Get the socket's receive buffer size.
      /// @return the size of the socket's receive buffer if connected, otherwise 0.
      int receive_buffer_size()
      {
        if (connected_)
        {
          boost::asio::socket_base::receive_buffer_size option;
          SocketAdaptor::socket().get_option(option);
          return option.value();
        }
        else
          return 0;
      }

      /// @fn set_receive_buffer_size
      /// Set the size of the tcp receive buffer.
      /// @param receive_buffer_size the size of the receive buffer in bytes.
      void set_receive_buffer_size(int receive_buffer_size)
      {
        receive_buffer_size_ = receive_buffer_size;
        if (connected_)
          resize_receive_buffer();
      }

      /// Get the socket's send buffer size.
      /// @return the size of the socket's send buffer if connected, otherwise 0.
      int send_buffer_size()
      {
        if (connected_)
        {
          boost::asio::socket_base::send_buffer_size option;
          SocketAdaptor::socket().get_option(option);
          return option.value();
        }
        else
          return 0;
      }

      /// @fn set_send_buffer_size
      /// Set the size of the tcp send buffer.
      /// @param send_buffer_size the size of the send buffer in bytes.
      void set_send_buffer_size(int send_buffer_size)
      {
        send_buffer_size_ = send_buffer_size;
        if (connected_)
          resize_send_buffer();
      }
    };

  }
}

#endif
