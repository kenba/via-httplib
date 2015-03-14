# HTTP Client Configuration #

## Response Parser Parameters ##



## Sending Requests ##

All HTTP requests are created from a instance of the `tx_request` class.

### Class `tx_request` ###

The `tx_request` class is defined in `<via/http/request.hpp>`.

The class has two Constructors:

    explicit tx_request(request_method::id method_id,
                        std::string uri,
                        std::string header_string = "",
                        int minor_version = 1,
                        int major_version = 1);
and

    explicit tx_request(const std::string& method,
                        std::string uri,
                        std::string header_string = "",
                        int minor_version = 1,
                        int major_version = 1);

The first constructor requires a `via::http::request_method::id`
and a uri string.
`request_method::id` is an enumeration of the standard HTTP request methods, see:
`<via/http/request_method.hpp>`

The second constructor requires a method string and a uri string.
It enables the application to send a non-standard HTTP request methods.

The other parameters are the same for both constructors and described below:  

 + `header_string`: a string containing one or more response headers.
    HTTP headers are just strings of the form "Header: Value\r\n" the application can
    set any number of headers in this string.
 + `minor_version`: the minor HTTP version number, set to zero for HTTP/1.0.
    Default 1 for HTTP/1.1.  
    Set to zero for HTTP/1.0
 + `major_version`: the major HTTP version number. Default 1 for HTTP/1.1.

There are also "set" and "add" functions in the `tx_request` class so that the 
headers and HTTP version can be set after the class has been constructed, i.e.:  

 + `void add_header(header_field::id field_id, const std::string& value)`  
    Add a standard HTTP header field (defined in `<via/http/header_field.hpp>`)
    to the request.
 + `void add_header(std::string const& field, const std::string& value)`  
    Add a non-standard header field  to the request.
 + `add_content_length_header(size_t size)`  
    Add a `Content-Length` header with the given size to the request.
 + `void set_minor_version(int minor_version)` Set the HTTP minor version.
 + `void set_major_version(int major_version)` Set the HTTP major version.

### The `Content-Length` Header ###

Although not required by [rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html),
some servers do not accept requests without a `Content-Length` Header, so the
`tx_request` class ensures that it (or a `Transfer-Encoding` header, see
[rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html) section 4.4 para 3)
is always present.

In short, this means an application need never set a `Content-Length` header in
`tx_request`.  
A `Content-Length` header should only be set with an `Expect: 100-continue` header,
to set the `Content-Length` size without a body.

### Sending Requests ###

All requests are sent via the `http_client`, e.g.:

    // Create an http request and send it to the host.
    via::http::tx_request request(via::http::request_method::id::GET, uri);
    http_client->send(request);

There are also functions for sending chunks, e.g.:

    std::string a_chunk("An HTTP message chunk");
    http_client->send_chunk(a_chunk);

And for sending a message body after a `100-continue` response, e.g.:

    std::string a_body("An HTTP message body");
    http_client->send_body(a_body);

## Examples ##

A simple HTTP Client:
[`simple_http_client.cpp`](examples/client/simple_http_client.cpp)

A simple HTTPS Client:
[`simple_https_client.cpp`](examples/client/simple_https_client.cpp)

A example HTTP Client with all of the handlers defined:
[`example_http_client.cpp`](examples/client/example_http_client.cpp)

An HTTP Client that sends a chunked request to PUT /hello:
[`chunked_http_client.cpp`](examples/client/chunked_http_client.cpp)
