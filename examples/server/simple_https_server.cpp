//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2023 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/ssl/ssl_tcp_adaptor.hpp"
#include "via/http_server.hpp"
#include <iostream>

/// Define an HTTPS server using std::string to store message bodies
typedef via::http_server<via::comms::ssl::ssl_tcp_adaptor, std::string>
                                                            https_server_type;
typedef https_server_type::http_connection_type http_connection;
typedef https_server_type::http_request http_request;

namespace
{
  /// The handler for HTTP requests.
  /// Outputs the request.
  /// Responds with 200 OK with the client address in the body.
  void request_handler(http_connection::weak_pointer weak_ptr,
                       http_request const& request,
                       std::string const& body)
  {
    std::cout << "Rx request: " << request.to_string();
    std::cout << request.headers().to_string();
    std::cout << "Rx body: "    << body << std::endl;

    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
    {
      // output the request
      via::http::tx_response response(via::http::response_status::code::OK);
      response.add_server_header();
      response.add_date_header();

      // respond with the client's address
      std::string response_body("Hello, ");
      response_body += connection->remote_address();
      connection->send(std::move(response), std::move(response_body));
    }
    else
      std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
  }
}

int main(int /* argc */, char *argv[])
{
  std::string app_name(argv[0]);
  unsigned short port_number(via::comms::ssl::ssl_tcp_adaptor::DEFAULT_HTTP_PORT);
  std::cout << app_name << ": " << port_number << std::endl;

  // The values for the SSL functions
  std::string password         = "test";
  std::string certificate_file = "server-certificate.pem";
  std::string private_key_file = "server-key.pem";

  // Set up SSL/TLS
  ASIO::ssl::context ssl_context(ASIO::ssl::context::tlsv13_server);
  ssl_context.set_options(ASIO::ssl::context_base::default_workarounds
                        | ASIO::ssl::context_base::no_sslv2);
  ssl_context.set_verify_mode(ASIO::ssl::verify_peer);

  ASIO_ERROR_CODE error;
  ssl_context.use_certificate_chain_file(certificate_file, error);
  if (error)
  {
    std::cerr << "Error, use_certificate_chain: " << error.message() << std::endl;
    return 1;
  }

  ssl_context.use_private_key_file(private_key_file, ASIO::ssl::context::pem, error);
  if (error)
  {
    std::cerr << "Error, use_private_key: " << error.message() << std::endl;
    return 1;
  }

  ssl_context.set_password_callback([password](std::size_t max_length,
      ASIO::ssl::context::password_purpose purpose)
      { return password; });

  try
  {
    // The asio io_context.
    ASIO::io_context io_context(1);

    // Create the HTTP server and attach the request handler
    https_server_type https_server(io_context, ssl_context);
    https_server.request_received_event(request_handler);

    // and accept IPV4 connections on the default port (443)
    error = https_server.accept_connections();
    if (error)
    {
      std::cerr << "Error, accept_connections: "  << error.message() << std::endl;
      return 1;
    }

    // Start the server
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
    return 1;
  }

  return 0;
}
