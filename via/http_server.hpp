#ifndef HTTP_SERVER_HPP_VIA_HTTPLIB_
#define HTTP_SERVER_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file http_server.hpp
/// @brief Contains the http_server template class.
//////////////////////////////////////////////////////////////////////////////
#include "http_connection.hpp"
#include "via/comms/server.hpp"
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
  /// The class can be configured to use either tcp or ssl sockets depending
  /// upon which class is provided as the SocketAdaptor: tcp_adaptor or
  /// ssl::ssl_tcp_adaptor respectively.
  /// @see http_connection
  /// @see comms::connection
  /// @see comms::tcp_adaptor
  /// @see comms::ssl::ssl_tcp_adaptor
  /// @param SocketAdaptor the type of socket, use: tcp_adaptor or
  /// ssl::ssl_tcp_adaptor
  /// @param Container the container to use for the tx buffer, default
  /// std::vector<char>.
  /// It must contain a contiguous array of bytes. E.g. std::string or
  /// std::array<char, size>
  /// @param use_strand if true use an asio::strand to wrap the handlers,
  /// default false.
  ////////////////////////////////////////////////////////////////////////////
  template <typename SocketAdaptor, typename Container = std::vector<char>,
            bool use_strand = false>
  class http_server
  {
  public:

    /// The server for the underlying connections, TCP or SSL.
    typedef comms::server<SocketAdaptor, Container, use_strand> server_type;

    /// The http_connections managed by this server.
    typedef http_connection<SocketAdaptor, Container, use_strand>
      http_connection_type;

    /// The underlying connection, TCP or SSL.
    typedef typename http_connection_type::connection_type connection_type;

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

    /// The type of the request_receiver.
    typedef typename http::request_receiver<Container> http_request;

    /// The chunk type
    typedef typename http::rx_chunk<Container> chunk_type;

    /// The RequestHandler type.
    typedef TR1::function <void (boost::weak_ptr<http_connection_type>,
                                 http::rx_request const&, Container const&)>
      RequestHandler;

    /// The ChunkHandler type.
    typedef TR1::function <void (boost::weak_ptr<http_connection_type>,
                                 chunk_type const&, Container const&)>
      ChunkHandler;

    /// The ConnectionHandler type.
    typedef TR1::function <void (boost::weak_ptr<http_connection_type>)>
      ConnectionHandler;

  private:
    boost::shared_ptr<server_type> server_;   ///< the communications server
    connection_collection http_connections_;  ///< the communications channels

    // Request parser parameters
    bool           strict_crlf_;       ///< enforce strict parsing of CRLF
    unsigned char  max_whitespace_;    ///< the max no of consectutive whitespace characters.
    unsigned char  max_method_length_; ///< the maximum length of a request method
    size_t         max_uri_length_;    ///< the maximum length of a uri.
    unsigned short max_line_length_;   ///< the max length of a field line
    unsigned short max_header_number_; ///< the max no of header fields
    size_t         max_header_length_; ///< the max cumulative length
    size_t         max_body_size_;     ///< the maximum size of a request body
    size_t         max_chunk_size_;    ///< the maximum size of a request chunk

    bool require_host_header_; ///< whether the http server requires a host header
    bool translate_head_;      ///< whether the http server translates HEAD requests
    bool trace_enabled_;       ///< whether the http server responds to TRACE requests

    RequestHandler    http_request_handler_; ///< the request callback function
    RequestHandler    http_continue_handler_;///< the continue callback function
    ChunkHandler      http_chunk_handler_;   ///< the http chunk callback function
    ConnectionHandler connected_handler_;    ///< the connected callback function
    ConnectionHandler packet_sent_handler_;  ///< the packet sent callback function
    ConnectionHandler disconnected_handler_; ///< the disconncted callback function

  public:

    /// Connect the request received callback function.
    /// @param handler the handler for a received HTTP request.
    void request_received_event(RequestHandler handler)
    { http_request_handler_ = handler; }

    /// Connect the expect continue received callback function.
    /// If the application registers a handler for this event, then the
    /// application must determine how to respond to a request containing an
    /// Expect: 100-continue header based upon it's other headers.
    /// Otherwise, the server will automatically send a 100 Continue response,
    /// so that the client can continue to send the body of the request.
    /// @post disables automatic sending of a 100 Continue response
    /// @param handler the handler for an "expects continue" request.
    void request_expect_continue_event(RequestHandler handler)
    { http_continue_handler_ = handler; }

    /// Connect the chunk received callback function.
    /// @param handler the handler for a received HTTP chunk.
    void chunk_received_event(ChunkHandler handler)
    { http_chunk_handler_ = handler; }

    /// Connect the connected callback function.
    /// @param handler the handler for the socket connected event.
    void socket_connected_event(ConnectionHandler handler)
    { connected_handler_= handler; }

    /// Connect the message sent callback function.
    /// @param handler the handler for the message sent signal.
    void message_sent_event(ConnectionHandler handler)
    { packet_sent_handler_= handler; }

    /// Connect the disconnected callback function.
    /// @param handler the handler for the socket disconnected signal.
    void socket_disconnected_event(ConnectionHandler handler)
    { disconnected_handler_ = handler; }

    /// Constructor.
    /// @param io_service a reference to the boost::asio::io_service.
    explicit http_server(boost::asio::io_service& io_service) :
      server_(server_type::create(io_service)),
      http_connections_(),

      // Set request parser parameters to default values
      strict_crlf_        (true),
      max_whitespace_     (http_request::DEFAULT_MAX_WHITESPACE_CHARS),
      max_method_length_  (http_request::DEFAULT_MAX_METHOD_LENGTH),
      max_uri_length_     (http_request::DEFAULT_MAX_URI_LENGTH),
      max_line_length_    (http_request::DEFAULT_MAX_LINE_LENGTH),
      max_header_number_  (http_request::DEFAULT_MAX_HEADER_NUMBER),
      max_header_length_  (http_request::DEFAULT_MAX_HEADER_LENGTH),
      max_body_size_      (http_request::DEFAULT_MAX_BODY_SIZE),
      max_chunk_size_     (http_request::DEFAULT_MAX_CHUNK_SIZE),

      require_host_header_(true),
      translate_head_     (true),

      trace_enabled_      (false),

      http_request_handler_ (NULL),
      http_continue_handler_(NULL),
      http_chunk_handler_   (NULL),
      connected_handler_    (NULL),
      packet_sent_handler_  (NULL),
      disconnected_handler_ (NULL)
    {
      server_->set_event_callback
          (boost::bind(&http_server::event_handler, this, _1, _2));
      server_->set_error_callback
          (boost::bind(&http_server::error_handler, this,
                        boost::asio::placeholders::error, _2));
      // Set no delay, i.e. disable the Nagle algorithm
      // An http_server will want to send messages immediately
      server_->set_no_delay(true);
    }

    /// Start accepting connections on the communications server from the
    /// given port.
    /// @param port the port number to serve.
    /// @param ipv6 true for an IPV6 server, false for IPV4, default false.
    /// @return the boost error code, false if no error occured
    boost::system::error_code accept_connections
                      (unsigned short port = SocketAdaptor::DEFAULT_HTTP_PORT,
                       bool ipv6 = false)
    { return server_->accept_connections(port, ipv6); }

    /// Handle a connected signal from an underlying comms connection.
    /// @param connection a weak ponter to the underlying comms connection.
    void connected_handler(boost::weak_ptr<connection_type> connection)
    {
      // Use the raw pointer of the connection as the map key.
      void* pointer(connection.lock().get());
      if (!pointer)
        return;

      // search for the connection in the collection
      connection_collection_iterator iter(http_connections_.find(pointer));
      if (iter == http_connections_.end())
      {
        // Create and configure a new http_connection_type.
        boost::shared_ptr<http_connection_type> http_connection
            (new http_connection_type(connection,
                                      strict_crlf_,
                                      max_whitespace_,
                                      max_method_length_,
                                      max_uri_length_,
                                      max_line_length_,
                                      max_header_number_,
                                      max_header_length_,
                                      max_body_size_,
                                      max_chunk_size_));

        http_connection->set_require_host_header(require_host_header_);
        http_connection->set_translate_head(translate_head_);
        http_connection->set_concatenate_chunks(http_chunk_handler_ == NULL);
        http_connection->set_auto_continue(http_continue_handler_ == NULL);
        http_connection->set_trace_enabled(trace_enabled_);

        http_connections_.insert
            (connection_collection_value_type(pointer, http_connection));
        // signal that the socket is connected
        if (connected_handler_ != NULL)
          connected_handler_(http_connection);
      }
      else
        std::cerr << "http_server, error: duplicate connection for "
                  << iter->second->remote_address() << std::endl;
    }

    /// Receive data packets on an underlying communications connection.
    /// @param connection a weak pointer to the underlying comms connection.
    void receive_handler(boost::weak_ptr<connection_type> connection)
    {
      // Use the raw pointer of the connection as the map key.
      void* pointer(connection.lock().get());
      if (pointer)
      {
        // search for the connection in the collection
        connection_collection_iterator iter(http_connections_.find(pointer));
        if (iter == http_connections_.end())
        {
          std::cerr << "http_server, receive_handler error: connection not found "
                    << std::endl;
          return;
        }

        boost::shared_ptr<http_connection_type> http_connection(iter->second);

        http::Rx rx_state(http_connection->receive());

        switch (rx_state)
        {
        case http::RX_VALID:
          http_request_handler_(http_connection,
                               http_connection->request(),
                               http_connection->body());
          break;

        case http::RX_EXPECT_CONTINUE:
          http_continue_handler_(http_connection,
                                http_connection->request(),
                                http_connection->body());
          break;

        case http::RX_CHUNK:
          http_chunk_handler_(http_connection,
                             http_connection->chunk(),
                             http_connection->chunk().data());
          break;

        default:
          break;
        }
      }
      else
        std::cerr << "Error: http_server::receive_handler connection expired"
                  << std::endl;
    }

    /// Handle a sent signal from an underlying comms connection.
    /// @param connection a weak ponter to the underlying comms connection.
    void sent_handler(boost::weak_ptr<connection_type> connection)
    {
      // Use the raw pointer of the connection as the map key.
      void* pointer(connection.lock().get());
      if (!pointer)
        return;

      if (packet_sent_handler_ != NULL)
      {
        connection_collection_iterator iter(http_connections_.find(pointer));
        if (iter != http_connections_.end())
          packet_sent_handler_(iter->second);
      }
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
      {
        // Signal the application
        if (disconnected_handler_ != NULL)
          disconnected_handler_(iter->second);

        http_connections_.erase(iter);
      }
    }

    /// Receive an event from the underlying comms connection.
    /// @param event the type of event.
    /// @param connection a weak ponter to the underlying comms connection.
    void event_handler(int event, boost::weak_ptr<connection_type> connection)
    {
      switch(event)
      {
      case via::comms::CONNECTED:
        connected_handler(connection);
        break;
      case via::comms::RECEIVED:
        receive_handler(connection);
        break;
      case via::comms::SENT:
        sent_handler(connection);
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
    // @param connection a weak ponter to the underlying comms connection.
    void error_handler(const boost::system::error_code &error,
                       boost::weak_ptr<connection_type>) // connection)
    {
      std::cerr << "error_handler" << std::endl;
      std::cerr << error <<  std::endl;
    }

    void set_translate_head(bool enable)
    { translate_head_ = enable; }

    void set_require_host_header(bool enable)
    { require_host_header_ = enable; }

    void set_trace_enabled(bool enable)
    { trace_enabled_ = enable; }

    /// @fn set_keep_alive
    /// Set the tcp keep alive status for all future connections.
    /// @param enable if true enables the tcp socket keep alive status.
    void set_keep_alive(bool enable)
    { server_->set_keep_alive(enable); }

    /// Set the send and receive timeout value for all future connections.
    /// @pre sockets may remain open forever
    /// @post sockets will close if no activity has occured after the
    /// timeout period.
    /// @param timeout the timeout in milliseconds.
    void set_timeout(int timeout)
    { server_->set_timeout(timeout); }

    /// Set the password for an SSL connection.
    /// Note: only valid for SSL connections, do NOT call for TCP servers.
    /// @param password the SSL password
    void set_password(std::string const& password)
    { server_->set_password(password); }

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
#ifdef HTTP_SSL
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
#endif // HTTP_SSL
      return error;
    }

    /// @fn close
    /// Close the http server and all of the connections associated with it.
    void close()
    {
      http_connections_.clear();
      server_->close();
    }
  };

}
#endif
