#ifndef SERVER_HPP_VIA_HTTPLIB_
#define SERVER_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2023 Ken Barker
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file server.hpp
/// @brief The server template class.
//////////////////////////////////////////////////////////////////////////////
#include "connection.hpp"
#ifdef HTTP_SSL
  #ifdef ASIO_STANDALONE
    #include <asio/ssl/context.hpp>
  #else
    #include <boost/asio/ssl/context.hpp>
  #endif
#endif
#include <string>
#include <sstream>
#ifdef HTTP_THREAD_SAFE
#include "via/thread/threadsafe_hash_map.hpp"
#else
#include <set>
#endif

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class server
    /// A template class for implementing a tcp or ssl server using buffered
    /// connections.
    /// The class can be configured to use either tcp or ssl sockets depending
    /// upon which class is provided as the SocketAdaptor: tcp_adaptor or
    /// @see connection
    /// @see tcp_adaptor
    /// @see ssl::ssl_tcp_adaptor
    /// @tparam SocketAdaptor the type of socket, use: tcp_adaptor or
    /// ssl::ssl_tcp_adaptor
    //////////////////////////////////////////////////////////////////////////
    template <typename SocketAdaptor>
    class server
    {
    public:

      /// The connection type used by this server.
      typedef connection<SocketAdaptor> connection_type;

      typedef typename connection_type::socket_type socket_type;

      /// A set of connections.
#ifdef HTTP_THREAD_SAFE
      typedef thread::threadsafe_hash_map<void *, std::shared_ptr<connection_type>>
        connections;
#else
      typedef std::set<std::shared_ptr<connection_type> > connections;
#endif

      /// Receive callback function type.
      typedef typename connection_type::receive_callback_type receive_callback_type;

      /// Event callback function type.
      typedef typename connection_type::event_callback_type event_callback_type;

      /// Error callback function type.
      typedef typename connection_type::error_callback_type error_callback_type;

    private:
      /// The asio::io_context to use.
      ASIO::io_context& io_context_;

#ifdef HTTP_SSL
      /// The ssl::context for ssl_tcp_adaptor
      ASIO::ssl::context& ssl_context_;
#endif

      /// The IPv6 acceptor for this server.
      ASIO::ip::tcp::acceptor acceptor_v6_;

      /// The IPv4 acceptor for this server.
      ASIO::ip::tcp::acceptor acceptor_v4_;

      /// The next socket to be accepted.
      ASIO::ip::tcp::socket next_socket_;

      /// The connections established with this server.
      connections connections_{};

      receive_callback_type receive_callback_{ nullptr }; ///< The receive callback function.
      event_callback_type event_callback_{nullptr};   ///< The event callback function.
      error_callback_type error_callback_{nullptr};   ///< The error callback function.

      size_t rx_buffer_size_{SocketAdaptor::DEFAULT_RX_BUFFER_SIZE}; ///< The size of the receive buffer.

      // Socket parameters

      int receive_buffer_size_{0}; ///< The tcp receive buffer size.
      int send_buffer_size_{0};    ///< The tcp send buffer size.

      /// The connection timeouts, in milliseconds, zero is disabled.
      int timeout_{0};
      bool keep_alive_{false};       ///< The tcp keep alive status.

      /// @accept_handler
      /// The callback function called by the acceptor when it accepts a
      /// new connection.
      /// If there is no error, it performs the following:
      /// - calls "start" on the new connection.
      /// - connects the connections event and error signals to the servers
      /// - add the new connection to the set
      /// - restart the acceptor to look for new connections.
      /// @param error the error, if any.
      void accept_handler(const ASIO_ERROR_CODE& error)
      {
        if ((acceptor_v6_.is_open() || acceptor_v4_.is_open())&&
            (ASIO::error::operation_aborted != error))
        {
          if (!error)
          {
            auto next_connection = std::make_shared<connection_type>
#ifdef HTTP_SSL
              (socket_type(std::move(next_socket_), ssl_context_),
#else
              (std::move(next_socket_),
#endif
#ifdef HTTP_THREAD_SAFE
              io_context_,
#endif
              rx_buffer_size_,
              [this](const char *data, size_t size, std::weak_ptr<connection_type> ptr)
                { receive_handler(data, size, ptr); },
              [this](unsigned char event, std::weak_ptr<connection_type> ptr)
                { event_handler(event, ptr); },
              [this](ASIO_ERROR_CODE const& error,
                    std::weak_ptr<connection_type> ptr)
                { error_handler(error, ptr); }
              );

            next_socket_ = ASIO::ip::tcp::socket(io_context_);

#ifdef HTTP_THREAD_SAFE
            connections_.emplace(next_connection.get(), next_connection);
#else
            connections_.emplace(next_connection);
#endif
            // Set no delay, i.e. disable the Nagle algorithm
            // A server will want to send messages immediately
            bool no_delay{true};
            next_connection->start(no_delay, keep_alive_, timeout_,
                                    receive_buffer_size_, send_buffer_size_);
          }

          start_accept();
        }
      }

      /// @fn receive_handler.
      /// It just forwards the connection's received event signal with the receive buffer.
      /// @param data pointer to the receive buffer.
      /// @param size the number of bytes received.
      /// @param connection a weak_pointer to the connection that received the data.
      void receive_handler(const char *data, size_t size, std::weak_ptr<connection_type> ptr)
      {
        receive_callback_(data, size, ptr);
      }

      /// @fn event_handler.
      /// It forwards the connection's event signal.
      /// For a disconnected event, it deletes the connection.
      /// @param event the event, @see event_type.
      /// @param connection a weak_pointer to the connection that sent the
      /// event.
      void event_handler(unsigned char event, std::weak_ptr<connection_type> ptr)
      {
        event_callback_(event, ptr);
        if (event == DISCONNECTED)
        {
          if (std::shared_ptr<connection_type> connection = ptr.lock())
          {
#ifdef HTTP_THREAD_SAFE
            connections_.erase(connection.get());
#else
            // search for the connection to delete
            auto iter(connections_.find(connection));
            if (iter != connections_.end())
              connections_.erase(iter);
#endif
          }
        }
      }

      /// @fn error_handler.
      /// It just forwards the connection's error signal.
      /// @param error the boost asio error.
      /// @param connection a weak_pointer to the connection that sent the
      /// error.
      void error_handler(const ASIO_ERROR_CODE& error,
                         std::weak_ptr<connection_type> connection)
      { error_callback_(error, connection); }

      /// @fn start_accept
      /// Wait for connections.
      void start_accept()
      {
        if (acceptor_v6_.is_open())
          acceptor_v6_.async_accept(next_socket_,
            [this](ASIO_ERROR_CODE const& error)
              { accept_handler(error); });
        if (acceptor_v4_.is_open())
          acceptor_v4_.async_accept(next_socket_,
            [this](ASIO_ERROR_CODE const& error)
              { accept_handler(error); });
      }

    public:

      /// Copy constructor deleted to disable copying.
      server(server const&) = delete;

      /// Assignment operator deleted to disable copying.
      server& operator=(server) = delete;


      /// The server constructor.
      /// @pre the event_callback and error_callback functions must exist.
      /// E.g. if either of them are class member functions then the class
      /// MUST have been constructed BEFORE this constructor is called.
      /// @param io_context the boost asio io_context used by the acceptor
      /// and connections.
      /// @param ssl_context the ssl context, only required for TLS servers.
      explicit server(ASIO::io_context& io_context
#ifdef HTTP_SSL
             , ASIO::ssl::context& ssl_context
#endif
      ) :
        io_context_(io_context),
#ifdef HTTP_SSL
        ssl_context_(ssl_context),
#endif
        acceptor_v6_(io_context),
        acceptor_v4_(io_context),
        next_socket_(io_context)
      {}

#ifdef HTTP_SSL

      /// @fn ssl_context
      /// A function to manage the ssl context for the ssl connections.
      /// @return ssl_context the ssl context.
      ASIO::ssl::context& ssl_context() noexcept
      {
        return ssl_context_;
      }

#endif

      /// Destructor, close the connections.
      ~server()
      { close(); }

      /// @fn set_receive_callback
      /// Set the receive_callback function.
      /// @param receive_callback the receive callback function.
      void set_receive_callback(receive_callback_type receive_callback) noexcept
      { receive_callback_ = receive_callback; }

      /// @fn set_event_callback
      /// Set the event_callback function.
      /// @param event_callback the event callback function.
      void set_event_callback(event_callback_type event_callback) noexcept
      { event_callback_ = event_callback; }

      /// @fn set_error_callback
      /// Set the error_callback function.
      /// @param error_callback the error callback function.
      void set_error_callback(error_callback_type error_callback) noexcept
      { error_callback_ = error_callback; }

      /// @fn accept_connections
      /// Create the acceptor and wait for connections.
      /// @param port the port number to serve.
      /// @param ipv4_only whether an IPV4 only server is required.
      /// @return the boost error code, false if no error occured
      ASIO_ERROR_CODE accept_connections(unsigned short port, bool ipv4_only)
      {
        // Determine whether the IPv6 acceptor accepts both IPv6 & IPv4
        ASIO::ip::v6_only ipv6_only(false);
        ASIO_ERROR_CODE ec;

        // Open the IPv6 acceptor unless IPv4 only mode
        if (!ipv4_only)
        {
          acceptor_v6_.open(ASIO::ip::tcp::v6(), ec);
          if (!ec)
          {
            acceptor_v6_.set_option(ipv6_only, ec);
            acceptor_v6_.get_option(ipv6_only);
            acceptor_v6_.set_option
              (ASIO::ip::tcp::acceptor::reuse_address(true));
            acceptor_v6_.bind
              (ASIO::ip::tcp::endpoint(ASIO::ip::tcp::v6(), port));
            acceptor_v6_.listen();
          }
        }

        // Open the IPv4 acceptor if the IPv6 acceptor is not open or it
        // only supports IPv6
        if (!acceptor_v6_.is_open() || ipv6_only)
        {
          acceptor_v4_.open(ASIO::ip::tcp::v4(), ec);
          if (!ec)
          {
            acceptor_v4_.set_option
                (ASIO::ip::tcp::acceptor::reuse_address(true));
            acceptor_v4_.bind
              (ASIO::ip::tcp::endpoint(ASIO::ip::tcp::v4(), port));
            acceptor_v4_.listen();
          }
        }

        start_accept();
        return ec;
      }

      /// Set the size of the receive buffer.
      /// @param size the new size of the receive buffer.
      void set_rx_buffer_size(size_t size) noexcept
      { rx_buffer_size_ = size; }

      /// @fn set_timeout
      /// Set the send and receive timeouts value for all future connections.
      /// @pre sockets may remain open forever
      /// @post sockets will close if no activity has occured after the
      /// timeout period.
      /// @param timeout the timeout in milliseconds.
      void set_timeout(int timeout) noexcept
      { timeout_ = timeout; }

      /// @fn set_keep_alive
      /// Set the tcp keep alive status for all future connections.
      /// @param enable if true enables the tcp socket keep alive status.
      void set_keep_alive(bool enable) noexcept
      { keep_alive_ = enable; }

      /// Set the size of the tcp sockets receive buffer.
      /// @param size the new size of the socket receive buffer, must be > 0.
      void set_receive_buffer_size(int size) noexcept
      { receive_buffer_size_ = size; }

      /// Set the size of the tcp sockets send buffer.
      /// @param size the new size of the socket send buffer, must be > 0.
      void set_send_buffer_size(int size) noexcept
      { send_buffer_size_ = size; }

      /// @fn close
      /// Close the server and all of the connections associated with it.
      void close()
      {
        if (acceptor_v6_.is_open())
          acceptor_v6_.close();

        if (acceptor_v4_.is_open())
          acceptor_v4_.close();

        connections_.clear();
      }
    };
  }
}

#endif
