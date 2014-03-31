//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file chunked_http_client.cpp
/// @brief An example HTTP client to send and receive chunks.
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_client.hpp"
#include <sstream>
#include <iostream>

/// Define an HTTP client using std::string to store message bodies
typedef via::http_client<via::comms::tcp_adaptor, std::string> http_client_type;
typedef http_client_type::chunk_type http_chunk_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  /// The number of chunks to send in a request.
  const int CHUNKS_TO_SEND(5);

  /// The number of chunks sent so far.
  int count(0);

  /// The period to call the timeout handler in milliseconds.
  unsigned int TIMEOUT_PERIOD(100);

  /// A deadline timer to send the chunks to the server.
  boost::shared_ptr<boost::asio::deadline_timer> chunk_timer;

  // An http_client.
  // Declared here so that it can be used in the response_handler and
  // send_a_chunk function.
  http_client_type::shared_pointer http_client;

  /// Something to send in the chunks.
  const std::string chunk_text("HTTP chunk number: ");

  /// Send a chunnk to the server.
  bool send_a_chunk()
  {
    if (++count < CHUNKS_TO_SEND)
    {
      std::stringstream chunk_stream;
      chunk_stream << chunk_text;
      chunk_stream << count << "\n" << std::ends;

      std::string chunk_to_send(chunk_stream.str());

      std::cout << "chunk_to_send: " << chunk_to_send << std::endl;

      http_client->send_chunk(chunk_to_send);
      return true;
    }
    else
      http_client->last_chunk();

    return false;
  }

  /// A timeout callback function. Used to send the chunks.
  void timeout_handler(const boost::system::error_code& ec)
  {
    if (ec != boost::asio::error::operation_aborted)
    {
      if (send_a_chunk())
      {
        // reset the timer to call this function again
        chunk_timer->expires_from_now
            (boost::posix_time::milliseconds(TIMEOUT_PERIOD));
        chunk_timer->async_wait(boost::bind(&timeout_handler,
                                boost::asio::placeholders::error));
      }
      else
        chunk_timer->cancel();
    }
  }

  /// The handler for incoming HTTP responses.
  /// Prints the response.
  void response_handler(via::http::rx_response const& response,
                        std::string const& body)
  {
    std::cout << "Rx response: " << response.to_string();
    std::cout << "Rx headers: "  << response.headers().to_string();
    std::cout << "Rx body: "     << body << std::endl;

    if (response.is_continue())
    {
      std::cout << "Rx is CONTINUE" << std::endl;

      chunk_timer->expires_from_now
          (boost::posix_time::milliseconds(TIMEOUT_PERIOD));
      chunk_timer->async_wait(boost::bind(&timeout_handler,
                              boost::asio::placeholders::error));
    }
    else
    {
      if (!response.is_chunked())
        http_client.reset();
    }
  }

  /// The handler for incoming HTTP chunks.
  /// Prints the chunk header and data to std::cout.
  void chunk_handler(http_chunk_type const& chunk, std::string const& data)
  {
    std::cout << "Rx chunk: " << chunk.to_string() << "\n";
    std::cout << "Rx data: "  << data << std::endl;

    if (chunk.is_last())
    {
      std::cout << "Rx last chunk" << std::endl;
      http_client.reset();
    }
  }

  /// The handler for the HTTP socket disconnecting.
  void disconnected_handler()
  {
    std::cout << "Socket disconnected" << std::endl;
    chunk_timer->cancel();
    http_client.reset();
  }
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  std::string app_name(argv[0]);

  // Get a hostname and uri from the user (assume default http port)
  if (argc != 3)
  {
    std::cout << "Usage: " << app_name << " [host] [uri]\n"
              << "E.g. "   << app_name << " localhost /hello"
              << std::endl;
    return 1;
  }

  std::string host_name(argv[1]);
  std::string uri(argv[2]);
  std::cout << app_name <<" host: " << host_name
            << " uri: " << uri << std::endl;
  try
  {
    // The asio io_service.
    boost::asio::io_service io_service;

    /// A deadline timer to send the chunks to the server.
    chunk_timer.reset
      (new boost::asio::deadline_timer(io_service,
                             boost::posix_time::milliseconds(TIMEOUT_PERIOD)));

    // Create an http_client
    http_client = http_client_type::create(io_service);

    // Attach the callback handlers
    // and attempt to connect to the host on the standard http port (80)
    http_client->response_received_event(response_handler);
    http_client->chunk_received_event(chunk_handler);
    http_client->disconnected_event(disconnected_handler);
    if (!http_client->connect(host_name))
    {
      std::cout << "Could not resolve host: " << host_name << std::endl;
      return 1;
    }

    // Create an http request and send it to the host.
    via::http::tx_request request(via::http::request_method::id::PUT, uri);
    request.add_header(via::http::header_field::id::TRANSFER_ENCODING, "Chunked");
    request.add_header(via::http::header_field::id::EXPECT, "100-continue");
    http_client->send(request);

    // run the io_service to start communications
    io_service.run();

    std::cout << "io_service.run, all work has finished" << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////////
