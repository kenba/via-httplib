//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_server.hpp"
#include "via/http/request_router.hpp"
#include <iostream>

/// Define an HTTP server using std::string to store message bodies
typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
typedef http_server_type::http_connection_type http_connection;

namespace
{
  using namespace via::http;

  tx_response get_hello_handler(rx_request const&, //request,
                                Parameters const&, //parameters,
                                std::string const&, // data,
                                std::string &response_body)
  {
    response_body += "Hello, whoever you are?!";
    return tx_response(response_status::code::OK);
  }

  tx_response get_hello_name_handler(rx_request const&, //request,
                                     Parameters const& parameters,
                                     std::string const&, // data,
                                     std::string &response_body)
  {
    response_body += "Hello, ";
    auto iter(parameters.find("name"));
    if (iter != parameters.end())
      response_body += iter->second;

    return tx_response(response_status::code::OK);
  }
}

int main(int /* argc */, char *argv[])
{
  std::string app_name(argv[0]);
  unsigned short port_number(via::comms::tcp_adaptor::DEFAULT_HTTP_PORT);
  std::cout << app_name << ": " << port_number << std::endl;

  try
  {
    // The asio io_service.
    boost::asio::io_service io_service;

    // Create the HTTP server, attach the request method handlers
    http_server_type http_server(io_service);

    http_server.add_method("GET", "/hello", get_hello_handler);
    http_server.add_method(via::http::request_method::GET, "/hello/:name",
                           get_hello_name_handler);

    // Accept IPV4 connections on the default port (80)
    boost::system::error_code error(http_server.accept_connections());
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
