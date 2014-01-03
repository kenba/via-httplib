//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file example_http_server.cpp
/// @brief An example HTTP server containing all of the callbacks.
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_server.hpp"
#include <iostream>

/// Define an HTTP server using std::string to store message bodies
typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
typedef http_server_type::http_connection_type http_connection;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  /// The stop callback function.
  /// Exits the application.
  /// Called whenever a SIGINT, SIGTERM or SIGQUIT signal is received.
  void handle_stop()
  {
    std::cout << "Exit, shutting down" << std::endl;
    exit(0);
  }

  /// A string to send in responses.
  const std::string response_body(std::string("<html>\r\n")
                     + std::string("<head><title>Accepted</title></head>\r\n")
                     + std::string("<body><h1>200 Accepted</h1></body>\r\n")
                     + std::string("</html>\r\n"));


  /// A function to send a response to a request.
  void respond_to_request(http_connection::weak_pointer weak_ptr)
  {
    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
    {
      // Get the last request sent on this connection.
      via::http::rx_request const& request(connection->request());

      // The default response is 404 Not Found
      via::http::tx_response response(via::http::response_status::NOT_FOUND);
      response.add_server_header();
      response.add_date_header();
      if (request.uri() == "/hello")
      {
        if ((request.method() == "GET") || (request.method() == "PUT"))
          response.set_status(via::http::response_status::OK);
        else
        {
          response.set_status(via::http::response_status::METHOD_NOT_ALLOWED);
          response.add_header(via::http::header_field::ALLOW, "GET, PUT");
        }
      }

      if ((request.method() == "GET") &&
          (response.status() == via::http::response_status::OK))
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
  void request_handler(http_connection::weak_pointer weak_ptr,
                       via::http::rx_request const& request,
                       std::string const& body)
  {
    std::cout << "Rx request: " << request.to_string();
    std::cout << "Rx headers: " << request.headers().to_string();
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
  void chunk_handler(http_connection::weak_pointer weak_ptr,
                     via::http::rx_chunk const& chunk,
                     std::string const& body)
  {
    std::cout << "Rx chunk: " << chunk.to_string() << "\n";
    std::cout << "Rx body: "    << body << std::endl;

    // Only send a response to the last chunk.
    if (chunk.is_last())
      respond_to_request(weak_ptr);
  }

  /// A handler for HTTP requests containing an "Expect: 100-continue" header.
  /// Prints the request and determines whether the request is too big.
  /// It either responds with a 100 CONTINUE or a 413 REQUEST_ENTITY_TOO_LARGE
  /// response.
  void expect_continue_handler(http_connection::weak_pointer weak_ptr,
                               via::http::rx_request const& request,
                               std::string const& /* body */)
  {
    static const size_t MAX_LENGTH(1048576);

    std::cout << "expect_continue_handler\n";
    std::cout << "rx request: " << request.to_string();
    std::cout << "rx headers: " << request.headers().to_string() << std::endl;

    // Reject the message if it's too big, otherwise continue
    via::http::tx_response response((request.content_length() > MAX_LENGTH) ?
                       via::http::response_status::REQUEST_ENTITY_TOO_LARGE :
                       via::http::response_status::CONTINUE);
    weak_ptr.lock()->send(response);
  }

  /// A handler for the signal sent when an HTTP socket is disconnected.
  void disconnected_handler(http_connection::weak_pointer weak_ptr)
  {
    std::cout << "socket_disconnected_handler" << std::endl;
  }

}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  // Get a port number from the user
  if (argc <= 1)
  {
    std::cerr << "Usage: complete_http_server [port number]\n"
              << "E.g. complete_http_server 80"
              << std::endl;
    return 1;
  }

  std::string port(argv[1]);
  unsigned short portNumber(atoi(port.c_str()));
  std::cout << "HTTP server port: " << portNumber << std::endl;

  try
  {
    // create an io_service for the server
    boost::asio::io_service io_service;

    // The signal set is used to register for termination notifications
    boost::asio::signal_set signals_(io_service);
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // #if defined(SIGQUIT)
    signals_.async_wait(boost::bind(&handle_stop));

    // create an http_server
    http_server_type http_server(io_service);

    // connect the handler callback functions
    http_server.request_received_event(request_handler);
    http_server.chunk_received_event(chunk_handler);
    http_server.request_expect_continue_event(expect_continue_handler);
    http_server.socket_disconnected_event(disconnected_handler);

    // start accepting http connections on the given port
    boost::system::error_code error(http_server.accept_connections(portNumber));
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
      return 1;
    }

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
