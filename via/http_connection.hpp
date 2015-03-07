#ifndef HTTP_CONNECTION_HPP_VIA_HTTPLIB_
#define HTTP_CONNECTION_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
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
#include <memory>
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
  /// @param SocketAdaptor the type of socket, use: tcp_adaptor or
  /// ssl::ssl_tcp_adaptor
  /// @param Container the container to use for the tx buffer, default
  /// std::vector<char>.
  /// It must contain a contiguous array of bytes. E.g. std::string or
  /// std::array<char, size>
  /// @param buffer_size the size of the receive buffer, default 8192 bytes.
  /// @param use_strand if true use an asio::strand to wrap the handlers,
  /// default false.
  ////////////////////////////////////////////////////////////////////////////
  template <typename SocketAdaptor, typename Container, size_t buffer_size, bool use_strand>
  class http_connection : public std::enable_shared_from_this
       <http_connection<SocketAdaptor, Container, buffer_size, use_strand>>
  {
  public:
    /// The underlying connection, TCP or SSL.
    using connection_type =
      comms::connection<SocketAdaptor, Container, buffer_size, use_strand>;

    /// A weak pointer to this type.
    using weak_pointer = typename
      std::weak_ptr<http_connection<SocketAdaptor, Container, buffer_size, use_strand>>;

    /// A strong pointer to this type.
    using shared_pointer = typename
      std::shared_ptr<http_connection<SocketAdaptor, Container, buffer_size, use_strand>>;

    /// The template requires a typename to access the iterator.
    using Container_const_iterator = typename Container::const_iterator;

  private:

    /// A weak pointer to underlying connection.
    typename connection_type::weak_pointer connection_;

    /// The request receiver for this connection.
    http::request_receiver<Container> rx_;

    /// The HTTP header of the message.
    std::string http_header_;

    /// The receive buffer for this connection.
    Container rx_buffer_;

    Container_const_iterator rx_iter_;

    /// The transmit buffer for this connection.
    Container tx_buffer_;

    /// Constructor.
    /// Note: the constructor is private to ensure that an http_connection
    /// can only be created as a shared pointer by the create method.
    /// @param connection a weak pointer to the underlying connection.
    /// @param concatenate_chunks if true the server shall always concatenate
    /// chunk data into the request body, otherwise the body shall contain
    /// the data for each chunk.
    /// @param translate_head if true the server shall always pass a HEAD request
    /// to the application as a GET request.
    /// @param require_host if true the server shall require all requests to
    /// include a "Host:" header field. Required by RFC2616.
    http_connection(typename connection_type::weak_pointer connection,
                    bool concatenate_chunks,
                    bool translate_head,
                    bool require_host) :
      connection_{connection},
      rx_{concatenate_chunks, translate_head, require_host},
      http_header_{},
      rx_buffer_{},
      rx_iter_{rx_buffer_.cend()},
      tx_buffer_{}
    {}

    /// Send buffers on the connection.
    /// @param buffers the data to write.
    bool send(comms::ConstBuffers buffers)
    {
      auto tcp_pointer(connection_.lock());
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
      bool keep_alive{rx_.request().keep_alive()};
      if (is_continue)
        rx_.set_continue_sent();
      else
        rx_.clear();

      auto tcp_pointer(connection_.lock());
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

  public:

    /// Create.
    /// A factory method to create a shared pointer to this type.
    /// @param connection a weak pointer to the underlying connection.
    /// @param concatenate_chunks if true the server shall always concatenate
    /// chunk data into the request body, otherwise the body shall contain
    /// the data for each chunk.
    /// @param translate_head if true the server shall always pass a HEAD request
    /// to the application as a GET request.
    /// @param require_host if true the server shall require all requests to
    /// include a "Host:" header field. Required by RFC2616.
    static shared_pointer create(typename connection_type::weak_pointer connection,
                                 bool concatenate_chunks,
                                 bool translate_head,
                                 bool require_host)
    { return shared_pointer(new http_connection(connection,
                                                concatenate_chunks,
                                                translate_head,
                                                require_host)); }

    /// Copy constructor deleted.
    http_connection(http_connection const&)=delete;

    /// Assignment operator deleted.
    http_connection& operator=(http_connection const&)=delete;

    /// Accessor for the HTTP request header.
    /// @return a constant reference to an rx_request.
    http::rx_request const& request() const
    { return rx_.request(); }

    /// Accessor for the received HTTP chunk.
    /// @return a constant reference to an rx_chunk.
    http::rx_chunk<Container> const& chunk() const
    { return rx_.chunk(); }

    /// Accessor for the body.
    /// @return a constant reference to the body.
    Container const& body() const
    { return rx_.body(); }

    /// Accessor for the beginning of the body.
    /// @return a constant iterator to the beginning of the body.
    Container_const_iterator body_begin() const
    { return rx_.body().begin(); }

    /// Accessor for the end of the body.
    /// @return a constant iterator to the end of the body.
    Container_const_iterator body_end() const
    { return rx_.body().end(); }

    /// Accessor for the received data buffer.
    Container const& rx_buffer()
    { return rx_buffer_; }

    /// Receive data on the underlying connection.
    /// @return an iterator to the start of the received data buffer.
    Container_const_iterator receive()
    {
      // attempt to get the pointer
      auto tcp_pointer(connection_.lock());
      if (tcp_pointer)
      {
        // read the data
        tcp_pointer->rx_buffer(rx_buffer_);
        rx_iter_ = rx_buffer_.cbegin();
      }
      else
        rx_iter_ = rx_buffer_.cend();

      return rx_iter_;
    }

    /// Parse the data in the received buffer
    /// @retval an iterator to the current position in the received data buffer.
    http::Rx parse_rx_buffer(Container_const_iterator& iter)
    { return rx_.receive(iter, rx_buffer_.cend()); }

    /// Send the appropriate HTTP response to the request.
    bool send_response()
    {
      http::tx_response response(rx_.response_code());
      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      http_header_ = response.message();

      return send(comms::ConstBuffers{boost::asio::buffer(http_header_)},
                  response.is_continue());
    }

    /// Send an HTTP response without a body.
    /// @param response the response to send.
    bool send(http::tx_response response)
    {
      if (!response.is_valid())
        return false;

      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      http_header_ = response.message();

      return send(comms::ConstBuffers{boost::asio::buffer(http_header_)},
                  response.is_continue());
    }

    /// Send an HTTP response with a body.
    /// @param response the response to send.
    /// @param body the body to send
    bool send(http::tx_response response, Container body)
    {
      if (!response.is_valid())
        return false;

      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      http_header_ = response.message(body.size());

      comms::ConstBuffers buffers{boost::asio::buffer(http_header_)};

      // Don't send a body in response to a HEAD request
      if (!rx_.is_head())
      {
        tx_buffer_.swap(body);
        buffers.push_back(boost::asio::buffer(tx_buffer_));
      }

      return send(std::move(buffers), response.is_continue());
    }

    /// Send an HTTP response with a body.
    /// @param response the response to send.
    /// @param begin a constant iterator to the beginning of the body to send.
    /// @param end a constant iterator to the end of the body to send.
    template<typename ForwardIterator>
    bool send(http::tx_response response,
              ForwardIterator begin, ForwardIterator end)
    {
      if (!response.is_valid())
        return false;

      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      size_t size(end - begin);
      http_header_ = response.message(size);

      comms::ConstBuffers buffers{boost::asio::buffer(http_header_)};

      if (!rx_.is_head())
      {
        tx_buffer_.assign(begin, end);
        buffers.push_back(boost::asio::buffer(tx_buffer_));
      }

      return send(std::move(buffers), response.is_continue());
    }

    /// Send an HTTP response with a body.
    /// NOTE: The body is NOT buffered.
    /// @param response the response to send.
    /// @param buffers a deque of asio::buffers containing the body to send.
    bool send(http::tx_response response, comms::ConstBuffers buffers)
    {
      if (!response.is_valid())
        return false;

      // Calculate the overall size of the data in the buffers
      size_t size(boost::asio::buffer_size(buffers));

      // Don't send a body in response to a HEAD request
      if (rx_.is_head())
        buffers.clear();

      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      http_header_ = response.message(size);
      buffers.push_front(boost::asio::buffer(http_header_));

      return send(std::move(buffers), response.is_continue());
    }

    /// Send an HTTP body chunk.
    /// @param chunk the body chunk to send
    /// @param extension the (optional) chunk extension.
    bool send_chunk(Container chunk, std::string extension = "")
    {
      size_t size{chunk.size()};
      http::chunk_header chunk_header{size, extension};
      http_header_ = chunk_header.to_string();
      tx_buffer_.swap(chunk);

      comms::ConstBuffers buffers{boost::asio::buffer(http_header_)};
      buffers.push_back(boost::asio::buffer(tx_buffer_));
      buffers.push_back(boost::asio::buffer(http::CRLF));
      return send(std::move(buffers));
    }

    /// Send an HTTP body chunk.
    /// @param begin a constant iterator to the beginning of the body to send.
    /// @param end a constant iterator to the end of the body to send.
    /// @param extension the (optional) chunk extension.
    template<typename ForwardIterator>
    bool send_chunk(ForwardIterator begin, ForwardIterator end,
                    std::string extension = "")
    {
      size_t size(end - begin);
      http::chunk_header chunk_header{size, extension};
      http_header_ = chunk_header.to_string();
      tx_buffer_.assign(begin, end);

      comms::ConstBuffers buffers{boost::asio::buffer(http_header_)};
      buffers.push_back(boost::asio::buffer(tx_buffer_));
      buffers.push_back(boost::asio::buffer(http::CRLF));
      return send(std::move(buffers));
    }

    /// Send an HTTP body chunk.
    /// NOTE: The body chunk is NOT buffered.
    /// @param chunk a pointer to the body chunk to send
    /// @param buffers a deque of asio::buffers containing the body to send.
    bool send_chunk(comms::ConstBuffers buffers, std::string extension = "")
    {
      // Get the overall size of the data in the buffers
      size_t size(boost::asio::buffer_size(buffers));

      http::chunk_header chunk_header{size, extension};
      http_header_ = chunk_header.to_string();
      buffers.push_front(boost::asio::buffer(http_header_));
      buffers.push_back(boost::asio::buffer(http::CRLF));

      return send(std::move(buffers));
    }

    /// Send the last HTTP chunk for a response.
    /// @param extension the (optional) chunk extension.
    /// @param trailer_string the (optional) chunk trailers.
    bool last_chunk(std::string extension = "",
                    std::string trailer_string = "")
    {
      http::last_chunk last_chunk{extension, trailer_string};
      http_header_ = last_chunk.message();

      return send(comms::ConstBuffers{boost::asio::buffer(http_header_)});
    }

    /// @fn remote_address
    /// Get the remote address of the connection.
    /// @return the remote address of the connection.
    std::string remote_address() const
    {
      auto tcp_pointer(connection_.lock());
      if (tcp_pointer)
        return tcp_pointer->remote_endpoint().address().to_string();
      else
        return std::string("");
    }

    /// Disconnect the underlying connection.
    void disconnect()
    { connection_.lock()->disconnect(); }
  };

}
#endif
