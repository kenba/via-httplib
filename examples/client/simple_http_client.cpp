//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2023 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file simple_http_client.cpp
/// @brief An example HTTP client.
//////////////////////////////////////////////////////////////////////////////
#include "via/http_client.hpp"
#include <iostream>

/// Define an HTTP client
typedef via::http_client<via::comms::tcp_socket> http_client_type;
typedef http_client_type::http_response http_response;
typedef http_client_type::chunk_type http_chunk_type;

namespace
{
  // An http_client.
  // Declared here so that it can be used in the connected_handler,
  // response_handler and chunk_handler.
  http_client_type::shared_pointer http_client;

  // The uri from the user
  std::string uri;

  /// A handler for the signal sent when an HTTP socket is connected.
  void connected_handler()
  {
    // Create an http GET request and send it to the host.
    // Note: via-httplib will add a host header with the host name
    // given in the call to connect
    via::http::tx_request request(via::http::request_method::id::GET, uri);
    http_client->send(std::move(request));
  }

  /// The handler for incoming HTTP requests.
  /// Prints the response.
  void response_handler(http_response const& response,
                        std::vector<char> const& body)
  {
    std::cout << "Rx response: " << response.to_string()
              << response.headers().to_string();
    std::cout << "Rx body: "     << std::string_view(body.data(), body.size()) << std::endl;

    if (!response.is_chunked())
      http_client->disconnect();
  }

  /// The handler for incoming HTTP chunks.
  /// Prints the chunk header and data to std::cout.
  void chunk_handler(http_chunk_type const& chunk, std::vector<char> const& data)
  {
    if (chunk.is_last())
    {
      std::cout << "Rx chunk is last, extension: " << chunk.extension()
                << " trailers: " << chunk.trailers().to_string() << std::endl;
      http_client->disconnect();
    }
    else
      std::cout << "Rx chunk, size: " << chunk.size()
                << " data: " << std::string_view(data.data(), data.size()) << std::endl;
  }

  /// A handler for the signal sent when an HTTP socket is disconnected.
  void disconnected_handler()
  { std::cout << "Socket disconnected" << std::endl; }
}

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
  uri = argv[2];
  std::cout << app_name <<" host: " << host_name
            << " uri: " << uri << std::endl;
  try
  {
    // The asio io_context.
    ASIO::io_context io_context;

    // Create an http_client and attach the response & chunk handlers
    http_client =
        http_client_type::create(io_context, response_handler, chunk_handler);

    // attach optional handlers
    http_client->connected_event(connected_handler);
    http_client->disconnected_event(disconnected_handler);

    // attempt to connect to the host on the standard http port (80)
    if (!http_client->connect(host_name))
    {
      std::cout << "Error, could not resolve host: " << host_name << std::endl;
      return 1;
    }

    // run the io_context to start communications
    io_context.run();

    http_client.reset();

    std::cout << "io_context.run complete, shutdown successful" << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////////
