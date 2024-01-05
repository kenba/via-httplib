#ifndef CONNECTION_HPP_VIA_HTTPLIB_
#define CONNECTION_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2024 Ken Barker
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
#ifndef ASIO_STANDALONE
#include <boost/system/error_code.hpp>
#endif
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
    /// @tparam SocketAdaptor the type of socket, use: tcp_adaptor or
    /// ssl::ssl_tcp_adaptor
    //////////////////////////////////////////////////////////////////////////
    template <typename SocketAdaptor>
    class connection : public SocketAdaptor,
      public std::enable_shared_from_this<connection<SocketAdaptor>>
    {
    public:

      /// The underlying socket type.
      typedef typename SocketAdaptor::socket_type socket_type;

      /// This type.
      typedef connection<SocketAdaptor> this_type;

      /// A weak pointer to a connection.
      typedef typename std::weak_ptr<connection<SocketAdaptor>> weak_pointer;

      /// A shared pointer to a connection.
      typedef typename std::shared_ptr<connection<SocketAdaptor>> shared_pointer;

      /// The enable_shared_from_this type of this class.
      typedef typename std::enable_shared_from_this<connection<SocketAdaptor>> enable;

      /// The resolver_iterator type of the SocketAdaptor
      typedef typename ASIO::ip::tcp::resolver::iterator resolver_iterator;

      /// Receive callback function type.
      typedef std::function<void (const char *, size_t, weak_pointer)> receive_callback_type;

      /// Event callback function type.
      typedef std::function<void (unsigned char, weak_pointer)> event_callback_type;

      /// Error callback function type.
      typedef std::function<void (ASIO_ERROR_CODE const&, weak_pointer)> error_callback_type;

    private:

      std::shared_ptr<std::vector<char>> rx_buffer_; ///< The receive buffer.
      ConstBuffers tx_buffers_{};                    ///< The transmit buffers.
      receive_callback_type receive_callback_{ nullptr }; ///< The receive callback function.
      event_callback_type event_callback_{ nullptr }; ///< The event callback function.
      error_callback_type error_callback_{ nullptr }; ///< The error callback function.
      /// The send and receive timeouts, in milliseconds, zero is disabled.
      int timeout_{ 0 };
      int receive_buffer_size_{ 0 };     ///< The socket receive buffer size.
      int send_buffer_size_{ 0 };        ///< The socket send buffer size.
      bool transmitting_{ false };       ///< Whether a write's in progress
      bool no_delay_{ false };           ///< The tcp no delay status.
      bool keep_alive_{ false };         ///< The tcp keep alive status.
      bool connected_{ false };          ///< If the socket is connected.
      bool disconnect_pending_{ false }; ///< Shutdown the socket after the next write.
      bool shutdown_sent_{ false };      ///< The SSL shutdown signal has been sent.

      /// @fn weak_from_this
      /// Get a weak_pointer to this instance.
      /// @return a weak_pointer to this connection.
      weak_pointer weak_from_this()
      { return weak_pointer(enable::shared_from_this()); }

      /// @fn write_data
      /// Write data via the socket adaptor.
      /// @param buffers the buffer(s) containing the message.
      /// @return true if connected, false otherwise.
      bool write_data(ConstBuffers&& buffers)
      {
        tx_buffers_ = std::move(buffers);

        if (connected_)
        {
          // local copies for lambdas
          weak_pointer weak_ptr(weak_from_this());
          std::shared_ptr<std::vector<char>> rx_buffer(rx_buffer_);
          SocketAdaptor::write(tx_buffers_,
            [weak_ptr, rx_buffer](ASIO_ERROR_CODE const& error,
                                  size_t bytes_transferred)
          { write_callback(weak_ptr, error, bytes_transferred, rx_buffer); });
        }

        return connected_;
      }

      /// @fn read_data
      /// Read data via the socket adaptor.
      void read_data()
      {
        // local copies for lambdas
        weak_pointer weak_ptr(weak_from_this());
        std::shared_ptr<std::vector<char>> rx_buffer(rx_buffer_);
        SocketAdaptor::read(ASIO::mutable_buffer(rx_buffer_->data(), rx_buffer_->size()),
          [weak_ptr, rx_buffer](ASIO_ERROR_CODE const& error,
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
      bool is_error_a_disconnect(ASIO_ERROR_CODE const& error)
      {
        switch(error.value())
        {
        case ASIO::error::eof:
        case ASIO::error::connection_refused:
        case ASIO::error::connection_reset:
        case ASIO::error::connection_aborted:
        case ASIO::error::bad_descriptor:
          return true;
        default:
          return false;
        }
      }

      /// @fn signal_error_or_disconnect
      /// This function is called whenever an error event occurs.
      /// It determines whether the error code is for an ssl shutdown
      /// in which case it sends a shutdown message or a disconnect in which
      /// case it sends a DISCONNECTED signal otherwise it sends the
      /// error signal.
      void signal_error_or_disconnect(ASIO_ERROR_CODE const& error)
      {
        bool is_an_ssl_disconnect(SocketAdaptor::is_disconnect(error));
        bool is_an_ssl_shutdown(is_an_ssl_disconnect &&
                                SocketAdaptor::is_shutdown(error));
        // if the other end has requested an SSL shutdown
        if (!shutdown_sent_ && is_an_ssl_shutdown)
          shutdown(); // reply with an SSL shutdown
        else
        {
          if (is_an_ssl_disconnect || is_error_a_disconnect(error))
            event_callback_(DISCONNECTED, weak_from_this());
          else
            error_callback_(error, weak_from_this());
        }
      }

      /// @fn read_callback
      /// The function called whenever a socket adaptor receives a data packet.
      /// It ensures that the connection still exists and the event is valid.
      /// If there was an error it calls the connection's signal_error_or_disconnect
      /// function, otherwise it calls the connection's read_handler.
      /// @param ptr a weak pointer to the connection
      /// @param error the boost asio error (if any).
      /// @param bytes_transferred the size of the received data packet.
      /// @param rx_buffer a shared pointer to the receive buffer to control
      /// object lifetime.
      static void read_callback(weak_pointer ptr,
                                ASIO_ERROR_CODE const& error,
                                size_t bytes_transferred,
                                std::shared_ptr<std::vector<char>> rx_buffer)
      {
        shared_pointer pointer(ptr.lock());
        if (pointer && (ASIO::error::operation_aborted != error))
        {
          if (error)
            pointer->signal_error_or_disconnect(error);
          else
          {
            pointer->receive_callback_(rx_buffer->data(), bytes_transferred, ptr);
            if (!pointer->shutdown_sent_)
              pointer->enable_reception();
          }
        }
      }

      /// @fn write_callback
      /// The function called whenever a socket adaptor has sent a data packet.
      /// It ensures that the connection still exists and the event is valid.
      /// If there was an error it calls the connection's signal_error_or_disconnect
      /// function, otherwise it calls the connection's write_handler.
      /// @param ptr a weak pointer to the connection
      /// @param error the boost asio error (if any).
      /// @param bytes_transferred the size of the sent data packet.
      /// @param rx_buffer a shared pointer to the receive buffer to control
      /// object lifetime.
      static void write_callback(weak_pointer ptr,
                                 ASIO_ERROR_CODE const& error,
                                 size_t bytes_transferred,
                                 std::shared_ptr<std::vector<char>>) // rx_buffer)
      {
        shared_pointer pointer(ptr.lock());
        if (pointer && (ASIO::error::operation_aborted != error))
        {
          // Disconnect the socket as soon as a shutdown request has been sent
          // I.e. don't wait for the reply
          if (pointer->shutdown_sent_)
            pointer->event_callback_(DISCONNECTED, ptr);
          else if (error)
          {
            pointer->signal_error_or_disconnect(error);
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
      /// @param bytes_transferred the size of the sent data packet.
      void write_handler(size_t) // bytes_transferred
      {
        transmitting_ = false;
        event_callback_(SENT, weak_from_this());
      }

      /// @fn handshake_callback
      /// The function called whenever a socket adaptor receives a connection
      /// handshake.
      /// It ensures that the connection still exists and the event is valid.
      /// If there was an error, it closes the connection, signals the error
      /// and disconnects the connection.
      /// Otherwise, it calls enable_reception to listen on the connection
      /// and signals that it's connected.
      /// @param ptr a weak pointer to the connection
      /// @param error the boost asio error (if any).
      static void handshake_callback(weak_pointer ptr,
                                     ASIO_ERROR_CODE const& error)
      {
        shared_pointer pointer(ptr.lock());
        if (pointer && (ASIO::error::operation_aborted != error))
        {
          if (!error)
          {
            pointer->connected_ = true;
            pointer->event_callback_(CONNECTED, ptr);
            pointer->set_socket_options();
            pointer->enable_reception();
          }
          else
          {
            pointer->close();
            pointer->error_callback_(error, ptr);
          }
        }
      }

      /// @fn connect_callback
      /// The function called whenever a socket adaptor attempts to connect.
      /// It ensures that the connection still exists and the event is valid.
      /// If there was no error, it attempts to handshake on the connection -
      /// this shall always be accepted for an unencrypted connection.
      /// Otherwise it closes the socket, signals an error and disconnects the
      /// connection.
      /// @param ptr a weak pointer to the connection
      /// @param error the boost asio error (if any).
      /// @param endpoint the host endpoint to connect to.
      static void connect_callback(weak_pointer ptr,
                                   ASIO_ERROR_CODE const& error,
                                   ASIO::ip::tcp::endpoint const&) // endpoint,
      {
        shared_pointer pointer(ptr.lock());
        if (pointer && (ASIO::error::operation_aborted != error))
        {
          if (!error)
          {
            pointer->handshake([ptr](ASIO_ERROR_CODE const& error)
              { handshake_callback(ptr, error); }, false);
          }
          else
          {
            pointer->close();
            pointer->error_callback_(error, ptr);
          }
        }
      }

      /// Set the socket's tcp no delay status.
      /// If no_delay_ is set it disables the Nagle algorithm on the socket.
      void no_delay()
      {
          SocketAdaptor::socket().set_option
              (ASIO::ip::tcp::no_delay(no_delay_));
      }

      /// Set the socket's tcp keep alive status.
      void keep_alive()
      {
          SocketAdaptor::socket().set_option
              (ASIO::socket_base::keep_alive(keep_alive_));
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
        SocketAdaptor::socket().set_option(ASIO::detail::
                   socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>(timeout_));
        SocketAdaptor::socket().set_option(ASIO::detail::
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
            (ASIO::socket_base::receive_buffer_size(receive_buffer_size_));
      }

      /// Set the socket's send buffer size.
      void resize_send_buffer()
      {
        SocketAdaptor::socket().set_option
            (ASIO::socket_base::send_buffer_size(send_buffer_size_));
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

      /// connection constructor
      /// @param socket the asio socket associated with this connection
      /// @param strand the asio strand used by the socket.
      /// @param rx_buffer_size the size of the receive_buffer.
      /// @param receive_callback the receive callback function, default nullptr.
      /// @param event_callback the event callback function, default nullptr.
      /// @param error_callback the error callback function, default nullptr.
      connection(socket_type socket,
                 size_t rx_buffer_size,
                 receive_callback_type receive_callback = nullptr,
                 event_callback_type event_callback = nullptr,
                 error_callback_type error_callback = nullptr) :
        SocketAdaptor(std::move(socket)),
        rx_buffer_(new std::vector<char>(rx_buffer_size, 0)),
        receive_callback_(receive_callback),
        event_callback_(event_callback),
        error_callback_(error_callback)
      {}

      connection(connection const&) = delete;
      connection& operator=(connection const&) = delete;

      /// The destructor calls close to ensure that all of the socket's
      /// callback functions are cancelled so that the object can (eventually)
      /// be destroyed.
      ~connection()
      { close(); }

      /// @fn set_receive_callback
      /// Function to set the receive callback function.
      /// @param receive_callback the receive callback function.
      void set_receive_callback(receive_callback_type receive_callback)
      { receive_callback_ = receive_callback; }

      /// @fn set_event_callback
      /// Function to set the event callback function.
      /// @param event_callback the event callback function.
      void set_event_callback(event_callback_type event_callback)
      { event_callback_ = event_callback; }

      /// @fn set_error_callback
      /// Function to set the error callback function.
      /// For use with the client connection factory function.
      /// @see create(ASIO::io_context& io_context)
      /// @param error_callback the error callback function.
      void set_error_callback(error_callback_type error_callback)
      { error_callback_ = error_callback; }

      /// Set the connection's rx_buffer_ size.
      void set_rx_buffer_size(size_t rx_buffer_size)
      { rx_buffer_->resize(rx_buffer_size, 0); }

      /// @fn connect
      /// Connect the underlying socket adaptor to the given host name and
      /// port.
      /// @pre To be called by "client" connections only after the event
      /// callbacks have been set.
      /// Server connections are accepted by the server instead.
      /// @param io_context the asio io_context associated with this connection.
      /// @param host_name the host to connect to.
      /// @param port_name the port to connect to.
      bool connect(ASIO::io_context& io_context,
                    const char* host_name, const char* port_name)
      {
        weak_pointer ptr(weak_from_this());
        return SocketAdaptor::connect(io_context, host_name, port_name,
          [ptr](ASIO_ERROR_CODE const& error, ASIO::ip::tcp::endpoint const& endpoint)
            { connect_callback(ptr, error, endpoint); });
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
        SocketAdaptor::start([weak_ptr](ASIO_ERROR_CODE const& error)
          { handshake_callback(weak_ptr, error); });
      }

      /// @fn disconnect
      /// Shutdown the socket after the last message has been sent.
      void disconnect()
      {
        // If nothing is currently being sent
        if (!transmitting_)
          shutdown();
        else // shutdown the socket in the write callback
          disconnect_pending_ = true;
      }

      /// @fn shutdown
      /// Shutdown the socket now.
      void shutdown()
      {
        // local copies for the lambda
        weak_pointer weak_ptr(weak_from_this());
        std::shared_ptr<std::vector<char>> rx_buffer(rx_buffer_);

        // Call shutdown with the callback
        shutdown_sent_ = true;
        SocketAdaptor::shutdown([weak_ptr, rx_buffer]
                        (ASIO_ERROR_CODE const& error, int bytes)
                        { write_callback(weak_ptr, error, bytes, rx_buffer); });
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
        read_data();
      }

      /// @fn connected
      /// Accessor for the connected_ flag.
      bool connected() const noexcept
      { return connected_; }

      /// Accessor to set the connected_ flag.
      void set_connected(bool enable) noexcept
      { connected_ = enable; }

      /// Send the data in the buffers.
      /// @param buffers the data to write.
      /// @return true if the buffers are being sent, false otherwise.
      bool send_data(ConstBuffers&& buffers)
      {
        if (!transmitting_)
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
          ASIO::socket_base::receive_buffer_size option;
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
          ASIO::socket_base::send_buffer_size option;
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
