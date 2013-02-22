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
/// @file tcp_adaptor.hpp
/// @brief The specific adaptor for tcp connections.
//////////////////////////////////////////////////////////////////////////////
#include "socket_adaptor.hpp"

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class tcp_adaptor
    /// This class enables the connection class to use tcp sockets.
    //////////////////////////////////////////////////////////////////////////
    class tcp_adaptor
    {
      boost::asio::io_service& io_service_; ///< The asio io_service.
      boost::asio::ip::tcp::socket socket_; ///< The asio TCP socket.
      /// The host iterator used by the resolver.
      boost::asio::ip::tcp::resolver::iterator host_iterator_;
      CommsHandler read_handler_;  ///< The read callback function.
      CommsHandler write_handler_; ///< The write callback function.
      EventHandler event_handler_; ///< The event callback function.
      ErrorHandler error_handler_; ///< The error callback function.

      /// @fn resolve_host
      /// resolves the host name and port.
      /// @param host_name the host name.
      /// @param port_name the host port.
      boost::asio::ip::tcp::resolver::iterator resolve_host
          (char const* host_name, char const* port_name) const
      {
        boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query(host_name, port_name);
        return resolver.resolve(query);
      }

      /// @fn connect_socket
      /// Attempts to connect to the given resolver iterator.
      /// @param itr the resolver iterator.
      void connect_socket(boost::asio::ip::tcp::resolver::iterator itr)
      {
        socket_.async_connect(*itr,
                              boost::bind(&tcp_adaptor::handle_connect, this,
                                          boost::asio::placeholders::error));
      }

      /// @fn handle_connect
      /// The connect callback function.
      /// @param error the boost asio error code.
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

      /// @fn Constructor
      /// @param io_service the asio io_service associted with this connection
      /// @param read_handler the read callback function.
      /// @param write_handler the write callback function.
      /// @param event_handler the event handler callback function.
      /// @param error_handler the error handler callback function.
      explicit tcp_adaptor(boost::asio::io_service& io_service,
                           CommsHandler read_handler,
                           CommsHandler write_handler,
                           EventHandler event_handler,
                           ErrorHandler error_handler,
                           unsigned int /*port_number*/) :
        io_service_(io_service),
        socket_(io_service_),
        host_iterator_(boost::asio::ip::tcp::resolver::iterator()),
        read_handler_(read_handler),
        write_handler_(write_handler),
        event_handler_(event_handler),
        error_handler_(error_handler)
      {}

      /// @fn connect
      /// Connect the tcp socket to the given host name and port.
      /// @pre To be called by "client" connections only.
      /// Server connections are accepted by the server instead.
      /// @param host_name the host to connect to.
      /// @param port_name the port to connect to.
      bool connect(const char* host_name, const char* port_name)
      {
        host_iterator_ = resolve_host(host_name, port_name);
        if (host_iterator_ == boost::asio::ip::tcp::resolver::iterator())
          return false;

        connect_socket(host_iterator_);
        return true;
      }

      /// @fn read
      /// The tcp socket read function.
      /// @param ptr pointer to the receive buffer.
      /// @param size the size of the receive buffer.
      void read(void* ptr, size_t& size)
      {
        socket_.async_read_some
            (boost::asio::buffer(ptr, size), read_handler_);
      }

      /// @fn write
      /// The tcp socket write function.
      /// @param ptr pointer to the send buffer.
      /// @param size the size of the send buffer.
      void write(void const* ptr, size_t size)
      {
        boost::asio::async_write
            (socket_, boost::asio::buffer(ptr, size), write_handler_);
      }

      /// @fn stop
      /// The tcp socket stop function.
      /// Disconnects the socket.
      void stop()
      {
        boost::system::error_code ignoredEc;
        socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_both,
                          ignoredEc);
      }

      /// @fn start
      /// The tcp socket start function.
      /// Signals that the socket is connected.
      void start()
      {
        // signal connected
        event_handler_(CONNECTED);
      }

      /// @fn is_disconnect
      /// This function determines whether the error is a socket disconnect.
      /// @return true if a disconnect error, false otherwise.
      bool is_disconnect(boost::system::error_code const& error)
      { return (boost::asio::error::connection_reset == error); }

      /// @fn socket
      /// Accessor for the underlying tcp socket.
      /// @return a reference to the tcp socket.
      boost::asio::ip::tcp::socket& socket()
      { return socket_; }
    };

  }
}

#endif
