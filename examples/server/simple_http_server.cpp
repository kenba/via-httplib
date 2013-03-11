//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http_server.hpp"
#include "via/comms/tcp_adaptor.hpp"
#include <iostream>

/// The http_connection and http_server types.
/// Note: this example uses a string for the Container.
typedef via::http_connection<via::comms::tcp_adaptor, std::string>
                                                        http_connection_type;
typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  /// A function to send a response to a request.
  /// @param http_connection
  /// @param request
  void send_response(boost::weak_ptr<http_connection_type> http_connection,
                     via::http::rx_request const& request)
  {
    std::string body_text("<html>\r\n");
    body_text += "<head><title>Accepted</title></head>\r\n";
    body_text += "<body><h1>202 Accepted</h1></body>\r\n";
    body_text += "</html>\r\ntest_body";
    size_t body_length(body_text.size());

    via::http::tx_response response
        (via::http::response_status::OK, body_length);
    response.add_header(via::http::header_field::ACCEPT_CHARSET, "ISO-8859-1");
    if (request.method() !=
        via::http::request_method::name(via::http::request_method::HEAD))
      http_connection.lock()->send(response, body_text.begin(), body_text.end());
    else
      http_connection.lock()->send(response);
  }

  /// The request callback function.
  /// Prints the request headers and body to std::cout.
  /// @param http_connection
  /// @param request the http request headers
  /// @param body the body (if any) associated with the request.
  void request_receiver(http_connection_type::weak_pointer http_connection,
                        via::http::rx_request const& request,
                        std::string body)
  {
    std::cout << request.to_string()
              << request.header().to_string()
              << body << std::endl;

    send_response(http_connection, request);
  }

  /// The chunk callback function.
  /// Prints the chunk header and body to std::cout.
  /// Defined in case the request is a chunked message.
  /// @param http_connection
  /// @param chunk the http chunk header
  /// @param body the body (if any) associated with the chunk.
  void chunk_receiver(http_connection_type::weak_pointer http_connection,
                      via::http::chunk_header const& chunk,
                      std::string body)
  {
      std::cout << chunk.to_string() << "\n"
                << body << std::endl;
  }
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  // Get a port number from the user (the default is 80)
  if (argc <= 1)
  {
    std::cerr << "Usage: simple_http_server [port number]\n"
              << "E.g. simple_http_server 80"
              << std::endl;
    return 1;
  }

  std::string port(argv[1]);
  unsigned short portNumber(atoi(port.c_str()));
  std::cout << "HTTP server port: " << portNumber << std::endl;

  try
  {
    // create an io_service for the tcp port
    boost::asio::io_service io_service;

    // create an http_server
    http_server_type http_server(io_service, portNumber);

    // connect the request and chunk received callback functions
    http_server.request_received_event(request_receiver);
    http_server.chunk_received_event(chunk_receiver);

    // start accepting http connections
    http_server.start_accept();

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
