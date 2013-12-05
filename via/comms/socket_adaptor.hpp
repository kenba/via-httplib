#pragma once

#ifndef SOCKET_ADAPTOR_HPP_VIA_HTTPLIB_
#define SOCKET_ADAPTOR_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file socket_adaptor.hpp
/// @brief Type definitions used by connection socket adaptors.
//////////////////////////////////////////////////////////////////////////////
#include <boost/asio.hpp>
#include <boost/bind.hpp>

// if C++11 or Visual Studio 2010 or newer
#if ((__cplusplus >= 201103L) || (_MSC_VER >= 1600))
#include <functional>
#else
#include <tr1/functional>
#endif

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

    /// An error hander callback function type.
    /// @param error the (boost) error code.
#if ((__cplusplus >= 201103L) || (_MSC_VER >= 1600))
    typedef std::function<void (boost::system::error_code const&)>
#else
    typedef std::tr1::function<void (boost::system::error_code const&)>
#endif
                                           ErrorHandler;

    /// A (read or write) comms hander callback function type.
    /// @param error the (boost) error code.
    /// @param size the number of bytes read or written.
#if ((__cplusplus >= 201103L) || (_MSC_VER >= 1600))
    typedef std::function<void (boost::system::error_code const&, size_t)>
#else
    typedef std::tr1::function<void (boost::system::error_code const&, size_t)>
#endif
                                           CommsHandler;
  }
}

#endif
