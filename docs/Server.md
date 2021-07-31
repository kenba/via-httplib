# HTTP Server User Guide

![HTTP Server Class Template](images/http_server_template_class_diagram.png)

An application creates an HTTP server type by instantiating the `http_server`
class template defined in `<via/http_server.hpp>`:

```C++
namespace via
{
  template <typename SocketAdaptor,
            typename Container                  = std::vector<char>,
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

The `http_server` class template parameters are:

| Parameter     | Default             | Description                            |
|---------------|---------------------|----------------------------------------|
| SocketAdaptor |                     | `via::comms::tcp_adaptor` for HTTP or<br>`via::comms::ssl::ssl_tcp_adaptor` for HTTPS. |
| Container     | `std::vector<char>` |`std::vector<char>` for data or<br>`std::string` for text |
| MAX_URI_LENGTH | 8190 | the maximum length of an HTTP request uri |
| MAX_METHOD_LENGTH | 8 | the maximum length of an HTTP of an HTTP request method |
| MAX_HEADER_NUMBER | 100 | the maximum number of HTTP header field lines |
| MAX_HEADER_LENGTH | 65534 | the maximum cumulative length the HTTP header fields |
| MAX_LINE_LENGTH | 1024 | the maximum length of an HTTP header field line |
| MAX_WHITESPACE_CHARS | 8 | the maximum number of consecutive whitespace characters permitted in a request |
| STRICT_CRLF | false | enforce strict parsing of CRLF |

The integer (and boolean) template parameters are the permitted HTTP request
parameters for an `http_server`, see [HTTP Parser Configuration](Configuration.md#HTTP-Parser-Configuration).

E.g. an HTTP server using std::string as a Container:

```C++
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_server.hpp"

typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
/// A typedef for http_connection_type to make following code easier to understand.
typedef http_server_type::http_connection_type http_connection;
typedef http_server_type::http_request http_request;
```

## Constructing a server

The server is constructed with an `asio::io_context`, e.g.:

```C++
boost::asio::io_context io_context;
http_server_type http_server(io_context);
```

## Configuring the server

The can either be configured with handlers for each specific method/uri
combination, see [Request Routing](Server_Request_Routing.md), e.g.:

```C++
http_server.request_router().add_method("GET", "/hello", get_hello_handler);
```

Or with a `RequestHandler`, see [Server Events](Server_Events.md), e.g.:

```C++
https_server.request_received_event(request_handler);
```

In which case the `request_handler` must route the HTTP requests.

### Server Events and Handlers

The server will call an application's event handler (callback function)
whenever a significant event occurs.  
See [Server Events](Server_Events.md) for more details.

| Event                 | Function to Register Callback | Description              |
|-----------------------|-------------------------------|--------------------------|
| **Request Received**  | request_received_event        | A valid HTTP request has been received. |
| Chunk Received        | chunk_received_event          | A valid HTTP chunk has been received. |
| Expect Continue       | request_expect_continue_event | A valid HTTP request has been received<br>with an "Expect: 100-continue" header. |
| Invalid Request       | invalid_request_event         | An invalid HTTP request has been received. |
| Socket Connected      | socket_connected_event        | A socket has connected. |
| Socket Disconnected   | socket_disconnected_event     | A socket has disconnected. |
| Message Sent          | message_sent_event            | A message has been sent on the connection. |

Note: if an event handler is provided for **Request Received** then the
internal `request_router()` is disabled.

### Server Configuration

The application can then configure the server parameters.  
See [Server Configuration](Server_Configuration.md) for more details.  

Some of the more significant parameters (with their default values) are:

| Parameter         | Default | Description                                         |
|-------------------|---------|-----------------------------------------------------|
| max_method_length | 8       | The maximum length of a request method.             |
| max_uri_length    | 1024    | The maximum length of a request URI.                |
| max_header_number | 100     | The maximum number of header fields in a request.   |
| max_header_length | 8190    | The maximum length of characters in the headers.    |
| max_body_size     | 1Mb     | The maximum size of a request body.                 |
| max_chunk_size    | 1Mb     | The maximum size of each request chunk.             |

### HTTPS Server Configuration

Note: only valid for servers using `via::comms::ssl::ssl_tcp_adaptor` as a template parameter.

| Parameter     | Description                                          |
|---------------|------------------------------------------------------|
| set_password  | Sets the SSL/TLS password. |
| set_ssl_files | Sets the SSL/TLS files: certificate_file, key file and dh_file. |

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

Other SSL/TLS options can be set via the ssl_context, e.g.:

```C++
boost::asio::ssl::context& ssl_context
    (https_server_type::connection_type::ssl_context());
    
ssl_context.set_options(boost::asio::ssl::context_base::default_workarounds);
```

See: [asio ssl context base](http://www.boost.org/doc/libs/1_67_0/doc/html/boost_asio/reference/ssl__context_base.html)
for options.

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

The default parameters for `accept_connections` are the default port
for the server type (80 for HTTP, 443 for HTTPS) and IPV6 and IPV4
connections. The parameters are:

| Parameter | Default   | Description                            |
|-----------|-----------------------|----------------------------------------|
| port      | 80 or 443 | The server port number.<br>Default 80 for HTTP, 443 for HTTPS. |
| ipv4_only | false     | If set, it only accepts connections from IPV4 clients.<br> Default accepts IPV6 and IPV4. |

The function returns a `boost::system::error_code` to determine whether the server was
able to open the TCP acceptor successfully. If unsuccessful, `error.message()`
may be called to determine the type of error as shown in the example above.

## "Running" the Server

The server's communication is handled by the `boost asio` library. For the server
to start accepting connections, `run` must be called on the`asio::io_context` to
run the server, see: [boost libs](http://www.boost.org/doc/libs/): Asio.

```C++
// run the io_context to start communications
io_context.run();
```

The server will call `request_handler` whenever it receives a valid HTTP request from a client.  
Note: the call to `io_service.run()` will not return until the server is closed.  

## Sending Responses

### http::tx_response

The application creates an HTTP response in a `via::http::tx_response` class,
either by just passing it the response status code, and adding HTTP headers e.g:

```C++
via::http::tx_response response(via::http::response_status::code::OK);  
response.add_server_header();
response.add_date_header();
```

Or the application can create a response with a status code and a string of
HTTP headers, e.g.:

```C++
std::string header_string("Allow: GET, HEAD, PUT\r\n");
via::http::tx_response response(via::http::response_status::code::OK, header_string);
```

However, the application must ensure that the header_string does not contain
any "split headers" (see: [Security Guide](Server_Security.md)) before sending it.
Since the server will not send an HTTP response containing "split headers".  
The `tx_response::is_valid()` can be used to test for split headers, e.g.:

```C++
if (response.is_valid())
    ... // OK
else
    ... // response contains split headers
```

### http_connection::send

The response is then sent by calling one of the `http_connection`'s send functions, e.g.:

```C++
weak_ptr.lock()->send(response);
```

Note: since the pointer passed to the request handler is a weak pointer, `lock()`
must be called to convert it into a `shared pointer` so that `send` can be called.

The server has an number of different `send` functions that the application may call:

| Function                     | Data         | Description                          |
|------------------------------|--------------|--------------------------------------|
| send_response()              |              | Send the standard response to an invalid request.<br>Only to be used in the invalid request handler. |
| send(response)               |              | Send an HTTP `response` without a body. |
| send(response, body)         | Container    | Send a `response` with `body`, data **buffered** by `http_connection`. |
| send(response, buffers)      | ConstBuffers | Send a `response` with `body`, data **unbuffered**. |
| send_chunk(data)             | Container    | Send response `chunk` data, **buffered** by `http_connection`. |
| send_chunk(buffers, buffers) | ConstBuffers | Send response `chunk` data, **unbuffered**. |
| last_chunk()                 |              | Send response HTTP `last chunk`.  |

All of the functions send the data asynchronously, i.e. they return before the data
is sent. The application can choose between two types of functions depending upon
whether the data that it is sending is temporary or not:

+ **buffered** functions, i.e.: those taking a copy of Container as a parameter.<br>
These functions take a copy of the data so the data is no longer required after the
function is called.

+ **unbuffered** functions, i.e.: those taking a ConstBuffers as a parameter.<br>
These functions take a `std::deque` of `asio::const_buffer`s that point to the data.<br>
Therefore the data must **NOT** be temporary, it must exist until the `Message Sent`
event, see [Server Events](Server_Events.md).

## Examples

An HTTP Server that uses the internal request router:
[`routing_http_server.cpp`](../examples/server/routing_http_server.cpp)

An HTTP Server that incorporates the example code above:
[`example_http_server.cpp`](../examples/server/example_http_server.cpp)

An HTTPS Server that incorporates the example code above:
[`example_https_server.cpp`](../examples/server/example_https_server.cpp)

An HTTP Server that uses `asio` strand wrapping and a thread pool: [`thread_pool_http_server.cpp`](../examples/server/thread_pool_http_server.cpp)
