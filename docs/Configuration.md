# Configuration

## Boost / Standalone `asio` Configuration

The library uses [boost asio](http://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio.html) by default.  
To use [standalone asio](http://think-async.com/):

+ set the environment variable `$ASIO_ROOT` to the path of the `asio` root directory;
+ add `$ASIO_ROOT/include` to your include path;
+ define the macro `ASIO_STANDALONE`.

Note: if you use `qmake` file and include the file [via-httplib.pri](via-httplib.pri) then you just
need to set the `$ASIO_ROOT` environment variable.

Portability between `boost asio` and `standalone asio` is provided by the macros:

+ ASIO,
+ ASIO_ERROR_CODE and
+ ASIO_TIMER.

They are defined in [socket_adaptor.hpp](include/via/comms/socket_adaptor.hpp):

```C++
#ifdef ASIO_STANDALONE
  #include <asio.hpp>
  #define ASIO asio
  #define ASIO_ERROR_CODE asio::error_code
  #define ASIO_TIMER asio::steady_timer
#else
  #include <boost/asio.hpp>
  #define ASIO boost::asio
  #define ASIO_ERROR_CODE boost::system::error_code
  #define ASIO_TIMER boost::asio::deadline_timer
#endif
```

It is hoped that they can continue to provide portability when `asio` becomes a standard C++ library:
see: [Networking Library Proposal](http://open-std.org/JTC1/SC22/WG21/docs/papers/2015/n4478.html).

## SSL / TLS Configuration

Both `via::http_server` and `via::http_client` template classes require a
`SocketAdaptor` to instantiate them:

+ a `tcp_adaptor` for a plain **HTTP** server/client
+ an `ssl_tcp_adaptor` for an **HTTPS** server/client

## Data / Text Configuration

Both `via::http_server` and `via::http_client` template classes take a
`Container` template parameter to configure the type of container used to pass
HTTP message bodies, e.g.:

+ `std::vector<char>` (the default) for handing binary data, e.g. images, files, etc.
+ `std::string` for handing textural data, e.g. HTML, JSON, etc.

| Socket Adaptor    | Container         | Description                   |
|-------------------|-------------------|-------------------------------|
| `tcp_adaptor`     | `std::vector<char>`   | An HTTP data server/client.  |
| `tcp_adaptor`     | `std::string`     | An HTTP text server/client.          |
| `ssl_tcp_adaptor` | `std::vector<char>`   | An HTTPS data server/client. |
| `ssl_tcp_adaptor` | `std::string`     | An HTTPS text server/client.         |

E.g.

```C++
// An HTTP data server.
typedef via::http_server<via::comms::tcp_adaptor> http_data_server_type;

// An HTTP text server.
typedef via::http_server<via::comms::tcp_adaptor, std::string> http_text_server_type;

// An HTTPS data server.
typedef via::http_server<via::comms::ssl_tcp_adaptor> https_data_server_type;

// An HTTPS text server.
typedef via::http_server<via::comms::ssl_tcp_adaptor, std::string> https_text_server_type;
```

## HTTP Parser Configuration

Both `via::http_server` and a `via::http_client` template classes take  further
template parameters to control their HTTP request and response parsers respectively.

The defaults in the `via::http_server` and a `via::http_client` template classes
are set to relatively relaxed values to permit most HTTP requests and all HTTP
responses to be parsed successfully.  
However, where it is known that HTTP request and/or response parameters will have
smaller values they may be reduced, enabling the HTTP parsers to reject invalid
HTTP requests and/or responses sooner.  
Conversely, if an HTTP server expects requests from longer URI's or with more
and/or larger headers then the request parameters can be increased.

### HTTP Server Request Parameters

| Parameter     | Description                            | Default             |
|---------------|----------------------------------------|---------------------|
| MAX_URI_LENGTH | The maximum length of an HTTP request URI: min 1, max 4 billion. | 8190 |
| MAX_METHOD_LENGTH | The maximum length of an HTTP of an HTTP request method: min 1, max 254.<br>Note: "GET" is the shortest HTTP method, length = 3. | 8 |

### HTTP Client Response Parameters

| Parameter     | Description                            | Default/Max         |
|---------------|----------------------------------------|---------------------|
| MAX_STATUS_NUMBER | The maximum number of an HTTP response status.<br>Note: "NETWORK_AUTHENTICATION_REQUIRED" is the highest standard HTTP response status = 511. | 65534 |
| MAX_REASON_LENGTH | The maximum length of a response reason string. | 65534 |

### HTTP Request/Response Common Parameters

| Parameter     | Description                            | Request Default     |
|---------------|----------------------------------------|---------------------|
| MAX_HEADER_NUMBER | The maximum number of HTTP header field lines: max 65534. | 100 |
| MAX_HEADER_LENGTH | The maximum cumulative length the HTTP header fields: max 4 billion. | 65534 |
| MAX_LINE_LENGTH | The maximum length of an HTTP header field line: max 65534. | 1024 |
| MAX_WHITESPACE_CHARS | The maximum number of consecutive whitespace characters permitted: max 254.| 8 |
| STRICT_CRLF | Enforce strict parsing of CRLF in requests/responses. | false |

## Multithreading Configuration

An HTTP server can be configured to use run the `asio::io_context` in multiple threads
(in a thread pool) by setting the macro `HTTP_THREAD_SAFE`, e.g.:

```C++
// A multithreading HTTP text server.
typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
.
.
.

// Create a thread pool for the threads and run the asio io_context
// in each of the threads.
std::vector<std::shared_ptr<std::thread>> threads;
for (std::size_t i = 0; i < no_of_threads; ++i)
{
  std::shared_ptr<std::thread> thread(std::make_shared<std::thread>
                        ([&io_context](){ io_context.run(); }));
  threads.push_back(thread);
}

// Wait for all threads in the pool to exit.
for (std::size_t i(0); i < threads.size(); ++i)
  threads[i]->join();
```

## IPV6 / IPV4 Configuration

Whether a server accepts IPV6 and IPV4 connections or just IPV4 connections
depends upon whether `http_server.accept_connections` is called with the
`ipv4_only` parameter set:

+ **false**, (the default) the server accepts both IPV6 and IPV4 connections
+ **true**, the server only accepts IPV4 connections.
