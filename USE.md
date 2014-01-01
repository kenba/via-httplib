# User Guide #

An application interfaces with **via-httplib** through the `http_server` template class.

The application must configure and initialise an instance of `http_server`.
The `http_server` will then signal the application whenever a significant HTTP
event occurs. Events that may be signalled by an instance of `http_server` are:

+ `request_received_event` signalled when an HTTP request is received from a client.
+ `chunk_received_event` signalled when an HTTP request chunk is received from a client.
+ `request_expect_continue_event` signalled when an HTTP request contains an
"Expect: 100-continue" header.
+ `socket_disconnected_event` signalled when an HTTP client's socket disconnects.

An application need only handle the `request_received_event`, all of the other
events are optional.

Note: the `http_server` class instance must be constructed with a `boost::asio::io_service`
and the `io_service` must be run or polled after `http_server` has been initialised to
enable TCP communications.
For more information see: [boost libs](http://www.boost.org/doc/libs/): Asio.

## http_server ##

The `http_server` template class is defined in `<via/http_server.hpp>`:

    namespace via
    {
      template <typename SocketAdaptor,
                typename Container,
                bool use_strand = false,
                bool translate_head = true,
                bool has_clock = true,
                bool require_host = true,
                bool trace_enabled = false>
      class http_server
      {
        ...
      }
    }

The template parameters are described in the table below:  

<table>
<tr> <td><b>Parameter</b></td> <td><b>Description</b></td> </tr>
<tr> <td>SocketAdaptor</td> <td>The adaptor for the type of socket to be used by the server.<br> Either: via::comms::tcp_adaptor for an HTTP server or<br>
via::comms::ssl::ssl_tcp_adaptor for an HTTPS server.</td> </tr>
<tr> <td>Container</td> <td>The container for request and response message bodies. One of:<br>
<ul>
<li>std::string,</li>
<li>std::vector< char > </li>
<li>std::vector< unsigned char > </li>
</ul>
std::string provides the simplest interface for HTML, XML, JSON, etc.<br>
The vectors may be easier when sending or receiving binary data.
</td> </tr>
<tr> <td>use_strand</td> <td>Use an asio::strand to allow the execution of code in a
multi-threaded program without the need for explicit locking, see: <a href="http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/overview/core/strands.html">boost asio strands</a>.<br>
Default: false.</td> </tr>
<tr> <td>translate_head</td> <td>An HTTP HEAD request is identical to a GET request
except that the server <b>MUST NOT</b> return a message-body in the response.<br>
If set, then via-httplib passes HEAD requests to the application as GET requests
and ensures that no body is sent in the response. Otherwise the application should handle HEAD requests itself.<br>
Default: true.</td> </tr>
<tr> <td>has_clock</td> <td>An HTTP response should normally contain a Date header. However, if the server does not have a clock that can provide a reasonable approximation
of the current time, its responses <b>MUST NOT</b> include a Date header field.<br>
If set, then via-httplib adds a Date header to all HTTP responses.<br>
Default: true. </td> </tr>
<tr> <td>require_host</td> <td>A client <b>MUST</b> include a Host header field in
all HTTP/1.1 request messages.<br>
If set, then via-httplib responds to all HTTP 1.1 requests <b>without</b> a Host
header with a 400 "Bad Request" response.
Otherwise, the request is passed on to the application.<br>
Default: true. 
</td> </tr>
<tr> <td>trace_enabled</td> <td>The standard HTTP response to a TRACE request
should echo back the TRACE message and all of it's headers in the body of the response.<br>
However, although required by RFC2616 it's considered a security vulnerability 
nowadays, so the default behaviour is to send a 405 "Method Not Allowed" response
instead.<br>
If set, then via-httplib sends the standard TRACE response.<br>
Default: false. </td> </tr>
</table>

Note: the descriptions of the HEAD request, Date and Host headers above are
from [rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html).
Sections: 9.4, 14.18 and 14.23 respectively.

E.g.

An HTTP server using `std::string` to store message bodies:

    #include <via/comms/tcp_adaptor.hpp>
    #include <via/http_server.hpp>
    via::http_server<via::comms::tcp_adaptor, std::string> http_server;

An HTTP server using `std::vector<char>` to store message bodies:

    #include <via/comms/tcp_adaptor.hpp>
    #include <via/http_server.hpp>
    via::http_server<via::comms::tcp_adaptor, std::vector<char> > http_server;

An HTTPS server using `std::string` to store message bodies:

    #include <via/comms/ssl/ssl_tcp_adaptor>
    #include <via/http_server.hpp>
    via::http_server<via::comms::ssl::ssl_tcp_adaptor, std::string> http_server;

## Constructor ##

No matter how the `http_server` class is parameterised, it only has one Constructor:

    explicit http_server(boost::asio::io_service& io_service);

i.e. it must be constructed with a reference to a `boost::asio::io_service`, for example:

    boost::asio::io_service io_service;
    http_server the_http_server(io_service);

## Callback Events ##

The events that may be signalled by an instance of `http_server` in more detail.

### request\_received\_event ###

The `request_received_event` signalled whenever an HTTP request is received from a client.  

A handler for this event **must** be registered by the application so that it can send
HTTP responses to HTTP requests.  

The `request_received_event` method is defined as:

    /// The signal sent when a request is received.
    typedef boost::signals2::signal
                <void (boost::weak_ptr<http_connection_type>,
                                       http::rx_request const&,
                                       Container const&)> http_request_signal;

    /// The slot type associated with a request received signal.
    typedef typename http_request_signal::slot_type http_request_signal_slot;

    /// Connect the request received slot.
    request_received_event(http_request_signal_slot const& slot);

The application's request handler must match the function signature defined by
`http_request_signal` above. The example code below shows how to declare and register
a request handler:

    /// Define the server's http_connection_type to simplify the code.
    typedef http_server_type::http_connection_type http_connection;

    /// The application's request handler.
    void request_handler(http_connection::weak_pointer weak_ptr,
                         via::http::rx_request const& request,
                         std::string const& body)
    {
    ...
    }

    /// register request_handler with the http_server
    http_server.request_received_event(request_handler);

The application's request handler is key to how the application responds to HTTP requests.
They are described in more detail here: [Request Handlers](REQUEST.md).

### chunk\_received\_event ###

The `chunk_received_event` is signalled whenever an HTTP request chunk is received
from a client.

Normally an application will receive the body with the request. However, from HTTP 1.1
onwards, both HTTP requests and responses may contain "chunked" bodies. In which case,
the body may be sent in a number of "chunks". The application may process the chunks as
they are received. However, a "chunked" HTTP request is not complete until the last chunk
has been received.

An application need not register a `chunk_handler` with `http_server`. If the application doesn't register a handler for this event then `http_server` will concatenate the received
chunks into the body of the request and only send the `request_received_event` when the last
chunk has been received, just like a normal request.

The `chunk_received_event` method is defined as:

    /// The signal sent when a chunk is received.
    typedef boost::signals2::signal
            <void (boost::weak_ptr<http_connection_type>,
                                   http::rx_chunk const&,
                                   Container const&)> http_chunk_signal;

    /// The slot type associated with a chunk received signal.
    typedef typename http_chunk_signal::slot_type http_chunk_signal_slot;

    /// Connect the chunk received slot.
    void chunk_received_event(http_chunk_signal_slot const& slot)

The application's chunk handler must match the function signature defined by
`http_chunk_signal` above. The example code below shows how to declare and register
a chunk handler:

    /// The application's chunk handler.
    void chunk_handler(http_connection::weak_pointer weak_ptr,
                       via::http::rx_chunk const& chunk,
                       std::string const& body)
    {
    ...
    }

    /// register chunk_handler with the http_server
    http_server.chunk_received_event(chunk_handler);

If an application registers a `chunk_handler` and it receives a chunked request, then it
must send an HTTP response to the client when the last chunk of the request is received, **not** in the request handler. See: Receiving Chunked Requests: TODO.

### request\_expect\_continue\_event ###

The `request_expect_continue_event` is signalled whenever an HTTP request contains an
"Expect: 100-continue" header.

Normally, an application should only send one response to each request that it receives.  
However, an HTTP client may send a request with an "Expect: 100-continue" header, in which case it expects the server to send a `100 Continue` response before it sends the
body of the request.  

An application need not register an `expect_continue` handler with `http_server`.
If the application doesn't register a handler for this event then `http_server` will
always send a "100 Continue" response to a request with an "Expect: 100-continue" header.
The application will then receive the request through the `request_handler` when the
body has been received, just like a normal request.

The `request_expect_continue_event` method requires a request handler like
`request_received_event`, it is defined as:

    /// The signal sent when a request is received.
    typedef boost::signals2::signal
                <void (boost::weak_ptr<http_connection_type>,
                                       http::rx_request const&,
                                       Container const&)> http_request_signal;

    /// The slot type associated with a request received signal.
    typedef typename http_request_signal::slot_type http_request_signal_slot;

    /// Connect the expect continue received slot.
    void request_expect_continue_event(http_request_signal_slot const& slot);

The application's expect continue handler must match the function signature defined
by `http_request_signal` above.

If the application registers a handler for this event, then it must determine whether
to send `100 Continue` or a different response to the client in the expect continue handler.
For example in the `expect_continue_handler` below, the application responds to all 
requests with a body greater than 1Mb with a "413 Request Entity Too Large" response:

    void expect_continue_handler(http_connection::weak_pointer weak_ptr,
                                 via::http::rx_request const& request,
                                 std::string const& body)
    {
      static const size_t MAX_LENGTH(1048576);

      // Reject the message if it's too big, otherwise continue
      via::http::tx_response response((request.content_length() > MAX_LENGTH) ?
                           via::http::response_status::REQUEST_ENTITY_TOO_LARGE :
                           via::http::response_status::CONTINUE);
      weak_ptr.lock()->send(response);
    }

    /// register expect_continue_handler with the http_server
    http_server.request_expect_continue_event(expect_continue_handler);

### socket\_disconnected\_event ###

The `socket_disconnected_event` is signalled whenever an HTTP client's socket
disconnects.

The `http_server` can accept many simultaneous connections from HTTP clients.
If the application is responding to a request asynchronously and keeping a track of the
connections, then it'll need to register with this event to determine whether the
client has disconnected before the application was able to send a response. 

The `socket_disconnected_event` method is defined as:

    /// The signal sent when a socket is disconnected.
    typedef boost::signals2::signal<void (boost::weak_ptr<http_connection_type>)>
                                                             http_disconnected_signal;

    /// The slot type associated with a socket disconnected signal.
    typedef typename http_disconnected_signal::slot_type http_disconnected_signal_slot;

    /// Connect the disconnected slot.
    void socket_disconnected_event(http_disconnected_signal_slot const& slot);

The application's disconnected handler must match the function signature defined by
`http_disconnected_signal` above. The example code below shows how to declare and 
register a disconnected handler:

    /// The application's disconnected handler.
    void disconnected_handler(http_connection::weak_pointer weak_ptr)
    {
    ...
    }

    /// register disconnected_handler with the http_server
    http_server.socket_disconnected_event(disconnected_handler);

## Connecting the Server to a Port ##

When all of the event handlers have been connected, the server can be connected to a
port and start listening for connections from HTTP clients. This is achieved by calling
`accept_connections`, e.g.:

    boost::system::error_code error(the_http_server.accept_connections());
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
      return 1;
    }

After `accept_connections` has been called, `run` can be called on the
`asio::io_service` to run the server.

### accept\_connections ###

The `accept_connections` method is defined as:

    boost::system::error_code accept_connections
                      (unsigned short port = SocketAdaptor::DEFAULT_HTTP_PORT,
                       bool ipv6 = false);

The default parameters create an IPV4 server on the default HTTP port, i.e. `port 80`
for an HTTP server or `port 443` for an HTTPS server.
They can be overridden to create a server on any port using either the IPV4 or IPV6
protocol.  

The function returns a boost::system::error_code to determine whether the server was
able to open the TCP acceptor successfully. If successful, a test against error will
return false, otherwise error.message() may be called to determine the type of error
as in the example above.

## SSL/TLS Support ##

If SSL support is required then in application it must have OpenSSL in the
include path and link with the OpenSSL libraries.
It must also define the macro: HTTP_SSL.
This macro is used in `<via::http_server.hpp>` and defined in
`<via/comms/ssl/ssl_tcp_adaptor.hpp>`. So simply including `ssl_tcp_adaptor.hpp` before
`http_server.hpp` works.
However, if you don't want to rely on the file inclusion order then the macro should
be defined in the Makefile / project settings.

Additionally an HTTPS server requires: a password, certificate file and key file.
These can be set (together with an optional dh_file) using the following functions
defined in the `http_server` class:

    void set_password(std::string const& password);

    static boost::system::error_code set_ssl_files(const std::string& certificate_file,
                                                   const std::string& key_file,
                                                   std::string        dh_file = "");

for example:

    std::string password         = "test";
    std::string certificate_file = "cacert.pem";
    std::string private_key_file = "privkey.pem";

    https_server.set_password(password);
    boost::system::error_code error
        (https_server_type::set_ssl_files(certificate_file, private_key_file));
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
      return 1;
    }
 
## Code Examples ##

An example HTTP Server that incorporates the example code above:
[`example_http_server.cpp`](examples/server/example_http_server.cpp)

An example HTTPS Server that incorporates the example code above:
[`example_https_server.cpp`](examples/server/example_https_server.cpp)
