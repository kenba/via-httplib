#pragma once

#ifndef CONNECTION_HPP_VIA_HTTPLIB_
#define CONNECTION_HPP_VIA_HTTPLIB_
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include <boost/system/error_code.hpp>
#include <boost/signal.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace via
{
  namespace comms
  {
    //////////////////////////////////////////////////////////////////////////
    /// @class connection
    /// The abstract base class for all asynchronous (non-blocking) connection
    /// classes.
    //////////////////////////////////////////////////////////////////////////
    class connection : public boost::enable_shared_from_this<connection>
    {
    public:
      /// A signal to indicate that an event occured.
      typedef boost::signal<void (boost::weak_ptr<connection>)> event_signal;

      /// A signal to indicate that an error occured.
      typedef boost::signal<void (const boost::system::error_code&,
                                  boost::weak_ptr<connection>)> error_signal;

    private:
      /// The signals
      event_signal received_;
      event_signal sent_;
      event_signal connected_;
      event_signal disconnected_;
      event_signal receive_timedout_;
      event_signal connection_timedout_;

      error_signal error_;

    protected:

      /// Functions to signal events and errors from derived classes.
      void signal_received()
      { received_(shared_from_this()); }

      void signal_sent()
      { sent_(shared_from_this()); }

      void signal_connected()
      { connected_(shared_from_this()); }

      void signal_disconnected()
      { disconnected_(shared_from_this()); }

      void signal_receive_timedout()
      { receive_timedout_(shared_from_this()); }

      void signal_connection_timedout()
      { connection_timedout_(shared_from_this()); }

      /// Note: the signal error is declared virtual so that derived classes
      /// can interogate the error code to determine whether it is a
      /// disconnect.
      /// @param error the error_code for the error.
      virtual void signal_error(const boost::system::error_code& error)
      { error_(error, shared_from_this()); }

      ///
      virtual void read(void* ptr, size_t& size) = 0;

      ///
      virtual void write(void const* ptr, size_t size) = 0;

      ///
      virtual void read_handler(const boost::system::error_code& error,
                                size_t bytes_transferred)
      {
        if (error)
          signal_error(error);
        else
          received_(shared_from_this());
      }

      ///
      virtual void write_handler(const boost::system::error_code& error,
                                 size_t bytes_transferred)
      {
        if (error)
          signal_error(error);
        else
          sent_(shared_from_this());
      }

      virtual void stop()
      { }

      explicit connection() :
        received_(),
        sent_(),
        connected_(),
        disconnected_(),
        receive_timedout_(),
        connection_timedout_(),
        error_()
      {}

    public:

      virtual ~connection()
      { disconnect(); }

      virtual void enable_reception()
      {}

      void disconnect()
      { stop(); }

      void received_event(const event_signal::slot_type& slot)
      { received_.connect(slot); }

      void sent_event(const event_signal::slot_type& slot)
      { sent_.connect(slot); }

      void connected_event(const event_signal::slot_type& slot)
      { connected_.connect(slot); }

      void disconnected_event(const event_signal::slot_type& slot)
      { disconnected_.connect(slot); }

      void receive_timedout_event(const event_signal::slot_type& slot)
      { receive_timedout_.connect(slot); }

      void connection_timedout_event(const event_signal::slot_type& slot)
      { connection_timedout_.connect(slot); }

      void error_event(const error_signal::slot_type& slot)
      { error_.connect(slot); }

    };

  }
}
#endif
