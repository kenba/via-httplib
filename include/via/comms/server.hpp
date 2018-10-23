#ifndef SERVER_HPP_VIA_HTTPLIB_
#define SERVER_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Ken Barker
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file server.hpp
/// @brief The server template class.
//////////////////////////////////////////////////////////////////////////////
#include "connection.hpp"
#include "via/no_except.hpp"
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
    /// @tparam Container the container to use for the rx & tx buffers,
    /// std::vector<char> or std::string.
    /// It must contain a contiguous array of bytes. E.g. std::string or
    /// std::array<char, size>
    //////////////////////////////////////////////////////////////////////////
    template <typename SocketAdaptor, typename Container = std::vector<char>>
    class server
    {
    public:

      /// The connection type used by this server.
      typedef connection<SocketAdaptor, Container> connection_type;

      /// A set of connections.
#ifdef HTTP_THREAD_SAFE
      typedef thread::threadsafe_hash_map<void *, std::shared_ptr<connection_type>>
        connections;
#else
      typedef std::set<std::shared_ptr<connection_type> > connections;
#endif

      /// Event callback function type.
      typedef typename connection_type::event_callback_type event_callback_type;

      /// Error callback function type.
      typedef typename connection_type::error_callback_type error_callback_type;

    private:
      /// The asio::io_service to use.
      ASIO::io_service& io_service_;

      /// The IPv6 acceptor for this server.
      ASIO::ip::tcp::acceptor acceptor_v6_;

      /// The IPv4 acceptor for this server.
      ASIO::ip::tcp::acceptor acceptor_v4_;

      /// The next connection to be accepted.
      std::shared_ptr<connection_type> next_connection_;

      /// The connections established with this server.
      connections connections_;

      /// The password. Only used by SSL servers.
      std::string password_;

      event_callback_type event_callback_;   ///< The event callback function.
      error_callback_type error_callback_;   ///< The error callback function.

      size_t rx_buffer_size_; ///< The size of the receive buffer.

      // Socket parameters

      int receive_buffer_size_; ///< The tcp receive buffer size.
      int send_buffer_size_;    ///< The tcp send buffer size.

      /// The connection timeouts, in milliseconds, zero is disabled.
      int timeout_;
      bool no_delay_;         ///< The tcp no delay status.
      bool keep_alive_;       ///< The tcp keep alive status.

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
          if (error)
            error_callback_(error, next_connection_);
          else
          {
#ifdef HTTP_THREAD_SAFE
            connections_.emplace(next_connection_.get(), next_connection_);
#else
            connections_.emplace(next_connection_);
#endif
            next_connection_->start(no_delay_, keep_alive_, timeout_,
                                    receive_buffer_size_, send_buffer_size_);
          }

          next_connection_.reset();

          start_accept();
        }
      }

      /// @fn event_handler.
      /// It forwards the connection's event signal.
      /// For a disconnected event, it deletes the connection.
      /// @param event the event, @see event_type.
      /// @param connection a weak_pointer to the connection that sent the
      /// event.
      void event_handler(int event, std::weak_ptr<connection_type> ptr)
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
        next_connection_ = connection_type::create(io_service_,
          [this](int event, std::weak_ptr<connection_type> ptr)
            { event_handler(event, ptr); },
          [this](ASIO_ERROR_CODE const& error,
                 std::weak_ptr<connection_type> ptr)
            { error_handler(error, ptr); });

        if (acceptor_v6_.is_open())
          acceptor_v6_.async_accept(next_connection_->socket(),
            [this](ASIO_ERROR_CODE const& error)
              { accept_handler(error); });
        if (acceptor_v4_.is_open())
          acceptor_v4_.async_accept(next_connection_->socket(),
            [this](ASIO_ERROR_CODE const& error)
              { accept_handler(error); });
      }

    public:

      /// Copy constructor deleted to disable copying.
      server(server const&) = delete;

      /// Assignment operator deleted to disable copying.
      server& operator=(server) = delete;

      /// The server constructor.
      /// @post the event_callback and error_callback functions MUST be set
      /// AFTER this constructor has been called.
      /// @see set_event_callback
      /// @see set_error_callback
      /// @param io_service the boost asio io_service used by the acceptor
      /// and connections.
      explicit server(ASIO::io_service& io_service) :
        io_service_(io_service),
        acceptor_v6_(io_service),
        acceptor_v4_(io_service),
        next_connection_(),
        connections_(),
        password_(),
        event_callback_(),
        error_callback_(),
        rx_buffer_size_(SocketAdaptor::DEFAULT_RX_BUFFER_SIZE),
        receive_buffer_size_(0),
        send_buffer_size_(0),
        timeout_(0),
        no_delay_(false),
        keep_alive_(false)
      {}

      /// The server constructor.
      /// @pre the event_callback and error_callback functions must exist.
      /// E.g. if either of them are class member functions then the class
      /// MUST have been constructed BEFORE this constructor is called.
      /// @param io_service the boost asio io_service used by the acceptor
      /// and connections.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      explicit server(ASIO::io_service& io_service,
                      event_callback_type event_callback,
                      error_callback_type error_callback) :
        io_service_(io_service),
        acceptor_v6_(io_service),
        acceptor_v4_(io_service),
        next_connection_(),
        connections_(),
        password_(),
        event_callback_(event_callback),
        error_callback_(error_callback),
        timeout_(0),
        no_delay_(false),
        keep_alive_(false)
      {}

      /// Destructor, close the connections.
      ~server()
      { close(); }

      /// @fn set_event_callback
      /// Set the event_callback function.
      /// For use with the Constructor or create function that doesn't take
      /// an event_callback parameter.
      /// @see server(ASIO::io_service& io_service)
      /// @see create(ASIO::io_service& io_service)
      /// @param event_callback the event callback function.
      void set_event_callback(event_callback_type event_callback) NOEXCEPT
      { event_callback_ = event_callback; }

      /// @fn set_error_callback
      /// Set the error_callback function.
      /// For use with the Constructor or create function that doesn't take
      /// an error_callback parameter.
      /// @see server(ASIO::io_service& io_service)
      /// @see create(ASIO::io_service& io_service)
      /// @param error_callback the error callback function.
      void set_error_callback(error_callback_type error_callback) NOEXCEPT
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

#ifdef HTTP_SSL
      /// @fn password
      /// Get the password.
      /// @pre It must be an SSL server.
      /// @return The password.
      const std::string password(std::size_t, // max_length,
                       ASIO::ssl::context::password_purpose)// purpose)
        const NOEXCEPT
      { return password_; }

      /// @fn set_password
      /// Set the password.
      /// @pre It must be an SSL server.
      /// @param password the password
      void set_password(std::string const& password)
      {
        password_ = password;
        connection_type::ssl_context().set_password_callback
            ([this](std::size_t max_length,
                    ASIO::ssl::context::password_purpose purpose)
        { return server::password(max_length, purpose); });
      }
#endif

      /// Set the size of the receive buffer.
      /// @param size the new size of the receive buffer.
      void set_rx_buffer_size(size_t size) NOEXCEPT
      { rx_buffer_size_ = size; }

      /// @fn set_timeout
      /// Set the send and receive timeouts value for all future connections.
      /// @pre sockets may remain open forever
      /// @post sockets will close if no activity has occured after the
      /// timeout period.
      /// @param timeout the timeout in milliseconds.
      void set_timeout(int timeout) NOEXCEPT
      { timeout_ = timeout; }

      /// @fn set_keep_alive
      /// Set the tcp keep alive status for all future connections.
      /// @param enable if true enables the tcp socket keep alive status.
      void set_keep_alive(bool enable) NOEXCEPT
      { keep_alive_ = enable; }

      /// Set the size of the tcp sockets receive buffer.
      /// @param size the new size of the socket receive buffer, must be > 0.
      void set_receive_buffer_size(int size) NOEXCEPT
      { receive_buffer_size_ = size; }

      /// Set the size of the tcp sockets send buffer.
      /// @param size the new size of the socket send buffer, must be > 0.
      void set_send_buffer_size(int size) NOEXCEPT
      { send_buffer_size_ = size; }

      /// @fn set_no_delay
      /// Set the tcp no delay status for all future connections.
      /// @param enable if true it disables the Nagle algorithm.
      void set_no_delay(bool enable) NOEXCEPT
      { no_delay_ = enable; }

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
