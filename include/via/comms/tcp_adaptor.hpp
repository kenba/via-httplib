#ifndef TCP_ADAPTOR_HPP_VIA_HTTPLIB_
#define TCP_ADAPTOR_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
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
#include "via/no_except.hpp"

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
    class tcp_adaptor : public socket_adaptor
    {
      boost::asio::ip::tcp::socket socket_; ///< The asio TCP socket.
    protected:
      /// @fn handshake
      /// Performs the SSL handshake. Since this isn't an SSL socket, it just
      /// calls the handshake_handler with a success error code.
      /// @param handshake_handler the handshake callback function.
      // @param is_server whether performing client or server handshaking,
      // not used by un-encrypted sockets.
      virtual void handshake(ErrorHandler handshake_handler, bool /*is_server*/ = false)
      {
        boost::system::error_code ec; // Default is success
        handshake_handler(ec);
      }

      /// The tcp_adaptor constructor.
      /// @param io_service the asio io_service associted with this connection
      explicit tcp_adaptor(boost::asio::io_service& io_service) :
        socket_adaptor(io_service),
        socket_{io_service_}
      {}

    public:
      /// A virtual destructor because connection inherits from this class.
      virtual ~tcp_adaptor()
      {}

      /// The default HTTP port.
      static const unsigned short DEFAULT_HTTP_PORT = 80;

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
      /// @param buffers the buffer(s) containing the message.
      /// @param write_handler the handler called after a message is sent.
      void write(ConstBuffers& buffers, CommsHandler write_handler)
      {
        boost::asio::async_write(socket_, buffers, write_handler);
      }

      /// @fn is_disconnect
      /// This function determines whether the error is a socket disconnect.
      // @param error the error_code
      // @retval ssl_shutdown - an ssl_disconnect should be performed
      /// @return true if a disconnect error, false otherwise.
      bool is_disconnect(boost::system::error_code const&, bool&) NOEXCEPT
      { return false; }

      /// @fn socket
      /// Accessor for the underlying tcp socket.
      /// @return a reference to the tcp socket.
      boost::asio::ip::tcp::socket& socket() NOEXCEPT
      { return socket_; }
    };
  }
}

#endif
