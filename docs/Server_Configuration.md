# HTTP Server Configuration

Much of the behaviour of an http_server is determined by the `via::http_server`
class template parameters.

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

## SSL / TLS Configuration

The `via::http_server` template class requires a `SocketAdaptor` to instantiate it:

+ a `tcp_adaptor` for a plain **HTTP** server/client
+ an `ssl_tcp_adaptor` for an **HTTPS** server/client

## Data / Text Configuration

The `via::http_server` template class also takes a template parameter to configure
the type of container used to pass HTTP message bodies, e.g.:

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

## IPV6 / IPV4 Configuration

Whether a server accepts IPV6 and IPV4 connections or just IPV4 connections
depends upon the `IPV4_ONLY` template parameter:

+ **false**, (the default) the server accepts both IPV6 and IPV4 connections
+ **true**, the server only accepts IPV4 connections.

## HTTP Request Parser Parameters

The other integer (and boolean) template parameters are the permitted HTTP request
parameters for an `http_server`, see [HTTP Parser Configuration](Configuration.md).

## HTTPS Server Configuration

The following functions can be called to set up the SSL/TLS parameters:

| Function      | Description                                          |
|---------------|------------------------------------------------------|
| set_password  | Sets the SSL/TLS password. |
| set_ssl_files | Sets the SSL/TLS files: certificate_file, key file and dh_file. |

Note: only valid for servers using `via::comms::ssl::ssl_tcp_adaptor` as a template parameter.  

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

See: [asio ssl context base](http://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio/reference/ssl__context_base.html)
for options.

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

## HTTP Server Option Parameters

| Parameter       | Default | Description                                         |
|-----------------|---------|-----------------------------------------------------|
| max_content_length| 1Mb     | The maximum size of a request body and chunks.      |
| max_chunk_size    | 1Mb     | The maximum size of each request chunk.             |
| trace_enabled   | false   | Echo back a TRACE request as per rfc7231.           |
| auto_disconnect | false   | Disconnect a connection after sending a response to an invalid request. |
| translate_head  | true    | Translate a HEAD request into a GET request.        |

### max_content_length

The maximum size of a request body or a body created by concatenating request chunks.  
It is set to a default of 1Mb, it is highly recommended to set it to specific value
for your application.

### max_chunk_size

The maximum size of a request chunk.  
It is set to a default of 1Mb, it is highly recommended to set it to specific value
for your application.

### trace_enabled

The standard HTTP response to a TRACE request is that it should echo back the
TRACE message and all of it's headers in the body of the response, see:
[rfc7231](https://tools.ietf.org/html/rfc7231) section 4.3.8.  

However, although required by rfc7231 it's considered a security vulnerability,
so the default behaviour is to send a 405 "Method Not Allowed" response instead.

### auto_disconnect

Disconnect the connection after sending a response to an invalid request.
Set for slightly improve security.

### translate_head

An HTTP HEAD request is identical to a GET request except that the server
**MUST NOT** return a message-body in the response, see:
[rfc7231](https://tools.ietf.org/html/rfc7231) section 4.3.2.  

If set, then the server passes HEAD requests to the application as GET requests.  
Note: the server **never** sends a body in a response to a HEAD request.

## TCP Server Option Parameters

Access using `tcp_server().set_`, e.g.:

```C++
http_server.tcp_server().set_timeout(1000);
```

| Parameter           | Description                                         |
|---------------------|-----------------------------------------------------|
| timeout             | The tcp send and receive timeout values (in mS).    |
| keep_alive          | The tcp keep alive status.                          |
| rx_buffer_size      | The maximum size of the connection receive buffer (default 8192).  |
| receive_buffer_size | The size of the tcp socket's receive buffer.        |
| send_buffer_size    | The size of the tcp socket's send buffer.           |
