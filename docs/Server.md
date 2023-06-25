# HTTP Server User Guide

![HTTP Server Class Template](images/http_server_template_class_diagram.png)

An application creates an HTTP(S) server type by instantiating the `http_server`
class template defined in `<via/http_server.hpp>`:

```C++
namespace via
{
  template <typename SocketAdaptor,
            typename Container                  = std::vector<char>,
            bool           IPV4_ONLY            = false,
            size_t         MAX_URI_LENGTH       = 8190,
            unsigned char  MAX_METHOD_LENGTH    = 8,
            unsigned short MAX_HEADER_NUMBER    = 100,
            size_t         MAX_HEADER_LENGTH    = 65534,
            unsigned short MAX_LINE_LENGTH      = 1024,
            unsigned char  MAX_WHITESPACE_CHARS = 8,
            bool           STRICT_CRLF          = false>
    class http_server
    {
    ...
    }
}
```

The `via::http_server` class template parameters determine the type and behaviour
of the http_server:

+ HTTP or HTTPS,
+ data or text,
+ IPv6 and IPv4 or IPv4 only,
+ and request parser parameters.

See [HTTP Server Configuration](Server_Configuration.md).

E.g. an HTTP server using std::string as a Container:

```C++
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_server.hpp"

typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
typedef http_server_type::http_connection_type http_connection;
typedef http_server_type::http_request http_request;
```

## Constructing a server

An HTTP server is constructed with an `ASIO::io_context` with optional parameters
for the maximum request body and chunk sizes, e.g.:

```C++
ASIO::io_context io_context(1);
http_server_type http_server(io_context);
```

An HTTPS server is constructed with an `ASIO::io_context`, and `ASIO::ssl::context`
and the same optional parameters as an HTTP server, e.g.:

```C++
ASIO::io_context io_context(1);
ASIO::ssl::context ssl_context(ASIO::ssl::context::tlsv13_server);
http_server_type http_server(io_context, ssl_context);
```

| Parameter         | Description                                    | Default |
|-------------------| -----------------------------------------------|---------|
| max_content_length| The maximum size of a request body and chunks. | 1Mb     |
| max_chunk_size    | The maximum size of each request chunk.        | 1Mb     |

### max_content_length

The maximum size of a request body or a body created by concatenating request chunks.  
It is set to a default of 1Mb, it is highly recommended to set it to specific value
for your application.

### max_chunk_size

The maximum size of a request chunk.  
It is set to a default of 1Mb, it is highly recommended to set it to specific value
for your application.

## Routing requests

The server can either be configured with handlers for each specific request
method/uri combination, see [Request Routing](Server_Request_Routing.md), e.g.:

```C++
http_server.request_router().add_method("GET", "/hello", get_hello_handler);
http_server.request_router().add_method(request_method::GET, "/hello/:name",get_hello_name_handler);
```

or with a `RequestHandler`, e.g.:

```C++
https_server.request_received_event(request_handler);
```

In which case the `request_handler` will route all HTTP requests.
A `RequestHandler` is just one of several event handlers (callback functions)
that the server may call whenever a significant event occurs.
See [Server Events](Server_Events.md) for more details.

Note: if an event handler is provided for **Request Received** then the
internal `request_router()` is disabled.

## HTTPS Configuration

The `ASIO::ssl::context` should be configured before constructing the HTTPS server.
E.g:

```C++
  // Set up SSL/TLS
  ASIO::ssl::context ssl_context(ASIO::ssl::context::tlsv13_server);
  ssl_context.set_options(ASIO::ssl::context_base::default_workarounds
                        | ASIO::ssl::context_base::no_sslv2);
  ssl_context.set_verify_mode(ASIO::ssl::verify_peer);

  ASIO_ERROR_CODE error;
  ssl_context.use_certificate_chain_file(certificate_file, error);
  if (error)
  {
    std::cerr << "Error, use_certificate_chain: " << error.message() << std::endl;
    return 1;
  }

  ssl_context.use_private_key_file(private_key_file, ASIO::ssl::context::pem, error);
  if (error)
  {
    std::cerr << "Error, use_private_key: " << error.message() << std::endl;
    return 1;
  }

  std::string password = "test";
  ssl_context.set_password_callback([password](std::size_t max_length,
      ASIO::ssl::context::password_purpose purpose)
      { return password; });
```

## Accept Connections

Once an `http_server` has been created and configured, it can accept connections
on the required port by calling `accept_connections`, e.g.:

```C++
boost::system::error_code error(http_server.accept_connections());
if (error)
{
    std::cerr << "Error: "  << error.message() << std::endl;
    return 1;
}
```

By default, `accept_connections` uses the standard port for the server type
(80 for HTTP, 443 for HTTPS).  
However, if a different port number is required, it may be passed as a parameter.

### Connection Filtering

By default an `http_server` accepts all incoming connections to the given port.  
However, incoming connections can be blocked with a connection filter function by calling  `http_server.set_connection_filter()` with a filter function prior to calling
`accept_connections`.

A connection filter function takes a const reference to a `tcp::socket` as a parameter and returns true if the connection is allowed, false is the connection is blocked.
The filter function can determine aspects of the client from the `tcp::socket`.  
For example, the filter function below compares the IP address of the client to
IP addresses stored in a "blocklist" (a.k.a. "blacklist") and an "allowlist" (a.k.a. "whitelist"), to determine whether to allow or block an incoming connection:

```C++
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
.
.
.
// create an http_server and connect the request handler
http_server_type http_server(io_context);
http_server.request_received_event(request_handler);

// Set the connection filter
http_server.set_connection_filter(filter_connection);
```

## Running the Server

The server's communication is handled by the `boost asio` library.  
For the server to start accepting connections, `run` must be called on the `asio::io_context`, e.g:

```C++
// run the io_context to start communications
io_context.run();
```

Note: the call to `io_service.run()` will not return until the server is closed.  

## Examples

An HTTP Server that uses the internal request router:
[`routing_http_server.cpp`](../examples/server/routing_http_server.cpp)

An HTTP Server that incorporates the example code above:
[`example_http_server.cpp`](../examples/server/example_http_server.cpp)

An HTTPS Server that incorporates the example code above:
[`example_https_server.cpp`](../examples/server/example_https_server.cpp)

An HTTP Server that uses `asio` strand wrapping and a thread pool: [`thread_pool_http_server.cpp`](../examples/server/thread_pool_http_server.cpp)

An HTTPS Server that requires TLS [Mutual Authentication](https://en.wikipedia.org/wiki/Mutual_authentication):
[`simple_mutual_authentication_https_server.cpp`](../examples/server/simple_mutual_authentication_https_server.cpp)
