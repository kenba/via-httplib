#pragma once

#ifndef SERVER_HPP_VIA_HTTPLIB_
#define SERVER_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file server.hpp
/// @brief The server template class.
//////////////////////////////////////////////////////////////////////////////
#include "connection.hpp"
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <set>
#include <string>
#include <sstream>

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class server
    /// A template class serving connections.
    //////////////////////////////////////////////////////////////////////////
    template <typename SocketAdaptor, typename Container = std::vector<char> >
    class server : boost::noncopyable
    {
    public:

      /// The connection type used by this server.
      typedef connection<SocketAdaptor, Container> connection_type;

      /// a set of connections.
      typedef std::set<boost::shared_ptr<connection_type> > connections;

      /// an iterator to the connections;
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
      boost::shared_ptr<connection_type> next_connection_;

      /// The connections established with this server.
      connections connections_;

      /// The password. Only used by SSL servers.
      std::string password_;

      event_callback_type event_callback_;   ///< The event callback function.
      error_callback_type error_callback_;   ///< The error callback function.

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
            next_connection_->start();
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
      void event_handler(int event, boost::weak_ptr<connection_type> ptr)
      {
        event_callback_(event, ptr);
        if (event == DISCONNECTED)
        {
          if (boost::shared_ptr<connection_type> connection = ptr.lock())
          {
            // search for the connection to delete
            connections_iterator iter(connections_.find(connection));
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
                         boost::weak_ptr<connection_type> connection)
      { error_callback_(error, connection); }

      /// @fn start_accept
      /// Wait for connections.
      void start_accept()
      {
        next_connection_ = connection_type::create(io_service_,
                                boost::bind(&server::event_handler, this, _1, _2),
                                boost::bind(&server::error_handler, this,
                                            boost::asio::placeholders::error, _2));
        acceptor_.async_accept(next_connection_->socket(),
                               boost::bind(&server::accept_handler, this,
                                           boost::asio::placeholders::error));
      }

    public:

      /// The server constructor.
      /// @param io_service the boost asio io_service used by the acceptor
      /// and connections.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      explicit server(boost::asio::io_service& io_service,
                      event_callback_type event_callback,
                      error_callback_type error_callback) :
        io_service_(io_service),
        acceptor_(io_service),
        next_connection_(),
        connections_(),
        password_(),
        event_callback_(event_callback),
        error_callback_(error_callback)
      {}

      /// @fn create
      /// Function to create a shared pointer to a server.
      /// @param io_service the boost asio io_service used by the server.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      static boost::shared_ptr<server> create(boost::asio::io_service& io_service,
                                              event_callback_type event_callback,
                                              error_callback_type error_callback)
      { return boost::shared_ptr<server>(new server(io_service, event_callback,
                                                    error_callback)); }

      /// @fn accept_connections
      /// Create the acceptor and wait for connections.
      /// @param port the port number to serve.
      /// @param address the address to listen on:
      /// @param ipv6 true for an IPV6 server, false for IPV4, default false.
      /// @return true if successful, false if the acceptor was not opened.
      bool accept_connections(unsigned short port, bool ipv6)
      {
        boost::asio::ip::tcp::resolver resolver(io_service_);
        std::string address(ipv6 ? "0::0" : "0.0.0.0");
        boost::asio::ip::tcp::resolver::query query(address, std::to_string(port));
        boost::asio::ip::tcp::resolver::iterator
            host_iterator(resolver.resolve(query));
        if (host_iterator == boost::asio::ip::tcp::resolver::iterator())
          return false;

        // Open the acceptor with the option to reuse the address
        // (i.e. SO_REUSEADDR).
        boost::asio::ip::tcp::endpoint endpoint(*host_iterator);
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option
          (boost::asio::ip::tcp::acceptor::reuse_address(true));
        boost::system::error_code ec;
        acceptor_.bind(endpoint, ec);
        if (ec)
          return false;

        acceptor_.listen();

        start_accept();

        return true;
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
            (boost::bind(&server::password, this));
      }

    };
  }
}

#endif
