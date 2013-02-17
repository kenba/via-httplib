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
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#ifdef _MSC_VER
#include <functional>
#else
#include <tr1/functional>
#endif

namespace via
{
  namespace comms
  {
    enum event_type
    {
      CONNECTED,
      RECEIVED,
      SENT,
      DISCONNECTED
    };

    typedef std::tr1::function<void (int)> EventHandler;

    typedef std::tr1::function<void (boost::system::error_code const&)>
                                           ErrorHandler;

    typedef std::tr1::function<void (boost::system::error_code const&,
                                     size_t)> CommsHandler;
  }
}

#endif
