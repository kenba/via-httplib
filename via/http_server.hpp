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
/// @file http_server.hpp
/// @brief Just contains the http_server template class.
//////////////////////////////////////////////////////////////////////////////
#include "http_connection.hpp"
#include "via/comms/server.hpp"
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#ifdef HTTP_SSL
#include <boost/asio/ssl/context.hpp>
#endif
#include <map>
#include <iostream>

namespace via
{
  ////////////////////////////////////////////////////////////////////////////
  /// @class http_server
  /// An HTTP server.
  ////////////////////////////////////////////////////////////////////////////
  template <typename SocketAdaptor, typename Container = std::vector<char> >
  class http_server
  {
  public:

    /// The underlying connection, TCP or SSL.
    typedef comms::connection<SocketAdaptor, Container> connection_type;

    /// The server for the underlying connections, TCP or SSL.
    typedef comms::server<SocketAdaptor, Container> server_type;

    /// The http_connections managed by this server.
    typedef http_connection<SocketAdaptor, Container> http_connection_type;

    /// A collection of http_connections keyed by the connection pointer.
    typedef std::map<void*, boost::shared_ptr<http_connection_type> >
        connection_collection;

    /// The template requires a typename to access the iterator.
    typedef typename connection_collection::iterator
      connection_collection_iterator;

    /// The template requires a typename to collection value_type.
    typedef typename connection_collection::value_type
      connection_collection_value_type;

    /// The template requires a typename to access the iterator.
    typedef typename Container::const_iterator Container_const_iterator;

    /// The signal sent when a request is received.
    typedef boost::signal<void (const boost::weak_ptr<http_connection_type>,
                                http::rx_request const&,
                                Container const&)> http_request_signal;

    /// The slot type associated with a request received signal.
    typedef typename http_request_signal::slot_type http_request_signal_slot;

    /// The signal sent when a chunk is received.
    typedef boost::signal<void (const boost::weak_ptr<http_connection_type>,
                                http::rx_chunk const&,
                                Container const&)> http_chunk_signal;

    /// The slot type associated with a chunk received signal.
    typedef typename http_chunk_signal::slot_type http_chunk_signal_slot;

  private:
    boost::shared_ptr<server_type> server_;   ///< the communications server
    connection_collection http_connections_;  ///< the communications channels
    http_request_signal http_request_signal_; ///< the request callback function
    http_chunk_signal http_chunk_signal_;     ///< the response chunk callback function

  public:

    /// Connect the request received slot.
    /// @param slot the slot for the request received signal.
    void request_received_event(http_request_signal_slot const& slot)
    { http_request_signal_.connect(slot); }

    /// Connect the chunk received slot.
    /// @param slot the slot for the chunk received signal.
    void chunk_received_event(http_chunk_signal_slot const& slot)
    { http_chunk_signal_.connect(slot); }

    /// Constructor.
    /// @param io_service a reference to the boost::asio::io_service.
    /// @param port the number of the comms port.
    http_server(boost::asio::io_service& io_service,
                unsigned short port) :
      server_(server_type::create(io_service, port)),
      http_connections_(),
      http_request_signal_(),
      http_chunk_signal_()
    {
      server_->get_event_signal
          (boost::bind(&http_server::event_handler, this, _1, _2));
      server_->get_error_signal
          (boost::bind(&http_server::error_handler, this, _1, _2));
    }

    /// Start accepting connections on the communiations server.
    void start_accept()
    { server_->start_accept(); }

    /// Receive data packets on an underlying communications connection.
    /// @param connection a weak pointer to the underlying comms connection.
    void receive_handler(boost::weak_ptr<connection_type> connection)
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

        http::receiver_parsing_state rx_state(http_connection->receive());

        switch (rx_state)
        {
        case http::RX_VALID:
          http_request_signal_(http_connection,
                               http_connection->request(),
                               http_connection->body());
          break;

        case http::RX_CHUNK:
          http_chunk_signal_(http_connection,
                             http_connection->chunk(),
                             http_connection->body());
          break;

        case http::RX_INVALID:
        {
          http::tx_response response(http::response_status::BAD_REQUEST, 0);
          http_connection->send(response);
        }
          break;

        default:
          break;
        }
      }
      else
        std::cerr << "Error: http_server::receive_handler connection expired"
                  << std::endl;
    }

    /// Handle a disconnected signal from an underlying comms connection.
    /// @param connection a weak ponter to the underlying comms connection.
    void disconnected_handler(boost::weak_ptr<connection_type> connection)
    {
      // Use the raw pointer of the connection as the map key.
      void* pointer(connection.lock().get());
      if (!pointer)
        return;

      connection_collection_iterator iter(http_connections_.find(pointer));
      if (iter != http_connections_.end())
        http_connections_.erase(iter);
    }

    /// Receive an event from the underlying comms connection.
    /// @param event the type of event.
    /// @param connection a weak ponter to the underlying comms connection.
    void event_handler(int event, boost::weak_ptr<connection_type> connection)
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

    /// Receive an error from the underlying comms connection.
    /// @param error the boost error_code.
    /// @param connection a weak ponter to the underlying comms connection.
    void error_handler(const boost::system::error_code &error,
                       boost::weak_ptr<connection_type> connection)
    {
      std::cerr << "error_handler" << std::endl;
      std::cerr << error <<  std::endl;
    }

    /// Set the password for an SSL connection.
    /// Note: only valid for SSL connections, do NOT call for TCP servers.
    /// @param password the SSL password
    void set_password(std::string const& password)
    { server_->set_password(password); }

#ifdef HTTP_SSL

    /// Set the files required for an SSL server.
    /// Note: only valid for SSL connections, do NOT call for TCP servers.
    /// @param certificate_file the server SSL certificate file.
    /// @param key_file the private key file
    /// @param dh_file the dh file.
    static boost::system::error_code set_ssl_files
                       (const std::string& certificate_file,
                        const std::string& key_file,
                        std::string        dh_file = "")
    {
      boost::system::error_code error;
      server_type::connection_type::ssl_context().
          use_certificate_file(certificate_file,
                               boost::asio::ssl::context::pem, error);
      if (error)
        return error;

      server_type::connection_type::ssl_context().
          use_private_key_file(key_file, boost::asio::ssl::context::pem,
                               error);
      if (error)
        return error;

      if (dh_file.empty())
        server_type::connection_type::ssl_context().
           set_options(boost::asio::ssl::context::default_workarounds |
                       boost::asio::ssl::context::no_sslv2);
      else
      {
        server_type::connection_type::ssl_context().use_tmp_dh_file(dh_file,
                                                                   error);
        if (error)
          return error;

        server_type::connection_type::ssl_context().
           set_options(boost::asio::ssl::context::default_workarounds |
                       boost::asio::ssl::context::no_sslv2 |
                       boost::asio::ssl::context::single_dh_use,
                       error);
      }

      return error;
    }
#endif

  };

}
#endif
