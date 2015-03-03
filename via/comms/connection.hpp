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
#include <memory>
#include <vector>
#include <deque>
#include <array>

namespace via
{
  namespace comms
  {
    /// The default size of the receive buffer.
    enum { DEFAULT_BUFFER_SIZE = 8192 };

    //////////////////////////////////////////////////////////////////////////
    /// @class connection
    /// A template class that buffers tcp or ssl comms sockets.
    /// The class can be configured to use either tcp or ssl sockets depending
    /// upon which class is provided as the SocketAdaptor: tcp_adaptor or
    /// ssl::ssl_tcp_adaptor respectively.
    /// The other template parameters configure the type of container to use
    /// for the transmit buffers, the size of the receive buffer and
    /// whether to use asio::strand for an io_service running in multiple
    /// treads.
    /// @see tcp_adaptor
    /// @see ssl::ssl_tcp_adaptor
    /// @param SocketAdaptor the type of socket, use: tcp_adaptor or
    /// ssl::ssl_tcp_adaptor
    /// @param Container the container to use for the rx & tx buffer, default
    /// std::vector<char>.
    /// It must contain a contiguous array of bytes. E.g. std::string or
    /// std::vector<char>
    /// @param buffer_size the size of the receive buffer, default 8192 bytes.
    /// @param use_strand if true use an asio::strand to wrap the handlers,
    /// default false.
    //////////////////////////////////////////////////////////////////////////
    template <typename SocketAdaptor, typename Container = std::vector<char>,
              size_t buffer_size = comms::DEFAULT_BUFFER_SIZE,
              bool use_strand = false>
    class connection : public SocketAdaptor,
        public std::enable_shared_from_this
            <connection<SocketAdaptor, Container, buffer_size, use_strand> >
    {
    public:

      /// A weak pointer to a connection.
      using weak_pointer = std::weak_ptr<connection<SocketAdaptor, Container,
                                                    buffer_size, use_strand>>;

      /// A shared pointer to a connection.
      using shared_pointer = std::shared_ptr<connection<SocketAdaptor, Container,
                                                        buffer_size, use_strand>>;

      /// The enable_shared_from_this type of this class.
      using enable = std::enable_shared_from_this<connection<SocketAdaptor, Container,
                                                  buffer_size, use_strand>>;

      /// Event callback function type.
      using event_callback_type = std::function<void (int, weak_pointer)>;

      /// Error callback function type.
      using error_callback_type =
        std::function<void (boost::system::error_code const&, weak_pointer)>;

    private:

      /// Strand to ensure the connection's handlers are not called concurrently.
      boost::asio::io_service::strand strand_;
      std::shared_ptr<Container> rx_buffer_; ///< The receive buffer.
      std::deque<Container> tx_queue_;     ///< The transmit buffers.
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
      bool write_data(ConstBuffers buffers)
      {
        if (connected_)
        {
          // local copies for the lambda captures
          weak_pointer w_ptr(weak_from_this());
          std::shared_ptr<Container> rx_buffer(rx_buffer_);

          if (use_strand)
            SocketAdaptor::write(std::move(buffers), strand_.wrap(
             [w_ptr, rx_buffer](boost::system::error_code const& ec, std::size_t bytes)
              { write_callback(w_ptr, ec, bytes, rx_buffer); }));
          else
            SocketAdaptor::write(std::move(buffers),
             [w_ptr, rx_buffer](boost::system::error_code const& ec, std::size_t bytes)
              { write_callback(w_ptr, ec, bytes, rx_buffer); });
        }

        return connected_;
      }

      /// @fn read_data
      /// Read data via the socket adaptor.
      void read_data()
      {
        // local copies for the lambda captures
        weak_pointer w_ptr(weak_from_this());
        std::shared_ptr<Container> rx_buffer(rx_buffer_);
        if (use_strand)
          SocketAdaptor::read(&(*rx_buffer_)[0], buffer_size, strand_.wrap(
           [w_ptr, rx_buffer](boost::system::error_code const& ec, std::size_t bytes)
            { read_callback(w_ptr, ec, bytes, rx_buffer); }));
        else
          SocketAdaptor::read(&(*rx_buffer_)[0], buffer_size,
           [w_ptr, rx_buffer](boost::system::error_code const& ec, std::size_t bytes)
            { read_callback(w_ptr, ec, bytes, rx_buffer); });
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
                                std::shared_ptr<Container >) // rx_buffer)
      {
        auto pointer(ptr.lock());
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
                                 std::shared_ptr<Container>) // rx_buffer_)
      {
        auto pointer(ptr.lock());
        if (pointer && (boost::asio::error::operation_aborted != error))
        {
          if (error)
          {
            pointer->tx_queue_.clear();
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
        if (!transmitting_ && !tx_queue_.empty())
          tx_queue_.pop_front();

        transmitting_ = false;

        if (!tx_queue_.empty())
          write_data(ConstBuffers{boost::asio::buffer(tx_queue_.front())});

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
        auto pointer(ptr.lock());
        if (pointer && (boost::asio::error::operation_aborted != error))
        {
          if (!error)
          {
            pointer->connected_ = true;
            pointer->set_socket_options();
            if (!pointer->tx_queue_.empty())
              pointer->write_data
                (ConstBuffers{boost::asio::buffer(pointer->tx_queue_.front())});
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
                        boost::asio::ip::tcp::resolver::iterator host_iterator)
      {
        auto pointer(ptr.lock());
        if (pointer && (boost::asio::error::operation_aborted != error))
        {
          if (!error)
            pointer->handshake([ptr](boost::system::error_code const& ec)
                               { handshake_callback(ptr, ec); }, false);
          else
          {
            if ((boost::asio::error::host_not_found == error) &&
                (boost::asio::ip::tcp::resolver::iterator() != host_iterator))
              pointer->connect_socket
                ([ptr](boost::system::error_code const& ec,
                       boost::asio::ip::tcp::resolver::iterator host_itr)
                 { connect_callback(ptr, ec, host_itr ); }, ++host_iterator);
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
      explicit connection(boost::asio::io_service& io_service,
                          event_callback_type event_callback,
                          error_callback_type error_callback) :
        SocketAdaptor{io_service},
        strand_{io_service},
        rx_buffer_{std::make_shared<Container>()},
        tx_queue_{},
        event_callback_{event_callback},
        error_callback_{error_callback},
        timeout_{0},
        transmitting_{false},
        no_delay_{false},
        keep_alive_{false},
        connected_{false}
      {}

      /// Constructor for client connections.
      /// The constructor is private to ensure that it instances of the class
      /// can only be created as shared pointers by calling the create
      /// function below.
      /// @param io_service the boost asio io_service used by the underlying
      /// socket adaptor.
      explicit connection(boost::asio::io_service& io_service) :
        SocketAdaptor{io_service},
        strand_{io_service},
        rx_buffer_{std::make_shared<Container>()},
        tx_queue_{},
        event_callback_{},
        error_callback_{},
        timeout_{0},
        transmitting_{false},
        no_delay_{false},
        keep_alive_{false},
        connected_{false}
      {}

      /// Copy constructor deleted.
      connection(connection const&)=delete;

      /// Assignment operator deleted.
      connection& operator=(connection const&)=delete;

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
      static shared_pointer create(boost::asio::io_service& io_service,
                                   event_callback_type event_callback,
                                   error_callback_type error_callback)
      {
        return shared_pointer{new connection{io_service, event_callback,
                                             error_callback}};
      }

      /// The factory function to create client connections.
      /// @param io_service the boost asio io_service for the socket adaptor.
      static shared_pointer create(boost::asio::io_service& io_service)
      { return shared_pointer{new connection{io_service}}; }

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
        // local copy for the lambda capture
        weak_pointer w_ptr(weak_from_this());
        return SocketAdaptor::connect(host_name, port_name,
          [w_ptr](boost::system::error_code const& error,
                  boost::asio::ip::tcp::resolver::iterator host_iterator)
          { connect_callback(w_ptr, error, host_iterator);} );
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
        // local copy for the lambda capture
        weak_pointer w_ptr(weak_from_this());
        SocketAdaptor::start([w_ptr](boost::system::error_code const& ec)
                                  { handshake_callback(w_ptr, ec); });
      }

      /// @fn disconnect
      /// Shutdown the underlying socket adaptor.
      void disconnect()
      { SocketAdaptor::shutdown(); }

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
        rx_buffer_->resize(buffer_size);
        read_data();
      }

      /// @fn rx_buffer
      /// Accessor for the receive buffer.
      /// @pre Only valid within the receive event callback function.
      /// @retval the receive buffer.
      void rx_buffer(Container& packet)
      { rx_buffer_->swap(packet); }

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
      void send_data(Container packet)
      {
        bool was_empty(tx_queue_.empty());
        tx_queue_.push_back(std::move(packet));

        if (!transmitting_ && was_empty)
          write_data(ConstBuffers{ boost::asio::buffer(tx_queue_.front()) } );
      }

      /// @fn send_data
      /// Send the data in the buffers.
      /// @param buffers the data to write.
      /// @return true if the buffers are being sent, false otherwise.
      bool send_data(ConstBuffers buffers)
      {
        if (!transmitting_ && tx_queue_.empty())
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

      /// @fn remote_endpoint
      /// Get the remote endpoint of the socket.
      /// @return the remote endpoint of the socket.
      boost::asio::ip::tcp::endpoint remote_endpoint()
      { return SocketAdaptor::socket().remote_endpoint(); }
    };
  }
}

#endif
