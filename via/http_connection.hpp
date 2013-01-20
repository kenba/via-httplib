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

namespace
{
  std::string test_body()
  {
    std::string body_text("<html>\r\n");
    body_text += "<head><title>Accepted</title></head>\r\n";
    body_text += "<body><h1>202 Accepted</h1></body>\r\n";
    body_text += "</html>\r\n";

    return body_text;
  }
}

namespace via
{
  ////////////////////////////////////////////////////////////////////////////
  /// @class http_connection
  ///
  ////////////////////////////////////////////////////////////////////////////
  template <typename Container>
  class http_connection :
      public boost::enable_shared_from_this<http_connection<Container> >
  {
  public:
    typedef typename boost::weak_ptr<http_connection<Container> >
              weak_http_connection;

    typedef typename boost::shared_ptr<http_connection<Container> >
              shared_http_connection;

    typedef typename Container::const_iterator Container_const_iterator;

    typedef typename via::http::rx_request request_type;
    typedef typename via::http::tx_response response_type;

    typedef boost::signal<void (request_type const&,
                                Container_const_iterator,
                                Container_const_iterator,
                                response_type&,
                                Container_const_iterator&,
                                Container_const_iterator&) > response_signal;

    typedef boost::signal<void (request_type const&,
                                Container_const_iterator,
                                Container_const_iterator,
                                weak_http_connection) > request_signal;

    typedef via::comms::tcp_buffered_connection<Container> tcp_connection;

  private:
    boost::weak_ptr<via::comms::connection> connection_;

    response_signal response_signal_;

    request_signal request_signal_;

  public:
    http_connection(boost::weak_ptr<via::comms::connection> connection) :
      connection_(connection),
      response_signal_(),
      request_signal_()
    {}

    static boost::shared_ptr<http_connection<Container> >
           create(boost::weak_ptr<via::comms::connection> connection)
    {
      return boost::shared_ptr<http_connection<Container> >
          (new http_connection(connection));
    }

    bool receive()
    {
//      std::cout << "http_connection receive" << std::endl;
      boost::shared_ptr<tcp_connection> tcp_pointer
          (boost::dynamic_pointer_cast<tcp_connection>(connection_.lock()));
      if (!tcp_pointer)
        return false;

      Container_const_iterator rx_begin;
      Container_const_iterator rx_end;
      while (tcp_pointer->read_packet(rx_begin, rx_end))
      {
        via::http::rx_request request;
        Container_const_iterator next(rx_begin);
        if (request.parse(next, rx_end))
        {
          // TODO send signals...

          std::cout << "request.parsed" << std::endl;
          std::cout << request.to_string() << std::endl;

          // TODO parse the message properly
          // i.e. send signals that the message was received
          std::string body_text(test_body());
          size_t body_length(body_text.size());

          via::http::tx_response response
              (via::http::response_status::OK, body_length);
          response.add_header(http::header_field::ACCEPT_CHARSET,
                              "ISO-8859-1");
          std::string http_message(response.message());
          if (request.method() !=
              http::request_method::name(http::request_method::HEAD))
          {
            http_message += body_text;
          }

          std::cout << http_message << std::endl;
          tcp_pointer->send_packet(http_message);
        }
        else
        {
          via::http::tx_response response
              (via::http::response_status::BAD_REQUEST, 0);
          tcp_pointer->send_packet(response.message());
        }

        tcp_pointer->next_packet();
      }

      return true;
    }

    /* Does not work when Container is a string
    void send(std::string const& http_header) const
    {
      Container tx_message(http_header.begin(), http_header.end());
      send(tx_message);
    }
    */

    /*
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
        ; // TODO log error http_connection::send weak pointer expired
    }

    void disconnect()
    {}

    */

  };

}
#endif
