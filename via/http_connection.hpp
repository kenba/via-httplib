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
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
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
  /// @param use_strand if true use an asio::strand to wrap the handlers,
  /// default false.
  ////////////////////////////////////////////////////////////////////////////
  template <typename SocketAdaptor, typename Container, bool use_strand>
  class http_connection : public boost::enable_shared_from_this
                      <http_connection<SocketAdaptor, Container, use_strand> >
  {
  public:
    /// The underlying connection, TCP or SSL.
    typedef comms::connection<SocketAdaptor, Container, use_strand>
                                                              connection_type;

    /// A weak pointer to this type.
    typedef typename boost::weak_ptr<http_connection<SocketAdaptor, Container,
                                     use_strand> > weak_pointer;

    /// A strong pointer to this type.
    typedef typename boost::shared_ptr<http_connection<SocketAdaptor, Container,
                                       use_strand> > shared_pointer;

    /// The template requires a typename to access the iterator.
    typedef typename Container::const_iterator Container_const_iterator;

  private:

    /// A weak pointer to underlying connection.
    typename connection_type::weak_pointer connection_;

    /// The request receiver for this connection.
    http::request_receiver<Container> rx_;

    /// A buffer for the HTTP header of the response message.
    std::string tx_header_;

    /// A buffer for the body of the response message.
    Container tx_body_;

    /// A flag to indicate that the server should always respond to an
    /// expect: 100-continue header with a 100 Continue response.
    bool continue_enabled_;

    /// A flag to indicate that the server will echo back the TRACE message.
    bool trace_enabled_;

    /// Send buffers on the connection.
    /// @param buffers the data to write.
    bool send(comms::ConstBuffers const& buffers)
    {
      boost::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (tcp_pointer)
      {
        tcp_pointer->send_data(buffers);
        return true;
      }
      else
        return false;
    }

    /// Send buffers on the connection.
    /// @param buffers the data to write.
    /// @param is_continue whether this is a 100 Continue response
    bool send(comms::ConstBuffers const& buffers, bool is_continue)
    {
      bool keep_alive(rx_.request().keep_alive());
      if (is_continue)
        rx_.set_continue_sent();
      else
        rx_.clear();

      boost::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (tcp_pointer)
      {
        tcp_pointer->send_data(buffers);

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

    /// Constructor.
    /// Note: only a shared pointer to this type should be created.
    /// @param connection a weak pointer to the underlying connection.
    /// @param strict_crlf enforce strict parsing of CRLF.
    /// @param max_whitespace the maximum number of consectutive whitespace
    /// characters allowed in a request:min 1, max 254.
    /// @param max_method_length the maximum length of an HTTP request method:
    /// min 1, max 254.
    /// @param max_uri_length the maximum length of an HTTP request uri:
    /// min 1, max 4 billion.
    /// @param max_line_length the maximum length of an HTTP header field line:
    /// min 1, max 65534.
    /// @param max_header_number the maximum number of HTTP header field lines:
    /// max 65534.
    /// @param max_header_length the maximum cumulative length the HTTP header
    /// fields: max 4 billion.
    /// @param max_body_size the maximum size of an HTTP request body:
    /// max 4 billion.
    /// @param max_chunk_size the maximum size of an HTTP request chunk:
    /// max 4 billion.
    http_connection(typename connection_type::weak_pointer connection,
                    bool           strict_crlf,
                    unsigned char  max_whitespace,
                    unsigned char  max_method_length,
                    size_t         max_uri_length,
                    unsigned short max_line_length,
                    unsigned short max_header_number,
                    size_t         max_header_length,
                    size_t         max_body_size,
                    size_t         max_chunk_size) :
      connection_(connection),
      rx_(strict_crlf, max_whitespace, max_method_length, max_uri_length,
          max_line_length, max_header_number, max_header_length,
          max_body_size, max_chunk_size),
      tx_header_(),
      tx_body_(),
      continue_enabled_(true),
      trace_enabled_(false)
    {}

    /// Enable whether the http server requires every HTTP request to contain
    /// a Host header. Note a Host header is required by RFC2616.
    /// @post Host header verification enabled/disabled.
    /// @param enable enable the function.
    void set_require_host_header(bool enable)
    { rx_.set_require_host_header(enable); }

    /// Enable whether the http server translates HEAD requests into GET
    /// requests for the application.
    /// Note: http_server never sends a body in a response to a HEAD request.
    /// @post HEAD translation enabled/disabled.
    /// @param enable enable the function.
    void set_translate_head(bool enable)
    { rx_.set_translate_head(enable); }

    /// Enable whether the http server echos TRACE requests.
    /// The standard HTTP response to a TRACE request is to echo back the
    /// TRACE message and all of it's headers in the body of the response.
    /// However it's considered a security vulnerability nowadays, so the
    /// default behaviour is to send a 405 "Method Not Allowed" response instead.
    /// Enable whether the http server responds to TRACE requests.
    /// @param enable enable the function.
    void set_trace_enabled(bool enable)
    { trace_enabled_ = enable; }

    /// Enable whether the http server concatenates chunked requests.
    /// If a ChunkHandler is not registered with the http_server then any
    /// recieved chunks will be concatenated into the request message body.
    /// @post chunk concatenation enabled/disabled.
    /// @param enable enable the function.
    void set_concatenate_chunks(bool enable)
    { rx_.set_concatenate_chunks(enable); }

    /// Enable whether the http server always sends a 100 Continue response.
    /// to a request containing an Expects: 100-Continue header.
    /// If a request_expect_continue_event is not registered with the
    /// http_server then the server will always sends a 100 Continue response.
    /// @post auto continue response enabled/disabled.
    /// @param enable enable the function.
    void set_auto_continue(bool enable)
    { continue_enabled_ = enable; }

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

    /// Receive data on the underlying connection.
    /// @return the receiver_parsing_state
    http::Rx receive()
    {
      // A buffer to store the body used to transmit a message.
      static std::string tx_body_buffer;

      // attempt to get the pointer
      boost::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (!tcp_pointer)
        return http::RX_INCOMPLETE;

      // read the data
      Container const& data(tcp_pointer->rx_buffer());
      Container_const_iterator iter(data.begin());
      Container_const_iterator end(data.end());
      http::Rx rx_state(rx_.receive(iter, end));

      // Handle special cases
      switch (rx_state)
      {
      case http::RX_INVALID:
#if defined(BOOST_ASIO_HAS_MOVE)
        send(http::tx_response(http::response_status::code::BAD_REQUEST));
#else
      {
        http::tx_response bad_request(http::response_status::code::BAD_REQUEST);
        send(bad_request);
      }
#endif // BOOST_ASIO_HAS_MOVE
        break;

      case http::RX_LENGTH_REQUIRED:
#if defined(BOOST_ASIO_HAS_MOVE)
        send(http::tx_response(http::response_status::code::LENGTH_REQUIRED));
#else
      {
        http::tx_response length_required(http::response_status::code::LENGTH_REQUIRED);
        send(length_required);
      }
#endif // BOOST_ASIO_HAS_MOVE
        rx_state = http::RX_INVALID;
        break;

      case http::RX_EXPECT_CONTINUE:
        // Determine whether the server should send a 100 Continue response
        if (continue_enabled_)
        {
#if defined(BOOST_ASIO_HAS_MOVE)
          send(http::tx_response(http::response_status::code::CONTINUE));
#else
          http::tx_response continue_response(http::response_status::code::CONTINUE);
          send(continue_response);
#endif // BOOST_ASIO_HAS_MOVE
          rx_state = http::RX_INCOMPLETE;
        }
        break;

      case http::RX_VALID:
        // Determine whether this is a TRACE request
        if (rx_.request().is_trace())
        {
          // if enabled, the server reflects the message back.
          if (trace_enabled_)
          {
            // Response is OK with a Content-Type: message/http header
            http::tx_response ok_response(http::response_status::code::OK,
                                   http::header_field::content_http_header());

            // The body of the response contains the TRACE request
            tx_body_buffer.clear();
            tx_body_buffer = rx_.request().to_string();
            tx_body_buffer += rx_.request().headers().to_string();

            send(ok_response,
                 comms::ConstBuffers(1, boost::asio::buffer(tx_body_buffer)));
          }
          else // otherwise, it responds with "Not Allowed"
          {
#if defined(BOOST_ASIO_HAS_MOVE)
            send(http::tx_response(http::response_status::code::METHOD_NOT_ALLOWED));
#else
            http::tx_response not_allowed(http::response_status::code::METHOD_NOT_ALLOWED);
            send(not_allowed);
#endif // BOOST_ASIO_HAS_MOVE
          }

          // Set the state as invalid, since the server has responded to the request
          rx_state = http::RX_INVALID;
        }
        break;

      default:
        ;
      }

      return rx_state;
    }

    /// Send the appropriate HTTP response to the request.
    /// @return true if sent, false otherwise.
    bool send_response()
    {
      http::tx_response response(rx_.response_code());
      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      tx_header_ = response.message();

      return send(comms::ConstBuffers(1, boost::asio::buffer(tx_header_)),
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

      return send(comms::ConstBuffers(1, boost::asio::buffer(tx_header_)),
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
      tx_header_ = response.message();
      comms::ConstBuffers buffers(1, boost::asio::buffer(tx_header_));

      // Don't send a body in response to a HEAD request
      if (!rx_.is_head())
      {
        tx_body_.swap(body);
        buffers.push_back(boost::asio::buffer(tx_body_));
      }

      return send(buffers, response.is_continue());
    }

    /// Send an HTTP response with a body.
    /// @pre The contents of the buffers are NOT buffered.
    /// Their lifetime MUST exceed that of the connection
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
      tx_header_ = response.message(size);
      buffers.push_front(boost::asio::buffer(tx_header_));

      return send(buffers, response.is_continue());
    }

    /// Send an HTTP response body.
    /// @pre the response must have been sent beforehand.
    /// @param body the body to send
    /// @return true if sent, false otherwise.
    bool send_body(Container body)
    {
      if (rx_.is_head())
        return false;

      tx_body_.swap(body);
      return send(comms::ConstBuffers(1, boost::asio::buffer(tx_body_)));
    }

    /// Send an HTTP response body.
    /// @pre the response must have been sent beforehand.
    /// @pre The contents of the buffers are NOT buffered.
    /// Their lifetime MUST exceed that of the connection
    /// @param buffers the body to send
    /// @return true if sent, false otherwise.
    bool send_body(comms::ConstBuffers buffers)
    {
      if (rx_.is_head())
        return false;

      return send(buffers);
    }

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

    /// Send the last HTTP chunk for a response.
    /// @param extension the (optional) chunk extension.
    /// @param trailer_string the (optional) chunk trailers.
    bool last_chunk(std::string extension = "",
                    std::string trailer_string = "")
    {
      http::last_chunk last_chunk(extension, trailer_string);
      tx_header_ = last_chunk.message();

      return send(comms::ConstBuffers(1, boost::asio::buffer(tx_header_)));
    }

    /// @fn remote_address
    /// Get the remote address of the connection.
    /// @return the remote address of the connection.
    std::string remote_address() const
    {
      boost::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (tcp_pointer)
        return tcp_pointer->socket().remote_endpoint().address().to_string();
      else
        return std::string("");
    }

    /// Disconnect the underlying connection.
    void disconnect()
    { connection_.lock()->shutdown(); }

    /// Close the underlying connection.
    void close()
    { connection_.lock()->close(); }
  };

}
#endif
