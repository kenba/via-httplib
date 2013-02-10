#pragma once

#ifndef HTTP_SSL_CONNECTION_HPP_VIA_HTTPLIB_
#define HTTP_SSL_CONNECTION_HPP_VIA_HTTPLIB_
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
#include "via/comms/ssl_tcp_buffered_connection.hpp"
#include <boost/signal.hpp>
#include <deque>
#include <iostream>

namespace via
{
  ////////////////////////////////////////////////////////////////////////////
  /// @class http_ssl_connection
  ///
  ////////////////////////////////////////////////////////////////////////////
  template <typename Container>
  class http_ssl_connection
  {
    typedef typename Container::const_iterator Container_const_iterator;

    boost::weak_ptr<via::comms::connection> connection_;
    http::request_receiver<Container> rx_;

    /// Constructor.
    http_ssl_connection(boost::weak_ptr<via::comms::connection> connection) :
      connection_(connection),
      rx_()
    {}

    void send(Container const& packet)
    {
      rx_.clear();

      boost::shared_ptr<ssl_tcp_connection> ssl_tcp_pointer
          (boost::dynamic_pointer_cast<ssl_tcp_connection>(connection_.lock()));
      if (ssl_tcp_pointer)
        ssl_tcp_pointer->send_data(packet);
      else
        std::cerr << "http_connection::send connection weak pointer expired"
                  << std::endl;
    }

  public:

    typedef via::comms::ssl_tcp_buffered_connection<Container>
                ssl_tcp_connection;

    /// Create.
    static boost::shared_ptr<http_ssl_connection<Container> >
           create(boost::weak_ptr<via::comms::connection> connection)
    {
      return boost::shared_ptr<http_ssl_connection<Container> >
          (new http_ssl_connection(connection));
    }

    http::rx_request const& request() const
    { return rx_.request(); }

    Container_const_iterator body_begin() const
    { return rx_.body().begin(); }

    Container_const_iterator body_end() const
    { return rx_.body().end(); }

    bool receive()
    {
      // attempt to get the pointer
      boost::shared_ptr<ssl_tcp_connection> ssl_tcp_pointer
          (boost::dynamic_pointer_cast<ssl_tcp_connection>(connection_.lock()));
      if (!ssl_tcp_pointer)
        return false;

      // attempt to read the data
      while (ssl_tcp_pointer->read_pending())
      {
        Container data(ssl_tcp_pointer->read_data());
        boost::logic::tribool rx_state
            (rx_.receive(data.begin(), data.end()));

        if (rx_state == boost::logic::tribool::true_value)
          return true;
        else
        {
          if (rx_state == boost::logic::tribool::false_value)
          {
            rx_.clear();
            via::http::tx_response response
                (via::http::response_status::BAD_REQUEST, 0);
            std::string response_txt(response.message());
            ssl_tcp_pointer->send_data(response_txt.begin(), response_txt.end());
          }
          return false;
        }
      }

      return false;
    }

    void send(http::tx_response& response)
    {
      response.set_major_version(rx_.request().major_version());
      response.set_minor_version(rx_.request().minor_version());
      std::string http_header(response.message());
      Container tx_message(http_header.begin(), http_header.end());
      send(tx_message);
    }

    template<typename ForwardIterator1, typename ForwardIterator2>
    void send(http::tx_response& response,
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
      send(tx_message);
    }

    void disconnect()
    {}

  };
}
#endif
