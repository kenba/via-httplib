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
            boost::shared_ptr<connection_type> new_connection
                (connection_type::create(io_service_,
                    boost::bind(&server::event_handler, this, _1, _2),
                    boost::bind(&server::error_handler, this,
                                boost::asio::placeholders::error, _2)));
            new_connection.swap(next_connection_);

            new_connection->start();

            connections_.insert(new_connection);
          }

          start_accept();
        }
      }

      /// @fn event_handler.
      /// It forwards the connection's event signal.
      /// For a disconnected event, it closes and deletes the connection.
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
            connection->close();
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

    public:

      /// The server constructor.
      /// @param io_service the boost asio io_service used by the acceptor
      /// and connections.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      /// @param port the port number to serve.
      explicit server(boost::asio::io_service& io_service,
                      event_callback_type event_callback,
                      error_callback_type error_callback,
                      unsigned short port) :
        io_service_(io_service),
        acceptor_(io_service),
        next_connection_(connection_type::create(io_service_,
            boost::bind(&server::event_handler, this, _1, _2),
            boost::bind(&server::error_handler, this,
                        boost::asio::placeholders::error, _2))),
        connections_(),
        password_(),
        event_callback_(event_callback),
        error_callback_(error_callback)
      {
        // Open the acceptor with the option to reuse the address
        // (i.e. SO_REUSEADDR).
        boost::asio::ip::tcp::endpoint
            endpoint(boost::asio::ip::tcp::v4(), port);
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option
          (boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();
      }

      /// @fn create
      /// Function to create a shared pointer to a server.
      /// @param io_service the boost asio io_service used by the server.
      /// @param event_callback the event callback function.
      /// @param error_callback the error callback function.
      /// @param port the port number to serve.
      static boost::shared_ptr<server> create(boost::asio::io_service& io_service,
                                              event_callback_type event_callback,
                                              error_callback_type error_callback,
                                              unsigned short port)
      { return boost::shared_ptr<server>(new server(io_service, event_callback,
                                                    error_callback, port)); }

      /// @fn start_accept
      /// Wait for connections.
      void start_accept()
      {
        acceptor_.async_accept(next_connection_->socket(),
                               boost::bind(&server::accept_handler, this,
                                           boost::asio::placeholders::error));
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
