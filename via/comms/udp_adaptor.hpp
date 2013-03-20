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
      boost::asio::io_service& io_service_; ///< The asio io_service.
      /// The socket endpoint, used by normal / unconnected sockets.
      boost::asio::ip::udp::endpoint endpoint_;
      boost::asio::ip::udp::socket socket_; ///< The asio UDP socket.
      unsigned short tx_port_number_;
      bool is_connected_; ///< True if the socket is in "connected" mode.
      CommsHandler read_handler_;  ///< The read callback function.
      CommsHandler write_handler_; ///< The write callback function.
      EventHandler event_handler_; ///< The event callback function.
      ErrorHandler error_handler_; ///< The error callback function.

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

      /// @fn handle_connect
      /// The connect callback function.
      /// @param error the boost asio error code.
      void handle_connect(boost::system::error_code const& error)
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

    public:

      /// The udp_adaptor onstructor.
      /// @param io_service the asio io_service associted with this connection
      /// @param read_handler the read callback function.
      /// @param write_handler the write callback function.
      /// @param event_handler the event handler callback function.
      /// @param error_handler the error handler callback function.
      /// @param port_number the port number of a udp server.
      explicit udp_adaptor(boost::asio::io_service& io_service,
                           CommsHandler read_handler,
                           CommsHandler write_handler,
                           EventHandler event_handler,
                           ErrorHandler error_handler,
                           unsigned int port_number = 0) :
        io_service_(io_service),
        endpoint_(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),
                                                 port_number)),
        socket_(io_service_, endpoint_),
        tx_port_number_(0),
        is_connected_(false),
        read_handler_(read_handler),
        write_handler_(write_handler),
        event_handler_(event_handler),
        error_handler_(error_handler)
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
        start();
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
      bool connect(const char* host_name, const char* port_name)
      {
        boost::asio::ip::udp::resolver::iterator host_iterator
            (resolve_host(host_name, port_name));
        if (host_iterator == boost::asio::ip::udp::resolver::iterator())
          return false;

        endpoint_ = *host_iterator;
        socket_.async_connect(endpoint_,
                              boost::bind(&udp_adaptor::handle_connect, this,
                                          boost::asio::placeholders::error));

        return true;
      }

      /// @fn read
      /// The udp socket read function.
      /// @param ptr pointer to the receive buffer.
      /// @param size the size of the receive buffer.
      void read(void* ptr, size_t& size)
      {
        if (is_connected_)
          socket_.async_receive(boost::asio::buffer(ptr, size), read_handler_);
        else
          socket_.async_receive_from
              (boost::asio::buffer(ptr, size), endpoint_, read_handler_);
      }

      /// @fn write
      /// The udp socket write function.
      /// @param ptr pointer to the send buffer.
      /// @param size the size of the send buffer.
      void write(void const* ptr, size_t size)
      {
        if (is_connected_)
          socket_.async_send(boost::asio::buffer(ptr, size), write_handler_);
        else
        {
          boost::asio::ip::udp::endpoint tx_endpoint((tx_port_number_ == 0)
            ? endpoint_ : boost::asio::ip::udp::endpoint
              (boost::asio::ip::address_v4::broadcast (), tx_port_number_));
          socket_.async_send_to(boost::asio::buffer(ptr, size),
                                tx_endpoint, write_handler_);
        }
      }

      /// @fn stop
      /// The udp socket stop function.
      /// Disconnects the socket.
      void stop()
      {
        boost::system::error_code ignoredEc;
        socket_.shutdown (boost::asio::ip::udp::socket::shutdown_both,
                          ignoredEc);
        socket_.close(ignoredEc);
      }

      /// @fn start
      /// The udp socket start function.
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
