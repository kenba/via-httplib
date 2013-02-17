#pragma once

#ifndef TCP_ADAPTOR_HPP_VIA_HTTPLIB_
#define TCP_ADAPTOR_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "socket_adaptor.hpp"

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class tcp_adaptor
    /// This class
    //////////////////////////////////////////////////////////////////////////
    class tcp_adaptor
    {
      /// The asio TCP socket.
      boost::asio::io_service& io_service_;
      boost::asio::ip::tcp::socket socket_;
      boost::asio::ip::tcp::resolver::iterator host_iterator_;
      CommsHandler read_handler_;
      CommsHandler write_handler_;
      EventHandler event_handler_;
      ErrorHandler error_handler_;

      boost::asio::ip::tcp::resolver::iterator resolve_host
          (char const* host_name, char const* port_name) const
      {
        boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query(host_name, port_name);
        return resolver.resolve(query);
      }

      void connect_socket(boost::asio::ip::tcp::resolver::iterator itr)
      {
        socket_.async_connect(*itr,
                              boost::bind(&tcp_adaptor::handle_connect, this,
                                          boost::asio::placeholders::error));
      }

      void handle_connect(boost::system::error_code const& error)
      {
        if (error)
        {
          if ((boost::asio::error::host_not_found == error) &&
              (boost::asio::ip::tcp::resolver::iterator() != host_iterator_))
          {
            socket_.close();
            connect_socket(++host_iterator_);
          }
          else
          {
            stop();
            error_handler_(error);
          }
        }
        else
          // signal connected
          event_handler_(CONNECTED);
      }

    public:
      // also require read, write callbacks
      explicit tcp_adaptor(boost::asio::io_service& io_service,
                           CommsHandler read_handler,
                           CommsHandler write_handler,
                           EventHandler event_handler,
                           ErrorHandler error_handler) :
        io_service_(io_service),
        socket_(io_service_),
        host_iterator_(boost::asio::ip::tcp::resolver::iterator()),
        read_handler_(read_handler),
        write_handler_(write_handler),
        event_handler_(event_handler),
        error_handler_(error_handler)
      {}

      bool connect(const char* host_name, const char* port_name)
      {
        host_iterator_ = resolve_host(host_name, port_name);
        if (host_iterator_ == boost::asio::ip::tcp::resolver::iterator())
          return false;

        connect_socket(host_iterator_);
        return true;
      }

      void read(void* ptr, size_t& size)
      {
        socket_.async_read_some
            (boost::asio::buffer(ptr, size), read_handler_);
      }

      void write(void const* ptr, size_t size)
      {
        boost::asio::async_write
            (socket_, boost::asio::buffer(ptr, size), write_handler_);
      }

      void stop()
      {
        boost::system::error_code ignoredEc;
        socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_both,
                          ignoredEc);
        socket_.close(ignoredEc);
      }

      void start()
      {
        // signal connected
        event_handler_(CONNECTED);
      }

      bool is_disconnect(boost::system::error_code const& error)
      { return (boost::asio::error::connection_reset == error); }

      boost::asio::ip::tcp::socket& socket()
      { return socket_; }
    };

  }
}

#endif
