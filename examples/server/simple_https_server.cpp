//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
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

namespace
{
  /// The handler for incoming HTTP requests.
  /// Prints the request and responds with 200 OK.
  void request_handler(http_connection::weak_pointer weak_ptr,
                       via::http::rx_request const& request,
                       std::string const& body)
  {
    std::cout << "Rx request: " << request.to_string();
    std::cout << "Rx headers: " << request.headers().to_string();
    std::cout << "Rx body: "    << body << std::endl;

    via::http::tx_response response(via::http::response_status::code::OK);
    response.add_server_header();
    response.add_date_header();
    weak_ptr.lock()->send(response);
  }
}

int main(int argc, char *argv[])
{
  // The values for the SSL functions
  std::string password         = "test";
  std::string certificate_file = "cacert.pem";
  std::string private_key_file = "privkey.pem";

  try
  {
    // The asio io_service.
    boost::asio::io_service io_service;

    // Create the HTTP server
    https_server_type https_server(io_service);

    // Set up SSL
    https_server.set_password(password);
    boost::system::error_code error
        (https_server_type::set_ssl_files(certificate_file, private_key_file));
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
      return 1;
    }

    // attach the request handler
    https_server.request_received_event(request_handler);

    // and accept IPV4 connections on the default port (443)
    error = https_server.accept_connections();
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
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
