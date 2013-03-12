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
#include "via/http/request.hpp"
#include "via/http/response.hpp"
#include "via/comms/connection.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>
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

    /// Constructor.
    /// Note: the constructor is private to ensure that an http_connection
    /// can only be created as a shared pointer by the create method.
    /// @param connection a weak pointer to the underlying connection.
    http_connection(boost::weak_ptr<connection_type> connection) :
      connection_(connection),
      rx_()
    {}

    /// Send a packet on the connection.
    /// @param packet the data packet to send.
    bool send(Container const& packet)
    {
      bool keep_alive(rx_.request().keep_alive());
      rx_.clear();

      boost::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (tcp_pointer)
      {
        tcp_pointer->send_data(packet);

        if (keep_alive)
          return tcp_pointer->read_pending();
        else
          tcp_pointer->disconnect();

        return false;
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
    static shared_pointer create(boost::weak_ptr<connection_type> connection)
    { return shared_pointer(new http_connection(connection)); }

    /// Accessor for the HTTP request header.
    /// @return a constant reference to an rx_request.
    http::rx_request const& request() const
    { return rx_.request(); }

    /// Accessor for the HTTP chunk header.
    /// @return a constant reference to a chunk_header.
    http::chunk_header const& chunk() const
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

      // attempt to read the data
      while (tcp_pointer->read_pending())
      {
        Container data(tcp_pointer->read_data());
        http::receiver_parsing_state rx_state
            (rx_.receive(data.begin(), data.end()));

        if (rx_state != http::RX_INCOMPLETE)
          return rx_state;
      }

      return http::RX_INCOMPLETE;
    }

    /// Send an HTTP response without a body.
    /// @param response the response to send.
    bool send(http::tx_response& response)
    {
      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      std::string http_header(response.message());
      Container tx_message(http_header.begin(), http_header.end());
      return send(tx_message);
    }

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
      std::string http_header(response.message());

      size_t size(end - begin);
      Container tx_message;
      tx_message.reserve(http_header.size() + size);
      tx_message.assign(http_header.begin(), http_header.end());
      tx_message.insert(tx_message.end(), begin, end);
      return send(tx_message);
    }

    /// Disconnect the underlying connection.
    void disconnect()
    { connection_.lock()->disconnect(); }
  };

}
#endif
