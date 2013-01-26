#pragma once

#ifndef HTTP_SSL_SERVER_HPP_VIA_HTTPLIB_
#define HTTP_SSL_SERVER_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "http_ssl_connection.hpp"
#include "via/comms/ssl_tcp_server.hpp"
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <map>
#include <iostream>

namespace via
{
  ////////////////////////////////////////////////////////////////////////////
  /// @class http_ssl_server
  ///
  ////////////////////////////////////////////////////////////////////////////
  template <typename Container>
  class http_ssl_server
  {
  public:
    typedef comms::ssl_tcp_buffered_connection<Container> ssl_tcp_connection;
    typedef comms::ssl_tcp_server<ssl_tcp_connection>     ssl_tcp_server;

    // Require a collection of http_connections keyed by the connction pointer.
    typedef http_ssl_connection<Container> http_ssl_connection_type;
    typedef std::map<void*, boost::shared_ptr<http_ssl_connection_type> >
        connection_collection;
    typedef typename connection_collection::iterator
      connection_collection_iterator;
    typedef typename connection_collection::value_type
      connection_collection_value_type;

    typedef typename Container::const_iterator Container_const_iterator;
    typedef boost::signal<void (const boost::weak_ptr<http_ssl_connection_type>,
                                http::rx_request const&,
                                Container_const_iterator,
                                Container_const_iterator)> http_request_signal;


  private:
    boost::shared_ptr<ssl_tcp_server> tcp_server_;
    connection_collection http_connections_;
    http_request_signal http_request_signal_;

  public:

    /// Connect the slot
    typedef typename http_request_signal::slot_type http_request_signal_slot;
    void request_received_event(const http_request_signal_slot& slot)
    { http_request_signal_.connect(slot); }

    /// Constructor.
    http_ssl_server(boost::asio::io_service& io_service,
                    unsigned short port) :
      tcp_server_(ssl_tcp_server::create(io_service, port)),
      http_connections_(),
      http_request_signal_()
    {
      tcp_server_->received_event
          (boost::bind(&http_ssl_server::receive_handler, this, _1));
      tcp_server_->disconnected_event
          (boost::bind(&http_ssl_server::disconnected_handler, this, _1));
      tcp_server_->error_event
          (boost::bind(&http_ssl_server::error_handler, this, _1, _2));
    }


    // TODO password and crypto files
    void set_password(const std::string& password)
    { tcp_server_->set_password(password); }

    void set_ssl_files(const std::string& certificate_file,
                       const std::string& key_file,
                       std::string        dh_file = "")
    {
      tcp_server_->set_ssl_files(certificate_file,
                                 key_file,
                                 dh_file);
    }

    void start_accept()
    { tcp_server_->start_accept(); }


    /// receive a packet
    void receive_handler(boost::weak_ptr<via::comms::connection> connection)
    {
      // Use the raw pointer of the connection as the map key.
      void* pointer(connection.lock().get());
      if (pointer)
      {
        boost::shared_ptr<http_ssl_connection_type> http_ssl_connection;
        connection_collection_iterator iter(http_connections_.find(pointer));
        if (iter != http_connections_.end())
          http_ssl_connection = iter->second;
        else
        {
          http_ssl_connection = http_ssl_connection_type::create(connection);
          http_connections_.insert
              (connection_collection_value_type(pointer, http_ssl_connection));
        }

        if (http_ssl_connection->receive())
        {
          http_request_signal_(http_ssl_connection,
                               http_ssl_connection->request(),
                               http_ssl_connection->body_begin(),
                               http_ssl_connection->body_end());
        }
      }
      else
        std::cerr << "Error: http_ssl_server::receive_handler connection expired"
                  << std::endl;
    }

/*
    void sent_handler(boost::weak_ptr<via::comms::connection> c)
    {
      std::cout << "sent_handler" << std::endl;
    }

    void connected_handler(boost::weak_ptr<via::comms::connection> c)
    {
      std::cout << "connected_handler" << std::endl;
    }
*/

    void disconnected_handler(boost::weak_ptr<via::comms::connection> c)
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

    /*
    void receive_timedout_handler
      (boost::weak_ptr<via::comms::connection> connection)
    {
      std::cout << "receive_timedout_handler " << std::endl;
      // disconnect?
    }
    */

    void error_handler(const boost::system::error_code &error,
                       boost::weak_ptr<via::comms::connection> connection)
    {
      std::cerr << "error_handler" << std::endl;
      std::cerr << error <<  std::endl;
    }

  };

}
#endif

