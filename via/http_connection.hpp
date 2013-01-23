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
#include "via/comms/tcp_buffered_connection.hpp"
#include <boost/signal.hpp>
#include <deque>
#include <iostream>

namespace via
{
  ////////////////////////////////////////////////////////////////////////////
  /// @class http_connection
  ///
  ////////////////////////////////////////////////////////////////////////////
  template <typename Container>
  class http_connection
  {
    typedef typename Container::const_iterator Container_const_iterator;

    boost::weak_ptr<via::comms::connection> connection_;
    http::rx_request request_;
    Container rx_buffer_;
    Container_const_iterator body_begin_;
    Container_const_iterator body_end_;

    /// Constructor.
    http_connection(boost::weak_ptr<via::comms::connection> connection) :
      connection_(connection),
      request_(),
      rx_buffer_(),
      body_begin_(rx_buffer_.end()),
      body_end_(rx_buffer_.end())
    {}

  public:

    typedef via::comms::tcp_buffered_connection<Container> tcp_connection;

    /// Create.
    static boost::shared_ptr<http_connection<Container> >
           create(boost::weak_ptr<via::comms::connection> connection)
    {
      return boost::shared_ptr<http_connection<Container> >
          (new http_connection(connection));
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
      boost::shared_ptr<tcp_connection> tcp_pointer
          (boost::dynamic_pointer_cast<tcp_connection>(connection_.lock()));
      if (!tcp_pointer)
        return false;

      // attempt to read the data
      bool is_valid(false);
      Container_const_iterator next(rx_buffer_.begin());
      Container_const_iterator end(rx_buffer_.end());
      while (tcp_pointer->read_packet(next, end))
      {
        // append the data to the end of the buffer.
        rx_buffer_.insert(rx_buffer_.end(), next, end);
        tcp_pointer->next_packet();

        next = rx_buffer_.begin();
        request_ = http::rx_request();
        if (!request_.parse(next, rx_buffer_.end()))
        {
          via::http::tx_response response
          (via::http::response_status::BAD_REQUEST, 0);
          std::string response_txt(response.message());
          tcp_pointer->send_packet(response_txt.begin(), response_txt.end());
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

    /* Does not work when Container is a string
    void send(std::string const& http_header) const
    {
      Container tx_message(http_header.begin(), http_header.end());
      send(tx_message);
    }
    */

    template<typename ForwardIterator1, typename ForwardIterator2>
    void send(std::string const& http_header,
              ForwardIterator1 begin, ForwardIterator2 end)
    {
      rx_buffer_.clear();

      size_t size(end - begin);
      Container tx_message;
      tx_message.reserve(http_header.size() + size);
      tx_message.assign(http_header.begin(), http_header.end());
      tx_message.insert(tx_message.end(), begin, end);
      send(tx_message);
    }

    void send(Container const& packet)
    {
      rx_buffer_.clear();

      boost::shared_ptr<tcp_connection> tcp_pointer
          (boost::dynamic_pointer_cast<tcp_connection>(connection_.lock()));
      if (tcp_pointer)
        tcp_pointer->send_packet(packet);
      else
        std::cerr << "http_connection::send connection weak pointer expired"
                  << std::endl;
    }

    void disconnect()
    {}



  };

}
#endif
