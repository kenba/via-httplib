# HTTP Server Configuration #

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

A strictly valid HTTP line must end with CRLF pair, however RFC2616 recommends
that servers are tolerant an allow an LF without the CR (the default).

Enabling this value enforces strict CRLF parsing.

### max_whitespace

RFC2616 recommends that severs should allow "any ammout of" whitespace characters,
however that's a clear security loophole.  
This value set the maximum number of whitespace characters between HTTP elements: 
default 8, 1 is the minimum.

### max_method_length

RFC2616 does not specify the length of the method names, however the longest
RFC2616 name is 7 characters long (OPTIONS).
Change this value if using non-standard names longer than 8 characters.

### max_uri_length

RFC2616 does not specify the length of the method names, but it does provide a
response if it's too long.
The default is a compromise value, change it if using uris that are known to be
much shorter or longer.

### max_line_length

This is the maximum length of a header field line, the default is 1024.
Note: cookie field lines may be longer than this.

### max_header_number

The maximum number of header fields alloed in a request

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
| require_host    | true    | HTTP 1.1 Requests must include a Host header field. |
| translate_head  | true    | Translate a HEAD request into a GET request.        |
| trace_enabled   | false   | Echo back a TRACE request as per RFC2616.           |
| auto_disconnect | false   | Disconnect a connection after sending a response to an invalid request. |

### require_host

A client **MUST** include a Host header field in all HTTP/1.1 request messages,
see: [rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html) section 14.23.  

If set, then the server enforces this rule on all HTTP 1.1 requests.

### translate_head

An HTTP HEAD request is identical to a GET request except that the server
**MUST NOT** return a message-body in the response, see:
[rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html) section 9.4.  

If set, then the server passes HEAD requests to the application as GET requests.  
Note: the server **never** sends a body in a response to a HEAD request.

### trace_enabled

The standard HTTP response to a TRACE request is that it should echo back the
TRACE message and all of it's headers in the body of the response, see:
[rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html) section 9.8.  

However, although required by RFC2616 it's considered a security vulnerability 
nowadays, so the default behaviour is to send a 405 "Method Not Allowed" response
instead.

### auto_disconnect

Disconnect the connection after sending a response to an invalid request.
Set for slightly improve security.

## Comms Server Option Parameters

Access via server().

| Parameter           | Default | Description                                         |
|---------------------|---------|-----------------------------------------------------|
| rx_buffer_size      | 8192    | The maximum size of the connection receive buffer.  |
| set_timeout         |         | Set the tcp send and receive timeout values for all future connections. |
| keep_alive          |         | Set the tcp keep alive status for all future connections. |
| receive_buffer_size |         | Set the size of the tcp sockets receive buffer for all future connections. |
| send_buffer_size    |         | Set the size of the tcp sockets send buffer for all future connections. |

## HTTPS Server Options

| Function      | Description                                         |
|---------------|-----------------------------------------------------|
| set_password  | Sets the SSL/TLS password. |
| set_ssl_files | Sets the SSL/TLS files: certificate_file, key file and dh_file. |
