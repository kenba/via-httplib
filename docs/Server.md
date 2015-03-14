# HTTP Server User Guide #

## http_server class template ##

An application creates an HTTP server type by instantiating the `http_server`
class template defined in `<via/http_server.hpp>`:

    namespace via
    {
      template <typename SocketAdaptor, typename Container = std::vector<char>, bool use_strand = false>
      class http_server
      {
        ...
      }
    }
    
The `http_server` class template parameters are:

| Parameter     | Default             | Description                            |
|---------------|---------------------|----------------------------------------|
| SocketAdaptor |                     | `via::comms::tcp_adaptor` for HTTP or `via::comms::ssl::ssl_tcp_adaptor` for HTTPS. |
| Container     | `std::vector<char>` |`std::vector<char>` for data or `std::string` for text |
| use_strand    | false               | Use an `asio::strand` to manage multiple threads, see: [boost asio strands](http://www.boost.org/doc/libs/1_57_0/doc/html/boost_asio/overview/core/strands.html) |
 
E.g. an HTTP server using std::string as a Container:
 
    #include "via/comms/tcp_adaptor.hpp"
    #include "via/http_server.hpp"
    
    typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
    /// A typedef for http_connection_type to make following code easier to understand.
    typedef http_server_type::http_connection_type http_connection;
    
![HTTP Server Class Template](images/http_server_template_class_diagram.png)

## Constructing and Configuring a server ##

The server is constructed with an `asio::io_service` and a `RequestHandler`, e.g.:

    boost::asio::io_service io_service;
    http_server_type http_server(io_service, request_handler);
    
Where `request_handler` is an instance of a `RequestHandler`, the event handler
for incoming HTTP requests, e.g.:

    /// The handler for incoming HTTP requests.
    /// Outputs the request and responds with: 204 No Content.
    void request_handler(http_connection::weak_pointer weak_ptr,
                         via::http::rx_request const& request,
                         std::string const& body)
    {
      std::cout << "Rx request: " << request.to_string();
      std::cout << request.headers().to_string();
      std::cout << "Rx body: "    << body << std::endl;

      via::http::tx_response response(via::http::response_status::code::NO_CONTENT);
      response.add_server_header();
      response.add_date_header();
      weak_ptr.lock()->send(response);
    }
    
### Server Events and Handlers ###
 
The server will call an application's event handler (callback function)
whenever a significant event occurs.  
See [Server Events](Server_Events.md) for more details. 

| Event                 | Handler Type      | Description                    |
|-----------------------|-------------------|----------------------------------|
| **Request Received**  | RequestHandler    | A valid HTTP request has been received. |
| Chunk Received        | ChunkHandler      | A valid HTTP chunk has been received. |
| Expect Continue       | RequestHandler    | A valid HTTP request header has been received containing "Expect: 100-continue" |
| Invalid Request       | RequestHandler    | An invalid HTTP request has been received. |
| Socket Connected      | ConnectionHandler | A socket has connected. |
| Socket Disconnected   | ConnectionHandler | A socket has disconnected. |
| Message Sent          | ConnectionHandler | A message has been sent on the connection. |

Note **Request Received** is the only event that the application is required to
provide an event handler for.

### Server Configuration ###

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

## Accept Connections ##

Once an `http_server` has been created and configured, it can accept connections
on the required port by calling `accept_connections`, e.g.:

    boost::system::error_code error(http_server.accept_connections());
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
      return 1;
    }

The default parameters for `accept_connections` are the default port
for the server type (80 for HTTP, 443 for HTTPS) and IPV6 and IPV4 
connections. The parameters are:

| Parameter | Default   | Description                            |
|-----------|-----------------------|----------------------------------------|
| port      | 80 or 443 | The server port number. Default 80 for HTTP, 443 for HTTPS. |
| ipv4_only | false     | If set, only accepts connections from IPV4 clients, otherwise accepts all. |

The function returns a `boost::system::error_code` to determine whether the server was
able to open the TCP acceptor successfully. If unsuccessful, `error.message()`
may be called to determine the type of error as shown in the example above.

## "Running" the Server ##

The server's communication is handled by the `boost asio` library. For the server
to start accepting connections, `run` must be called on the`asio::io_service` to
run the server, see: [boost libs](http://www.boost.org/doc/libs/): Asio.

    // run the io_service to start communications
    io_service.run();

The server will call `request_handler` whenever it receives a valid HTTP request from a client.  
Note: the call to `io_service.run()` will not return until the server is closed.  

## Sending Responses ##

The server normally creates an HTTP response in a `via::http::tx_response` class
passing it the response status code, e.g:

    via::http::tx_response response(via::http::response_status::code::OK);
    
The server can then add whatever headers it requires to the response, e.g.:

    response.add_server_header();
    response.add_date_header();
    
The response is then sent by calling one of the `http_connection`'s send functions, e.g.:

    weak_ptr.lock()->send(response);
    
Note: since the pointer passed to the request handler is a weak pointer, `lock()`
must be called to convert it into a `shared pointer` so that `send` can be called.

The server has an number of different `send` functions that the application may call:

| Function                     | Data         | Description                          |
|------------------------------|--------------|--------------------------------------|
| send(response)               |              | Send an HTTP `response` without a body. |
| send(response, body)         | Container    | Send a `response` with `body`, data **buffered** by `http_connection`. |
| send(response, buffers)      | ConstBuffers | Send a `response` with `body`, data **unbuffered**. |
| send_body(body)              | Container    | Send response `body` data, **buffered** by `http_connection`. |
| send_body(body, buffers)     | ConstBuffers | Send response `body` data, **unbuffered**. |
| send_chunk(data)             | Container    | Send response `chunk` data, **buffered** by `http_connection`. |
| send_chunk(buffers, buffers) | ConstBuffers | Send response `chunk` data, **unbuffered**. |
| last_chunk()                 |              | Send response HTTP `last chunk`.  |

All of the functions send the data asynchronously, i.e. they return before the data
is sent. The application can choose between two types of functions depending upon
whether the data that it is sending is temporary or not:

 + **buffered** functions, i.e.: those taking a copy of Container as a parameter.
 These functions take a copy of the data so the data is no longer required after the
 function is called.
 
 + **unbuffered** functions, i.e.: those taking a ConstBuffers as a parameter.
 These functions take a `std::deque` of `asio::const_buffer`s that point to the data.
 Therefore the data must **NOT** be temporary. It must exist until the `Message Sent`
 event, see [Server Events](Server_Events.md).

## Examples ##

An HTTP Server that incorporates the example code above:
[`example_http_server.cpp`](examples/server/example_http_server.cpp)

An HTTPS Server that incorporates the example code above:
[`example_https_server.cpp`](examples/server/example_https_server.cpp)

An HTTP Server that uses `asio` strand wrapping and a thread pool: [`thread_pool_http_server.cpp`](examples/server/thread_pool_http_server.cpp)
