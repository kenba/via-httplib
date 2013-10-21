#pragma once

#ifndef HTTP_CONNECTION_HPP_VIA_HTTPLIB_
#define HTTP_CONNECTION_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file http_connection.hpp
/// @brief Just contains the http_connection template class as used by the
/// http_server template class.
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
  ////////////////////////////////////////////////////////////////////////////
  template <typename SocketAdaptor, typename Container = std::vector<char> >
  class http_connection : public boost::enable_shared_from_this
          <http_connection<SocketAdaptor, Container> >
  {
  public:
    /// The underlying connection, TCP or SSL.
    typedef comms::connection<SocketAdaptor, Container> connection_type;

    /// A weak pointer to this type.
    typedef typename boost::weak_ptr<http_connection<SocketAdaptor, Container> >
       weak_pointer;

    /// A strong pointer to this type.
    typedef typename boost::shared_ptr<http_connection<SocketAdaptor, Container> >
       shared_pointer;

    /// The template requires a typename to access the iterator.
    typedef typename Container::const_iterator Container_const_iterator;

  private:

    /// A weak pointer to underlying connection.
    boost::weak_ptr<connection_type> connection_;

    /// The request receiver for this connection.
    http::request_receiver<Container> rx_;

    /// A flag to indicate that the server should always respond to an
    /// expect: 100-continue header with a 100 Continue response.
    bool continue_enabled_;

    /// A flag to indicate that the server has a clock.
    bool has_clock_;

    /// Constructor.
    /// Note: the constructor is private to ensure that an http_connection
    /// can only be created as a shared pointer by the create method.
    /// @param connection a weak pointer to the underlying connection.
    /// @param continue_enabled if true the server shall always immediately
    /// respond to an HTTP1.1 request containing an Expect: 100-continue
    /// header with a 100 Continue response.
    /// @param has_clock if true the server shall always send a date header
    /// in the response.
    http_connection(boost::weak_ptr<connection_type> connection,
                    bool translate_head,
                    bool continue_enabled,
                    bool has_clock) :
      connection_(connection),
      rx_(translate_head),
      continue_enabled_(continue_enabled),
      has_clock_(has_clock)
    {}

    /// Send a packet on the connection.
    /// @param packet the data packet to send.
    /// @param is_continue whether this is a 100 Continue response
    bool send(Container const& packet, bool is_continue)
    {
      bool keep_alive(rx_.request().keep_alive());
      if (is_continue)
        rx_.set_continue_sent();
      else
        rx_.clear();

      boost::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (tcp_pointer)
      {
        tcp_pointer->send_data(packet);

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

#if defined(BOOST_ASIO_HAS_MOVE)
    /// Send a packet on the connection.
    /// @param packet the data packet to send.
    /// @param is_continue whether this is a 100 Continue response
    bool send(Container&& packet, bool is_continue)
    {
      bool keep_alive(rx_.request().keep_alive());
      if (is_continue)
        rx_.set_continue_sent();
      else
        rx_.clear();

      boost::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (tcp_pointer)
      {
        tcp_pointer->send_data(packet);

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
#endif

  public:

    /// Create.
    /// A factory method to create a shared pointer to this type.
    /// @param connection a weak pointer to the underlying connection.
    /// @param translate_head if true the server passes a HEAD request to the
    /// application as a GET request.
    /// @param continue_enabled if true the server shall always immediately
    /// respond to an HTTP1.1 request containing an Expect: 100-continue
    /// header with a 100 Continue response.
    /// @param has_clock if true the server shall always send a date header
    /// in the response.
    static shared_pointer create(boost::weak_ptr<connection_type> connection,
                                 bool translate_head,
                                 bool continue_enabled,
                                 bool has_clock)
    { return shared_pointer(new http_connection(connection,
                                                translate_head,
                                                continue_enabled,
                                                has_clock)); }

    /// Accessor for the HTTP request header.
    /// @return a constant reference to an rx_request.
    http::rx_request const& request() const
    { return rx_.request(); }

    /// Accessor for the received HTTP chunk.
    /// @return a constant reference to an rx_chunk.
    http::rx_chunk const& chunk() const
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
    http::receiver_parsing_state receive()
    {
      // attempt to get the pointer
      boost::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (!tcp_pointer)
        return http::RX_INCOMPLETE;

      // read the data
      Container const& data(tcp_pointer->rx_buffer());
      http::receiver_parsing_state rx_state
          (rx_.receive(data.begin(), data.end()));

      // Determine whether the server should send a 100 Continue response
      if (continue_enabled_ && (rx_state == http::RX_EXPECT_CONTINUE))
      {
#if defined(BOOST_ASIO_HAS_MOVE)
        send(http::tx_response(http::response_status::CONTINUE));
#else
        http::tx_response continue_response(http::response_status::CONTINUE);
        send(continue_response);
#endif // BOOST_ASIO_HAS_MOVE
        rx_state = http::RX_INCOMPLETE;
      }
      // Determine whether this is a TRACE request,
      // in which case the server reflects the message back.
      else if ((rx_state == http::RX_VALID) && rx_.request().is_trace())
      {
        // Response is OK with a  Content-Type: message/http header
        http::tx_response ok_response(http::response_status::OK);
        ok_response.add_content_http_header();

        // The body of the response contains the TRACE request
        std::string trace_request(rx_.request().to_string());
        trace_request += rx_.request().header().to_string();

        send(ok_response, trace_request.begin(), trace_request.end());
        rx_state = http::RX_INVALID;
      }

      return rx_state;
    }

    /// Send an HTTP response without a body.
    /// @param response the response to send.
    bool send(http::tx_response& response)
    {
      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      std::string http_header(response.message(has_clock_));

      Container tx_message(http_header.begin(), http_header.end());
      return send(tx_message, response.is_continue());
    }

#if defined(BOOST_ASIO_HAS_MOVE)
    /// Send an HTTP response without a body.
    /// @param response the response to send.
    bool send(http::tx_response&& response)
    {
      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      std::string http_header(response.message(has_clock_));

      Container tx_message(http_header.begin(), http_header.end());
      return send(tx_message, response.is_continue());
    }
#endif // BOOST_ASIO_HAS_MOVE

    /// Send an HTTP response with a body.
    /// @param response the response to send.
    /// @param body the body to send
    bool send(http::tx_response& response, Container const& body)
    {
      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      std::string http_header(response.message(has_clock_));

      Container tx_message(body);

      // Don't send a body in response to a HEAD request
      if (rx_.is_head())
        tx_message.clear();

      tx_message.insert(tx_message.begin(),
                        http_header.begin(), http_header.end());
      return send(tx_message, response.is_continue());
    }

#if defined(BOOST_ASIO_HAS_MOVE)
    /// Send an HTTP response with a body.
    /// @param response the response to send.
    /// @param body the body to send
    bool send(http::tx_response&& response, Container&& body)
    {
      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      std::string http_header(response.message(has_clock_));

      // Don't send a body in response to a HEAD request
      if (rx_.is_head())
        body.clear();

      body.insert(body.begin(),
                  http_header.begin(), http_header.end());
      return send(body, response.is_continue());
    }
#endif // BOOST_ASIO_HAS_MOVE

    /// Send an HTTP response with a body.
    /// @param response the response to send.
    /// @param begin a constant iterator to the beginning of the body to send.
    /// @param end a constant iterator to the end of the body to send.
    template<typename ForwardIterator1, typename ForwardIterator2>
    bool send(http::tx_response& response,
              ForwardIterator1 begin, ForwardIterator2 end)
    {
      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      std::string http_header(response.message(has_clock_));

      size_t size(end - begin);
      Container tx_message;
      tx_message.reserve(http_header.size() + size);
      tx_message.assign(http_header.begin(), http_header.end());

      // Don't send a body in response to a HEAD request
      if (!rx_.is_head())
        tx_message.insert(tx_message.end(), begin, end);
      return send(tx_message, response.is_continue());
    }

    /// Send an HTTP body chunk.
    /// @param chunk the body chunk to send
    /// @param extension the (optional) chunk extension.
    bool send_chunk(Container const& chunk, std::string extension = "")
    {
      size_t size(chunk.size());
      http::chunk_header chunk_header(size, extension);
      std::string chunk_string(chunk_header.to_string());

      Container tx_message(chunk);
      tx_message.insert(tx_message.begin(),
                        chunk_string.begin(), chunk_string.end());
      return send(tx_message);
    }

#if defined(BOOST_ASIO_HAS_MOVE)
    /// Send an HTTP body chunk.
    /// @param chunk the body chunk to send
    /// @param extension the (optional) chunk extension.
    bool send_chunk(Container&& chunk, std::string extension = "")
    {
      size_t size(chunk.size());
      http::chunk_header chunk_header(size, extension);
      std::string chunk_string(chunk_header.to_string());

      chunk.insert(chunk.begin(),
                   chunk_string.begin(), chunk_string.end());
      return send(chunk);
    }
#endif // BOOST_ASIO_HAS_MOVE

    /// Send an HTTP body chunk.
    /// @param begin a constant iterator to the beginning of the body to send.
    /// @param end a constant iterator to the end of the body to send.
    /// @param extension the (optional) chunk extension.
    template<typename ForwardIterator1, typename ForwardIterator2>
    bool send_chunk(ForwardIterator1 begin, ForwardIterator2 end,
                    std::string extension = "")
    {
      size_t size(end - begin);
      http::chunk_header chunk_header(size, extension);
      std::string chunk_string(chunk_header.to_string());

      Container tx_message;
      tx_message.reserve(chunk_string.size() + size);
      tx_message.assign(chunk_string.begin(), chunk_string.end());
      tx_message.insert(tx_message.end(), begin, end);
      return send(tx_message);
    }

    /// Send the last HTTP chunk for a response.
    /// @param extension the (optional) chunk extension.
    /// @param trailer_string the (optional) chunk trailers.
    bool last_chunk(std::string extension = "",
                    std::string trailer_string = "")
    {
      http::last_chunk last_chunk(extension, trailer_string);
      std::string chunk_string(last_chunk.message());

      Container tx_message(chunk_string.begin(), chunk_string.end());
      return send(tx_message);
    }

    /// Disconnect the underlying connection.
    void disconnect()
    { connection_.lock()->disconnect(); }
  };

}
#endif
