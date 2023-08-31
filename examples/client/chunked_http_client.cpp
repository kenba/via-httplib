//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2023 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file chunked_http_client.cpp
/// @brief An example HTTP client to send and receive chunks.
//////////////////////////////////////////////////////////////////////////////
#include "via/http_client.hpp"
#include <sstream>
#include <iostream>

/// Define an HTTP client using std::string to store message bodies
typedef via::http_client<via::comms::tcp_socket, std::string> http_client_type;
typedef http_client_type::http_response http_response;
typedef http_client_type::chunk_type http_chunk_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  /// The number of chunks to send in a request.
  const int CHUNKS_TO_SEND(5);

  /// The number of chunks remaining.
  int count(0);

  // An http_client.
  // Declared here so that it can be used in the connected_handler,
  // response_handler and send_a_chunk function.
  http_client_type::shared_pointer http_client;

  // The uri from the user
  std::string uri;

  /// A handler for the signal sent when an HTTP socket is connected.
  void connected_handler()
  {
    // Create an http request and send it to the host.
    via::http::tx_request request(via::http::request_method::id::PUT, uri);
    request.add_header(via::http::header_field::id::TRANSFER_ENCODING, "Chunked");
    request.add_header(via::http::header_field::id::EXPECT, "100-continue");
    http_client->send(std::move(request));
  }

  /// Something to send in the chunks.
  const std::string chunk_text("HTTP chunk number: ");

  /// Send a chunnk to the server.
  void send_a_chunk()
  {
    if (--count > 0)
    {
      std::stringstream chunk_stream;
      chunk_stream << chunk_text;
      chunk_stream << CHUNKS_TO_SEND - count << std::ends;

      std::string chunk_to_send(chunk_stream.str());

      std::cout << "send_chunk: " << chunk_to_send << std::endl;

      http_client->send_chunk(std::move(chunk_to_send));
    }
    else if (count >= 0)
    {
      std::cout << "last_chunk" << std::endl;
      http_client->last_chunk();
    }
  }

  /// A handler for the signal sent when an HTTP message is sent.
  void message_sent_handler()
  {
    if (count > 0)
      send_a_chunk();
  }

  /// The handler for incoming HTTP responses.
  /// Prints the response nd determines whether the response is continue.
  /// If so it sends chunks, otherwise it disconnects the connection unless
  /// the response is chunked.
  void response_handler(http_response const& response,
                        std::string const& body)
  {
    std::cout << "Rx response: " << response.to_string();
    std::cout << response.headers().to_string();
    std::cout << "Rx body: "    << body << std::endl;

    if (response.is_continue())
    {
      std::cout << "Response is 100-Continue" << std::endl;
      count = CHUNKS_TO_SEND;
      send_a_chunk();
    }
    else
    {
      if (!response.is_chunked())
       http_client->disconnect();
    }
  }

  /// The handler for incoming HTTP chunks.
  /// Prints the chunk header and data to std::cout.
  void chunk_handler(http_chunk_type const& chunk, std::string const& data)
  {
    if (chunk.is_last())
    {
      std::cout << "Rx chunk is last, extension: " << chunk.extension()
                << " trailers: " << chunk.trailers().to_string() << std::endl;
      http_client->disconnect();
    }
    else
      std::cout << "Rx chunk, size: " << chunk.size()
                << " data: " << data << std::endl;
  }

  /// The handler for the HTTP socket disconnecting.
  void disconnected_handler()
  {
    std::cout << "Socket disconnected" << std::endl;
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
  uri = argv[2];
  std::cout << app_name <<" host: " << host_name
            << " uri: " << uri << std::endl;
  try
  {
    // The asio io_context.
    ASIO::io_context io_context;

    // Create an http_client and attach the response & chunk handlers
    http_client =
        http_client_type::create(io_context, response_handler, chunk_handler);

    // Create an http_client and attach the response & chunk handlers
    http_client =
        http_client_type::create(io_context, response_handler, chunk_handler);

    // attach the optional handlers
    http_client->connected_event(connected_handler);
    http_client->message_sent_event(message_sent_handler);
    http_client->disconnected_event(disconnected_handler);

    // attempt to connect to the host on the standard http port (80)
    if (!http_client->connect(host_name))
    {
      std::cout << "Could not resolve host: " << host_name << std::endl;
      return 1;
    }

    // run the io_context to start communications
    io_context.run();

    http_client.reset();

    std::cout << "io_context.run complete, shutdown successful" << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////////
