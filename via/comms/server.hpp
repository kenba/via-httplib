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

      /// @typedef a set of connections.
      typedef std::set<boost::shared_ptr<connection_type> > connections;

      /// @typedef the boost signal to indicate that an event occured.
      typedef typename connection_type::event_signal_type event_signal_type;

      /// @typedef the boost slot associated with the event_signal_type.
      typedef typename event_signal_type::slot_type event_slot_type;

      /// @typedef the boost signal to indicate that an error occured.
      typedef typename connection_type::error_signal_type error_signal_type;

      /// @typedef the boost slot associated with the error_signal_type.
      typedef typename error_signal_type::slot_type error_slot_type;

    private:
      /// The asio::io_service to use.
      boost::asio::io_service& io_service_;

      /// The acceptor for this server.
      boost::asio::ip::tcp::acceptor acceptor_;

      /// The next connection to be accepted.
      boost::shared_ptr<connection_type> new_connection_;

      /// The connections established with this server.
      connections connections_;

      /// The password. Only used by SSL servers.
      std::string password_;

      event_signal_type signal_event_; ///< The event signal.
      error_signal_type signal_error_; ///< The error signal.

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
        if (!acceptor_.is_open())
          return;

        if (!error)
        {
          new_connection_->start();

          new_connection_->get_event_signal
            (boost::bind(&server::event_handler, this, _1, new_connection_));

          new_connection_->get_error_signal
            (boost::bind(&server::error_handler, this,
                         boost::asio::placeholders::error,
                         new_connection_));

          connections_.insert(new_connection_);
        }
        else
          signal_error_(error, new_connection_);

        start_accept();
      }

      /// @fn event_handler.
      /// It just forwards the connection's event signal.
      /// @param event the event, @see event_type.
      /// @param connection a weak_pointer to the connection that sent the
      /// event.
      void event_handler(int event, boost::weak_ptr<connection_type> connection)
      { signal_event_(event, connection); }

      /// @fn error_handler.
      /// It just forwards the connection's error signal.
      /// @param error the boost asio error.
      /// @param connection a weak_pointer to the connection that sent the
      /// error.
      void error_handler(const boost::system::error_code& error,
                         boost::weak_ptr<connection_type> connection)
      { signal_error_(error, connection); }

    public:

      /// @fn Constructor
      /// @param io_service the boost asio io_service used by the acceptor
      /// and connections.
      /// @param port the port number to serve.
      explicit server(boost::asio::io_service& io_service,
                      unsigned short port) :
        io_service_(io_service),
        acceptor_(io_service),
        new_connection_(),
        connections_(),
        password_(),
        signal_event_(),
        signal_error_()
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
      /// @param port the port number to serve.
      static boost::shared_ptr<server> create(boost::asio::io_service& io_service,
                                              unsigned short port)
      { return boost::shared_ptr<server>(new server(io_service, port)); }

      /// @fn get_event_signal
      /// A function to connect a slot to the event signal.
      /// @param slot the slot to connect.
      void get_event_signal(const event_slot_type& slot)
      { signal_event_.connect(slot); }

      /// @fn get_error_signal
      /// A function to connect a slot to the error signal.
      /// @param slot the slot to connect.
      void get_error_signal(const error_slot_type& slot)
      { signal_error_.connect(slot); }

      /// @fn start_accept
      /// Wait for connections.
      void start_accept()
      {
        new_connection_.reset();
        new_connection_ = connection_type::create(io_service_);

        acceptor_.async_accept(new_connection_->socket(),
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
