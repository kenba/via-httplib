#ifndef SERVER_HPP_VIA_HTTPLIB_
#define SERVER_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file server.hpp
/// @brief The server template class.
//////////////////////////////////////////////////////////////////////////////
#include "connection.hpp"
#include <set>
#include <string>
#include <sstream>

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
    /// ssl::ssl_tcp_adaptor respectively.
    /// The other template parameters configure the type of container to use
    /// for the transmit buffers, the size of the receive buffer and
    /// whether to use asio::strand for an io_service running in multiple
    /// treads.
    /// @see connection
    /// @see tcp_adaptor
    /// @see ssl::ssl_tcp_adaptor
    /// @param SocketAdaptor the type of socket, use: tcp_adaptor or
    /// ssl::ssl_tcp_adaptor
    /// @param Container the container to use for the tx buffer, default
    /// std::vector<char>.
    /// It must contain a contiguous array of bytes. E.g. std::string or
    /// std::array<char, size>
    /// @param buffer_size the size of the receive buffer, default 8192 bytes.
    /// @param use_strand if true use an asio::strand to wrap the handlers,
    /// default false.
    //////////////////////////////////////////////////////////////////////////
    template <typename SocketAdaptor, typename Container = std::vector<char>,
              size_t buffer_size = DEFAULT_BUFFER_SIZE, bool use_strand = false>
    class server
    {
    public:

      /// The connection type used by this server.
      typedef connection<SocketAdaptor, buffer_size, use_strand>
                                                              connection_type;

      /// A set of connections.
      typedef std::set<std::shared_ptr<connection_type> > connections;

      /// An iterator to the connections;
      typedef typename connections::iterator connections_iterator;

      /// Event callback function type.
      typedef typename connection_type::event_callback_type event_callback_type;

      /// Error callback function type.
      typedef typename connection_type::error_callback_type error_callback_type;

    private:
      /// The asio::io_service to use.
      boost::asio::io_service& io_service_;

      /// The acceptor for this server.
      boost::asio::ip::tcp::acceptor acceptor_;

      /// The next connection to be accepted.
      std::shared_ptr<connection_type> next_connection_;

      /// The connections established with this server.
      connections connections_;

      /// The password. Only used by SSL servers.
      std::string password_;

      event_callback_type event_callback_;   ///< The event callback function.
      error_callback_type error_callback_;   ///< The error callback function.

      /// The send and receive timeouts, in milliseconds, zero is disabled.
      int timeout_;
      bool no_delay_;                        ///< The tcp no delay status.
      bool keep_alive_;                      ///< The tcp keep alive status.

      /// @accept_handler
      /// The callback function called by the acceptor when it accepts a
      /// new connection.
      /// If there is no error, it performs the following:
      /// - calls "start" on the new connection.
      /// - connects the connections event and error signals to the servers
      /// - add the new connection to the set
      /// - restart the acceptor to look for new connections.
      /// @param error the error, if any.
      void accept_handler(const boost::system::error_code& error)
      {
        if (acceptor_.is_open() &&
            (boost::asio::error::operation_aborted != error))
        {
          if (error)
            error_callback_(error, next_connection_);
          else
          {
            next_connection_->start(no_delay_, keep_alive_, timeout_);
            connections_.insert(next_connection_);
            next_connection_.reset();
          }

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
          if (auto connection = ptr.lock())
          {
            // search for the connection to delete
            auto iter(connections_.find(connection));
            if (iter != connections_.end())
              connections_.erase(iter);
          }
        }
      }

      /// @fn error_handler.
      /// It just forwards the connection's error signal.
      /// @param error the boost asio error.
      /// @param connection a weak_pointer to the connection that sent the
      /// error.
      void error_handler(const boost::system::error_code& error,
                         std::weak_ptr<connection_type> connection)
      { error_callback_(error, connection); }

      /// @fn start_accept
      /// Wait for connections.
      void start_accept()
      {
        next_connection_ = connection_type::create(io_service_,
                                std::bind(&server::event_handler, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2),
                                std::bind(&server::error_handler, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2));
        acceptor_.async_accept(next_connection_->socket(),
                               std::bind(&server::accept_handler, this,
                                         std::placeholders::_1));
      }

    public:

      /// The server constructor.
      /// @post the event_callback and error_callback functions MUST be set
      /// AFTER this constructor has been called.
      /// @see set_event_callback
      /// @see set_error_callback
      /// @param io_service the boost asio io_service used by the acceptor
      /// and connections.
      explicit server(boost::asio::io_service& io_service) :
        io_service_(io_service),
        acceptor_{io_service},
        next_connection_{},
        connections_{},
        password_{},
        event_callback_{},
        error_callback_{},
        timeout_{0},
        no_delay_{false},
        keep_alive_{false}
      {}

      /// The server constructor.
      /// @pre the event_callback and error_callback functions must exist.
      /// E.g. if either of them are class member functions then the class
      /// MUST have been constructed BEFORE this constructor is called.
      /// @param io_service the boost asio io_service used by the acceptor
      /// and connections.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      explicit server(boost::asio::io_service& io_service,
                      event_callback_type event_callback,
                      error_callback_type error_callback) :
        io_service_(io_service),
        acceptor_{io_service},
        next_connection_{},
        connections_{},
        password_{},
        event_callback_{event_callback},
        error_callback_{error_callback},
        timeout_{0},
        no_delay_{false},
        keep_alive_{false}
      {}

      /// Copy constructor deleted.
      server(server const&)=delete;

      /// Assignment operator deleted.
      server& operator=(server const&)=delete;

      /// Destructor, close the connections.
      ~server()
      { close(); }

      /// Function to create a shared pointer to a server.
      /// @post the event_callback and error_callback functions MUST be set
      /// AFTER this function has been called.
      /// @see set_event_callback
      /// @see set_error_callback
      /// @param io_service the boost asio io_service used by the server.
      static std::shared_ptr<server> create(boost::asio::io_service& io_service)
      { return std::shared_ptr<server>{new server{io_service}}; }

      /// Function to create a shared pointer to a server.
      /// @pre the event_callback and error_callback functions must exist.
      /// E.g. if either of them are class member functions then the class
      /// MUST have been constructed BEFORE this function is called.
      /// @param io_service the boost asio io_service used by the server.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      static std::shared_ptr<server> create(boost::asio::io_service& io_service,
                                              event_callback_type event_callback,
                                              error_callback_type error_callback)
      { return std::shared_ptr<server>{new server{io_service, event_callback,
                                                  error_callback}}; }

      /// @fn accept_connections
      /// Create the acceptor and wait for connections.
      /// @param port the port number to serve.
      /// @param ipv6 true for an IPV6 server, false for IPV4.
      /// @return the boost error code, false if no error occured
      boost::system::error_code accept_connections(unsigned short port, bool ipv6)
      {
        // Open the acceptor with the option to reuse the address
        // (i.e. SO_REUSEADDR).
        boost::asio::ip::tcp::endpoint endpoint
          {ipv6 ? boost::asio::ip::tcp::v6() : boost::asio::ip::tcp::v4(), port};
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option
          (boost::asio::ip::tcp::acceptor::reuse_address(true));
        boost::system::error_code ec;
        acceptor_.bind(endpoint, ec);
        if (ec)
          return ec;

        acceptor_.listen();
        start_accept();
        return ec;
      }

      /// @fn password
      /// Get the password.
      /// @pre It must be an SSL server.
      /// @return The password.
      const std::string password() const
      { return password_; }

      /// @fn set_password
      /// Set the password.
      /// @pre It must be an SSL server.
      /// @param password the password
      void set_password(std::string const& password)
      {
        password_ = password;
        connection_type::ssl_context().set_password_callback
            (std::bind(&server::password, this));
      }

      /// @fn set_event_callback
      /// Set the event_callback function.
      /// For use with the Constructor or create function that doesn't take
      /// an event_callback parameter.
      /// @see server(boost::asio::io_service& io_service)
      /// @see create(boost::asio::io_service& io_service)
      /// @param event_callback the event callback function.
      void set_event_callback(event_callback_type event_callback)
      { event_callback_ = event_callback; }

      /// @fn set_error_callback
      /// Set the error_callback function.
      /// For use with the Constructor or create function that doesn't take
      /// an error_callback parameter.
      /// @see server(boost::asio::io_service& io_service)
      /// @see create(boost::asio::io_service& io_service)
      /// @param error_callback the error callback function.
      void set_error_callback(error_callback_type error_callback)
      { error_callback_ = error_callback; }

      /// @fn set_no_delay
      /// Set the tcp no delay status for all future connections.
      /// @param enable if true it disables the Nagle algorithm.
      void set_no_delay(bool enable)
      { no_delay_ = enable; }

      /// @fn set_keep_alive
      /// Set the tcp keep alive status for all future connections.
      /// @param enable if true enables the tcp socket keep alive status.
      void set_keep_alive(bool enable)
      { keep_alive_ = enable; }

      /// @fn set_timeout
      /// Set the send and receive timeout value for all future connections.
      /// @pre sockets may remain open forever
      /// @post sockets will close if no activity has occured after the
      /// timeout period.
      /// @param timeout the timeout in milliseconds.
      void set_timeout(int timeout)
      { timeout_ = timeout; }

      /// @fn close
      /// Close the server and all of the connections associated with it.
      void close()
      {
        acceptor_.close();
        next_connection_.reset();
        connections_.clear();
      }
    };
  }
}

#endif
