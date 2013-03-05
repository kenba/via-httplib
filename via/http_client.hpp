#pragma once

#ifndef HTTP_CLIENT_HPP_VIA_HTTPLIB_
#define HTTP_CLIENT_HPP_VIA_HTTPLIB_
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
  /// @class http_client
  /// An HTTP client.
  ////////////////////////////////////////////////////////////////////////////
  template <typename SocketAdaptor, typename Container = std::vector<char> >
  class http_client : public boost::enable_shared_from_this
          <http_client<SocketAdaptor, Container> >
  {
  public:
    /// The underlying connection, TCP or SSL.
    typedef comms::connection<SocketAdaptor, Container> connection_type;

    /// A weak pointer to this type.
    typedef typename boost::weak_ptr<http_client<SocketAdaptor, Container> >
       weak_pointer;

    /// The template requires a typename to access the iterator.
    typedef typename Container::const_iterator Container_const_iterator;

    /// The signal sent when a response is received.
    typedef boost::signal<void (const weak_pointer,
                                http::rx_response const&,
                                Container_const_iterator,
                                Container_const_iterator)> http_response_signal;

    /// The slot type associated with a response received signal.
    typedef typename http_response_signal::slot_type http_response_signal_slot;

    /// The signal sent when a chunk is received.
    typedef boost::signal<void (const weak_pointer,
                                http::chunk_header const&,
                                Container_const_iterator,
                                Container_const_iterator)> http_chunk_signal;

    /// The slot type associated with a chunk received signal.
    typedef typename http_chunk_signal::slot_type http_chunk_signal_slot;

  private:

    boost::shared_ptr<connection_type> connection_; ///< the comms connection
    http::response_receiver<Container> rx_;       ///< the response receiver
    http_response_signal http_response_signal_;   ///< the response callback function
    http_chunk_signal http_chunk_signal_;         ///< the response chunk callback function

    /// Send a packet on the connection.
    /// @param packet the data packet to send.
    bool send(Container const& packet)
    {
      rx_.clear();

      bool keep_alive(true);
      connection_->send_data(packet);

      if (keep_alive)
        return connection_->read_pending();
      else
        connection_->disconnect();

      return false;
    }

  public:

    /// Connect the response received slot.
    /// @param slot the slot for the response received signal.
    void response_received_event(http_response_signal_slot const& slot)
    { http_response_signal_.connect(slot); }

    /// Connect the chunk received slot.
    /// @param slot the slot for the chunk received signal.
    void chunk_received_event(http_chunk_signal_slot const& slot)
    { http_chunk_signal_.connect(slot); }

    /// Constructor.
    /// @param connection a weak pointer to the underlying connection.
    explicit http_client(boost::asio::io_service& io_service,
                         unsigned short port_number) :
      connection_(connection_type::create(io_service, port_number)),
      rx_(),
      http_response_signal_(),
      http_chunk_signal_()
    {}

    /// Connect to the given host name and port.
    /// @param host_name the host to connect to.
    /// @param port_name the port to connect to.
    /// @return true if resolved, false otherwise.
    bool connect(const char *host_name, const char *port_name)
    { return connection_->connect(host_name, port_name); }

    /// Accessor for the HTTP response header.
    /// @return a constant reference to an rx_response.
    http::rx_response const& response() const
    { return rx_.response(); }

    /// Accessor for the beginning of the body.
    /// @return a constant iterator to the beginning of the body.
    Container_const_iterator body_begin() const
    { return rx_.body().begin(); }

    /// Accessor for the end of the body.
    /// @return a constant iterator to the end of the body.
    Container_const_iterator body_end() const
    { return rx_.body().end(); }

    /// Receive data on the underlying connection.
    bool receive()
    {
      // attempt to get the pointer
      boost::shared_ptr<connection_type> tcp_pointer(connection_.lock());
      if (!tcp_pointer)
        return false;

      // attempt to read the data
      while (tcp_pointer->read_pending())
      {
        Container data(tcp_pointer->read_data());
        http::receiver_parsing_state rx_state
            (rx_.receive(data.begin(), data.end()));

        switch (rx_state)
        {
        case http::RX_VALID:
          http_response_signal_(connection_,
                                rx_.response(),
                                rx_.body().begin(),
                                rx_.body().end());
          return true;

        case http::RX_CHUNK:
          http_chunk_signal_(connection_,
                             rx_.chunk(),
                             rx_.body().begin(),
                             rx_.body().end());
          return true;

        case http::RX_INVALID:
          break;

        default:
          break;
        }
      }

      return false;
    }

    /// Send an HTTP request without a body.
    /// @param request the request to send.
    bool send(http::tx_request& request)
    {
      std::string http_header(request.message());
      Container tx_message(http_header.begin(), http_header.end());
      return send(tx_message);
    }

    /// Send an HTTP request with a body.
    /// @param request the request to send.
    /// @param begin a constant iterator to the beginning of the body to send.
    /// @param end a constant iterator to the end of the body to send.
    template<typename ForwardIterator1, typename ForwardIterator2>
    bool send(http::tx_request& request,
              ForwardIterator1 begin, ForwardIterator2 end)
    {
      std::string http_header(request.message());

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
