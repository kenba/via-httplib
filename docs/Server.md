# HTTP Server User Guide

![HTTP Server Class Template](images/http_server_template_class_diagram.png)

An application creates an HTTP server type by instantiating the `http_server`
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

The server is constructed with an `asio::io_context` with optional parameters
for the maximum request body and chunk sizes, e.g.:

```C++
boost::asio::io_context io_context;
http_server_type http_server(io_context);
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

The `set_password` and `set_ssl_files` functions can be used to set up SSL/TLS parameters for servers instantiated with `via::comms::ssl::ssl_tcp_adaptor`.

E.g:

```C++
// Set up SSL
https_server.set_password(password);
boost::system::error_code error
    (https_server_type::set_ssl_files(certificate_file, private_key_file));
if (error)
{
    std::cerr << "Error, set_ssl_files: "  << error.message() << std::endl;
    return 1;
}
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
