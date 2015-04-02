#ifndef SOCKET_ADAPTOR_HPP_VIA_HTTPLIB_
#define SOCKET_ADAPTOR_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file socket_adaptor.hpp
/// @brief Type definitions used by the connection socket adaptors.
/// @see tcp_adaptor
/// @see ssl_tcp_adaptor
//////////////////////////////////////////////////////////////////////////////
#include <boost/asio.hpp>
#include <deque>
#include <functional>

namespace via
{
  namespace comms
  {
    /// @enum event_type the types of events handled by a socket_adaptor.
    enum event_type
    {
      CONNECTED,   ///< The socket is now connected.
      RECEIVED,    ///< Data received.
      SENT,        ///< Data sent.
      DISCONNECTED ///< The socket is now disconnected.
    };

    /// @typedef ErrorHandler
    /// An error hander callback function type.
    /// @param error the (boost) error code.
    typedef std::function<void (boost::system::error_code const&)>
                                               ErrorHandler;

    /// @typedef CommsHandler
    /// A (read or write) comms hander callback function type.
    /// @param error the (boost) error code.
    /// @param size the number of bytes read or written.
    typedef std::function<void (boost::system::error_code const&, size_t)>
                                               CommsHandler;

    /// @typedef ConnectHandler
    /// A connect hander callback function type.
    /// @param error the (boost) error code.
    /// @param host_iterator the resolver_iterator
    typedef std::function<void (boost::system::error_code const&,
                                boost::asio::ip::tcp::resolver::iterator)>
                                           ConnectHandler;

    /// @typedef ConstBuffers
    /// A deque of asio::const_buffers.
    typedef std::deque<boost::asio::const_buffer> ConstBuffers;

    class socket_adaptor
    {
    protected:
      boost::asio::io_service& io_service_; ///< The asio io_service.
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

      /// @fn handshake
      /// Performs the SSL handshake. Since this isn't an SSL socket, it just
      /// calls the handshake_handler with a success error code.
      /// @param handshake_handler the handshake callback function.
      // @param is_server whether performing client or server handshaking,
      // not used by un-encrypted sockets.
      virtual void handshake(ErrorHandler handshake_handler, bool /*is_server*/ = false) = 0;

      /// @fn connect_socket
      /// Attempts to connect to the given resolver iterator.
      /// @param connect_handler the connect callback function.
      /// @param host_iterator the resolver iterator.
      virtual void connect_socket(ConnectHandler connect_handler,
                                  boost::asio::ip::tcp::resolver::iterator host_iterator)
      { boost::asio::async_connect(socket(), host_iterator, connect_handler); }

      /// The tcp_adaptor constructor.
      /// @param io_service the asio io_service associted with this connection
      explicit socket_adaptor(boost::asio::io_service& io_service) :
      io_service_(io_service),
      host_iterator_{}
      {}

    public:

      /// The default size of the receive buffer.
      static const size_t DEFAULT_RX_BUFFER_SIZE = 8192;

      /// The type of resolver iterator used by this socket.
      typedef boost::asio::ip::tcp::resolver::iterator resolver_iterator;

      /// A virtual destructor because connection inherits from this class.
      virtual ~socket_adaptor()
      {}

      /// @fn socket
      /// Accessor for the underlying tcp socket.
      /// @return a reference to the tcp socket.
      virtual boost::asio::ip::tcp::socket& socket() = 0;

      /// @fn read
      /// The tcp socket read function.
      /// @param ptr pointer to the receive buffer.
      /// @param size the size of the receive buffer.
      /// @param read_handler the handler for received messages.
      virtual void read(void* ptr, size_t size, CommsHandler read_handler) = 0;

      /// @fn write
      /// The tcp socket write function.
      /// @param ptr pointer to the send buffer.
      /// @param size the size of the send buffer.
      /// @param write_handler the handler called after a message is sent.
      virtual void write(ConstBuffers& buffers, CommsHandler write_handler) = 0;

      /// @fn connect
      /// Connect the tcp socket to the given host name and port.
      /// @pre To be called by "client" connections only.
      /// Server connections are accepted by the server instead.
      /// @param host_name the host to connect to.
      /// @param port_name the port to connect to.
      /// @param connectHandler the handler to call when connected.
      virtual bool connect(const char* host_name, const char* port_name,
                           ConnectHandler connectHandler)
      {
        host_iterator_ = resolve_host(host_name, port_name);
        if (host_iterator_ == boost::asio::ip::tcp::resolver::iterator())
          return false;

        connect_socket(connectHandler, host_iterator_);
        return true;
      }

      /// @fn shutdown
      /// The tcp socket shutdown function.
      /// Disconnects the socket.
      virtual void shutdown()
      {
        boost::system::error_code ignoredEc;
        socket().shutdown
          (boost::asio::ip::tcp::socket::shutdown_both, ignoredEc);
        close();
      }

      /// @fn close
      /// The tcp socket close function.
      /// Cancels any send, receive or connect operations and closes the socket.
      virtual void close()
      {
        boost::system::error_code ignoredEc;
        socket().close (ignoredEc);
      }

      /// @fn start
      /// The tcp socket start function.
      /// Signals that the socket is connected.
      /// @param handshake_handler the handshake callback function.
      virtual void start(ErrorHandler handshake_handler)
      { handshake(handshake_handler, true); }

      /// @fn is_disconnect
      /// This function determines whether the error is a socket disconnect.
      /// @return true if a disconnect error, false otherwise.
      virtual bool is_disconnect(boost::system::error_code const& error)
      { return (boost::asio::error::connection_reset == error); }
    };
  }
}

#endif
