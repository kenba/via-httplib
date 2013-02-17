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
#include "via/comms/socket_adaptor.hpp"
#include <boost/asio/ssl.hpp>

namespace via
{
  namespace comms
  {
    namespace ssl
    {
      ////////////////////////////////////////////////////////////////////////
      /// @class ssl_tcp_adaptor
      /// This class
      ////////////////////////////////////////////////////////////////////////
      class ssl_tcp_adaptor
      {
        boost::asio::io_service& io_service_;
        /// The asio SSL TCP socket.
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
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

        bool connect_socket(boost::asio::ip::tcp::resolver::iterator itr)
        {
          if (boost::asio::ip::tcp::resolver::iterator() == itr)
            return false;

          socket_.set_verify_callback
              (boost::bind(&ssl_tcp_adaptor::verify_certificate,
                           this, _1, _2));

          // Attempt to connect to the host
          boost::asio::async_connect(socket_.lowest_layer(), itr,
                         boost::bind(&ssl_tcp_adaptor::handle_connect, this,
                                     boost::asio::placeholders::error));

          return true;
        }

        void handle_connect(boost::system::error_code const& error)
        {
          if (error)
          {
            if ((boost::asio::error::host_not_found == error) &&
                (boost::asio::ip::tcp::resolver::iterator() != host_iterator_))
            {
              socket_.lowest_layer().close();
              connect_socket(++host_iterator_);
            }
            else
            {
              stop();
              error_handler_(error);
            }
          }
          else
            // Perform the client SSL handshake
            // Note: this is NOT the same as start(), which is the server
            // handshake.
            socket_.async_handshake(boost::asio::ssl::stream_base::client,
                       boost::bind(&ssl_tcp_adaptor::handle_handshake, this,
                                   boost::asio::placeholders::error));
        }

        bool verify_certificate(bool preverified,
                                boost::asio::ssl::verify_context& ctx)
        {
          // The verify callback can be used to check whether the certificate that is
          // being presented is valid for the peer. For example, RFC 2818 describes
          // the steps involved in doing this for HTTPS. Consult the OpenSSL
          // documentation for more details. Note that the callback is called once
          // for each certificate in the certificate chain, starting from the root
          // certificate authority.

          // In this example we will simply print the certificate's subject name.
          char subject_name[256];
          X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
          X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);

          return preverified;
        }

        void handle_handshake(boost::system::error_code const& error)
        {
          if (error)
          {
            stop();
            error_handler_(error);
          }
          else
            // signal connected
            event_handler_(CONNECTED);
        }

      protected:

        // also require read, write callbacks
        explicit ssl_tcp_adaptor(boost::asio::io_service& io_service,
                                 CommsHandler read_handler,
                                 CommsHandler write_handler,
                                 EventHandler event_handler,
                                 ErrorHandler error_handler) :
          //      boost::asio::ssl::context& ssl_context) :
          io_service_(io_service),
          socket_(io_service_, ssl_context()),
          host_iterator_(boost::asio::ip::tcp::resolver::iterator()),
          read_handler_(read_handler),
          write_handler_(write_handler),
          event_handler_(event_handler),
          error_handler_(error_handler)
        { ssl_context().set_verify_mode(boost::asio::ssl::verify_peer); }

      public:

        static boost::asio::ssl::context& ssl_context()
        {
          static boost::asio::ssl::context context_
              (boost::asio::ssl::context::sslv23);
          return context_;
        }

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
          socket_.lowest_layer().shutdown
              (boost::asio::ip::tcp::socket::shutdown_both, ignoredEc);
          socket_.lowest_layer().close(ignoredEc);
        }

        void start()
        {
          socket_.async_handshake
              (boost::asio::ssl::stream_base::server,
               boost::bind(&ssl_tcp_adaptor::handle_handshake,
                           this, boost::asio::placeholders::error));
        }

        bool is_disconnect(boost::system::error_code const& error)
        { return (boost::asio::error::connection_reset == error); }

        boost::asio::ssl::stream<boost::asio::ip::tcp::socket>
        ::lowest_layer_type& socket()
        { return socket_.lowest_layer(); }
      };
    }
  }
}

#endif
