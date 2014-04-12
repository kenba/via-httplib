#pragma once

#ifndef TCP_ADAPTOR_HPP_VIA_HTTPLIB_
#define TCP_ADAPTOR_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file tcp_adaptor.hpp
/// @brief Contains the tcp_adaptor socket adaptor class.
//////////////////////////////////////////////////////////////////////////////
#include "socket_adaptor.hpp"

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class tcp_adaptor
    /// This class enables the connection class to use tcp sockets.
    /// This class and ssl_tcp_adaptor provide a common interface that
    /// enables connection to be configured for either tcp or ssl sockets.
    /// @see connection
    /// @see ssl::ssl_tcp_adaptor
    //////////////////////////////////////////////////////////////////////////
    class tcp_adaptor
    {
      boost::asio::io_service& io_service_; ///< The asio io_service.
      boost::asio::ip::tcp::socket socket_; ///< The asio TCP socket.
      /// The host iterator used by the resolver.
      boost::asio::ip::tcp::resolver::iterator host_iterator_;

      /// @fn resolve_host
      /// resolves the host name and port.
      /// @param host_name the host name.
      /// @param port_name the host port.
      boost::asio::ip::tcp::resolver::iterator resolve_host
          (char const* host_name, char const* port_name) const
      {
        boost::asio::ip::tcp::resolver resolver{io_service_};
        boost::asio::ip::tcp::resolver::query query{host_name, port_name};
        return resolver.resolve(query);
      }

    protected:

      /// @fn handshake
      /// Performs the SSL handshake. Since this isn't an SSL socket, it just
      /// calls the handshake_handler with a success error code.
      /// @param handshake_handler the handshake callback function.
      // @param is_server whether performing client or server handshaking,
      // not used by un-encrypted sockets.
      void handshake(ErrorHandler handshake_handler, bool /*is_server*/ = false)
      {
        boost::system::error_code ec; // Default is success
        handshake_handler(ec);
      }
        
      /// @fn connect_socket
      /// Attempts to connect to the given resolver iterator.
      /// @param connect_handler the connect callback function.
      /// @param host_iterator the resolver iterator.
      void connect_socket(ConnectHandler connect_handler,
                          boost::asio::ip::tcp::resolver::iterator host_iterator)
      { boost::asio::async_connect(socket_, host_iterator, connect_handler); }

      /// The tcp_adaptor constructor.
      /// @param io_service the asio io_service associted with this connection
      explicit tcp_adaptor(boost::asio::io_service& io_service) :
        io_service_(io_service),
        socket_{io_service_},
        host_iterator_{}
      {}

    public:

      /// The type of resolver iterator used by this socket.
      typedef boost::asio::ip::tcp::resolver::iterator resolver_iterator;
      
      /// A virtual destructor because connection inherits from this class.
      virtual ~tcp_adaptor()
      {}

      /// The default HTTP port.
      static const unsigned short DEFAULT_HTTP_PORT = 80;

      /// @fn connect
      /// Connect the tcp socket to the given host name and port.
      /// @pre To be called by "client" connections only.
      /// Server connections are accepted by the server instead.
      /// @param host_name the host to connect to.
      /// @param port_name the port to connect to.
      /// @param connectHandler the handler to call when connected.
      bool connect(const char* host_name, const char* port_name,
                   ConnectHandler connectHandler)
      {
        host_iterator_ = resolve_host(host_name, port_name);
        if (host_iterator_ == boost::asio::ip::tcp::resolver::iterator())
          return false;

        connect_socket(connectHandler, host_iterator_);
        return true;
      }

      /// @fn read
      /// The tcp socket read function.
      /// @param ptr pointer to the receive buffer.
      /// @param size the size of the receive buffer.
      /// @param read_handler the handler for received messages.
      void read(void* ptr, size_t size, CommsHandler read_handler)
      {
        socket_.async_read_some
            (boost::asio::buffer(ptr, size), read_handler);
      }

      /// @fn write
      /// The tcp socket write function.
      /// @param ptr pointer to the send buffer.
      /// @param size the size of the send buffer.
      /// @param write_handler the handler called after a message is sent.
      void write(void const* ptr, size_t size, CommsHandler write_handler)
      {
        boost::asio::async_write
            (socket_, boost::asio::buffer(ptr, size), write_handler);
      }

      /// @fn shutdown
      /// The tcp socket shutdown function.
      /// Disconnects the socket.
      void shutdown()
      {
        boost::system::error_code ignoredEc;
        socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_both,
                          ignoredEc);
      }

      /// @fn close
      /// The tcp socket close function.
      /// Cancels any send, receive or connect operations and closes the socket.
      void close()
      {
        boost::system::error_code ignoredEc;
        socket_.close (ignoredEc);
      }

      /// @fn start
      /// The tcp socket start function.
      /// Signals that the socket is connected.
      /// @param handshake_handler the handshake callback function.
      void start(ErrorHandler handshake_handler)
      { handshake(handshake_handler, true); }

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
