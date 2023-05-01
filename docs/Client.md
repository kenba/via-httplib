# HTTP Client User Guide

![HTTP Client Class Template](images/http_client_template_class_diagram.png)

An application create an HTTP(S) client type by instantiating the `http_client` class template defined in `<via/http_client.hpp>`:

```C++
namespace via
{
  template <typename SocketAdaptor,
            typename Container                  = std::vector<char>,
            unsigned short MAX_STATUS_NUMBER    = 65534,
            unsigned short MAX_REASON_LENGTH    = 65534,
            unsigned short MAX_HEADER_NUMBER    = 65534,
            size_t         MAX_HEADER_LENGTH    = LONG_MAX,
            unsigned short MAX_LINE_LENGTH      = 65534,
            unsigned char  MAX_WHITESPACE_CHARS = 254,
            bool           STRICT_CRLF          = false>
  class http_client
  {
    ...
  }
}
```

The `http_client` class template parameters are:

| Parameter     | Default             | Description                            |
|---------------|---------------------|----------------------------------------|
| SocketAdaptor |                     | `via::comms::tcp_adaptor` for HTTP or<br>`via::comms::ssl::ssl_tcp_adaptor` for HTTPS. |
| Container     | `std::vector<char>` |`std::vector<char>` for data or<br>`std::string` for text |
| MAX_STATUS_NUMBER | 65534 | the maximum number of an HTTP response status |
| MAX_REASON_LENGTH | 65534 | the maximum length of a response reason string |
| MAX_HEADER_NUMBER | 65534 | the maximum number of HTTP header field lines |
| MAX_HEADER_LENGTH | LONG_MAX | the maximum cumulative length the HTTP header fields |
| MAX_LINE_LENGTH | 65534 | the maximum length of an HTTP header field line |
| MAX_WHITESPACE_CHARS | 254 | the maximum number of consecutive whitespace characters permitted in a response |
| STRICT_CRLF | false | enforce strict parsing of CRLF |

The integer (and boolean) template parameters are the permitted HTTP response
parameters for an `http_client`, see [HTTP Parser Configuration](Configuration.md#HTTP-Parser-Configuration).

 E.g. an HTTP client using std::string as a Container:

 ```C++
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_client.hpp"

typedef via::http_client<via::comms::tcp_adaptor, std::string> http_client_type;
```

## Constructing and Configuring a client

An HTTP client is constructed with an `asio::io_context`, a `ResponseHandler` and a
`ChunkHandler` e.g.:

```C++
boost::asio::io_context io_context;
http_client_type http_client(io_context, response_handler, chunk_handler);
```

An HTTPS client is constructed with an `asio::io_context`, an `ASIO::ssl::context`,
a `ResponseHandler` and a`ChunkHandler` e.g.:

```C++
ASIO::io_context io_context(1);
ASIO::ssl::context ssl_context(ASIO::ssl::context::tlsv13_server);
http_client_type http_client(io_context, ssl_context, response_handler, chunk_handler);
```

Where `response_handler` is an instance of `ResponseHandler`, the event handler
for incoming HTTP responses, e.g.:

 ```C++
/// The handler for incoming HTTP responses.
/// Outputs the response.
void response_handler(via::http::rx_response const& response,
                      std::string const& body)
{
  std::cout << "Rx response: " << response.to_string()
            << response.headers().to_string();
  std::cout << "Rx body: "     << body << std::endl;

  if (!response.is_chunked())
    http_client->disconnect();
}
```

And `chunk_handler` is an instance of `ChunkHandler`, the event handler
for incoming HTTP chunks, e.g.:

```C++
/// The handler for incoming HTTP chunks.
/// Outputs the chunk header and data to std::cout.
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
```

### Client Events and Handlers

The client will call an application's event handlers (callback functions)
whenever a significant event occurs.  
See [Client Events](Client_Events.md) for more details.

| Event                 | Function to Register Callback | Description              |
|-----------------------|-------------------------------|--------------------------|
| **Response Received** | Constructor                   | A valid HTTP response has been received. |
| **Chunk Received**    | Constructor                   | A valid HTTP chunk has been received. |
| Invalid Response      | invalid_response_event        | An invalid HTTP response has been received. |
| Socket Connected      | connected_event               | The socket is connected.  |
| Socket Disconnected   | disconnected_event            | The socket has disconnected.  |
| Message Sent          | message_sent_event            | A message has been sent on the connection. |

Note **Response Received** and **Chunk Received** are the only events for which
the application is required to provide an event handlers.

### Client Configuration

The application can then configure the client parameters.  
See [Client Configuration](Client_Configuration.md) for more details.  

Some of the more significant parameters (with their default values) are:

| Parameter         | Default | Description                                         |
|-------------------|---------|-----------------------------------------------------|
| max_body_size     | 4Gb     | The maximum size of a response body.                |
| max_chunk_size    | 4Gb     | The maximum size of each response chunk.            |

### HTTPS Client Configuration

The `ASIO::ssl::context` should be configured before constructing the HTTPS client.
E.g:

```C++
  // Set up SSL/TLS
  ASIO::ssl::context ssl_context(ASIO::ssl::context::tlsv13_client);
  ssl_context.set_options(ASIO::ssl::context_base::default_workarounds
                        | ASIO::ssl::context_base::no_sslv2);
  ssl_context.set_verify_mode(ASIO::ssl::verify_peer);
  std::string certificate_file = "ca-certificate.pem";
  ASIO_ERROR_CODE error;
  ssl_context.load_verify_file(certificate_file, error);
  if (error)
  {
    std::cerr << "Error, certificate_file: " << error.message() << std::endl;
    return 1;
  }
```

## Making Connections

Once an `http_client` has been created and configured, it can connect to a server
by calling `connect` with the host name, e.g.:

```C++
// attempt to connect to the host on the standard http port (80)
if (!http_client->connect(host_name))
{
  std::cout << "Error, could not resolve host: " << host_name << std::endl;
  return 1;
}
```

The default parameter for `connect` is the default HTTP port: http.

Note: the `http_client` uses the host name to populate the HTTP request "host" header,
so the application does not need to set it.

## "Running" the Client

The clients's communication is handled by the `boost asio` library. For the client
to connect, `run` must be called on the`asio::io_context` to run the client,
see: [boost libs](http://www.boost.org/doc/libs/): Asio.

```C++
    // run the io_context to start communications
    io_context.run();
```

The client will call `response_handler` whenever it receives a valid HTTP response
or `chunk_handler` whenever it receives a valid HTTP chunk.

Note: the call to `io_context.run()` will not return until the connection is closed.  

## Sending Requests

The client normally creates an HTTP request in a `via::http::tx_request` class
passing it the request method id and the uri, e.g:

```C++
    via::http::tx_request request(via::http::request_method::id::GET, uri);
```

The request is then sent by calling one of the client's send functions, e.g.:

```C++
    http_client->send(request);
```

The client will be notified when the response is received by the `response_handler`
passed to the `http_client` constructor.

The client has an number of different `send` functions that the application may call:

| Function                     | Data         | Description                          |
|------------------------------|--------------|--------------------------------------|
| send(request)                |              | Send an HTTP `request` without a body. |
| send(request, body)          | Container    | Send a `request` with `body`, data **buffered** by `http_client`. |
| send(request, buffers)       | ConstBuffers | Send a `request` with `body`, data **unbuffered**. |
| send_body(body)              | Container    | Send request `body` data, **buffered** by `http_client`. |
| send_body(body, buffers)     | ConstBuffers | Send request `body` data, **unbuffered**. |
| send_chunk(data)             | Container    | Send request `chunk` data, **buffered** by `http_client`. |
| send_chunk(buffers, buffers) | ConstBuffers | Send request `chunk` data, **unbuffered**. |
| last_chunk()                 |              | Send request HTTP `last chunk`.  |

All of the functions send the data asynchronously, i.e. they return before the data
is sent. The application can choose between two types of functions depending upon
whether the data that it is sending is temporary or not:

+ **buffered** functions, i.e.: those taking a copy of Container as a parameter.
These functions take a copy of the data so the data is no longer required after the
function is called.

+ **unbuffered** functions, i.e.: those taking a ConstBuffers as a parameter.
These functions take a `std::deque` of `asio::const_buffer`s that point to the data.
Therefore the data must **NOT** be temporary. It must exist until the `Message Sent`
event, see [Client Events](Client_Events.md).

## Examples

A simple HTTP Client:
[`simple_http_client.cpp`](examples/client/simple_http_client.cpp)

A simple HTTPS Client:
[`simple_https_client.cpp`](examples/client/simple_https_client.cpp)

A example HTTP Client with all of the handlers defined:
[`example_http_client.cpp`](examples/client/example_http_client.cpp)

An HTTP Client that sends a chunked request to PUT /hello:
[`chunked_http_client.cpp`](examples/client/chunked_http_client.cpp)

An HTTPS Client that suports TLS [Mutual Authentication](https://en.wikipedia.org/wiki/Mutual_authentication):
[`simple_mutual_authentication_https_client.cpp`](../examples/client/simple_mutual_authentication_https_client.cpp)
