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
#ifdef ASIO_STANDALONE
  #include <asio.hpp>
  #define ASIO asio
  #define ASIO_ERROR_CODE asio::error_code
#else
  #include <boost/asio.hpp>
  #define ASIO boost::asio
  #define ASIO_ERROR_CODE boost::system::error_code
#endif
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
    typedef std::function<void (ASIO_ERROR_CODE const&)>
      ErrorHandler;

    /// @typedef CommsHandler
    /// A (read or write) comms hander callback function type.
    /// @param error the (boost) error code.
    /// @param size the number of bytes read or written.
    typedef std::function<void (ASIO_ERROR_CODE const&, size_t)>
      CommsHandler;

    /// @typedef ConnectHandler
    /// A connect hander callback function type.
    /// @param error the (boost) error code.
    /// @param host_iterator the resolver_iterator
    typedef std::function<void (ASIO_ERROR_CODE const&,
                                ASIO::ip::tcp::resolver::iterator)>
      ConnectHandler;

    /// @typedef ConstBuffers
    /// A deque of asio::const_buffers.
    typedef std::deque<ASIO::const_buffer> ConstBuffers;
  }
}

#endif
