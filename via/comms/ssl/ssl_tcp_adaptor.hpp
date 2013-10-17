#pragma once

#ifndef SSL_TCP_ADAPTOR_HPP_VIA_HTTPLIB_
#define SSL_TCP_ADAPTOR_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file ssl_tcp_adaptor.hpp
/// @brief The specific adaptor for ssl tcp connections.
/// Only include this file if you need an HTTPS server or client. SSL support
/// is provided by the OpenSSL library which must be included with this file.
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/socket_adaptor.hpp"
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
      ////////////////////////////////////////////////////////////////////////
      class ssl_tcp_adaptor
      {
        /// A connection hander callback function type.
        /// @param error the (boost) error code.
        /// @param host_iterator the resolver_iterator
        typedef std::tr1::function<void (boost::system::error_code const&,
                                         boost::asio::ip::tcp::resolver::iterator)>
                                               ConnectHandler;

        boost::asio::io_service& io_service_; ///< The asio io_service.
        /// The asio SSL TCP socket.
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
        /// The host iterator used by the resolver.
        boost::asio::ip::tcp::resolver::iterator host_iterator_;

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
          // In this example we can simply print the certificate's subject name.
          char subject_name[256];
          X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
          X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);

          return preverified;
        }

      protected:

        /// @fn handshake
        /// Asynchorously performs the ssl handshake.
        /// @param handshake_handler the handshake callback function.
        /// @param is_server whether performing client or server handshaking,
        /// default client.
        void handshake(ErrorHandler handshake_handler, bool is_server = false)
        {
          socket_.async_handshake(is_server ? boost::asio::ssl::stream_base::server
                                            : boost::asio::ssl::stream_base::client, 
                                  handshake_handler);
        }

        /// @fn connect_socket
        /// Attempts to connect to the given resolver iterator.
        /// @param itr the resolver iterator.
        void connect_socket(ConnectHandler connectHandler,
                            boost::asio::ip::tcp::resolver::iterator host_iterator)
        {
          socket_.set_verify_callback
              (boost::bind(&ssl_tcp_adaptor::verify_certificate, _1, _2));

          // Attempt to connect to the host
          boost::asio::async_connect(socket_.lowest_layer(), host_iterator,
                                     connectHandler);
        }

        /// The ssl_tcp_adaptor constructor.
        /// @param io_service the asio io_service associted with this connection
        /// @param port_number not required for tcp connections.
        explicit ssl_tcp_adaptor(boost::asio::io_service& io_service,
                                 unsigned short /*port_number*/) :
          io_service_(io_service),
          socket_(io_service_, ssl_context()),
          host_iterator_()
        {}

      public:

        /// The type of resolver iterator used by this socket.
        typedef boost::asio::ip::tcp::resolver::iterator resolver_iterator;

        /// A virtual destructor because connection inherits from this class.
        virtual ~ssl_tcp_adaptor()
        {}

        /// The default HTTPS port.
        static const unsigned short DEFAULT_HTTP_PORT = 443;

        /// @fn ssl_context
        /// A static function to manage the ssl context for the ssl
        /// connections.
        /// @return ssl_context the ssl context.
        static boost::asio::ssl::context& ssl_context()
        {
          static boost::asio::ssl::context context_
              (boost::asio::ssl::context::sslv23);
          return context_;
        }

        /// @fn connect
        /// Connect the ssl tcp socket to the given host name and port.
        /// @pre To be called by "client" connections only.
        /// Server connections are accepted by the server instead.
        /// @param host_name the host to connect to.
        /// @param port_name the port to connect to.
        bool connect(const char* host_name, const char* port_name,
                     ConnectHandler connectHandler)
        {
          ssl_context().set_verify_mode(boost::asio::ssl::verify_peer);

          host_iterator_ = resolve_host(host_name, port_name);
          if (host_iterator_ == boost::asio::ip::tcp::resolver::iterator())
            return false;

          connect_socket(connectHandler, host_iterator_);
          return true;
        }

        /// @fn read
        /// The ssl tcp socket read function.
        /// @param ptr pointer to the receive buffer.
        /// @param size the size of the receive buffer.
        void read(void* ptr, size_t size, CommsHandler read_handler)
        {
          socket_.async_read_some
              (boost::asio::buffer(ptr, size), read_handler);
        }

        /// @fn write
        /// The ssl tcp socket write function.
        /// @param ptr pointer to the send buffer.
        /// @param size the size of the send buffer.
        void write(void const* ptr, size_t size, CommsHandler write_handler)
        {
          boost::asio::async_write
              (socket_, boost::asio::buffer(ptr, size), write_handler);
        }

        /// @fn shutdown
        /// The ssl tcp socket shutdown function.
        /// Disconnects the socket.
        void shutdown()
        {
          boost::system::error_code ignoredEc;
          socket_.lowest_layer().shutdown
              (boost::asio::ip::tcp::socket::shutdown_both, ignoredEc);
        }

        /// @fn close
        /// The tcp socket close function.
        /// Cancels any send, receive or connect operations and closes the socket.
        void close()
        {
          boost::system::error_code ignoredEc;
          socket_.lowest_layer().close (ignoredEc);
        }

        /// @fn start
        /// The ssl tcp socket start function.
        /// Signals that the socket is connected.
        void start(ErrorHandler handshake_handler)
        {
          handshake(handshake_handler, true);
        }

        /// @fn is_disconnect
        /// This function determines whether the error is a socket disconnect.
        /// @return true if a disconnect error, false otherwise.
        bool is_disconnect(boost::system::error_code const& error)
        { return (boost::asio::error::connection_reset == error); }

        /// @fn socket
        /// Accessor for the underlying tcp socket.
        /// @return a reference to the tcp socket.
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket>
        ::lowest_layer_type& socket()
        { return socket_.lowest_layer(); }
      };

    }
  }
}

#endif
