#ifndef HTTP_CONNECTION_HPP_VIA_HTTPLIB_
#define HTTP_CONNECTION_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file http_connection.hpp
/// @brief Contains the http_connection template class.
/// @see http_server
//////////////////////////////////////////////////////////////////////////////
#include "via/http/request.hpp"
#include "via/http/response.hpp"
#include "via/comms/connection.hpp"
#include <deque>
#include <iostream>

namespace via
{
  ////////////////////////////////////////////////////////////////////////////
  /// @class http_connection
  /// An HTTP connection.
  /// The class can be configured to use either tcp or ssl sockets depending
  /// upon which class is provided as the SocketAdaptor: tcp_adaptor or
  /// ssl::ssl_tcp_adaptor respectively.
  /// @see comms::connection
  /// @see comms::tcp_adaptor
  /// @see comms::ssl::ssl_tcp_adaptor
  /// @tparam SocketAdaptor the type of socket.
  /// @tparam Container the type of container in which the request is held.
  /// @tparam MAX_CONTENT_LENGTH the maximum size of an HTTP request body
  /// including any chunks.
  /// @tparam MAX_CHUNK_SIZE the maximum size of an HTTP request chunk.
  /// @tparam MAX_URI_LENGTH the maximum length of an HTTP request uri.
  /// @tparam MAX_METHOD_LENGTH the maximum length of an HTTP request method.
  /// @tparam MAX_HEADER_NUMBER the maximum number of HTTP header field lines.
  /// @tparam MAX_HEADER_LENGTH the maximum cumulative length the HTTP header
  /// fields.
  /// @tparam MAX_LINE_LENGTH the maximum length of an HTTP header field line.
  /// @tparam MAX_WHITESPACE_CHARS the maximum number of consectutive whitespace
  /// characters allowed in a request.
  /// @tparam STRICT_CRLF enforce strict parsing of CRLF.
  ////////////////////////////////////////////////////////////////////////////
  template <typename SocketAdaptor,
            typename Container,
            size_t         MAX_CONTENT_LENGTH,
            size_t         MAX_CHUNK_SIZE,
            size_t         MAX_URI_LENGTH,
            unsigned char  MAX_METHOD_LENGTH,
            unsigned short MAX_HEADER_NUMBER,
            size_t         MAX_HEADER_LENGTH,
            unsigned short MAX_LINE_LENGTH,
            unsigned char  MAX_WHITESPACE_CHARS,
            bool           STRICT_CRLF>
  class http_connection : public std::enable_shared_from_this
                                   <http_connection<SocketAdaptor,
                                                    Container,
                                                    MAX_CONTENT_LENGTH,
                                                    MAX_CHUNK_SIZE,
                                                    MAX_URI_LENGTH,
                                                    MAX_METHOD_LENGTH,
                                                    MAX_HEADER_NUMBER,
                                                    MAX_HEADER_LENGTH,
                                                    MAX_LINE_LENGTH,
                                                    MAX_WHITESPACE_CHARS,
                                                    STRICT_CRLF>>
  {
  public:
    /// The underlying connection, TCP or SSL.
    typedef comms::connection<SocketAdaptor, Container> connection_type;

    typedef http_connection<SocketAdaptor,
                            Container,
                            MAX_CONTENT_LENGTH,
                            MAX_CHUNK_SIZE,
                            MAX_URI_LENGTH,
                            MAX_METHOD_LENGTH,
                            MAX_HEADER_NUMBER,
                            MAX_HEADER_LENGTH,
                            MAX_LINE_LENGTH,
                            MAX_WHITESPACE_CHARS,
                            STRICT_CRLF> this_type;

    /// A weak pointer to this type.
    typedef typename std::weak_ptr<this_type> weak_pointer;

    /// A shared pointer to this type.
    typedef typename std::shared_ptr<this_type> shared_pointer;

    /// The template requires a typename to access the iterator.
    typedef typename Container::const_iterator Container_const_iterator;

    /// The type of the request_receiver.
    typedef typename http::request_receiver<Container,
                                            MAX_CONTENT_LENGTH,
                                            MAX_CHUNK_SIZE,
                                            MAX_URI_LENGTH,
                                            MAX_METHOD_LENGTH,
                                            MAX_HEADER_NUMBER,
                                            MAX_HEADER_LENGTH,
                                            MAX_LINE_LENGTH,
                                            MAX_WHITESPACE_CHARS,
                                            STRICT_CRLF> http_request_rx;

    /// The type of the http rx_request.
    typedef typename http_request_rx::Request http_request;

    /// The chunk type
    typedef typename http_request_rx::Chunk chunk_type;

  private:

    ////////////////////////////////////////////////////////////////////////
    // Variables

    /// A weak pointer to underlying connection.
    typename connection_type::weak_pointer connection_;

    /// The remote address of the connection_.
    std::string remote_address_;

    /// The request receiver for this connection.
    http_request_rx rx_;

    /// A buffer for the HTTP header of the response message.
    std::string tx_header_;

    /// A buffer for the body of the response message.
    Container tx_body_;

    /// A buffer for the last packet read on the connection.
    Container rx_buffer_;

    ////////////////////////////////////////////////////////////////////////
    // Functions

    /// Send buffers on the connection.
    /// @param buffers the data to write.
    bool send(comms::ConstBuffers buffers)
    {
      std::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (tcp_pointer)
      {
        tcp_pointer->send_data(std::move(buffers));
        return true;
      }
      else
        return false;
    }

    /// Send buffers on the connection.
    /// @param buffers the data to write.
    /// @param is_continue whether this is a 100 Continue response
    bool send(comms::ConstBuffers buffers, bool is_continue)
    {
      bool keep_alive(rx_.request().keep_alive());
      if (is_continue)
        rx_.set_continue_sent();
      else
        rx_.clear();

      std::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (tcp_pointer)
      {
        tcp_pointer->send_data(std::move(buffers));

        if (keep_alive)
          return true;
        else
          tcp_pointer->shutdown();
      }
      else
        std::cerr << "http_connection::send connection weak pointer expired"
                  << std::endl;
      return false;
    }

    ////////////////////////////////////////////////////////////////////////

  public:

    /// Constructor.
    /// Note: only a shared pointer to this type should be created.
    /// @param connection a weak pointer to the underlying connection.
    http_connection(typename connection_type::weak_pointer connection) :
      connection_(connection),
      remote_address_(connection_.lock()->socket().
                      remote_endpoint().address().to_string()),
      rx_(),
      tx_header_(),
      tx_body_(),
      rx_buffer_()
    {}

    /// The destructor calls close to ensure that all of the socket's
    /// callback functions are cancelled.
    ~http_connection()
    { close(); }

    ////////////////////////////////////////////////////////////////////////
    // Request Parser Parameters

    /// Enable whether the http server translates HEAD requests into GET
    /// requests for the application.
    /// Note: http_server never sends a body in a response to a HEAD request.
    /// @post HEAD translation enabled/disabled.
    /// @param enable enable the function.
    void set_translate_head(bool enable) noexcept
    { rx_.set_translate_head(enable); }

    /// Enable whether the http server concatenates chunked requests.
    /// If a ChunkHandler is not registered with the http_server then any
    /// recieved chunks will be concatenated into the request message body.
    /// @post chunk concatenation enabled/disabled.
    /// @param enable enable the function.
    void set_concatenate_chunks(bool enable) noexcept
    { rx_.set_concatenate_chunks(enable); }

    ////////////////////////////////////////////////////////////////////////
    // Accessors

    /// Read the last packet into the receive buffer.
    /// @post the receive buffer is invalid to read again.
    /// @return the receive buffer.
    Container const& read_rx_buffer()
    {
      connection_.lock()->read_rx_buffer(rx_buffer_);
      return rx_buffer_;
    }

    /// Accessor for the receive buffer.
    /// @return the receive buffer.
    Container const& rx_buffer() const noexcept
    { return rx_buffer_; }

    /// Accessor for the remote address of the connection.
    /// @return the remote address of the connection.
    std::string const& remote_address() const noexcept
    { return remote_address_; }

    /// The request receiver for this connection.
    http_request_rx& rx() noexcept
    { return rx_; }

    /// Accessor for the HTTP request header.
    /// @return a constant reference to an rx_request.
    http_request const& request() const noexcept
    { return rx_.request(); }

    /// Accessor for the body.
    /// @return a constant reference to the body.
    Container const& body() const noexcept
    { return rx_.body(); }

    /// Accessor for the received HTTP chunk.
    /// @return a constant reference to an rx_chunk.
    chunk_type const& chunk() const noexcept
    { return rx_.chunk(); }

    ////////////////////////////////////////////////////////////////////////
    // send (response) functions

    /// Send the appropriate HTTP response to the request.
    /// @return true if sent, false otherwise.
    bool send_response()
    {
      http::tx_response response(rx_.response_code());
      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      tx_header_ = response.message();

      return send(comms::ConstBuffers(1, ASIO::buffer(tx_header_)),
                  response.is_continue());
    }

    /// Send an HTTP response without a body.
    /// @pre the response must not contain any split headers.
    /// @param response the response to send.
    /// @return true if sent, false otherwise.
    bool send(http::tx_response response)
    {
      if (!response.is_valid())
        return false;

      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      tx_header_ = response.message();

      return send(comms::ConstBuffers(1, ASIO::buffer(tx_header_)),
                  response.is_continue());
    }

    /// Send an HTTP response with a body.
    /// @pre the response must not contain any split headers.
    /// @param response the response to send.
    /// @param body the body to send
    /// @return true if sent, false otherwise.
    bool send(http::tx_response response, Container body)
    {
      if (!response.is_valid())
        return false;

      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      tx_header_ = response.message(body.size());
      comms::ConstBuffers buffers(1, ASIO::buffer(tx_header_));

      // Don't send a body in response to a HEAD request
      if (!rx_.is_head())
      {
        tx_body_.swap(body);
        buffers.push_back(ASIO::buffer(tx_body_));
      }

      return send(std::move(buffers), response.is_continue());
    }

    /// Send an HTTP response with a body.
    /// @pre the response must not contain any split headers.
    /// @pre The contents of the buffers are NOT buffered.
    /// Their lifetime MUST exceed that of the write
    /// @param response the response to send.
    /// @param buffers a deque of asio::buffers containing the body to send.
    bool send(http::tx_response response, comms::ConstBuffers buffers)
    {
      if (!response.is_valid())
        return false;

      // Calculate the overall size of the data in the buffers
      size_t size(ASIO::buffer_size(buffers));

      // Don't send a body in response to a HEAD request
      if (rx_.is_head())
        buffers.clear();

      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      tx_header_ = response.message(size);
      buffers.push_front(ASIO::buffer(tx_header_));

      return send(std::move(buffers), response.is_continue());
    }

    ////////////////////////////////////////////////////////////////////////
    // send_chunk functions

    /// Send an HTTP body chunk.
    /// @param chunk the body chunk to send
    /// @param extension the (optional) chunk extension.
    bool send_chunk(Container chunk,
                     std::string_view extension = std::string_view())
    {
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
    bool send_chunk(comms::ConstBuffers buffers,
                     std::string_view extension = std::string_view())
    {
      // Calculate the overall size of the data in the buffers
      size_t size(ASIO::buffer_size(buffers));

      http::chunk_header chunk_header(size, extension);
      tx_header_ = chunk_header.to_string();
      buffers.push_front(ASIO::buffer(tx_header_));
      buffers.push_back(ASIO::buffer(http::CRLF));
      return send(std::move(buffers));
    }

    /// Send the last HTTP chunk for a response.
    /// @param extension the (optional) chunk extension.
    /// @param trailer_string the (optional) chunk trailers.
    bool last_chunk(std::string_view extension = std::string_view(),
                     std::string_view trailer_string = std::string_view())
    {
      http::last_chunk last_chunk(extension, trailer_string);
      tx_header_ = last_chunk.to_string();

      return send(comms::ConstBuffers(1, ASIO::buffer(tx_header_)));
    }

    ////////////////////////////////////////////////////////////////////////
    // other functions

    /// Disconnect the underlying connection.
    void disconnect()
    { connection_.lock()->disconnect(); }

    /// Close the underlying connection.
    void close()
    { connection_.lock()->close(); }

    /// Accessor function for the comms connection.
    /// @return a weak pointer to the connection
    typename connection_type::weak_pointer connection() noexcept
    { return connection_; }
  };

}
#endif
