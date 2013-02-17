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
    //////////////////////////////////////////////////////////////////////////
    template <typename Connection>
    class server : boost::noncopyable
    {
      typedef std::set<boost::shared_ptr<Connection> > connections;

      typedef typename Connection::event_signal_type event_signal_type;
      typedef typename event_signal_type::slot_type event_slot_type;

      typedef typename Connection::error_signal_type error_signal_type;
      typedef typename error_signal_type::slot_type error_slot_type;

      /// The asio::io_service to use.
      boost::asio::io_service& io_service_;

      /// The acceptor for this server.
      boost::asio::ip::tcp::acceptor acceptor_;

      /// The next connection to be accepted.
      boost::shared_ptr<Connection> new_connection_;

      /// The connections established with this server.
      connections connections_;

      std::string password_;

      event_signal_type signal_event_;
      error_signal_type signal_error_;

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

      void event_handler(int event,
                         boost::weak_ptr<Connection> weak_connection)
      { signal_event_(event, weak_connection); }

      /// Connection error handler. It  just forwards the signal.
      void error_handler(const boost::system::error_code& error,
                         boost::weak_ptr<Connection> weak_connection)
      { signal_error_(error, weak_connection); }

    public:

      typedef Connection connection_type;

      void get_event_signal(const event_slot_type& slot)
      { signal_event_.connect(slot); }

      void get_error_signal(const error_slot_type& slot)
      { signal_error_.connect(slot); }

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

      static boost::shared_ptr<server> create(boost::asio::io_service& io_service,
                                              unsigned short port)
      { return boost::shared_ptr<server>(new server(io_service, port)); }

      /// Asynchorously wait for connections.
      void start_accept()
      {
        new_connection_.reset();
        new_connection_ = Connection::create(io_service_);

        acceptor_.async_accept(new_connection_->socket(),
                               boost::bind(&server::accept_handler, this,
                                           boost::asio::placeholders::error));
      }

      const std::string password() const
      { return password_; }

      void set_password(std::string const& password)
      {
        password_ = password;
        Connection::ssl_context().set_password_callback
            (boost::bind(&server::password, this));
      }
    };
  }
}

#endif
