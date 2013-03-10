//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http_client.hpp"
#include "via/comms/tcp_adaptor.hpp"
#include <iostream>

// The http_client type. Note: this example uses a string for the Container.
typedef via::http_client<via::comms::tcp_adaptor, std::string> http_client_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  /// The response callback function.
  /// Prints the response headers and body to std::cout.
  /// @param response the http response headers
  /// @param body the body (if any) associated with the response.
  void response_receiver(via::http::rx_response const& response,
                         std::string const& body)
  {
    std::cout << response.to_string()
              << response.header().to_string()
              << body << std::endl;
  }

  /// The chunk callback function.
  /// Prints the chunk header and body to std::cout.
  /// Defined in case the site responds with a chunked message.
  /// @param chunk the http chunk header
  /// @param body the body (if any) associated with the chunk.
  void chunk_receiver(via::http::chunk_header const& chunk,
                      std::string const& body)
  {
    std::cout << chunk.to_string() << "\n"
              << body << std::endl;
  }
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  // Get a hostname and uri from the user (assume default http port)
  if (argc <= 2)
  {
    std::cout << "Usage: simple_http_client [host] [uri]\n"
              << "E.g. simple_http_client www.boost.org /LICENSE_1_0.txt"
              << std::endl;
    return 1;
  }

  std::string host_name(argv[1]);
  std::string uri(argv[2]);
  std::cout << "HTTP client host: " << host_name
            << " uri: " << uri << std::endl;
  try
  {
    // create an io_service for the tcp port
    boost::asio::io_service io_service;

    // create an http_client
    http_client_type::shared_pointer http_client
        (http_client_type::create(io_service));

    // connect the response and chunk received callback functions
    http_client->response_received_event(response_receiver);
    http_client->chunk_received_event(chunk_receiver);

    // attempt to connect to the host on the standard http port (80)
    if (http_client->connect(host_name))
    {
      // create an http request
      // Note: adds a "connection close" header since we're only sending
      // one request, so we'll get the server to close the connection after
      // it's response
      via::http::tx_request request(via::http::request_method::GET, uri);
      request.add_header(via::http::header_field::CONNECTION, "close");

      // send the request to the host.
      http_client->send(request);

      // run the io_service to start communications
      io_service.run();
    }
    else
    {
      std::cout << "Could not resolve host: " << host_name << std::endl;
      return 1;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////////
