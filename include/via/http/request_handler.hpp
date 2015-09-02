#ifndef REQUEST_HANDLER_HPP_VIA_HTTPLIB_
#define REQUEST_HANDLER_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file request_handler.hpp
/// @brief Classes to handle HTTP requests.
//////////////////////////////////////////////////////////////////////////////
#include "via/http/request.hpp"
#include "via/http/response.hpp"
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace via
{
  namespace http
  {
    /// @class request_handler
    /// An abstract base template class for defining the interface to be
    /// provided by HTTP request handlers.
    /// The handle_request function is the pure virtual function that must be
    /// implemented by the derived classes.
    template <typename Container>
    class request_handler
        : public std::enable_shared_from_this<request_handler<Container>>
    {
    public:

      /// The function signature for the send response functions.
      typedef std::function<void (tx_response response,
                                  Container data)> SendResponseHandler;

      /// The function signature for the send chunk functions.
      typedef std::function<void (std::string const& uri,
                                  Container data)> SendChunkHandler;

    protected:

      /// The signal for asynchronous responses.
      SendResponseHandler send_response_handler_;

      /// The signal for asynchronous chunks.
      SendChunkHandler send_chunk_handler_;

    public:

      /// Constructor
      request_handler()
        : send_response_handler_()
        , send_chunk_handler_()
      {}

      /// Destructor
      virtual ~request_handler()
      {}

      /// The handle_request function to be implemented by derived classes.
      /// @param request the HTTP request.
      /// @param request_body the body of the HTTP request.
      /// @retval response_body the body for the HTTP response.
      /// @return the response header.
      virtual tx_response handle_request(rx_request const& request,
                                         Container const& request_body,
                                         Container& response_body) const = 0;

      /// Set the handler to send an HTTP response.
      void set_send_response_handler(SendResponseHandler handler)
      { send_response_handler_ = handler; }

      /// Set the handler to send an HTTP chunk.
      void set_send_chunk_handler(SendChunkHandler handler)
      { send_chunk_handler_ = handler; }

      /// Send the HTTP response.
      void send_response(tx_response response, Container data)
      {
        if (send_response_handler_)
          send_response_handler_(std::move(response), std::move(data));
      }

      /// Send the HTTP chunk.
      void send_chunk(std::string const& uri, Container data) const
      {
        if (send_chunk_handler_)
          send_chunk_handler_(uri, std::move(data));
      }
    };
  }
}

#endif // REQUEST_HANDLER_HPP_VIA_HTTPLIB_

