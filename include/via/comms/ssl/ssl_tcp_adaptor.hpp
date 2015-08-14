#ifndef SSL_TCP_ADAPTOR_HPP_VIA_HTTPLIB_
#define SSL_TCP_ADAPTOR_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
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
#include "via/comms/socket_adaptor.hpp"
#include "via/no_except.hpp"
#include <boost/asio/ssl.hpp>

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
        /// The asio io_service.
        boost::asio::io_service& io_service_;
        /// The asio SSL TCP socket.
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
        /// The host iterator used by the resolver.
        boost::asio::ip::tcp::resolver::iterator host_iterator_;

        /// @fn resolve_host
        /// Resolves the host name and port.
        /// @param host_name the host name.
        /// @param port_name the host port.
        boost::asio::ip::tcp::resolver::iterator resolve_host
          (char const* host_name, char const* port_name) const
        {
          boost::system::error_code ignoredEc;
          boost::asio::ip::tcp::resolver resolver(io_service_);
          boost::asio::ip::tcp::resolver::query query(host_name, port_name);
          return resolver.resolve(query, ignoredEc);
        }

        /// @fn verify_certificate
        /// The verify callback function.
        /// The verify callback can be used to check whether the certificate
        /// that is being presented is valid for the peer. For example,
        /// RFC 2818 describes the steps involved in doing this for HTTPS.
        /// Consult the OpenSSL documentation for more details.
        /// Note that the callback is called once for each certificate in the
        /// certificate chain, starting from the root certificate authority.
        static bool verify_certificate(bool preverified,
                                       boost::asio::ssl::verify_context& ctx)
        {
          char subject_name[256];
          X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
          X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
          return preverified;
        }

      protected:

        /// @fn handshake
        /// Asynchorously performs the ssl handshake.
        /// @param handshake_handler the handshake callback function.
        /// @param is_server whether performing client or server handshaking
        void handshake(ErrorHandler handshake_handler, bool is_server)
        {
          socket_.async_handshake(is_server ? boost::asio::ssl::stream_base::server
                                            : boost::asio::ssl::stream_base::client,
                                  handshake_handler);
        }

        /// @fn connect_socket
        /// Attempts to connect to the given resolver iterator.
        /// @param connect_handler the connect callback function.
        /// @param host_iterator the resolver iterator.
        void connect_socket(ConnectHandler connect_handler,
                            boost::asio::ip::tcp::resolver::iterator host_iterator)
        {
          // Attempt to connect to the host
          boost::asio::async_connect(socket_.lowest_layer(), host_iterator,
                                     connect_handler);
        }

        /// The ssl_tcp_adaptor constructor.
        /// @param io_service the asio io_service associted with this connection
        explicit ssl_tcp_adaptor(boost::asio::io_service& io_service) :
          io_service_(io_service),
          socket_(io_service_, ssl_context()),
          host_iterator_()
        {}

      public:

        /// A virtual destructor because connection inherits from this class.
        virtual ~ssl_tcp_adaptor()
        {}

        /// The default HTTPS port.
        static const unsigned short DEFAULT_HTTP_PORT = 443;

        /// The default size of the receive buffer.
        static const size_t DEFAULT_RX_BUFFER_SIZE = 8192;

        /// @fn ssl_context
        /// A static function to manage the ssl context for the ssl
        /// connections.
        /// @return ssl_context the ssl context.
        static boost::asio::ssl::context& ssl_context()
        {
          static boost::asio::ssl::context context_
              (boost::asio::ssl::context::tlsv12);
          return context_;
        }

        /// @fn connect
        /// Connect the ssl tcp socket to the given host name and port.
        /// @pre To be called by "client" connections only.
        /// Server connections are accepted by the server instead.
        /// @param host_name the host to connect to.
        /// @param port_name the port to connect to.
        /// @param connect_handler the handler to call when connected.
        bool connect(const char* host_name, const char* port_name,
                     ConnectHandler connect_handler)
        {
          ssl_context().set_verify_mode(boost::asio::ssl::verify_peer);
          socket_.set_verify_callback([]
            (bool preverified, boost::asio::ssl::verify_context& ctx)
              { return verify_certificate(preverified, ctx); });

          host_iterator_ = resolve_host(host_name, port_name);
          if (host_iterator_ == boost::asio::ip::tcp::resolver::iterator())
            return false;

          connect_socket(connect_handler, host_iterator_);
          return true;
        }

        /// @fn read
        /// The ssl tcp socket read function.
        /// @param ptr pointer to the receive buffer.
        /// @param size the size of the receive buffer.
        /// @param read_handler the handler for received messages.
        void read(void* ptr, size_t size, CommsHandler read_handler)
        {
          socket_.async_read_some
              (boost::asio::buffer(ptr, size), read_handler);
        }

        /// @fn write
        /// The ssl tcp socket write function.
        /// @param buffers the buffer(s) containing the message.
        /// @param write_handler the handler called after a message is sent.
        void write(ConstBuffers& buffers, CommsHandler write_handler)
        {
          boost::asio::async_write(socket_, buffers, write_handler);
        }

        /// @fn shutdown
        /// The ssl tcp socket shutdown function.
        /// Disconnects the socket.
        /// Note: the handlers are required to shutdown SSL gracefully, see:
        /// http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error/25703699#25703699
        /// This function sends an SSL close_notify message to the other side
        /// but does not wait for the response so it can be used in all cases
        /// to shutdown an SSL socket gracefully.
        /// @param write_handler the handler for the call to async_write.
        void shutdown(CommsHandler write_handler)
        {
          // Something to send in the call to write
          static const char buffer[] = "";

          // Call async_shutdown with a handler to ignore the response.
          // This sends an async SSL close_notify message, shuts down the
          // write side of the SSL stream and then waits (asynchronously) for
          // the SSL close_notify response from the other side.
          socket_.async_shutdown([](boost::system::error_code const&){});

          // Write some data to the SSL socket
          // This will fail because the async_shutdown above shutdown the write
          // side of the SSL stream. The failed write will close the underlying
          // socket causing the async_shutdown operation to call it's callback
          // with boost::asio::error::operation_aborted (which is ignored).
          // The failed write will call write_handler with the SSL error code:
          // SSL_R_PROTOCOL_IS_SHUTDOWN this indicates that the socket is now
          // disconnected.
          boost::asio::async_write(socket_,
                                   boost::asio::const_buffers_1(&buffer[0], 1),
                                   write_handler);
        }

        /// @fn close
        /// The tcp socket close function.
        /// Cancels any send, receive or connect operations and closes the socket.
        void close()
        {
          boost::system::error_code ignoredEc;
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
        /// it also determines whether the caller should perfrom an SSL
        /// shutdown.
        /// @param error the error_code
        /// @retval ssl_shutdown - an SSL shutdown should be performed
        /// @return true if the socket is disconnected, false otherwise.
        bool is_disconnect(boost::system::error_code const& error,
                           bool& ssl_shutdown) NOEXCEPT
        {
          bool ssl_error(boost::asio::error::get_ssl_category() == error.category());
          ssl_shutdown = ssl_error &&
               (SSL_R_SHORT_READ != ERR_GET_REASON(error.value())) &&
               (SSL_R_PROTOCOL_IS_SHUTDOWN != ERR_GET_REASON(error.value()));

          return ssl_error && !ssl_shutdown;
        }

        /// @fn socket
        /// Accessor for the underlying tcp socket.
        /// @return a reference to the tcp socket.
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket>
        ::lowest_layer_type& socket() NOEXCEPT
        { return socket_.lowest_layer(); }
      };

    }
  }
}

#endif
