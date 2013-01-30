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
    http::rx_request request_;
    Container rx_buffer_;
    Container_const_iterator body_begin_;
    Container_const_iterator body_end_;

    /// Constructor.
    http_ssl_connection(boost::weak_ptr<via::comms::connection> connection) :
      connection_(connection),
      request_(),
      rx_buffer_(),
      body_begin_(rx_buffer_.end()),
      body_end_(rx_buffer_.end())
    {}

    void send(Container const& packet)
    {
      rx_buffer_.clear();

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
    { return request_; }

    Container_const_iterator body_begin() const
    { return body_begin_; }

    Container_const_iterator body_end() const
    { return body_end_; }

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
        // append the data to the end of the buffer.
        Container data(ssl_tcp_pointer->read_data());
        rx_buffer_.insert(rx_buffer_.end(), data.begin(), data.end());

        Container_const_iterator next(rx_buffer_.begin());
        request_ = http::rx_request();
        if (!request_.parse(next, rx_buffer_.end()))
        {
          via::http::tx_response response
          (via::http::response_status::BAD_REQUEST, 0);
          std::string response_txt(response.message());
          ssl_tcp_pointer->send_data(response_txt.begin(), response_txt.end());
          rx_buffer_.clear();
          return false;
        }
        else
        {
          body_begin_ = next;
          body_end_ = body_begin_;
          size_t rx_body_size(rx_buffer_.end() - body_begin_);
          if (request_.content_length() <= rx_body_size)
          {
            body_end_ += request_.content_length();
            return true;
          }
        }
      }

      return false;
    }

    void send(http::tx_response& response)
    {
      response.set_major_version(request_.major_version());
      response.set_minor_version(request_.minor_version());
      std::string http_header(response.message());
      Container tx_message(http_header.begin(), http_header.end());
      send(tx_message);
    }

    template<typename ForwardIterator1, typename ForwardIterator2>
    void send(http::tx_response& response,
              ForwardIterator1 begin, ForwardIterator2 end)
    {
      response.set_major_version(request_.major_version());
      response.set_minor_version(request_.minor_version());
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
