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
  class http_connection// :
   //   public boost::enable_shared_from_this<http_connection<Container> >
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
//      std::cout << "http_connection receive" << std::endl;
      // attempt to get the pointer
      boost::shared_ptr<tcp_connection> tcp_pointer
          (boost::dynamic_pointer_cast<tcp_connection>(connection_.lock()));
      if (!tcp_pointer)
        return false;

      // attempt to read the data
      bool is_valid(false);
      rx_buffer_.append(tcp_pointer->current_packet(is_valid));
      while (is_valid)
      {
        tcp_pointer->next_packet();

        http::rx_request request;
        Container_const_iterator next(rx_buffer_.begin());
        if (!request.parse(next, rx_buffer_.end()))
        {
          via::http::tx_response response
          (via::http::response_status::BAD_REQUEST, 0);
          tcp_pointer->send_packet(response.message());
          rx_buffer_.clear();
          return false;
        }
        else
        {
        //  std::cout << "request.parsed" << std::endl;
       //   std::cout << request.to_string() << std::endl;
          body_begin_ = next;
          body_end_ = body_begin_ + request.content_length();
          if (body_end_ <= rx_buffer_.end())
            return true;
        }

        rx_buffer_.append(tcp_pointer->current_packet(is_valid));
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
              ForwardIterator1 begin, ForwardIterator2 end) const
    {
      size_t size(end - begin);
      Container tx_message;
      tx_message.reserve(http_header.size() + size);
      tx_message.assign(http_header.begin(), http_header.end());
      tx_message.insert(tx_message.end(), begin, end);
      send(tx_message);
    }

    void send(Container const& packet) const
    {
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
