//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
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
namespace via
{
	namespace http
	{
		const std::string CRLF("\r\n");
	}
}
namespace
{
  /// The handler for HTTP requests.
  /// Outputs the request.
  /// Responds with 200 OK with the client address in the body.
  void request_handler(http_connection::weak_pointer weak_ptr,
                       via::http::rx_request const& request,
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
  std::string certificate_file = "cacert.pem";
  std::string private_key_file = "privkey.pem";

  try
  {
    // The asio io_service.
    asio::io_service io_service;

    // Create the HTTP server and attach the request handler
    https_server_type https_server(io_service);
    https_server.request_received_event(request_handler);

    // Set up SSL
    https_server.set_password(password);
	std::error_code error
        (https_server_type::set_ssl_files(certificate_file, private_key_file));
    if (error)
    {
      std::cerr << "Error, set_ssl_files: "  << error.message() << std::endl;
      return 1;
    }

    // and accept IPV4 connections on the default port (443)
    error = https_server.accept_connections();
    if (error)
    {
      std::cerr << "Error, accept_connections: "  << error.message() << std::endl;
      return 1;
    }

    // Start the server
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
    return 1;
  }

  return 0;
}
