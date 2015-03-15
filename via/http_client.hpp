#ifndef HTTP_CLIENT_HPP_VIA_HTTPLIB_
#define HTTP_CLIENT_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file http_client.hpp
/// @brief Contains the http_client template class.
//////////////////////////////////////////////////////////////////////////////
#include "via/http/request.hpp"
#include "via/http/response.hpp"
#include "via/comms/connection.hpp"
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <boost/bind.hpp>

namespace via
{
  ////////////////////////////////////////////////////////////////////////////
  /// @class http_client
  /// The class template can be configured to use either tcp or ssl sockets
  /// depending upon which class is provided as the SocketAdaptor:
  /// tcp_adaptor or ssl::ssl_tcp_adaptor.
  /// @see comms::tcp_adaptor
  /// @see comms::ssl::ssl_tcp_adaptor
  /// @param SocketAdaptor the type of socket to use:
  /// tcp_adaptor or ssl::ssl_tcp_adaptor
  /// @param Container the container to use for the tx buffer:
  /// std::vector<char> or std::string, default std::vector<char>.
  /// @param use_strand if true use an asio::strand to wrap the handlers,
  /// default false.
  ////////////////////////////////////////////////////////////////////////////
  template <typename SocketAdaptor, typename Container = std::vector<char>,
            bool use_strand = false>
  class http_client
  {
  public:
    /// The underlying connection, TCP or SSL.
    typedef comms::connection<SocketAdaptor, Container, use_strand>
                                                              connection_type;

    /// A shared pointer to this type.
    typedef typename boost::shared_ptr<http_client<SocketAdaptor, Container,
                                                   use_strand> > shared_pointer;

    /// The template requires a typename to access the iterator.
    typedef typename Container::const_iterator Container_const_iterator;

    /// The type of the response_receiver.
    typedef typename http::response_receiver<Container> http_response;

    /// The chunk type
    typedef typename http::rx_chunk<Container> chunk_type;

    /// The ResponseHandler type.
    typedef TR1::function <void (http::rx_response const&, Container const&)>
      ResponseHandler;

    /// The ChunkHandler type.
    typedef TR1::function <void (chunk_type const&, Container const&)>
      ChunkHandler;

    /// The ConnectionHandler type.
    typedef TR1::function <void (void)>
      ConnectionHandler;

  private:

    ////////////////////////////////////////////////////////////////////////
    // Variables

    boost::shared_ptr<connection_type> connection_; ///< the comms connection
    http::response_receiver<Container> rx_;         ///< the response receiver
    std::string host_name_;                         ///< the name of the host

    std::string tx_header_; /// A buffer for the HTTP request header.
    Container   tx_body_;   /// A buffer for the HTTP request body.

    ResponseHandler   http_response_handler_; ///< the response callback function
    ChunkHandler      http_chunk_handler_;    ///< the chunk callback function
    ResponseHandler   http_invalid_handler;   ///< the invalid callback function
    ConnectionHandler connected_handler_;     ///< the connected callback function
    ConnectionHandler packet_sent_handler_;   ///< the sent callback function
    ConnectionHandler disconnected_handler_;  ///< the disconnected callback function

    ////////////////////////////////////////////////////////////////////////
    // Functions

    /// Send buffers on the connection.
    /// @param buffers the data to write.
    bool send(comms::ConstBuffers buffers)
    {
      rx_.clear();
      return connection_->send_data(buffers);
    }

    /// Receive data on the underlying connection.
    void receive_handler()
    {
      // Get the receive buffer
      Container rx_buffer;
      connection_->read_rx_buffer(rx_buffer);
      Container_const_iterator iter(rx_buffer.begin());
      Container_const_iterator end(rx_buffer.end());

      // Get the receive parser for this connection
      http::Rx rx_state(http::RX_VALID);

      // Loop around the received buffer while there's valid data to read
      while ((iter != end) && (rx_state != http::RX_INVALID))
      {
        rx_state = rx_.receive(iter, end);

        switch (rx_state)
        {
        case http::RX_VALID:
          http_response_handler_(rx_.response(), rx_.body());
          break;

        case http::RX_CHUNK:
          if (http_chunk_handler_ != NULL)
            http_chunk_handler_(rx_.chunk(), rx_.chunk().data());
          break;

        case http::RX_INVALID:
          if (http_invalid_handler != NULL)
            http_invalid_handler(rx_.response(), rx_.body());
          break;

        default:
          break;
        } // end switch
      } // end while
    }

    /// Receive an event from the underlying comms connection.
    /// @param event the type of event.
    /// @param weak_ptr a weak ponter to the underlying comms connection.
    void event_handler(int event,
                       typename connection_type::weak_pointer weak_ptr)
    {
      // Use the raw pointer of the connection as the map key.
      void* pointer(weak_ptr.lock().get());
      if (!pointer)
        return;

      switch(event)
      {
      case via::comms::CONNECTED:
        if (connected_handler_ != NULL)
          connected_handler_();
        break;
      case via::comms::RECEIVED:
        receive_handler();
        break;
      case via::comms::SENT:
        if (packet_sent_handler_ != NULL)
          packet_sent_handler_();
        break;
      case via::comms::DISCONNECTED:
        if (disconnected_handler_ != NULL)
          disconnected_handler_();
        connection_->close();
        break;
      default:
        break;
      }
    }

    /// Receive an error from the underlying comms connection.
    /// @param error the boost error_code.
    // @param weak_ptr a weak ponter to the underlying comms connection.
    void error_handler(const boost::system::error_code &error,
                       typename connection_type::weak_pointer) // weak_ptr)
    {
      std::cerr << "error_handler" << std::endl;
      std::cerr << error <<  std::endl;
    }

    /// Constructor.
    /// @param io_service the asio io_service to use.
    /// @param response_handler the handler for received HTTP responses.
    /// @param chunk_handler the handler for received HTTP chunks.
    /// @param rx_buffer_size the size of the receive_buffer, default
    /// SocketAdaptor::DEFAULT_RX_BUFFER_SIZE
    explicit http_client(boost::asio::io_service& io_service,
                         ResponseHandler response_handler,
                         ChunkHandler    chunk_handler,
                         size_t          rx_buffer_size) :
      connection_(connection_type::create(io_service, rx_buffer_size)),
      rx_(),
      host_name_(),
      tx_header_(),
      tx_body_(),
      http_response_handler_(response_handler),
      http_chunk_handler_(chunk_handler),
      http_invalid_handler(),
      connected_handler_(),
      packet_sent_handler_(),
      disconnected_handler_()
    {
      connection_->set_event_callback
          (boost::bind(&http_client::event_handler, this, _1, _2));
      connection_->set_error_callback
          (boost::bind(&http_client::error_handler, this,
                       boost::asio::placeholders::error, _2));
      // Set no delay, i.e. disable the Nagle algorithm
      // An http_client will want to send messages immediately
      connection_->set_no_delay(true);
    }

    ////////////////////////////////////////////////////////////////////////

  public:

    /// @fn create
    /// The factory function to create connections.
    /// @param io_service the boost asio io_service used by the underlying
    /// connection.
    /// @param response_handler the handler for received HTTP responses.
    /// @param chunk_handler the handler for received HTTP chunks.
    /// @param rx_buffer_size the size of the receive_buffer, default
    /// SocketAdaptor::DEFAULT_RX_BUFFER_SIZE
    static shared_pointer create(boost::asio::io_service& io_service,
                                 ResponseHandler response_handler,
                                 ChunkHandler    chunk_handler,
               size_t rx_buffer_size = SocketAdaptor::DEFAULT_RX_BUFFER_SIZE)
    { return shared_pointer(new http_client(io_service, response_handler,
                                            chunk_handler, rx_buffer_size)); }

    /// Connect to the given host name and port.
    /// @param host_name the host to connect to.
    /// @param port_name the port to connect to.
    /// @return true if resolved, false otherwise.
    bool connect(const std::string& host_name, std::string port_name = "http")
    {
      host_name_ = host_name;
      if ((port_name != "http") && (port_name != "https"))
      {
        host_name_ += ":";
        host_name_ += port_name;
      }

      return connection_->connect(host_name.c_str(), port_name.c_str());
    }

    ////////////////////////////////////////////////////////////////////////
    // Event Handlers

    /// Connect the connected callback function.
    /// @param handler the handler for the socket connected signal.
    void connected_event(ConnectionHandler handler) NOEXCEPT
    { connected_handler_ = handler; }

    /// Connect the message sent callback function.
    /// @param handler the handler for the message sent signal.
    void msg_sent_event(ConnectionHandler handler) NOEXCEPT
    { packet_sent_handler_ = handler; }

    /// Connect the disconnected callback function.
    /// @param handler the handler for the socket disconnected signal.
    void disconnected_event(ConnectionHandler handler) NOEXCEPT
    { disconnected_handler_ = handler; }

    ////////////////////////////////////////////////////////////////////////
    // Accessors

    /// Accessor for the HTTP response header.
    /// @return a constant reference to an rx_response.
    http::rx_response const& response() const NOEXCEPT
    { return rx_.response(); }

    /// Accessor for the received HTTP chunk.
    /// @return a constant reference to an rx_chunk.
    http::rx_chunk<Container> const& chunk() const NOEXCEPT
    { return rx_.chunk(); }

    /// Accessor for the body.
    /// @return a constant reference to the body.
    Container const& body() const NOEXCEPT
    { return rx_.body(); }

    ////////////////////////////////////////////////////////////////////////
    // send (request) functions

    /// Send an HTTP request without a body.
    /// @param request the request to send.
    bool send(http::tx_request request)
    {
      request.add_header(http::header_field::id::HOST, host_name_);
      tx_header_ = request.message();
      return send(comms::ConstBuffers(1, boost::asio::buffer(tx_header_)));
    }

    /// Send an HTTP request with a body.
    /// @param request the request to send.
    /// @param body the body to send
    bool send(http::tx_request request, Container body)
    {
      request.add_header(http::header_field::id::HOST, host_name_);
      tx_header_ = request.message(body.size());
      comms::ConstBuffers buffers(1, boost::asio::buffer(tx_header_));

      tx_body_.swap(body);
      buffers.push_back(boost::asio::buffer(tx_body_));
      return send(buffers);
    }

    /// Send an HTTP request with a body.
    /// @pre The contents of the buffers are NOT buffered.
    /// Their lifetime MUST exceed that of the write
    /// @param request the request to send.
    /// @param buffers the body to send
    bool send(http::tx_request request, comms::ConstBuffers buffers)
    {
      request.add_header(http::header_field::id::HOST, host_name_);
      tx_header_ = request.message(boost::asio::buffer_size(buffers));

      buffers.push_front(boost::asio::buffer(tx_header_));
      return send(buffers);
    }

    ////////////////////////////////////////////////////////////////////////
    // send_body functions

    /// Send an HTTP request body.
    /// @pre the request must have been sent beforehand.
    /// @param body the body to send
    /// @return true if sent, false otherwise.
    bool send_body(Container body)
    {
      tx_body_.swap(body);
      return send(comms::ConstBuffers(1, boost::asio::buffer(tx_body_)));
    }

    /// Send an HTTP request body.
    /// @pre the request must have been sent beforehand.
    /// @pre The contents of the buffers are NOT buffered.
    /// Their lifetime MUST exceed that of the write
    /// @param buffers the body to send
    /// @return true if sent, false otherwise.
    bool send_body(comms::ConstBuffers buffers)
    { return send(buffers); }

    ////////////////////////////////////////////////////////////////////////
    // send_chunk functions

    /// Send an HTTP body chunk.
    /// @param chunk the body chunk to send
    /// @param extension the (optional) chunk extension.
    bool send_chunk(Container chunk, std::string extension = "")
    {
      size_t size(chunk.size());
      http::chunk_header chunk_header(size, extension);
      tx_header_ = chunk_header.to_string();
      tx_body_.swap(chunk);

      comms::ConstBuffers buffers(1, boost::asio::buffer(tx_header_));
      buffers.push_back(boost::asio::buffer(tx_body_));
      buffers.push_back(boost::asio::buffer(http::CRLF));
      return send(buffers);
    }

    /// Send an HTTP body chunk.
    /// @pre The contents of the buffers are NOT buffered.
    /// Their lifetime MUST exceed that of the write
    /// @param buffers the body chunk to send
    /// @param extension the (optional) chunk extension.
    bool send_chunk(comms::ConstBuffers buffers, std::string extension = "")
    {
      // Calculate the overall size of the data in the buffers
      size_t size(boost::asio::buffer_size(buffers));

      http::chunk_header chunk_header(size, extension);
      tx_header_ = chunk_header.to_string();
      buffers.push_front(boost::asio::buffer(tx_header_));
      buffers.push_back(boost::asio::buffer(http::CRLF));
      return send(buffers);
    }

    /// Send the last HTTP chunk for a request.
    /// @param extension the (optional) chunk extension.
    /// @param trailer_string the (optional) chunk trailers.
    bool last_chunk(std::string extension = "",
                    std::string trailer_string = "")
    {
      http::last_chunk last_chunk(extension, trailer_string);
      tx_header_ = last_chunk.message();

      return send(comms::ConstBuffers(1, boost::asio::buffer(tx_header_)));
    }

    ////////////////////////////////////////////////////////////////////////
    // other functions

    /// Disconnect the underlying connection.
    void disconnect()
    { connection_->shutdown(); }

    /// Accessor function for the comms connection.
    /// @return a shared pointer to the connection
    boost::shared_ptr<connection_type> connection() NOEXCEPT
    { return connection_; }
  };
}

#endif
