//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file chunked_http_client.cpp
/// @brief An example HTTP client to send and receive chunks.
//////////////////////////////////////////////////////////////////////////////
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_client.hpp"
#include <sstream>
#include <iostream>

/// Define an HTTP client using std::string to store message bodies
using http_client_type = via::http_client<via::comms::tcp_adaptor, std::string>;
using http_chunk_type  = http_client_type::chunk_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  /// The number of chunks to send in a request.
  const int CHUNKS_TO_SEND(5);

  /// The number of chunks remaining.
  int count(0);

  // An http_client.
  // Declared here so that it can be used in the response_handler and
  // send_a_chunk function.
  http_client_type::shared_pointer http_client;

  /// Something to send in the chunks.
  const std::string chunk_text("HTTP chunk number: ");

  /// Send a chunnk to the server.
  bool send_a_chunk()
  {
    if (--count > 0)
    {
      std::stringstream chunk_stream;
      chunk_stream << chunk_text;
      chunk_stream << CHUNKS_TO_SEND - count << "\n" << std::ends;

      std::string chunk_to_send(chunk_stream.str());

      std::cout << "chunk_to_send: " << chunk_to_send << std::endl;

      http_client->send_chunk(chunk_to_send);
      return true;
    }
    else
    {
      std::cout << "last_chunk" << std::endl;
      http_client->last_chunk();
    }

    return false;
  }

  /// The handler for the HTTP socket disconnecting.
  void msg_sent_handler()
  {
    std::cout << "msg_sent_handler" << std::endl;

    if (count > 0)
      send_a_chunk();
  }

  /// The handler for incoming HTTP responses.
  /// Prints the response.
  void response_handler(via::http::rx_response const& response,
                        std::string const& body)
  {
    std::cout << "Rx response: " << response.to_string();
    std::cout << "Rx headers: "  << response.headers().to_string();
    std::cout << "Rx body: "     << body << std::endl;

    if (response.is_continue())
    {
      std::cout << "Rx is CONTINUE" << std::endl;

      count = CHUNKS_TO_SEND;
      send_a_chunk();
    }
    else
    {
      if (!response.is_chunked())
        http_client.reset();
    }
  }

  /// The handler for incoming HTTP chunks.
  /// Prints the chunk header and data to std::cout.
  void chunk_handler(http_chunk_type const& chunk, std::string const& data)
  {
    std::cout << "Rx chunk: " << chunk.to_string() << "\n";
    std::cout << "Rx data: "  << data << std::endl;

    if (chunk.is_last())
    {
      std::cout << "Rx last chunk" << std::endl;
      http_client.reset();
    }
  }

  /// The handler for the HTTP socket disconnecting.
  void disconnected_handler()
  {
    std::cout << "Socket disconnected" << std::endl;
    http_client.reset();
  }
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  std::string app_name(argv[0]);

  // Get a hostname and uri from the user (assume default http port)
  if (argc != 3)
  {
    std::cout << "Usage: " << app_name << " [host] [uri]\n"
              << "E.g. "   << app_name << " localhost /hello"
              << std::endl;
    return 1;
  }

  std::string host_name(argv[1]);
  std::string uri(argv[2]);
  std::cout << app_name <<" host: " << host_name
            << " uri: " << uri << std::endl;
  try
  {
    // The asio io_service.
    boost::asio::io_service io_service;

    // Create an http_client
    http_client = http_client_type::create(io_service);

    // Attach the callback handlers
    // and attempt to connect to the host on the standard http port (80)
    http_client->response_received_event(response_handler);
    http_client->chunk_received_event(chunk_handler);
    http_client->msg_sent_event(msg_sent_handler);
    http_client->disconnected_event(disconnected_handler);
    if (!http_client->connect(host_name))
    {
      std::cout << "Could not resolve host: " << host_name << std::endl;
      return 1;
    }

    // Create an http request and send it to the host.
    via::http::tx_request request(via::http::request_method::id::PUT, uri);
    request.add_header(via::http::header_field::id::TRANSFER_ENCODING, "Chunked");
    request.add_header(via::http::header_field::id::EXPECT, "100-continue");
    http_client->send(request);

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
