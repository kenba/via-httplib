# HTTP Server Configuration #

All of the parameters can be set by calling the function with the name of the
parameter preceded by `set_`, e.g.:

    http_server.set_max_body_size(40000000);

## Request Parser Parameters

These parameters affect how the parser identifies invalid HTTP requests.
The default values are relatively tolerant, stricter values would be required
for security.

| Parameter         | Default | Description                                         |
|-------------------|---------|-----------------------------------------------------|
| strict_crlf       | false   | Enforce strict parsing of CRLF.                     |
| max_whitespace    | 8       | The maximum number of consectutive whitespace characters. |
| max_method_length | 8       | The maximum length of a request method.             |
| max_uri_length    | 1024    | The maximum length of a request URI.                |
| max_line_length   | 1024    | The maximum length of a header field line.          |
| max_header_number | 100     | The maximum number of header fields in a request.   |
| max_header_length | 8190    | The maximum length of characters in the headers.    |
| max_body_size     | 1Mb     | The maximum size of a request body.                 |
| max_chunk_size    | 1Mb     | The maximum size of each request chunk.             |

### strict_crlf

A strictly valid HTTP line must end with CRLF pair, however rfc7230 recommends
that servers are tolerant so allow an LF without the CR (the default).

Enabling this value enforces strict CRLF parsing.

### max_whitespace

rfc7230 allows "optional" whitespace characters, see: [rfc7230](https://tools.ietf.org/html/rfc7230) section 3.2.3.  
This value sets the maximum number of optional whitespace characters between HTTP elements: 
default 8, 1 is the minimum.

### max_method_length

rfc7230 does not specify the length of the method names, however the longest
rfc7231 name is 7 characters long (OPTIONS).  
Change this value if using non-standard names longer than 8 characters.

### max_uri_length

rfc7230 does not specify the maximum length of the uri, but it does provide a
response if it's too long.  
The default is a compromise value, change it if using uris that are known to be
much shorter or longer.

### max_line_length

This is the maximum length of a header field line, the default is 1024.  
Note: cookie field lines may be longer than this.

### max_header_number

The maximum number of header fields allowed in a request

### max_header_length

The maximum total size of the header fields for each request message.

### max_body_size

The maximum size of a request body or a body created by concatenating request chunks.  
It is set to a default of 1Mb, it is highly recommended to set it to specific value
for your application.

### max_chunk_size

The maximum size of a request chunk.  
It is set to a default of 1Mb, it is highly recommended to set it to specific value
for your application.

## HTTP Server Option Parameters

| Parameter       | Default | Description                                         |
|-----------------|---------|-----------------------------------------------------|
| trace_enabled   | false   | Echo back a TRACE request as per rfc7231.           |
| auto_disconnect | false   | Disconnect a connection after sending a response to an invalid request. |
| translate_head  | true    | Translate a HEAD request into a GET request.        |

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

    http_server.tcp_server().set_timeout(1000);

| Parameter           | Description                                         |
|---------------------|-----------------------------------------------------|
| timeout             | The tcp send and receive timeout values (in mS).    |
| keep_alive          | The tcp keep alive status.                          |
| rx_buffer_size      | The maximum size of the connection receive buffer (default 8192).  |
| receive_buffer_size | The size of the tcp socket's receive buffer.        |
| send_buffer_size    | The size of the tcp socket's send buffer.           |
