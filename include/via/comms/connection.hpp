#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2023 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file connection.hpp
/// @brief The connection template class.
//////////////////////////////////////////////////////////////////////////////
#include "asio_if.hpp"
#ifdef ASIO_STANDALONE
  #include <asio/ssl.hpp>
  #include <asio/ssl/context.hpp>
#else
  #include <boost/asio/ssl.hpp>
  #include <boost/asio/ssl/context.hpp>
#endif
#include <memory>
#include <vector>
#include <type_traits>

namespace via
{
  namespace comms
  {
    /// @brief A TCP socket. 
    typedef ASIO::ip::tcp::socket tcp_socket;

    namespace ssl
    {
      /// @brief An SSL socket. 
      typedef ASIO::ssl::stream<ASIO::ip::tcp::socket> ssl_socket;
    }

    /// @fn resolve_host
    /// resolves the host name and port.
    /// @param io_context the asio io_context associated with the connection.
    /// @param host_name the host name.
    /// @param port_name the host port.
    /// @return a TCP resolver::results_type collection of endpoints
    inline ASIO::ip::tcp::resolver::results_type resolve_host
                      (ASIO::io_context& io_context,
                       const char* host_name, const char* port_name)
    {
      ASIO_ERROR_CODE ignoredEc;
      ASIO::ip::tcp::resolver resolver(io_context);
      return resolver.resolve(host_name, port_name, ignoredEc);
    }

    //////////////////////////////////////////////////////////////////////////
    /// @class connection
    /// A template class that buffers tcp or ssl comms sockets.
    /// The class can be configured to use either tcp or ssl sockets depending
    /// upon which class is provided as the S: tcp_socket or
    /// ssl::ssl_socket respectively.
    /// @see tcp_socket
    /// @see ssl::ssl_socket
    /// @tparam S the type of socket, use: tcp_socket or ssl::ssl_socket
    //////////////////////////////////////////////////////////////////////////
    template <typename S>
    class connection : public std::enable_shared_from_this<connection<S>>
    {
    public:

      /// This type.
      typedef connection<S> this_type;

      /// A weak pointer to a connection.
      typedef typename std::weak_ptr<connection<S>> weak_pointer;

      /// A shared pointer to a connection.
      typedef typename std::shared_ptr<connection<S>> shared_pointer;

      /// The enable_shared_from_this type of this class.
      typedef typename std::enable_shared_from_this<connection<S>> enable;

      /// The resolver_iterator type of the S
      typedef typename ASIO::ip::tcp::resolver::iterator resolver_iterator;

      /// Receive callback function type.
      typedef std::function<void (const char *, size_t, weak_pointer)> receive_callback_type;

      /// Event callback function type.
      typedef std::function<void (unsigned char, weak_pointer)> event_callback_type;

      /// Error callback function type.
      typedef std::function<void (ASIO_ERROR_CODE const&, weak_pointer)> error_callback_type;

      /// @fn default_port
      /// Get the default port number for the connection type
      /// @return the default port number
      static constexpr unsigned short default_port() noexcept
      {
        if constexpr (std::is_same<S, ssl::ssl_socket>::value)
          return 443;
        else
          return 80;
      }

      /// The default HTTP port.
      static constexpr unsigned short DEFAULT_HTTP_PORT{ default_port() };

      /// @fn is_disconnect
      /// This function determines whether the error is a socket disconnect,
      /// it also determines whether the caller should perform an SSL
      /// shutdown.
      /// @param error the error_code
      /// @retval ssl_shutdown - an SSL shutdown should be performed
      /// @return true if the socket is disconnected, false otherwise.
      static constexpr bool is_ssl_disconnect(ASIO_ERROR_CODE const& error) noexcept
      {
        if constexpr (std::is_same<S, ssl::ssl_socket>::value)
          return ASIO::error::get_ssl_category() == error.category();
        else
          return false;
      }

      static constexpr bool is_ssl_shutdown(ASIO_ERROR_CODE const& error) noexcept
      {
        if constexpr (std::is_same<S, ssl::ssl_socket>::value)
        {
          return
// SSL_R_SHORT_READ is no longer defined in openssl 1.1.x
#ifdef SSL_R_SHORT_READ
               (SSL_R_SHORT_READ != ERR_GET_REASON(error.value())) &&
#endif
               (SSL_R_PROTOCOL_IS_SHUTDOWN != ERR_GET_REASON(error.value()));
        }
        else
          return false;
      }

      /// The default size of the receive buffer.
      static constexpr size_t DEFAULT_RX_BUFFER_SIZE = 8192;

    private:

      S socket_;
      std::shared_ptr<std::vector<char>> rx_buffer_; ///< The receive buffer.
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
      bool write_data(ConstBuffers const& buffers)
      {
        if (connected_)
        {
          // local copies for lambdas
          weak_pointer weak_ptr(weak_from_this());
          std::shared_ptr<std::vector<char>> rx_buffer(rx_buffer_);
          ASIO::async_write(socket_, buffers,
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
        socket_.async_read_some(ASIO::mutable_buffer(rx_buffer_->data(), rx_buffer_->size()),
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
        const bool is_an_ssl_disconnect{is_ssl_disconnect(error)};
        const bool is_an_ssl_shutdown{is_an_ssl_disconnect && is_ssl_shutdown(error)};

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
          pointer->handshake_handler(error);
        }
      }

      void handshake_handler(ASIO_ERROR_CODE const& error)
      {
        weak_pointer ptr(weak_from_this());
        if (!error)
        {
          connected_ = true;
          event_callback_(CONNECTED, ptr);
          set_socket_options();
          enable_reception();
        }
        else
        {
          close();
          error_callback_(error, ptr);
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
            if constexpr (std::is_same<S, ssl::ssl_socket>::value)
            {
              pointer->socket_.async_handshake(ASIO::ssl::stream_base::client,
                [ptr](ASIO_ERROR_CODE const& error)
                { handshake_callback(ptr, error); });
            }
            else
            {
              ASIO_ERROR_CODE ec; // Default is success
              pointer->handshake_handler(ec);
            }
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
          socket().set_option
              (ASIO::ip::tcp::no_delay(no_delay_));
      }

      /// Set the socket's tcp keep alive status.
      void keep_alive()
      {
          socket().set_option
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
        socket().set_option(ASIO::detail::
                   socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>(timeout_));
        socket().set_option(ASIO::detail::
                   socket_option::integer<SOL_SOCKET, SO_SNDTIMEO>(timeout_));
#else
        // assume everything else is posix
        struct timeval tv;
        tv.tv_sec  = timeout_ / 1000;
        tv.tv_usec = 1000 * (timeout_ % 1000);
        setsockopt(socket().native_handle(),
                   SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&tv), sizeof(tv));
        setsockopt(socket().native_handle(),
                   SOL_SOCKET, SO_SNDTIMEO,
                   reinterpret_cast<const char*>(&tv), sizeof(tv));
#endif
      }

      /// Set the socket's receive buffer size.
      void resize_receive_buffer()
      {
        socket().set_option
            (ASIO::socket_base::receive_buffer_size(receive_buffer_size_));
      }

      /// Set the socket's send buffer size.
      void resize_send_buffer()
      {
        socket().set_option
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
      connection(S socket,
#ifdef HTTP_THREAD_SAFE
                 ASIO::strand<ASIO::io_context::executor_type> strand,
#endif
                 size_t rx_buffer_size,
                 receive_callback_type receive_callback = nullptr,
                 event_callback_type event_callback = nullptr,
                 error_callback_type error_callback = nullptr) :
        socket_(std::move(socket)),
        rx_buffer_(new std::vector<char>(rx_buffer_size, 0)),
        receive_callback_(receive_callback),
        event_callback_(event_callback),
        error_callback_(error_callback)
      {}

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

      /// @fn socket
      /// Accessor for the underlying socket.
      /// @return a reference to the socket.
      auto& socket()
      {
        if constexpr (std::is_same<S, ssl::ssl_socket>::value)
          return socket_.lowest_layer();
        else
          return socket_;
      }

      /// @fn connect_socket
      /// Attempts to connect to the host endpoints.
      /// @param connect_handler the connect callback function.
      /// @param endpoints the host endpoints.
      void connect_socket(ConnectHandler connect_handler,
                          ASIO::ip::tcp::resolver::results_type const& endpoints)
      {
        ASIO::async_connect(socket(), endpoints, connect_handler); 
      }

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

        if constexpr (std::is_same<S, ssl::ssl_socket>::value)
          socket_.set_verify_callback(ASIO::ssl::host_name_verification(host_name));

        auto endpoints{resolve_host(io_context, host_name, port_name)};
        if (endpoints.empty())
          return false;

        connect_socket([ptr]
            (ASIO_ERROR_CODE const& error, ASIO::ip::tcp::endpoint const& endpoint)
        { connect_callback(ptr, error, endpoint); }, endpoints);
        return true;
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
        if constexpr (std::is_same<S, ssl::ssl_socket>::value)
        {
          socket_.async_handshake(ASIO::ssl::stream_base::server,
                [weak_ptr](ASIO_ERROR_CODE const& error)
               { handshake_callback(weak_ptr, error); });
        }
        else
        {
          ASIO_ERROR_CODE ec; // Default is success
          handshake_handler(ec);
        }
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

        if constexpr (std::is_same<S, ssl::ssl_socket>::value)
        {
          // Cancel any pending operations
          ASIO_ERROR_CODE ec;
          socket().cancel(ec);
          connected_ = false;
          
          // Call shutdown with the callback
          std::shared_ptr<std::vector<char>> rx_buffer(rx_buffer_);
          shutdown_sent_ = true;
          socket_.async_shutdown([weak_ptr, rx_buffer]
                          (ASIO_ERROR_CODE const& error)
                          { write_callback(weak_ptr, error, 0, rx_buffer); });
        }
        else
        {
          ASIO_ERROR_CODE ec;
          socket_.shutdown(ASIO::ip::tcp::socket::shutdown_both, ec);

          ec = ASIO_ERROR_CODE(ASIO::error::eof);
          write_callback(weak_ptr, ec, 0, rx_buffer_);
        }
      }

      /// @fn close
      /// Close the underlying socket adaptor.
      /// Cancels all of the socket's callback functions.
      void close()
      {
        ASIO_ERROR_CODE ignoredEc;
        if (socket().is_open())
          socket().close (ignoredEc);
      }

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
      bool send_data(ConstBuffers buffers)
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
          socket().get_option(option);
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
          socket().get_option(option);
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
