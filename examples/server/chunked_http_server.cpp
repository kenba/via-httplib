//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2024 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file chunked_http_server.cpp
/// @brief An HTTP server that sends response bodies in chunks.
//////////////////////////////////////////////////////////////////////////////
#include "via/http_server.hpp"
#include <sstream>
#include <iostream>

/// Define an HTTP server
typedef via::http_server<via::comms::tcp_socket> http_server_type;
typedef http_server_type::http_connection_type http_connection;
typedef http_server_type::http_request http_request;
typedef http_server_type::chunk_type http_chunk_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  const int CHUNKS_TO_SEND(5);

  /// The number of chunks remaining.
  int count(0);

  /// The stop callback function.
  /// Closes the server and all it's connections leaving io_context.run
  /// with no more work to do.
  /// Called whenever a SIGINT, SIGTERM or SIGQUIT signal is received.
  void handle_stop(ASIO_ERROR_CODE const&, // error,
                   int, // signal_number,
                   http_server_type& http_server)
  {
    std::cout << "Shutting down" << std::endl;
    http_server.shutdown();
  }

  /// Something to send in the chunks.
  std::string chunk_text("HTTP chunk number: ");

  /// Send a chunnk to the client.
  void send_a_chunk(http_connection::shared_pointer connection)
  {
    if (--count > 0)
    {
      std::stringstream chunk_stream;
      chunk_stream << chunk_text;
      chunk_stream << CHUNKS_TO_SEND - count << std::ends;

      std::string chunk_to_send(chunk_stream.str());

      std::cout << "send_chunk: " << chunk_to_send << std::endl;

      connection->send_chunk(std::vector<char>(chunk_to_send.cbegin(), chunk_to_send.cend()));
    }
    else if (count >= 0)
    {
      std::cout << "last_chunk" << std::endl;
      connection->last_chunk();
    }
  }

  /// A handler for the signal sent when an HTTP message is sent.
  void message_sent_handler(http_connection::weak_pointer weak_ptr)
  {
    if (count > 0)
    {
      http_connection::shared_pointer connection(weak_ptr.lock());
      if (connection)
        send_a_chunk(connection);
    }
    else
      std::cout << "response sent" << std::endl;
  }

  /// A function to send a response to a request.
  void respond_to_request(http_connection::weak_pointer weak_ptr)
  {
    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
    {
      // Get the last request on this connection.
      http_request const& request(connection->request());

      // The default response is 404 Not Found
      via::http::tx_response response(via::http::response_status::code::NOT_FOUND);
      // add the server and date headers
      response.add_server_header();
      response.add_date_header();

      if (request.uri() == "/hello")
      {
        if ((request.method() == "GET") ||
            (request.method() == "POST") || (request.method() == "PUT"))
          response.set_status(via::http::response_status::code::OK);
        else
        {
          response.set_status(via::http::response_status::code::METHOD_NOT_ALLOWED);
          response.add_header(via::http::header_field::id::ALLOW,
                              "GET, HEAD, POST, PUT");
        }
      }

      // If sending an OK response to a GET (not a HEAD)
      // send the response in "chunks"
      if ((response.status() == static_cast<int>(via::http::response_status::code::OK)) &&
          (request.method() == "GET") && !request.is_head())
      {
        response.add_header(via::http::header_field::id::TRANSFER_ENCODING,
                            "Chunked");
        count = CHUNKS_TO_SEND;
      }

      connection->send(std::move(response));
    }
    else
      std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
  }

  /// The handler for a new conection. Prints the client's address..
  void connected_handler(http_connection::weak_pointer weak_ptr)
  {
    std::cout << "Connected: " << weak_ptr.lock()->remote_address() << std::endl;
  }

  /// The handler for incoming HTTP requests.
  /// Prints the request and determines whether the request is chunked.
  /// If not, it responds with a 200 OK response with some HTML in the body.
  void request_handler(http_connection::weak_pointer weak_ptr,
                       http_request const& request,
                       std::vector<char> const& body)
  {
    std::cout << "Rx request: " << request.to_string();
    std::cout << request.headers().to_string();
    std::cout << "Rx body: "    << std::string_view(body.data(), body.size()) << std::endl;

    if (!request.is_chunked())
      respond_to_request(weak_ptr);
  }

  /// The handler for incoming HTTP chunks.
  /// Prints the chunk header and body to std::cout.
  /// Defined in case the request is a chunked message.
  /// @param http_connection
  /// @param chunk the http chunk
  void chunk_handler(http_connection::weak_pointer weak_ptr,
                     http_chunk_type const& chunk,
                     std::vector<char> const& data)
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
                << " data: " << std::string_view(data.data(), data.size()) << std::endl;
  }

  /// A handler for HTTP requests containing an "Expect: 100-continue" header.
  /// Prints the request and determines whether the request is too big.
  /// It either responds with a 100 CONTINUE or a 413 REQUEST_ENTITY_TOO_LARGE
  /// response.
  void expect_continue_handler(http_connection::weak_pointer weak_ptr,
                               http_request const& request,
                               std::vector<char> const& /* body */)
  {
    static const auto MAX_LENGTH(1024);

    std::cout << "expect_continue_handler\n";
    std::cout << "Rx request: " << request.to_string();
    std::cout << request.headers().to_string() << std::endl;

    // Reject the message if it's too big, otherwise continue
    via::http::tx_response response((request.content_length() > MAX_LENGTH) ?
                       via::http::response_status::code::PAYLOAD_TOO_LARGE :
                       via::http::response_status::code::CONTINUE);
    weak_ptr.lock()->send(std::move(response));
  }

  /// A handler for the signal sent when an HTTP socket is disconnected.
  void disconnected_handler(http_connection::weak_pointer weak_ptr)
  {
    std::cout << "Disconnected: " << weak_ptr.lock()->remote_address() << std::endl;
  }
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  std::string app_name(argv[0]);
  unsigned short port_number(http_server_type::connection_type::DEFAULT_HTTP_PORT);

  // Get a port number from the user (the default is 80)
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

  try
  {
    /// The asio io_context.
    ASIO::io_context io_context(1);

    // create an http_server
    http_server_type http_server(io_context, nullptr);

    // connect the handler callback functions
    http_server.request_received_event(request_handler);
    http_server.socket_connected_event(connected_handler);
    http_server.chunk_received_event(chunk_handler);
    http_server.request_expect_continue_event(expect_continue_handler);
    http_server.message_sent_event(message_sent_handler);
    http_server.socket_disconnected_event(disconnected_handler);

    http_server.set_auto_disconnect(true);

    // start accepting http connections on the given port
    ASIO_ERROR_CODE error(http_server.accept_connections(port_number));
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
      return 1;
    }

    // The signal set is used to register for termination notifications
    ASIO::signal_set signals_(io_context);
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // #if defined(SIGQUIT)

    // register the handle_stop callback
    signals_.async_wait([&http_server]
      (ASIO_ERROR_CODE const& error, int signal_number)
    { handle_stop(error, signal_number, http_server); });

    // run the io_context to start communications
    io_context.run();

    std::cout << "io_context.run complete, shutdown successful" << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////////
