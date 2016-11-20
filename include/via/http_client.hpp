#ifndef HTTP_CLIENT_HPP_VIA_HTTPLIB_
#define HTTP_CLIENT_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2016 Ken Barker
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
#include <iostream>

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
  class http_client : public std::enable_shared_from_this
                          <http_client<SocketAdaptor, Container, use_strand> >
  {
  public:
    /// The underlying connection, TCP or SSL.
    typedef comms::connection<SocketAdaptor, Container, use_strand>
                                                              connection_type;

    /// A weak pointer to this type.
    typedef typename std::weak_ptr<http_client<SocketAdaptor, Container,
                                                   use_strand> > weak_pointer;

    /// A shared pointer to this type.
    typedef typename std::shared_ptr<http_client<SocketAdaptor, Container,
                                                   use_strand> > shared_pointer;

    /// The enable_shared_from_this type of this class.
    typedef typename std::enable_shared_from_this
                <http_client<SocketAdaptor, Container, use_strand> > enable;

    /// The template requires a typename to access the iterator.
    typedef typename Container::const_iterator Container_const_iterator;

    /// The type of the response_receiver.
    typedef typename http::response_receiver<Container> http_response;

    /// The chunk type
    typedef typename http::rx_chunk<Container> chunk_type;

    /// The ResponseHandler type.
    typedef std::function <void (http::rx_response const&, Container const&)>
      ResponseHandler;

    /// The ChunkHandler type.
    typedef std::function <void (chunk_type const&, Container const&)>
      ChunkHandler;

    /// The ConnectionHandler type.
    typedef std::function <void (void)>
      ConnectionHandler;

  private:

    ////////////////////////////////////////////////////////////////////////
    // Variables

    std::shared_ptr<connection_type> connection_; ///< the comms connection
    ASIO_TIMER timer_;                            ///< a deadline timer
    http::response_receiver<Container> rx_;       ///< the response receiver
    std::string host_name_;                       ///< the name of the host
    std::string port_name_;                       ///< the port name / number
    unsigned long period_;                        ///< the reconnection period

    std::string tx_header_; /// A buffer for the HTTP request header.
    Container   tx_body_;   /// A buffer for the HTTP request body.
    Container   rx_buffer_; /// A buffer for the last packet read.

    ResponseHandler   http_response_handler_; ///< the response callback function
    ChunkHandler      http_chunk_handler_;    ///< the chunk callback function
    ResponseHandler   http_invalid_handler_;  ///< the invalid callback function
    ConnectionHandler connected_handler_;     ///< the connected callback function
    ConnectionHandler disconnected_handler_;  ///< the disconnected callback function
    ConnectionHandler message_sent_handler_;  ///< the message sent callback function

    ////////////////////////////////////////////////////////////////////////
    // Functions

    /// @fn weak_from_this
    /// Get a weak_pointer to this instance.
    /// @return a weak_pointer to this http_client.
    weak_pointer weak_from_this()
    { return weak_pointer(enable::shared_from_this()); }

    /// Attempt to connect to the host.
    bool connect()
    {
      if (connection_->connected())
        return true;
      return connection_->connect(host_name_.c_str(), port_name_.c_str());
    }

    /// The callback function for the timer_.
    /// @param ptr a weak pointer to this http_client.
    /// @param error the asio error code.
    static void timeout_handler(weak_pointer ptr,
                                ASIO_ERROR_CODE const& error)
    {
      shared_pointer pointer(ptr.lock());
      if (pointer && (ASIO::error::operation_aborted != error))
        pointer->connect();
    }

    /// Send buffers on the connection.
    /// @param buffers the data to write.
    bool send(comms::ConstBuffers buffers)
    {
      rx_.clear();
      return connection_->send_data(std::move(buffers));
    }

    /// Receive data on the underlying connection.
    void receive_handler()
    {
      // Get the receive buffer
      connection_->read_rx_buffer(rx_buffer_);
      Container_const_iterator iter(rx_buffer_.begin());
      Container_const_iterator end(rx_buffer_.end());

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
          if (!rx_.response().is_chunked())
            rx_.clear();
          break;

        case http::RX_CHUNK:
          if (http_chunk_handler_)
            http_chunk_handler_(rx_.chunk(), rx_.chunk().data());

          if (rx_.chunk().is_last())
            rx_.clear();
          break;

        case http::RX_INVALID:
          if (http_invalid_handler_)
            http_invalid_handler_(rx_.response(), rx_.body());

          rx_.clear();
          break;

        default:
          break;
        } // end switch
      } // end while
    }

    /// Handle a diconnect on the underlying connection.
    void disconnected_handler()
    {
      if (connection_->connected())
      {
        connection_->set_connected(false);
        connection_->close();
      }

      if (disconnected_handler_)
        disconnected_handler_();

      // attempt to reconnect in period_ miliseconds
      if (period_ > 0)
      {
#ifdef ASIO_STANDALONE
        timer_.expires_from_now(std::chrono::milliseconds(period_));
#else
        timer_.expires_from_now(boost::posix_time::milliseconds(period_));
#endif
        weak_pointer weak_ptr(weak_from_this());
        timer_.async_wait([weak_ptr](ASIO_ERROR_CODE const& error)
                           { timeout_handler(weak_ptr, error); });
      }
    }

    /// Callback function for a comms::connection event.
    /// @param ptr a weak pointer to this http_client.
    /// @param event the type of event.
    /// @param weak_ptr a weak ponter to the underlying comms connection.
    static void event_callback(weak_pointer ptr, int event,
                               typename connection_type::weak_pointer weak_ptr)
    {
      shared_pointer pointer(ptr.lock());
      if (pointer)
        pointer->event_handler(event, weak_ptr);
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
        timer_.cancel();
        rx_buffer_.clear();
        rx_.clear();
        if (connected_handler_)
          connected_handler_();
        break;
      case via::comms::RECEIVED:
        receive_handler();
        break;
      case via::comms::SENT:
        if (message_sent_handler_)
          message_sent_handler_();
        break;
      case via::comms::DISCONNECTED:
        disconnected_handler();
        break;
      default:
        break;
      }
    }

    /// Receive an error from the underlying comms connection.
    /// @param error the boost error_code.
    // @param weak_ptr a weak pointer to the underlying comms connection.
    static void error_handler(const ASIO_ERROR_CODE &error,
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
    explicit http_client(ASIO::io_service& io_service,
                         ResponseHandler response_handler,
                         ChunkHandler    chunk_handler,
                         size_t          rx_buffer_size) :
      connection_(connection_type::create(io_service, rx_buffer_size)),
      timer_(io_service),
      rx_(),
      host_name_(),
      tx_header_(),
      tx_body_(),
      rx_buffer_(),
      http_response_handler_(response_handler),
      http_chunk_handler_(chunk_handler),
      http_invalid_handler_(),
      connected_handler_(),
      disconnected_handler_(),
      message_sent_handler_()
    {
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
    static shared_pointer create(ASIO::io_service& io_service,
                                 ResponseHandler response_handler,
                                 ChunkHandler    chunk_handler,
               size_t rx_buffer_size = SocketAdaptor::DEFAULT_RX_BUFFER_SIZE)
    {
      shared_pointer client_ptr(new http_client(io_service, response_handler,
                                            chunk_handler, rx_buffer_size));
      weak_pointer ptr(client_ptr);
      client_ptr->connection_->set_error_callback([]
        (const ASIO_ERROR_CODE &error,
         typename connection_type::weak_pointer weak_ptr)
           { error_handler(error, weak_ptr); });
      client_ptr->connection_->set_event_callback([ptr]
        (int event, typename connection_type::weak_pointer weak_ptr)
           { event_callback(ptr, event, weak_ptr); });
      return client_ptr;
    }

    /// Destructor
    /// Close the socket and cancel the timer.
    virtual ~http_client()
    { close(); }

    /// Connect to the given host name and port.
    /// @param host_name the host to connect to.
    /// @param port_name the port to connect to.
    /// @param period the time to wait after a disconnect before attempting to
    /// re-connect, default zero. I.e. don't attempt to re-connect.
    /// @return true if resolved, false otherwise.
    bool connect(const std::string& host_name, std::string port_name = "http",
                 unsigned long period = 0)
    {
      host_name_ = host_name;
      port_name_ = port_name;
      period_    = period;

      return connect();
    }

    /// Accessor for whether the underlying socket is connected.
    bool is_connected() const NOEXCEPT
    { return connection_->connected(); }

    ////////////////////////////////////////////////////////////////////////
    // Event Handlers

    /// Connect the invalid response received callback function.
    /// @param handler the handler for an invalid response received.
    void invalid_response_event(ResponseHandler handler) NOEXCEPT
    { http_invalid_handler_ = handler; }

    /// Connect the connected callback function.
    /// @param handler the handler for the socket connected signal.
    void connected_event(ConnectionHandler handler) NOEXCEPT
    { connected_handler_ = handler; }

    /// Connect the disconnected callback function.
    /// @param handler the handler for the socket disconnected signal.
    void disconnected_event(ConnectionHandler handler) NOEXCEPT
    { disconnected_handler_ = handler; }

    /// Connect the message sent callback function.
    /// @param handler the handler for the message sent signal.
    void message_sent_event(ConnectionHandler handler) NOEXCEPT
    { message_sent_handler_ = handler; }

    ////////////////////////////////////////////////////////////////////////
    // Accessors

    /// Accessor for the receive buffer..
    /// @return a constant reference to rx_buffer_.
    Container const& rx_buffer() const
    { return rx_buffer_; }

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

    /// Get the host name to send in the http "Host:" header.
    /// @return http host name.
    std::string http_host_name() const
    {
      if ((port_name_ == "http") || (port_name_ == "https"))
        return host_name_;
      else
        return host_name_ + ":" + port_name_;
    }

    ////////////////////////////////////////////////////////////////////////
    // send (request) functions

    /// Send an HTTP request without a body.
    /// @param request the request to send.
    bool send(http::tx_request request)
    {
      if (!is_connected())
        return false;

      request.add_header(http::header_field::id::HOST, http_host_name());
      tx_header_ = request.message();
      return send(comms::ConstBuffers(1, ASIO::buffer(tx_header_)));
    }

    /// Send an HTTP request with a body.
    /// @param request the request to send.
    /// @param body the body to send
    bool send(http::tx_request request, Container body)
    {
      if (!is_connected())
        return false;

      request.add_header(http::header_field::id::HOST, http_host_name());
      tx_header_ = request.message(body.size());
      comms::ConstBuffers buffers(1, ASIO::buffer(tx_header_));

      tx_body_.swap(body);
      buffers.push_back(ASIO::buffer(tx_body_));
      return send(std::move(buffers));
    }

    /// Send an HTTP request with a body.
    /// @pre The contents of the buffers are NOT buffered.
    /// Their lifetime MUST exceed that of the write
    /// @param request the request to send.
    /// @param buffers the body to send
    bool send(http::tx_request request, comms::ConstBuffers buffers)
    {
      if (!is_connected())
        return false;

      request.add_header(http::header_field::id::HOST, http_host_name());
      tx_header_ = request.message(ASIO::buffer_size(buffers));

      buffers.push_front(ASIO::buffer(tx_header_));
      return send(std::move(buffers));
    }

    ////////////////////////////////////////////////////////////////////////
    // send_body functions

    /// Send an HTTP request body.
    /// @pre the request must have been sent beforehand.
    /// @param body the body to send
    /// @return true if sent, false otherwise.
    bool send_body(Container body)
    {
      if (!is_connected())
        return false;

      tx_body_.swap(body);
      return send(comms::ConstBuffers(1, ASIO::buffer(tx_body_)));
    }

    /// Send an HTTP request body.
    /// @pre the request must have been sent beforehand.
    /// @pre The contents of the buffers are NOT buffered.
    /// Their lifetime MUST exceed that of the write
    /// @param buffers the body to send
    /// @return true if sent, false otherwise.
    bool send_body(comms::ConstBuffers buffers)
    {
      if (!is_connected())
        return false;

      return send(std::move(buffers));
    }

    ////////////////////////////////////////////////////////////////////////
    // send_chunk functions

    /// Send an HTTP body chunk.
    /// @param chunk the body chunk to send
    /// @param extension the (optional) chunk extension.
    bool send_chunk(Container chunk, std::string extension = "")
    {
      if (!is_connected())
        return false;

      size_t size(chunk.size());
      http::chunk_header chunk_header(size, extension);
      tx_header_ = chunk_header.to_string();
      tx_body_.swap(chunk);

      comms::ConstBuffers buffers(1, ASIO::buffer(tx_header_));
      buffers.push_back(ASIO::buffer(tx_body_));
      buffers.push_back(ASIO::buffer(http::CRLF));
      return send(std::move(buffers));
    }

    /// Send an HTTP body chunk.
    /// @pre The contents of the buffers are NOT buffered.
    /// Their lifetime MUST exceed that of the write
    /// @param buffers the body chunk to send
    /// @param extension the (optional) chunk extension.
    bool send_chunk(comms::ConstBuffers buffers, std::string extension = "")
    {
      if (!is_connected())
        return false;

      // Calculate the overall size of the data in the buffers
      size_t size(ASIO::buffer_size(buffers));

      http::chunk_header chunk_header(size, extension);
      tx_header_ = chunk_header.to_string();
      buffers.push_front(ASIO::buffer(tx_header_));
      buffers.push_back(ASIO::buffer(http::CRLF));
      return send(std::move(buffers));
    }

    /// Send the last HTTP chunk for a request.
    /// @param extension the (optional) chunk extension.
    /// @param trailer_string the (optional) chunk trailers.
    bool last_chunk(std::string extension = "",
                    std::string trailer_string = "")
    {
      if (!is_connected())
        return false;

      http::last_chunk last_chunk(extension, trailer_string);
      tx_header_ = last_chunk.to_string();

      return send(comms::ConstBuffers(1, ASIO::buffer(tx_header_)));
    }

    ////////////////////////////////////////////////////////////////////////
    // other functions

    /// Disconnect the underlying connection.
    void disconnect()
    { connection_->shutdown(); }

    /// Close the socket and cancel the timer.
    void close()
    {
      period_ = 0;
      timer_.cancel();
      connection_->close();
    }

    /// Accessor function for the comms connection.
    /// @return a shared pointer to the connection
    std::shared_ptr<connection_type> connection() NOEXCEPT
    { return connection_; }
  };
}

#endif
