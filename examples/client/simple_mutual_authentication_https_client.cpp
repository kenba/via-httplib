//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Ken Barker
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
typedef via::http_client<via::comms::ssl::ssl_tcp_adaptor, std::string>
                                                            https_client_type;
typedef https_client_type::http_response http_response;
typedef https_client_type::chunk_type http_chunk_type;

namespace
{
  // An https_client.
  // Declared here so that it can be used in the connected_handler,
  // response_handler and chunk_handler.
  https_client_type::shared_pointer http_client;

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

  /// A handler for the signal sent when an HTTP socket is disconnected.
  void disconnected_handler()
  {
    std::cout << "Socket disconnected" << std::endl;
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
  uri = argv[2];
  std::cout << app_name <<" host: " << host_name
            << " uri: " << uri << std::endl;

  // Values for the SSL functions
  std::string password         = "test";
  std::string verify_file      = "ca-crt.pem";
  std::string certificate_file = "clientcert.pem";
  std::string private_key_file = "clientkey.pem";

  // Set up SSL/TLS
  ASIO::ssl::context ssl_context(ASIO::ssl::context::tlsv13_client);
  ssl_context.set_options(ASIO::ssl::context_base::default_workarounds
                        | ASIO::ssl::context_base::no_sslv2);
  
  // Client does NOT need to set verify_fail_if_no_peer_cert for mutual authentication
  ssl_context.set_verify_mode(ASIO::ssl::context::verify_peer);
  ssl_context.load_verify_file(verify_file);

  // Load client certificate and key for mutual authentication
  ssl_context.use_certificate_file(certificate_file, ASIO::ssl::context::pem);
  ssl_context.use_private_key_file(private_key_file, ASIO::ssl::context::pem);
  ssl_context.set_password_callback([password](std::size_t max_length,
      ASIO::ssl::context::password_purpose purpose)
      { return password; });

  try
  {
    // The asio io_context.
    ASIO::io_context io_context(1);

    // Create an http_client and attach the response & chunk handlers
    http_client =
        https_client_type::create(io_context, ssl_context, response_handler, chunk_handler);

    // attach optional handlers
    http_client->connected_event(connected_handler);
    http_client->disconnected_event(disconnected_handler);

    // attempt to connect to the host on the standard https port (443)
    if (!http_client->connect(host_name, "https"))
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
