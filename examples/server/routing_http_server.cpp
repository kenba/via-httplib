//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2023 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file routing_http_server.cpp
/// @brief An example HTTP server using the request_router.
//////////////////////////////////////////////////////////////////////////////
#include "via/http_server.hpp"
#include "via/http/request_router.hpp"
#include <iostream>

/// Define an HTTP server
typedef via::http_server<via::comms::tcp_socket> http_server_type;
typedef http_server_type::http_connection_type http_connection;
typedef http_server_type::http_request http_request;

namespace
{
  using namespace via::http;

  void connected_handler(std::weak_ptr<http_connection> weak_ptr)
  {
    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
      std::cout << "Connected to: "
                << connection->remote_address()
                << std::endl;
  }

  void disconnected_handler(std::weak_ptr<http_connection> weak_ptr)
  {
    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
      std::cout << "Disconnected from: "
                << connection->remote_address()
                << std::endl;
  }

  tx_response get_hello_handler(http_request const&, //request,
                                Parameters const&, //parameters,
                                std::vector<char> const&, // data,
                                std::vector<char> &response_body)
  {
    static const std::string text{"Hello, whoever you are?!"};
    response_body = std::vector<char>(text.cbegin(), text.cend());
    return tx_response(response_status::code::OK);
  }

  tx_response get_hello_name_handler(http_request const&, //request,
                                     Parameters const& parameters,
                                     std::vector<char> const&, // data,
                                     std::vector<char> &response_body)
  {
    std::string text{"Hello, "};
    auto iter(parameters.find("name"));
    if (iter != parameters.end())
      text += iter->second;

    response_body = std::vector<char>(text.cbegin(), text.cend());
    return tx_response(response_status::code::OK);
  }
}

int main(int /* argc */, char *argv[])
{
  std::string app_name(argv[0]);
  unsigned short port_number(http_server_type::connection_type::DEFAULT_HTTP_PORT);
  std::cout << app_name << ": " << port_number << std::endl;

  try
  {
    // The asio io_context.
    ASIO::io_context io_context;

    // Create the HTTP server, attach the request method handlers
    http_server_type http_server(io_context);

    http_server.socket_connected_event(connected_handler);
    http_server.socket_disconnected_event(disconnected_handler);

    http_server.request_router().add_method("GET", "/hello", get_hello_handler);
    http_server.request_router().add_method
                          (via::http::request_method::GET, "/hello/:name",
                           get_hello_name_handler);

    // Accept IPV4 connections on the default port (80)
    ASIO_ERROR_CODE error(http_server.accept_connections());
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
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
