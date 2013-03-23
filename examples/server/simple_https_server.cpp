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

/// The http_connection and http_server types.
/// Note: this example uses a string for the Container.
typedef via::http_connection<via::comms::ssl::ssl_tcp_adaptor, std::string>
  http_connection_type;
typedef via::http_server<via::comms::ssl::ssl_tcp_adaptor, std::string>
  http_server_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  /// The request callback function.
  /// Prints the request headers and body to std::cout.
  /// @param http_connection
  /// @param request the http request headers
  /// @param body the body (if any) associated with the request.
  void request_receiver(http_connection_type::weak_pointer connection,
                        via::http::rx_request const& request,
                        std::string const& body)
  {
    std::cout << request.to_string()
              << request.header().to_string()
              << body << std::endl;

    if (!request.is_chunked())
    {
      http_connection_type::shared_pointer pointer(connection.lock());
      if (pointer)
      {
        via::http::tx_response response(via::http::response_status::OK);
        pointer->send(response);
      }
      else
        std::cerr << "Failed to lock connection weak_pointer" << std::endl;
    }
  }

  /// The chunk callback function.
  /// Prints the chunk header and body to std::cout.
  /// Defined in case the request is a chunked message.
  /// @param http_connection
  /// @param chunk the http chunk header
  /// @param body the body (if any) associated with the chunk.
  void chunk_receiver(http_connection_type::weak_pointer connection,
                      via::http::rx_chunk const& chunk,
                      std::string const& body)
  {
    std::cout << chunk.to_string() << "\n"
              << body << std::endl;

    if (chunk.is_last())
    {
      // Get
      http_connection_type::shared_pointer pointer(connection.lock());
      if (pointer)
      {
        via::http::tx_response response(via::http::response_status::OK);
        pointer->send(response);
      }
      else
        std::cerr << "Failed to lock connection weak_pointer" << std::endl;
    }
  }

  /// The stop callback function.
  /// Exits the application.
  /// Called whenever a SIGINT, SIGTERM or SIGQUIT signal is received.
  void handle_stop()
  {
    std::cout << "Exit, shutting down" << std::endl;
    exit(0);
  }
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  // Get a port number from the user
  if (argc <= 1)
  {
    std::cerr << "Usage: simple_https_server [port number]\n"
              << "E.g. simple_http_server 80"
              << std::endl;
    return 1;
  }

  // The values for the SSL functions
  std::string password         = "test";
  std::string certificate_file = "cacert.pem";
  std::string private_key_file = "privkey.pem";

  std::string port(argv[1]);
  unsigned short portNumber(atoi(port.c_str()));
  std::cout << "HTTPS server port: " << portNumber << std::endl;

  try
  {
    // create an io_service for the tcp port
    boost::asio::io_service io_service;

    // create an http_server
    http_server_type http_server(io_service, portNumber);

    // Set up SSL
    http_server.set_password(password);
    boost::system::error_code error
        (http_server_type::set_ssl_files(certificate_file, private_key_file));
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
      exit(1);
    }

    // connect the request and chunk received callback functions
    http_server.request_received_event(request_receiver);
    http_server.chunk_received_event(chunk_receiver);

    // start accepting http connections
    http_server.start_accept();

    // The signal set is used to register for termination notifications
    boost::asio::signal_set signals_(io_service);
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // #if defined(SIGQUIT)
    signals_.async_wait(boost::bind(&handle_stop));

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
