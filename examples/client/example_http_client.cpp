//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file example_http_client.cpp
/// @brief An example HTTP client with chunk and disconnected handlers.
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_client.hpp"
#include <iostream>

/// Define an HTTP client using std::string to store message bodies
using http_client_type = via::http_client<via::comms::tcp_adaptor, std::string>;
using http_chunk_type  = http_client_type::chunk_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  /// The handler for incoming HTTP requests.
  /// Prints the response.
  void response_handler(via::http::rx_response const& response,
                        std::string const& body)
  {
    std::cout << "Rx response: " << response.to_string();
    std::cout << "Rx headers: "  << response.headers().to_string();
    std::cout << "Rx body: "     << body << std::endl;

    if (!response.is_chunked())
      exit(0);
  }

  /// The handler for incoming HTTP chunks.
  /// Prints the chunk header and data to std::cout.
  void chunk_handler(http_chunk_type const& chunk, std::string const& data)
  {
    std::cout << "Rx chunk: " << chunk.to_string();
    std::cout << "Chunk data: " << data << std::endl;

    if (chunk.is_last())
    {
      std::cout << "Rx last chunk" << std::endl;
      exit(0);
    }
  }

  /// The handler for the HTTP socket disconnecting.
  void disconnected_handler()
  {
    std::cout << "Socket disconnected" << std::endl;
    exit(0);
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
  std::string method(argv[2]);
  std::string uri(argv[3]);
  std::cout << app_name << " host: " << host_name
            << " method: " << method
            << " uri: " << uri << std::endl;
  try
  {
    // The asio io_service.
    boost::asio::io_service io_service;

    // Create an http_client
    http_client_type::shared_pointer http_client
        (http_client_type::create(io_service));

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
    via::http::tx_request request(method, uri);
    http_client->send(request);

    // run the io_service to start communications
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////////
