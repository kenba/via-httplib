//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2023 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file example_https_server.cpp
/// @brief An example HTTPS server containing all of the callbacks.
//////////////////////////////////////////////////////////////////////////////
#include "via/http_server.hpp"
#include "../examples/certificates/server/server_crypto.hpp"
#include <set>
#include <iostream>

/// Define an HTTPS server using std::string to store message bodies
typedef via::http_server<via::comms::ssl::ssl_socket, std::string>
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

  /// An example connection filter function
  /// @param socket the client socket attempting to connect
  /// @return true if allowing the connection, false if blocking the connection
  bool filter_connection(ASIO::ip::tcp::socket const& socket)
  {
    // Get the address of the client attempting to connect
    const auto client_address{socket.remote_endpoint().address()};

    // A list of clients to block
    const std::set<ASIO::ip::address> blocklist
    {
      // Uncomment to block localhost connections
      // ASIO::ip::address{ASIO::ip::make_address_v4("127.0.0.1")},
      // ASIO::ip::address{ASIO::ip::make_address_v6("::1")}
    }; 

    // Reject the connection if the client is in the blocklist
    if (blocklist.find(client_address) != blocklist.end())
      return false;

    // A list of clients to allow
    const std::set<ASIO::ip::address> allowlist
    {
      // Uncomment to only permit localhost connections
      // ASIO::ip::address{ASIO::ip::make_address_v4("127.0.0.1")},
      // ASIO::ip::address{ASIO::ip::make_address_v6("::1")}
    };

    // Reject the client if it is NOT in the allowlist
    if (!allowlist.empty() && (allowlist.find(client_address) == allowlist.end()))
      return false;
    
    return true; 
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
        connection->send(response,
             via::comms::ConstBuffers(1, ASIO::buffer(response_body)));
      }
      else
        // Send the response without a body.
        connection->send(std::move(response));
    }
    else
      std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
  }

  /// The handler for incoming HTTP requests.
  /// Outputs the request and determines whether the request is chunked.
  /// If not, it responds with a 200 OK response with some HTML in the body.
  void request_handler(http_connection::weak_pointer weak_ptr,
                       http_request const& request,
                       std::string const& body)
  {
    std::cout << "Rx request: " << request.to_string();
    std::cout << request.headers().to_string();
    std::cout << "Rx body: "    << body << std::endl;

    // Don't respond to chunked requests until the last chunk is received
    if (!request.is_chunked())
      respond_to_request(weak_ptr);
  }

  /// The handler for incoming HTTP chunks.
  /// Outputs the chunk header and body to std::cout.
  void chunk_handler(http_connection::weak_pointer weak_ptr,
                     http_chunk_type const& chunk,
                     std::string const& data)
  {
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
  /// Outputs the request and determines whether the request is too big.
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
                               std::string const&) // body)
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
  unsigned short port_number(https_server_type::connection_type::DEFAULT_HTTP_PORT);

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
    // create an io_context for the server
    ASIO::io_context io_context(1);

    // create an https_server and connect the request handler
    https_server_type https_server(io_context, ssl_context);
    https_server.request_received_event(request_handler);

    // Set the connection filter
    https_server.set_connection_filter(filter_connection);

    // connect the optional handler callback functions
    https_server.chunk_received_event(chunk_handler);
    https_server.request_expect_continue_event(expect_continue_handler);
    https_server.invalid_request_event(invalid_request_handler);
    https_server.socket_connected_event(connected_handler);
    https_server.socket_disconnected_event(disconnected_handler);
    https_server.message_sent_event(message_sent_handler);

    // set the connection timeout (10 seconds)
    https_server.set_timeout(10000);

    // set the connection buffer sizes
    https_server.set_rx_buffer_size(16384);
    https_server.tcp_server()->set_receive_buffer_size(16384);
    https_server.tcp_server()->set_send_buffer_size(16384);

    // start accepting http connections on the given port
    error = https_server.accept_connections(port_number);
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

    // run the io_context to start communications
    io_context.run();

    std::cout << "io_context.run complete, shutdown successful." << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////////
