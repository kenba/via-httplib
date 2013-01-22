#pragma once

#ifndef SSL_TCP_SERVER_HPP_VIA_HTTPLIB_
#define SSL_TCP_SERVER_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "ssl_tcp_buffered_connection.hpp"
#include <boost/noncopyable.hpp>
#include <set>

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class ssl_tcp_server
    //////////////////////////////////////////////////////////////////////////
    template <typename SslTcpConnection>
    class ssl_tcp_server : boost::noncopyable
    {
      typedef std::set<boost::shared_ptr<connection> > connections;

      /// The asio::io_service to use.
      boost::asio::io_service& io_service_;

      /// The acceptor for this server.
      boost::asio::ip::tcp::acceptor acceptor_;

      /// The SSl context for this server.
      boost::asio::ssl::context context_;

      /// The Ssl password.
      std::string password_;

      /// The next connection to be accepted.
      boost::shared_ptr<SslTcpConnection> new_connection_;

      /// The connections established with this server.
      connections connections_;

      size_t receive_timeout_;

      /// The data received signal.
      connection::event_signal received_;

      /// The data sent signal.
      connection::event_signal sent_;

      /// The connection_established signal.
      connection::event_signal connected_;

      /// The disconnected signal.
      connection::event_signal disconnected_;

      /// The receive timedout signal.
      connection::event_signal receive_timedout_;

      /// The error signal.
      connection::error_signal error_;

      /// Handler called by the io_service for a connection request.
      /// If succesfull it will contain a shared pointer to the new connection.
      /// This function configures the connection, adds it to the servers
      /// connections collection, signals that a new connection has been
      /// established and prepares to accept the next connection.
      /// @param error the error code (if any)
      /// @param tcp_connection_ptr a tcp_connection pointer to the new
      /// connection.
      void accept_handler(const boost::system::error_code& error)
      {
        if (!acceptor_.is_open())
          return;

        if (!error)
        {
          // Perform the SSL handshaking
          new_connection_->start();

          new_connection_->set_no_delay(true);
          new_connection_->enable_reception();

          connected_(new_connection_);

          new_connection_->received_event
              (boost::bind(&ssl_tcp_server::received_handler, this,
                           new_connection_));
          new_connection_->sent_event
              (boost::bind(&ssl_tcp_server::sent_handler, this,
                             new_connection_));
          new_connection_->disconnected_event
              (boost::bind(&ssl_tcp_server::disconnected_handler, this,
                           new_connection_));
          new_connection_->receive_timedout_event
              (boost::bind(&ssl_tcp_server::receive_timedout_handler, this,
                             new_connection_));
          new_connection_->error_event
              (boost::bind(&ssl_tcp_server::error_handler, this,
                          boost::asio::placeholders::error,
                          new_connection_));

          connections_.insert(new_connection_);
        }
        else
          error_(error, new_connection_);

        start_accept();
      }

      /// Packet received handler.
      void received_handler(boost::weak_ptr<connection> weak_connection)
      { received_(weak_connection.lock()); }

      void sent_handler(boost::weak_ptr<connection> weak_connection)
      { sent_(weak_connection.lock()); }

      /// The disconnected handler. It searches for the connection
      /// in the servers connections collection and (if found)
      /// signals that it's been disconnected and erases from the collection.
      void disconnected_handler(boost::weak_ptr<connection> weak_connection)
      {
        boost::shared_ptr<comms::connection> pointer(weak_connection.lock());
        if (pointer)
        {
          connections::iterator iter
              (std::find(connections_.begin(), connections_.end(), pointer));
          if (iter != connections_.end())
          {
            disconnected_(pointer);
            connections_.erase(iter);
          }
        }
      }

      void receive_timedout_handler(boost::weak_ptr<connection> weak_connection)
      { receive_timedout_(weak_connection.lock()); }

      /// Connection error handler. It  just forwards the signal.
      void error_handler(const boost::system::error_code& error,
                         boost::weak_ptr<connection> weak_connection)
      { error_(error, weak_connection); }

    public:

      /// Constructor.
      /// Create a TCP server on the given address and port.
      /// @param io_service the asio::io_service that this socket is bound to.
      /// @param address the address of the server
      /// @param port the address of the server
      /// @param buffer_size the size of the read and write buffers
      /// @param noDelay if true disables the Nagle algorithm, default true.
      explicit ssl_tcp_server(boost::asio::io_service& io_service,
                              const std::string& address,
                              const std::string& port,
                              size_t receive_timeout = 0) :
        io_service_(io_service),
        acceptor_(io_service),
        context_(boost::asio::ssl::context::sslv23),
        password_(""),
        new_connection_(),
        connections_(),
        receive_timeout_(receive_timeout),
        received_(),
        sent_(),
        connected_(),
        disconnected_(),
        receive_timedout_(),
        error_()
      {
        // Open the acceptor with the option to reuse the address
        // (i.e. SO_REUSEADDR).
        boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query(address, port);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option
          (boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();

        start_accept();
      }

      static boost::shared_ptr<ssl_tcp_server> create
                                        (boost::asio::io_service& io_service,
                                         const std::string& address,
                                         const std::string& port,
                                         size_t receive_timeout = 0)
      {
        return boost::shared_ptr<ssl_tcp_server>
            (new ssl_tcp_server(io_service, address, port, receive_timeout));
      }

      void set_password(const std::string& password)
      {
        password_ = password;
        context_.set_password_callback
            (boost::bind(&ssl_tcp_server::password, this));
      }

      /// Accessor for the certificate file password.
      /// @return password the password for the SSL certificate file.
      const std::string& password() const
      { return password_; }

      void set_ssl_files(const std::string& certificate_file,
                         const std::string& key_file,
                         std::string        dh_file = "")
      {
        if (dh_file.empty())
          context_.set_options(boost::asio::ssl::context::default_workarounds
                        | boost::asio::ssl::context::no_sslv2);
        else
          context_.set_options(boost::asio::ssl::context::default_workarounds
                        | boost::asio::ssl::context::no_sslv2
                        | boost::asio::ssl::context::single_dh_use);

        // either of the lines below works
    //  context_.use_certificate_chain_file(certificate_file); // Asio example
        context_.use_certificate_file(certificate_file,
                                      boost::asio::ssl::context::pem);
        context_.use_private_key_file(key_file,
                                      boost::asio::ssl::context::pem);

        if (!dh_file.empty())
          context_.use_tmp_dh_file(dh_file);
      }

      /// Asynchorously wait for connections.
      void start_accept()
      {
        new_connection_.reset();
        new_connection_ = SslTcpConnection::create(io_service_, context_,
                                                   receive_timeout_);
        acceptor_.async_accept(new_connection_->socket(),
                               boost::bind(&ssl_tcp_server::accept_handler, this,
                                           boost::asio::placeholders::error));
      }

      /// Destructor.
      /// Shutdown all of the connections.
      ~ssl_tcp_server()
      { shutdown(); }

      /// Close all of the connections.
      void shutdown()
      { connections_.clear(); }

      /// Connect a slot to the received_ signal.
      /// @param slot the slot to connect.
      void received_event(const connection::event_signal::slot_type& slot)
      { received_.connect(slot); }

      /// Connect a slot to the sent_ signal.
      /// @param slot the slot to connect.
      void sent_event(const connection::event_signal::slot_type& slot)
      { sent_.connect(slot); }

      /// Connect a slot to the error signal.
      /// @param slot the slot to connect.
      void error_event(const connection::error_signal::slot_type& slot)
      { error_.connect(slot); }

      /// Connect a slot to the connected_ signal.
      /// @param slot the slot to connect.
      void connected_event(const connection::event_signal::slot_type& slot)
      { connected_.connect(slot); }

      /// Connect a slot to the disconnected signal.
      /// @param slot the slot to connect.
      void disconnected_event
        (const connection::event_signal::slot_type& slot)
      { disconnected_.connect(slot); }

      /// Connect a slot to the receive_timeout_ signal.
      /// @param slot the slot to connect.
      void receive_timedout_event
         (const connection::event_signal::slot_type& slot)
      { receive_timedout_.connect(slot); }

    };
  }
}

#endif
