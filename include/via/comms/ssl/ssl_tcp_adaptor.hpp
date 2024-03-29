#ifndef SSL_TCP_ADAPTOR_HPP_VIA_HTTPLIB_
#define SSL_TCP_ADAPTOR_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2023 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file ssl_tcp_adaptor.hpp
/// @brief Contains the ssl_tcp_adaptor socket adaptor class.
/// Only include this file if you need an HTTPS server or client. SSL support
/// is provided by the OpenSSL library which must be included with this file.
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/tcp_adaptor.hpp"
#ifdef ASIO_STANDALONE
  #include <asio/ssl.hpp>
#else
  #include <boost/asio/ssl.hpp>
#endif

// Enable SSL support.
#ifndef HTTP_SSL
#define HTTP_SSL
#endif

namespace via
{
  namespace comms
  {
    namespace ssl
    {
      ////////////////////////////////////////////////////////////////////////
      /// @class ssl_tcp_adaptor
      /// This class enables the connection class to use ssl tcp sockets.
      /// This class and tcp_adaptor provide a common interface that
      /// enables connection to be configured for either tcp or ssl sockets.
      /// @see connection
      /// @see tcp_adaptor
      ////////////////////////////////////////////////////////////////////////
      class ssl_tcp_adaptor
      {
        /// The asio SSL TCP socket.
        ASIO::ssl::stream<ASIO::ip::tcp::socket> socket_;

      protected:

        /// @fn handshake
        /// Asynchorously performs the ssl handshake.
        /// @param handshake_handler the handshake callback function.
        /// @param is_server whether performing client or server handshaking
        void handshake(ErrorHandler handshake_handler, bool is_server)
        {
          socket_.async_handshake(is_server ? ASIO::ssl::stream_base::server
                                            : ASIO::ssl::stream_base::client,
                                  handshake_handler);
        }

        /// @fn connect_socket
        /// Attempts to connect to the host endpoints.
        /// @param connect_handler the connect callback function.
        /// @param endpoints the host endpoints.
        void connect_socket(ConnectHandler connect_handler,
                            ASIO::ip::tcp::resolver::results_type const& endpoints)
        { ASIO::async_connect(socket_.lowest_layer(), endpoints, connect_handler); }

        /// The ssl_tcp_adaptor constructor.
        /// @param socket the asio socket associated with this adaptor
        explicit ssl_tcp_adaptor(ASIO::ssl::stream<ASIO::ip::tcp::socket> socket) :
          socket_(std::move(socket))
        {}

      public:

        /// The underlying socket type.
        typedef typename ASIO::ssl::stream<ASIO::ip::tcp::socket> socket_type;

        /// A virtual destructor because connection inherits from this class.
        virtual ~ssl_tcp_adaptor()
        {}

        /// The default HTTPS port.
        static const unsigned short DEFAULT_HTTP_PORT = 443;

        /// The default size of the receive buffer.
        static const size_t DEFAULT_RX_BUFFER_SIZE = 8192;

        /// @fn connect
        /// Connect the ssl tcp socket to the given host name and port.
        /// @pre To be called by "client" connections only.
        /// Server connections are accepted by the server instead.
        /// @param io_context the asio io_context associated with this connection
        /// @param host_name the host to connect to.
        /// @param port_name the port to connect to.
        /// @param connectHandler the handler to call when connected.
        bool connect(ASIO::io_context& io_context, const char* host_name,
                      const char* port_name, ConnectHandler connectHandler)
        {
          socket_.set_verify_callback(ASIO::ssl::host_name_verification(host_name));

          auto endpoints{resolve_host(io_context, host_name, port_name)};
          if (endpoints.empty())
            return false;

          connect_socket(connectHandler, endpoints);
          return true;
        }

        /// @fn read
        /// The ssl tcp socket read function.
        /// @param ptr pointer to the receive buffer.
        /// @param size the size of the receive buffer.
        /// @param read_handler the handler for received messages.
        void read(ASIO::mutable_buffer const& buffer, CommsHandler read_handler)
        {
          socket_.async_read_some(buffer, read_handler);
        }

        /// @fn write
        /// The ssl tcp socket write function.
        /// @param buffers the buffer(s) containing the message.
        /// @param write_handler the handler called after a message is sent.
        void write(ConstBuffers const& buffers, CommsHandler write_handler)
        {
          ASIO::async_write(socket_, buffers, write_handler);
        }

        /// @fn shutdown
        /// The ssl tcp socket shutdown function.
        /// Disconnects the socket an notifies the write handler.
        /// @param write_handler the handler for the call to async_shutdown.
        void shutdown(CommsHandler write_handler)
        {
          // Cancel any pending operations
          ASIO_ERROR_CODE ignoredEc;
          socket().cancel(ignoredEc);

          // Call async_shutdown with the write_handler as a shutdown handler.
          // This sends an async SSL close_notify message, shuts down the
          // write side of the SSL stream and then waits (asynchronously) for
          // the SSL close_notify response from the other side.
          socket_.async_shutdown([write_handler]
             (ASIO_ERROR_CODE const& ec){ write_handler(ec, 0); });
        }

        /// @fn close
        /// The tcp socket close function.
        /// Cancels any send, receive or connect operations and closes the socket.
        void close()
        {
          ASIO_ERROR_CODE ignoredEc;
          if (socket().is_open())
            socket().close (ignoredEc);
        }

        /// @fn start
        /// The ssl tcp socket start function.
        /// Signals that the socket is connected.
        void start(ErrorHandler handshake_handler)
        {
          handshake(handshake_handler, true);
        }

        /// @fn is_disconnect
        /// This function determines whether the error is a socket disconnect,
        /// it also determines whether the caller should perform an SSL
        /// shutdown.
        /// @param error the error_code
        /// @retval ssl_shutdown - an SSL shutdown should be performed
        /// @return true if the socket is disconnected, false otherwise.
        bool is_disconnect(ASIO_ERROR_CODE const& error) noexcept
        { return ASIO::error::get_ssl_category() == error.category(); }

        /// @fn is_shutdown
        /// This function determines whether the caller should perform an SSL
        /// shutdown.
        // @param error the error_code
        bool is_shutdown(ASIO_ERROR_CODE const& error) noexcept
        {
          return
// SSL_R_SHORT_READ is no longer defined in openssl 1.1.x
#ifdef SSL_R_SHORT_READ
               (SSL_R_SHORT_READ != ERR_GET_REASON(error.value())) &&
#endif
               (SSL_R_PROTOCOL_IS_SHUTDOWN != ERR_GET_REASON(error.value()));
        }

        /// @fn socket
        /// Accessor for the underlying tcp socket.
        /// @return a reference to the tcp socket.
        ASIO::ssl::stream<ASIO::ip::tcp::socket>::lowest_layer_type& socket() noexcept
        { return socket_.lowest_layer(); }
      };

    }
  }
}

#endif
