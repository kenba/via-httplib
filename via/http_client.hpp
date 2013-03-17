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
#include <boost/signal.hpp>

#include <deque>
#include <iostream>
#include <boost/bind.hpp>
namespace via
{
  ////////////////////////////////////////////////////////////////////////////
  /// @class http_client
  /// An HTTP client.
  ////////////////////////////////////////////////////////////////////////////
  template <typename SocketAdaptor, typename Container = std::vector<char> >
  class http_client
  {
  public:
    /// The underlying connection, TCP or SSL.
    typedef comms::connection<SocketAdaptor, Container> connection_type;

    /// A shared pointer to this type.
    typedef typename boost::shared_ptr<http_client<SocketAdaptor, Container> >
       shared_pointer;

    /// The template requires a typename to access the iterator.
    typedef typename Container::const_iterator Container_const_iterator;

    /// The signal sent when a response is received.
    typedef boost::signal<void (http::rx_response const&,
                                Container const&)> http_response_signal;

    /// The slot type associated with a response received signal.
    typedef typename http_response_signal::slot_type http_response_signal_slot;

    /// The signal sent when a chunk is received.
    typedef boost::signal<void (http::rx_chunk const&,
                                Container const&)> http_chunk_signal;

    /// The slot type associated with a chunk received signal.
    typedef typename http_chunk_signal::slot_type http_chunk_signal_slot;

    /// The signal sent when a socket is disconnected.
    typedef boost::signal<void (void)> http_disconnected_signal;

    /// The slot type associated with a disconnected signal.
    typedef typename http_disconnected_signal::slot_type
                                                http_disconnected_signal_slot;

  private:

    boost::shared_ptr<connection_type> connection_; ///< the comms connection
    http::response_receiver<Container> rx_;       ///< the response receiver
    http_response_signal http_response_signal_;   ///< the response callback function
    http_chunk_signal http_chunk_signal_;         ///< the response chunk callback function
    http_disconnected_signal http_disconnected_signal_;
    std::string host_name_;                       ///< the name of the host

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

    /// Constructor.
    /// @param io_service the asio io_service to use.
    explicit http_client(boost::asio::io_service& io_service) :
      connection_(connection_type::create(io_service)),
      rx_(),
      http_response_signal_(),
      http_chunk_signal_(),
      http_disconnected_signal_(),
      host_name_()
    {
      connection_->get_event_signal
          (boost::bind(&http_client::event_handler, this, _1, _2));
      connection_->get_error_signal
          (boost::bind(&http_client::error_handler, this, _1, _2));
    }

  public:

    /// @fn create
    /// The factory function to create connections.
    /// @param io_service the boost asio io_service used by the underlying
    /// connection.
    static shared_pointer create(boost::asio::io_service& io_service)
    { return shared_pointer(new http_client(io_service)); }

    /// Connect the response received slot.
    /// @param slot the slot for the response received signal.
    void response_received_event(http_response_signal_slot const& slot)
    { http_response_signal_.connect(slot); }

    /// Connect the chunk received slot.
    /// @param slot the slot for the chunk received signal.
    void chunk_received_event(http_chunk_signal_slot const& slot)
    { http_chunk_signal_.connect(slot); }

    /// Connect the disconnected slot.
    /// @param slot the slot for the disconnected signal.
    void disconnected_event(http_disconnected_signal_slot const& slot)
    { http_disconnected_signal_.connect(slot); }

    /// Connect to the given host name and port.
    /// @param host_name the host to connect to.
    /// @param port_name the port to connect to.
    /// @return true if resolved, false otherwise.
    bool connect(const std::string& host_name, std::string port_name = "http")
    {
      host_name_ = host_name;
      if (port_name != "http")
      {
        host_name_ += ":";
        host_name_ += port_name;
      }

      return connection_->connect(host_name.c_str(), port_name.c_str());
    }

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
    void receive_handler()
    {
      // attempt to read the data
      while (connection_->read_pending())
      {
        Container data(connection_->read_data());
        http::receiver_parsing_state rx_state
            (rx_.receive(data.begin(), data.end()));

        switch (rx_state)
        {
        case http::RX_VALID:
          http_response_signal_(rx_.response(),
                                rx_.body());
          return;

        case http::RX_CHUNK:
          http_chunk_signal_(rx_.chunk(),
                             rx_.body());
          return;

        case http::RX_INVALID:
          break;

        default:
          break;
        }
      }

      return;
    }

    /// Send an HTTP request without a body.
    /// @param request the request to send.
    bool send(http::tx_request& request)
    {
      request.add_header(http::header_field::HOST, host_name_);
      std::string http_header(request.message());
      Container tx_message(http_header.begin(), http_header.end());
      return send(tx_message);
    }

    /// Send an HTTP request with a body.
    /// @param request the request to send.
    /// @param body the body to send
    bool send(http::tx_request& request, Container const& body)
    {
      request.add_header(http::header_field::HOST, host_name_);
      std::string http_header(request.message());

      Container tx_message(body);
      tx_message.insert(tx_message.begin(),
                        http_header.begin(), http_header.end());
      return send(tx_message);
    }

#if defined(BOOST_ASIO_HAS_MOVE)
    /// Send an HTTP request with a body.
    /// @param request the request to send.
    /// @param body the body to send
    bool send(http::tx_request& request, Container&& body)
    {
      request.add_header(http::header_field::HOST, host_name_);
      std::string http_header(request.message());

      tx_message.insert(body.begin(),
                        http_header.begin(), http_header.end());
      return send(body);
    }
#endif // BOOST_ASIO_HAS_MOVE

    /// Send an HTTP request with a body.
    /// @param request the request to send.
    /// @param begin a constant iterator to the beginning of the body to send.
    /// @param end a constant iterator to the end of the body to send.
    template<typename ForwardIterator1, typename ForwardIterator2>
    bool send(http::tx_request& request,
              ForwardIterator1 begin, ForwardIterator2 end)
    {
      request.add_header(http::header_field::HOST, host_name_);
      std::string http_header(request.message());

      size_t size(end - begin);
      Container tx_message;
      tx_message.reserve(http_header.size() + size);
      tx_message.assign(http_header.begin(), http_header.end());
      tx_message.insert(tx_message.end(), begin, end);
      return send(tx_message);
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

      tx_message.insert(chunk.begin(),
                        chunk_string.begin(), chunk_string.end());
      return send(chunk);
    }
#endif // BOOST_ASIO_HAS_MOVE

    /// Send an HTTP body chunk.
    /// @param begin a constant iterator to the beginning of the chunk to send.
    /// @param end a constant iterator to the end of the chunk to send.
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

    /// Receive an event from the underlying comms connection.
    /// @param connection a weak ponter to the underlying comms connection.
    void event_handler(int event, boost::weak_ptr<connection_type> connection)
    {
      switch(event)
      {
      case via::comms::RECEIVED:
        receive_handler();
        break;
      case via::comms::DISCONNECTED:
        http_disconnected_signal_();
        break;
      default:
        break;
      }
    }

    /// Receive an error from the underlying comms connection.
    /// @param connection a weak ponter to the underlying comms connection.
    void error_handler(const boost::system::error_code &error,
                       boost::weak_ptr<connection_type> connection)
    {
      std::cerr << "error_handler" << std::endl;
      std::cerr << error <<  std::endl;
    }
  };
}

#endif
