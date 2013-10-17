#pragma once

#ifndef UDP_ADAPTOR_HPP_VIA_HTTPLIB_
#define UDP_ADAPTOR_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file udp_adaptor.hpp
/// @brief The specific adaptor for udp connections.
//////////////////////////////////////////////////////////////////////////////
#include "socket_adaptor.hpp"

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class udp_adaptor
    /// This class enables the connection class to use udp sockets.
    //////////////////////////////////////////////////////////////////////////
    class udp_adaptor
    {
      /// A connection hander callback function type.
      /// @param error the (boost) error code.
      /// @param host_iterator the resolver_iterator
      typedef std::tr1::function<void (boost::system::error_code const&,
                                       boost::asio::ip::udp::resolver::iterator)>
                                             ConnectHandler;

      boost::asio::io_service& io_service_; ///< The asio io_service.
      /// The socket endpoint, used by normal / unconnected sockets.
      boost::asio::ip::udp::endpoint endpoint_;
      boost::asio::ip::udp::socket socket_; ///< The asio UDP socket.
      /// The host iterator used by the resolver.
      boost::asio::ip::udp::resolver::iterator host_iterator_;
      unsigned short tx_port_number_;
      bool is_connected_; ///< True if the socket is in "connected" mode.

      /// @fn resolve_host
      /// resolves the host name and port.
      /// @param host_name the host name.
      /// @param port_name the host port.
      boost::asio::ip::udp::resolver::iterator resolve_host
          (char const* host_name, char const* port_name) const
      {
        boost::asio::ip::udp::resolver resolver(io_service_);
        boost::asio::ip::udp::resolver::query query(host_name, port_name);
        return resolver.resolve(query);
      }

    protected:

      /// @fn handshake
      /// Performs the SSL handshake. Since this isn't an SSL socket, it just
      /// calls the handshake_handler with a success error code.
      /// @param handshake_handler the handshake callback function.
      /// @param is_server whether performing client or server handshaking,
      /// not used by un-encrypted sockets.
      void handshake(ErrorHandler handshake_handler, bool /*is_server*/ = false)
      {
        boost::system::error_code ec; // Default is success
        handshake_handler(ec);
      }

      /// @fn connect_socket
      /// Attempts to connect to the given resolver iterator.
      /// @param connect_handler the connect callback function.
      /// @param host_iterator the resolver iterator..
      void connect_socket(ConnectHandler connectHandler,
                          boost::asio::ip::udp::resolver::iterator host_iterator)
      { boost::asio::async_connect(socket_, host_iterator, connectHandler); }

      /// The udp_adaptor onstructor.
      /// @param io_service the asio io_service associted with this connection
      /// @param port_number the port number of a udp server.
      explicit udp_adaptor(boost::asio::io_service& io_service,
                           unsigned short port_number = 0) :
        io_service_(io_service),
        endpoint_(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),
                                                 port_number)),
        socket_(io_service_, endpoint_),
        host_iterator_(),
        tx_port_number_(0),
        is_connected_(false)
      {}

    public:

      /// The type of resolver iterator used by this socket.
      typedef boost::asio::ip::udp::resolver::iterator resolver_iterator;

      /// A virtual destructor because connection inherits from this class.
      virtual ~udp_adaptor()
      {}

      /// Set up the UDP socket for broadcast mode.
      /// @param port_number the UDP port to use in "normal" mode.
      void enable_broadcast_mode(unsigned short port_number)
      {
        socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        boost::asio::socket_base::broadcast option(true);
        socket_.set_option(option);
        endpoint_ = boost::asio::ip::udp::endpoint
                      (boost::asio::ip::address_v4::broadcast(), port_number);
      }

      /// Set transmit port number for a UDP socket in broadcast mode.
      /// Normally, the socket receives and transmits on the same port.
      /// However, some sockets (e.g. DHCP servers) receive and transmit on
      /// different ports.
      /// @param port_number for the UDP port to transmit on.
      void set_tx_port_number(unsigned short port_number)
      { tx_port_number_ = port_number; }

      /// @fn connect
      /// Connect the tcp socket to the given host name and port.
      /// @pre To be called by "client" connections only.
      /// Server connections are accepted by the server instead.
      /// @param host_name the host to connect to.
      /// @param port_name the port to connect to.
      /// @return true if able to connect to the port, false otherwise.
      bool connect(const char* host_name, const char* port_name,
                   ConnectHandler connectHandler)
      {
        host_iterator_ = resolve_host(host_name, port_name);
        if (host_iterator_ == boost::asio::ip::udp::resolver::iterator())
          return false;

        connect_socket(connectHandler, host_iterator_);
        return true;
      }

      /// @fn read
      /// The udp socket read function.
      /// @param ptr pointer to the receive buffer.
      /// @param size the size of the receive buffer.
      void read(void* ptr, size_t size, CommsHandler read_handler)
      {
        if (is_connected_)
          socket_.async_receive(boost::asio::buffer(ptr, size), read_handler);
        else
          socket_.async_receive_from
              (boost::asio::buffer(ptr, size), endpoint_, read_handler);
      }

      /// @fn write
      /// The udp socket write function.
      /// @param ptr pointer to the send buffer.
      /// @param size the size of the send buffer.
      void write(void const* ptr, size_t size, CommsHandler write_handler)
      {
        if (is_connected_)
          socket_.async_send(boost::asio::buffer(ptr, size), write_handler);
        else
        {
          boost::asio::ip::udp::endpoint tx_endpoint((tx_port_number_ == 0)
            ? endpoint_ : boost::asio::ip::udp::endpoint
              (boost::asio::ip::address_v4::broadcast (), tx_port_number_));
          socket_.async_send_to(boost::asio::buffer(ptr, size),
                                tx_endpoint, write_handler);
        }
      }

      /// @fn shutdown
      /// The udp socket shutdown function.
      /// Disconnects the socket.
      void shutdown()
      {
        boost::system::error_code ignoredEc;
        socket_.shutdown (boost::asio::ip::udp::socket::shutdown_both,
                          ignoredEc);
      }

      /// @fn close
      /// The udp socket close function.
      /// Cancels any send, receive or connect operations and closes the socket.
      void close()
      {
        boost::system::error_code ignoredEc;
        socket_.close (ignoredEc);
      }

      /// @fn start
      /// The udp socket start function.
      /// Signals that the socket is connected.
      void start(ErrorHandler handshake_handler)
      { handshake(handshake_handler, true); }

      /// @fn is_disconnect
      /// This function determines whether the error is a socket disconnect.
      /// @return true if a disconnect error, false otherwise.
      bool is_disconnect(boost::system::error_code const& error)
      { return (error == boost::asio::error::connection_refused); }

      /// @fn socket
      /// Accessor for the underlying tcp socket.
      /// @return a reference to the tcp socket.
      boost::asio::ip::udp::socket& socket()
      { return socket_; }

      /// Accessor for the last endpoint that the UDP socket received from.
      /// @return endpoint the last endpoint received by the connection
      const boost::asio::ip::udp::endpoint& endpoint() const
      { return endpoint_; }

    };

  }
}

#endif
