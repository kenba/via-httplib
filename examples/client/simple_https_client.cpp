//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file simple_https_client.cpp
/// @brief An example HTTPS client.
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/ssl/ssl_tcp_adaptor.hpp"
#include "via/http_client.hpp"
#include <iostream>

/// Define an HTTPS client using std::string to store message bodies
using https_client_type =
  via::http_client<via::comms::ssl::ssl_tcp_adaptor, std::string>;
using http_chunk_type = https_client_type::chunk_type;

namespace
{
  // An https_client.
  // Declared here so that it can be used in the response_handler and
  // chunk_handler.
  https_client_type::shared_pointer http_client;

  /// The handler for incoming HTTP requests.
  /// Prints the response.
  void response_handler(via::http::rx_response const& response,
                        std::string const& body)
  {
    std::cout << "Rx response: " << response.to_string();
    std::cout << "Rx headers: "  << response.headers().to_string();
    std::cout << "Rx body: "     << body << std::endl;

    if (!response.is_chunked())
      http_client.reset();
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
      http_client.reset();
    }
  }
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
  std::string uri(argv[2]);
  std::cout << app_name <<" host: " << host_name
            << " uri: " << uri << std::endl;
  try
  {
    // The asio io_service.
    boost::asio::io_service io_service;

    // Create an http_client
    http_client = https_client_type::create(io_service);

    // Set up SSL
    std::string certificate_file = "cacert.pem";
    boost::asio::ssl::context& ssl_context
       (https_client_type::connection_type::ssl_context());
    ssl_context.load_verify_file(certificate_file);

    // attach the response handler
    // and attempt to connect to the host on the standard https port (443)
    http_client->response_received_event(response_handler);
    http_client->chunk_received_event(chunk_handler);
    if (!http_client->connect(host_name, "https"))
    {
      std::cout << "Error, could not resolve host: " << host_name << std::endl;
      return 1;
    }

    // Create an http request and send it to the host.
    via::http::tx_request request(via::http::request_method::id::GET, uri);
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
