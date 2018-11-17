#ifndef TCP_ADAPTOR_HPP_VIA_HTTPLIB_
#define TCP_ADAPTOR_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2018 Ken Barker
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
      ASIO::io_service& io_service_; ///< The asio io_service.
      ASIO::ip::tcp::socket socket_; ///< The asio TCP socket.
      /// The host iterator used by the resolver.
      ASIO::ip::tcp::resolver::iterator host_iterator_;

      /// @fn resolve_host
      /// resolves the host name and port.
      /// @param host_name the host name.
      /// @param port_name the host port.
      ASIO::ip::tcp::resolver::iterator resolve_host
          (char const* host_name, char const* port_name) const
      {
        ASIO_ERROR_CODE ignoredEc;
        ASIO::ip::tcp::resolver resolver(io_service_);
        ASIO::ip::tcp::resolver::query query(host_name, port_name);
        return resolver.resolve(query, ignoredEc);
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
        ASIO_ERROR_CODE ec; // Default is success
        handshake_handler(ec);
      }

      /// @fn connect_socket
      /// Attempts to connect to the given resolver iterator.
      /// @param connect_handler the connect callback function.
      /// @param host_iterator the resolver iterator.
      void connect_socket(ConnectHandler connect_handler,
                          ASIO::ip::tcp::resolver::iterator host_iterator)
      { ASIO::async_connect(socket_, host_iterator, connect_handler); }

      /// The tcp_adaptor constructor.
      /// @param io_service the asio io_service associted with this connection
      explicit tcp_adaptor(ASIO::io_service& io_service) :
        io_service_(io_service),
        socket_(io_service_),
        host_iterator_()
      {}

    public:

      /// A virtual destructor because connection inherits from this class.
      virtual ~tcp_adaptor()
      {}

      /// The default HTTP port.
      static const unsigned short DEFAULT_HTTP_PORT = 80;

      /// The default size of the receive buffer.
      static const size_t DEFAULT_RX_BUFFER_SIZE = 8192;

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
        if (host_iterator_ == ASIO::ip::tcp::resolver::iterator())
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
            (ASIO::buffer(ptr, size), read_handler);
      }

      /// @fn write
      /// The tcp socket write function.
      /// @param buffers the buffer(s) containing the message.
      /// @param write_handler the handler called after a message is sent.
      void write(ConstBuffers& buffers, CommsHandler write_handler)
      {
        ASIO::async_write(socket_, buffers, write_handler);
      }

      /// @fn shutdown
      /// The tcp socket shutdown function.
      /// Disconnects the socket.
      /// @param write_handler the handler to notify that the socket is
      /// disconnected.
      void shutdown(CommsHandler write_handler)
      {
        ASIO_ERROR_CODE ec;
        socket_.shutdown(ASIO::ip::tcp::socket::shutdown_both, ec);

        ec = ASIO_ERROR_CODE(ASIO::error::eof);
        write_handler(ec, 0);
      }

      /// @fn close
      /// The tcp socket close function.
      /// Cancels any send, receive or connect operations and closes the socket.
      void close()
      {
        ASIO_ERROR_CODE ignoredEc;
        if (socket_.is_open())
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
      // @param error the error_code
      /// @return true if a disconnect error, false otherwise.
      bool is_disconnect(ASIO_ERROR_CODE const&) noexcept
      { return false; }

      /// @fn is_shutdown
      /// This function determines whether the caller should perform an SSL
      /// shutdown.
      // @param error the error_code
      bool is_shutdown(ASIO_ERROR_CODE const&) noexcept
      { return false; }

      /// @fn socket
      /// Accessor for the underlying tcp socket.
      /// @return a reference to the tcp socket.
      ASIO::ip::tcp::socket& socket() noexcept
      { return socket_; }
    };

  }
}

#endif
