//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file example_http_client.cpp
/// @brief An example HTTP client with optional handlers.
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_client.hpp"
#include <iostream>

/// Define an HTTP client using std::string to store message bodies
typedef via::http_client<via::comms::tcp_adaptor, std::string> http_client_type;
typedef http_client_type::chunk_type http_chunk_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  // An http_client.
  // Declared here so that it can be used in the connected_handler,
  // response_handler and chunk_handler.
  http_client_type::shared_pointer http_client;

  // The method from the user
  std::string method;

  // The uri from the user
  std::string uri;

  /// A handler for the signal sent when an HTTP socket is connected.
  void connected_handler()
  {
    // Create an http request and send it to the host.
    // Note: via-httplib will add a host header with the host name
    // given in the call to connect
    via::http::tx_request request(method, uri);
    http_client->send(std::move(request));
  }

  /// The handler for incoming HTTP requests.
  /// Prints the response.
  void response_handler(via::http::rx_response const& response,
                        std::string const& body)
  {
    std::cout << "Rx response: " << response.to_string()
              << response.headers().to_string();
    std::cout << "Rx body: "     << body << std::endl;

    if (!response.is_chunked())
      http_client->disconnect();
  }

  /// The handler for incoming HTTP chunks.
  /// Prints the chunk header and data to std::cout.
  void chunk_handler(http_chunk_type const& chunk, std::string const& data)
  {
    if (chunk.is_last())
    {
      std::cout << "Rx chunk is last, extension: " << chunk.extension()
                << " trailers: " << chunk.trailers().to_string() << std::endl;
      http_client->disconnect();
    }
    else
      std::cout << "Rx chunk, size: " << chunk.size()
                << " data: " << data << std::endl;
  }

  /// The handler for invalid HTTP requests.
  /// Outputs the last receive buffer contents
  void invalid_response_handler(via::http::rx_response const&, // response,
                                std::string const&) // body)
  {
    std::cout << "Invalid response: "
              << http_client->rx_buffer() << std::endl;
    http_client->disconnect();
  }

  /// The handler for the HTTP socket disconnecting.
  void disconnected_handler()
  {
    std::cout << "Socket disconnected" << std::endl;
  }

  /// A handler for the signal when a message is sent.
  void message_sent_handler()
  {
    std::cout << "request sent" << std::endl;
  }
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  std::string app_name(argv[0]);

  // Get a hostname, method and uri from the user (assume default http port)
  if (argc != 4)
  {
    std::cout << "Usage: " << app_name << " [host] [method] [uri]\n"
              << "E.g. "   << app_name << " localhost GET /hello"
              << std::endl;
    return 1;
  }

  std::string host_name(argv[1]);
  method = argv[2];
  uri    = argv[3];
  std::cout << app_name << " host: " << host_name
            << " method: " << method
            << " uri: " << uri << std::endl;
  try
  {
    // The asio io_service.
    ASIO::io_service io_service;

    // Create an http_client and attach the response & chunk handlers
    http_client =
        http_client_type::create(io_service, response_handler, chunk_handler);

    // attach the optional handlers
    http_client->invalid_response_event(invalid_response_handler);
    http_client->connected_event(connected_handler);
    http_client->disconnected_event(disconnected_handler);
    http_client->message_sent_event(message_sent_handler);

    // set tcp keep alive
    http_client->connection()->set_keep_alive(true);

    // set the connection buffer sizes
    http_client->connection()->set_rx_buffer_size(16384);
    http_client->connection()->set_receive_buffer_size(16384);
    http_client->connection()->set_send_buffer_size(16384);

    // attempt to connect to the host on the standard http port (80)
    if (!http_client->connect(host_name))
    {
      std::cout << "Error, could not resolve host: " << host_name << std::endl;
      return 1;
    }

    // run the io_service to start communications
    io_service.run();

    http_client.reset();

    std::cout << "io_service.run complete, shutdown successful" << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////////
