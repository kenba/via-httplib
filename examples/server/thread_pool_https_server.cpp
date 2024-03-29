//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2024 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file thread_pool_http_server.cpp
/// @brief An example HTTP server containing all of the callbacks using
/// a single io_context and a thread pool calling io_context::run().
//////////////////////////////////////////////////////////////////////////////
#define HTTP_THREAD_SAFE
#include "via/comms/ssl/ssl_tcp_adaptor.hpp"
#include "via/http_server.hpp"
#include "../examples/certificates/server/server_crypto.hpp"
#include <thread>
#include <iostream>

/// Define an HTTPS server using std::string to store message bodies
typedef via::http_server<via::comms::ssl::ssl_tcp_adaptor, std::string>
                                                            https_server_type;
typedef https_server_type::http_connection_type http_connection;
typedef https_server_type::http_request http_request;
typedef https_server_type::chunk_type http_chunk_type;

//////////////////////////////////////////////////////////////////////////////
namespace
{
  /// The stop callback function.
  /// Closes the server and all it's connections leaving io_context.run
  /// with no more work to do.
  /// Called whenever a SIGINT, SIGTERM or SIGQUIT signal is received.
  void handle_stop(ASIO_ERROR_CODE const&, // error,
                   int, // signal_number,
                   https_server_type& http_server)
  {
    std::cout << "Shutting down" << std::endl;
    http_server.shutdown();
  }

  /// A string to send in responses.
  const std::string response_body
    (std::string("<html>\r\n") +
     std::string("<head><title>Accepted</title></head>\r\n") +
     std::string("<body><h1>200 Accepted</h1></body>\r\n") +
     std::string("</html>\r\n"));

  /// A function to send a response to a request.
  void respond_to_request(http_connection::weak_pointer weak_ptr)
  {
    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
    {
      // Get the last request on this connection.
      http_request const& request(connection->request());

      // Set the default response to 404 Not Found
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

      if (response.status() ==
          static_cast<int>(via::http::response_status::code::OK))
      {
        // send the body in an unbuffered response i.e. in ConstBuffers
        // ok because the response_body is persistent data
        connection->send(std::move(response),
             via::comms::ConstBuffers(1, ASIO::buffer(response_body)));
      }
      else
        // Send the response without a body.
        connection->send(response);
    }
    else
      std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
  }

  /// The handler for incoming HTTP requests.
  /// Prints the request and determines whether the request is chunked.
  /// If not, it responds with a 200 OK response with some HTML in the body.
  void request_handler(http_connection::weak_pointer weak_ptr,
                       http_request const& request,
                       std::string const& body)
  {
    std::cout << "Rx request: " << request.to_string();
    std::cout << request.headers().to_string();
    std::cout << "Rx body: "    << body << std::endl;

    if (!request.is_chunked())
      respond_to_request(weak_ptr);
  }

  /// The handler for incoming HTTP chunks.
  /// Outputs the chunk header and body to std::cout.
  void chunk_handler(http_connection::weak_pointer weak_ptr,
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
  void expect_continue_handler(http_connection::weak_pointer weak_ptr,
                               http_request const& request,
                               std::string const& /* body */)
  {
    static const auto MAX_LENGTH(1024);

    std::cout << "expect_continue_handler\n";
    std::cout << "Rx request: " << request.to_string();
    std::cout << request.headers().to_string() << std::endl;

    // Reject the message if it's too big, otherwise continue
    via::http::tx_response response((request.content_length() > MAX_LENGTH) ?
                       via::http::response_status::code::PAYLOAD_TOO_LARGE :
                       via::http::response_status::code::CONTINUE);

    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
      connection->send(std::move(response));
    else
      std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
  }

  /// A handler for the signal sent when an invalid HTTP mesasge is received.
  void invalid_request_handler(http_connection::weak_pointer weak_ptr,
                               http_request const&, // request,
                               std::string const& /* body */)
  {
    std::cout << "Invalid request from: ";
    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
    {
      std::cout << weak_ptr.lock()->remote_address() << std::endl;
      // Send the default response
      connection->send_response();
      // Disconnect the client
      connection->disconnect();
    }
    else
      std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
  }

  /// A handler for the signal sent when an HTTP socket is connected.
  void connected_handler(http_connection::weak_pointer weak_ptr)
  {
    std::cout << "Connected: " << weak_ptr.lock()->remote_address() << std::endl;
  }

  /// A handler for the signal sent when an HTTP socket is disconnected.
  void disconnected_handler(http_connection::weak_pointer weak_ptr)
  {
    std::cout << "Disconnected: " << weak_ptr.lock()->remote_address() << std::endl;
  }

  /// A handler for the signal when a message is sent.
  void message_sent_handler(http_connection::weak_pointer) // weak_ptr)
  {
    std::cout << "response sent" << std::endl;
  }
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  std::string app_name(argv[0]);
  unsigned short port_number(via::comms::ssl::ssl_tcp_adaptor::DEFAULT_HTTP_PORT);

  // Get a port number from the user (the default is 443)
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

  // Set up SSL/TLS
  ASIO::ssl::context ssl_context(ASIO::ssl::context::tlsv13_server);
  ssl_context.set_options(ASIO::ssl::context_base::default_workarounds
                        | ASIO::ssl::context_base::no_sslv2);
  ssl_context.set_verify_mode(ASIO::ssl::verify_peer);

  ASIO_ERROR_CODE error;
  ssl_context.use_certificate_chain(SERVER_CERTIFICATE, error);
  if (error)
  {
    std::cerr << "Error, use_certificate_chain: " << error.message() << std::endl;
    return 1;
  }

  ssl_context.use_private_key(SERVER_KEY, SERVER_KEY_TYPE, error);
  if (error)
  {
    std::cerr << "Error, use_private_key: " << error.message() << std::endl;
    return 1;
  }

  std::string password{SERVER_KEY_PASSWORD};
  ssl_context.set_password_callback([password](std::size_t max_length,
      ASIO::ssl::context::password_purpose purpose)
      { return password; });

  try
  {
    // Determine the number of concurrent threads supported
    const auto no_of_threads(std::thread::hardware_concurrency());
    std::cout << "No of threads: " << no_of_threads << std::endl;

    // create an io_context for the server
    ASIO::io_context io_context(no_of_threads);

    // create an https_server and connect the request handler
    https_server_type https_server(io_context, ssl_context);
    https_server.request_received_event(request_handler);

    // connect the handler callback functions
    https_server.chunk_received_event(chunk_handler);
    https_server.request_expect_continue_event(expect_continue_handler);
    https_server.invalid_request_event(invalid_request_handler);
    https_server.socket_connected_event(connected_handler);
    https_server.socket_disconnected_event(disconnected_handler);
    https_server.message_sent_event(message_sent_handler);

    // start accepting http connections on the given port
    ASIO_ERROR_CODE error(https_server.accept_connections(port_number));
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
    signals_.async_wait([&https_server]
      (ASIO_ERROR_CODE const& error, int signal_number)
    { handle_stop(error, signal_number, https_server); });

    if (no_of_threads > 0)
    {
      // Create a thread pool for the threads and run the asio io_context
      // in each of the threads.
      std::vector<std::thread> threads;
      for (std::size_t i = 0; i < no_of_threads; ++i)
        threads.emplace_back([&io_context]{ io_context.run(); });

      // Wait for all threads in the pool to exit.
      for (std::size_t i(0); i < threads.size(); ++i)
        threads[i].join();
    }
    else
      io_context.run();

    std::cout << "io_context.run, all work has finished" << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////////
