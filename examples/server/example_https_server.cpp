//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file example_https_server.cpp
/// @brief An example HTTPS server containing all of the callbacks.
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/ssl/ssl_tcp_adaptor.hpp"
#include "via/http_server.hpp"
#include <iostream>

/// Define an HTTPS server using std::string to store message bodies
typedef via::http_server<via::comms::ssl::ssl_tcp_adaptor, std::string>
                                                            https_server_type;
typedef https_server_type::http_connection_type https_connection;
typedef https_server_type::chunk_type http_chunk_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  /// The stop callback function.
  /// Exits the application.
  /// Called whenever a SIGINT, SIGTERM or SIGQUIT signal is received.
  void handle_stop(boost::system::error_code const&, // error,
                   int, // signal_number,
                   https_server_type* http_server)
  {
    std::cout << "Shutting down" << std::endl;
    http_server->close();
  }

  /// A string to send in responses.
  const std::string response_body(std::string("<html>\r\n")
                     + std::string("<head><title>Accepted</title></head>\r\n")
                     + std::string("<body><h1>200 Accepted</h1></body>\r\n")
                     + std::string("</html>\r\n"));

  /// A function to send a response to a request.
  void respond_to_request(https_connection::weak_pointer weak_ptr)
  {
    https_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
    {
      // Get the last request sent on this connection.
      via::http::rx_request const& request(connection->request());

      // The default response is 404 Not Found
      via::http::tx_response response(via::http::response_status::code::NOT_FOUND);
      response.add_server_header();
      response.add_date_header();
      if (request.uri() == "/hello")
      {
        if ((request.method() == "GET") || (request.method() == "PUT"))
          response.set_status(via::http::response_status::code::OK);
        else
        {
          response.set_status(via::http::response_status::code::METHOD_NOT_ALLOWED);
          response.add_header(via::http::header_field::id::ALLOW, "GET, PUT");
        }
      }

      if ((request.method() == "GET") &&
          (response.status() == static_cast<int>(via::http::response_status::code::OK)))
        connection->send(response, response_body);
      else
        connection->send(response);
    }
    else
      std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
  }

  /// The handler for incoming HTTP requests.
  /// Prints the request and determines whether the request is chunked.
  /// If not, it responds with a 200 OK response with some HTML in the body.
  void request_handler(https_connection::weak_pointer weak_ptr,
                       via::http::rx_request const& request,
                       std::string const& body)
  {
    std::cout << "Rx request: " << request.to_string();
    std::cout << request.headers().to_string();
    std::cout << "Rx body: "    << body << std::endl;

    if (!request.is_chunked())
      respond_to_request(weak_ptr);
  }

  /// The handler for incoming HTTP chunks.
  /// Prints the chunk header and body to std::cout.
  /// Defined in case the request is a chunked message.
  /// @param http_connection
  /// @param chunk the http chunk header
  /// @param body the body (if any) associated with the chunk.
  void chunk_handler(https_connection::weak_pointer weak_ptr,
                     http_chunk_type const& chunk,
                     std::string const& data)
  {
    // Only send a response to the last chunk.
    if (chunk.is_last())
    {
      std::cout << "Rx chunk is last, extension: " << chunk.extension()
                << " trailers: " << chunk.trailers().to_string() << std::endl;
      respond_to_request(weak_ptr);
    }
    else
      std::cout << "Rx chunk, size: " << chunk.size()
                << " data: " << data << std::endl;
  }

  /// A handler for HTTP requests containing an "Expect: 100-continue" header.
  /// Prints the request and determines whether the request is too big.
  /// It either responds with a 100 CONTINUE or a 413 REQUEST_ENTITY_TOO_LARGE
  /// response.
  void expect_continue_handler(https_connection::weak_pointer weak_ptr,
                               via::http::rx_request const& request,
                               std::string const& /* body */)
  {
    static const size_t MAX_LENGTH(1048576);

    std::cout << "expect_continue_handler\n";
    std::cout << "Rx request: " << request.to_string();
    std::cout << request.headers().to_string() << std::endl;

    // Reject the message if it's too big, otherwise continue
    via::http::tx_response response((request.content_length() > MAX_LENGTH) ?
                       via::http::response_status::code::REQUEST_ENTITY_TOO_LARGE :
                       via::http::response_status::code::CONTINUE);
    weak_ptr.lock()->send(response);
  }

  /// A handler for the signal sent when an HTTP socket is disconnected.
  void disconnected_handler(https_connection::weak_pointer weak_ptr)
  {
    std::cout << "Disconnected: " << weak_ptr.lock()->remote_address() << std::endl;
  }
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  std::string app_name(argv[0]);
  unsigned short port_number(via::comms::ssl::ssl_tcp_adaptor::DEFAULT_HTTP_PORT);

  // Get a port number from the user
  if (argc > 2)
  {
    std::cerr << "Usage: " << app_name << " [port number]\n"
              << "E.g. "   << app_name << " " << port_number
              << std::endl;
    return 1;
  }
  else if (argc == 2)
  {
    std::string port(argv[1]);
    port_number = atoi(port.c_str());
  }

  std::cout << app_name << ": " << port_number << std::endl;

  // The values for the SSL functions
  std::string password         = "test";
  std::string certificate_file = "cacert.pem";
  std::string private_key_file = "privkey.pem";

  try
  {
    // create an io_service for the server
    boost::asio::io_service io_service;

    // create an https_server
    https_server_type https_server(io_service, request_handler);

    // Set up SSL
    https_server.set_password(password);
    boost::system::error_code error
        (https_server_type::set_ssl_files(certificate_file, private_key_file));
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
      return 1;
    }

    // connect the handler callback functions
    https_server.chunk_received_event(chunk_handler);
    https_server.request_expect_continue_event(expect_continue_handler);
    https_server.socket_disconnected_event(disconnected_handler);

    https_server.set_auto_disconnect(true);

    // start accepting http connections on the given port
    error = https_server.accept_connections(port_number);
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
      return 1;
    }

    // The signal set is used to register for termination notifications
    boost::asio::signal_set signals_(io_service);
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // #if defined(SIGQUIT)

    // register the handle_stop callback
    signals_.async_wait(boost::bind(&handle_stop,
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::signal_number,
                                    &https_server));

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
