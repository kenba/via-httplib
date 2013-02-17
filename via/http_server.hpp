#pragma once

#ifndef HTTP_SERVER_HPP_VIA_HTTPLIB_
#define HTTP_SERVER_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "http_connection.hpp"
#include "via/comms/server.hpp"
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <map>
#include <iostream>

namespace via
{
  ////////////////////////////////////////////////////////////////////////////
  /// @class http_server
  ///
  ////////////////////////////////////////////////////////////////////////////
  template <typename Connection,
            typename Container = std::vector<char> >
  class http_server
  {
  public:
    typedef comms::server<Connection> tcp_server;

    // Require a collection of http_connections keyed by the connction pointer.
    typedef http_connection<Connection, Container> http_connection_type;

    typedef std::map<void*, boost::shared_ptr<http_connection_type> >
        connection_collection;
    typedef typename connection_collection::iterator
      connection_collection_iterator;
    typedef typename connection_collection::value_type
      connection_collection_value_type;

    typedef typename Container::const_iterator Container_const_iterator;

    typedef boost::signal<void (const boost::weak_ptr<http_connection_type>,
                                http::rx_request const&,
                                Container_const_iterator,
                                Container_const_iterator)> http_request_signal;
    typedef typename http_request_signal::slot_type http_request_signal_slot;

  private:
    boost::shared_ptr<tcp_server> tcp_server_;
    connection_collection http_connections_;
    http_request_signal http_request_signal_;

  public:

    /// Connect the slot
    void request_received_event(const http_request_signal_slot& slot)
    { http_request_signal_.connect(slot); }

    /// Constructor.
    http_server(boost::asio::io_service& io_service,
                unsigned short port) :
      tcp_server_(tcp_server::create(io_service, port)),
      http_connections_(),
      http_request_signal_()
    {
      tcp_server_->get_event_signal
          (boost::bind(&http_server::event_handler, this, _1, _2));
      tcp_server_->get_error_signal
          (boost::bind(&http_server::error_handler, this, _1, _2));
    }

    void start_accept()
    { tcp_server_->start_accept(); }

    void receive_handler(boost::weak_ptr<Connection> connection)
    {
      // Use the raw pointer of the connection as the map key.
      void* pointer(connection.lock().get());
      if (pointer)
      {
        boost::shared_ptr<http_connection_type> http_connection;
        connection_collection_iterator iter(http_connections_.find(pointer));
        if (iter != http_connections_.end())
          http_connection = iter->second;
        else
        {
          http_connection = http_connection_type::create(connection);
          http_connections_.insert
              (connection_collection_value_type(pointer, http_connection));
        }

        if (http_connection->receive())
        {
          http_request_signal_(http_connection,
                               http_connection->request(),
                               http_connection->body_begin(),
                               http_connection->body_end());
        }
      }
      else
        std::cerr << "Error: http_server::receive_handler connection expired"
                  << std::endl;
    }

    void disconnected_handler(boost::weak_ptr<Connection> c)
    {
      // Use the raw pointer of the connection as the map key.
      void* pointer(c.lock().get());
      if (!pointer)
        return;

      connection_collection_iterator iter(http_connections_.find(pointer));
      if (iter != http_connections_.end())
      {
        // TODO disconnect
        http_connections_.erase(iter);
      }
    }

    /// receive a packet
    void event_handler(int event, boost::weak_ptr<Connection> connection)
    {
      switch(event)
      {
      case via::comms::RECEIVED:
        receive_handler(connection);
        break;
      case via::comms::DISCONNECTED:
        disconnected_handler(connection);
        break;
      default:
        break;
      }
    }

    void error_handler(const boost::system::error_code &error,
                       boost::weak_ptr<Connection> connection)
    {
      std::cerr << "error_handler" << std::endl;
      std::cerr << error <<  std::endl;
    }

    void set_password(std::string const& password)
    { tcp_server_->set_password(password); }

#ifdef VIA_SSL
    void set_ssl_files(const std::string& certificate_file,
                       const std::string& key_file,
                       std::string        dh_file = "")
    {
      tcp_server::connection_type::ssl_context().
          use_certificate_chain_file(certificate_file);
      tcp_server::connection_type::ssl_context().
          use_private_key_file(key_file, boost::asio::ssl::context::pem);

      if (dh_file.empty())
        tcp_server::connection_type::ssl_context().
           set_options(boost::asio::ssl::context::default_workarounds |
                       boost::asio::ssl::context::no_sslv2);
      else
      {
        tcp_server::connection_type::ssl_context().
           set_options(boost::asio::ssl::context::default_workarounds |
                       boost::asio::ssl::context::no_sslv2 |
                       boost::asio::ssl::context::single_dh_use);
        tcp_server::connection_type::ssl_context().use_tmp_dh_file(dh_file);
      }
    }
#endif

  };

}
#endif
