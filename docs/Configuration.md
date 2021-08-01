# HTTP Parser Configuration

Both `via::http_server` and a `via::http_client` template classes take template
parameters to control their HTTP request and response parsers respectively.

The defaults in the `via::http_server` and a `via::http_client` template classes
are set to relatively relaxed values to permit most HTTP requests and all HTTP
responses to be parsed successfully.  
However, where it is known that HTTP request and/or response parameters will have
smaller values they may be reduced, enabling the HTTP parsers to reject invalid
HTTP requests and/or responses sooner.  
Conversely, if an HTTP server expects requests from longer URI's or with more
and/or larger headers then the request parameters can be increased.

## HTTP Server Request Parameters

| Parameter     | Description                            | Default             |
|---------------|----------------------------------------|---------------------|
| MAX_URI_LENGTH | The maximum length of an HTTP request URI: min 1, max 4 billion. | 8190 |
| MAX_METHOD_LENGTH | The maximum length of an HTTP of an HTTP request method: min 1, max 254.<br>Note: "GET" is the shortest HTTP method, length = 3. | 8 |

### MAX_URI_LENGTH

rfc7230 does not specify the maximum length of a uri, however it recommends a
minimum of 8000.
The default is based on rfc7230's recommendation, change it if using uris that
are known to be much shorter or longer.

### MAX_METHOD_LENGTH

rfc7230 does not specify the length of the method names, however the longest
rfc7231 name is 7 characters long (OPTIONS).  
Change this value if using only short names or non-standard names longer than 8
characters.

## HTTP Client Response Parameters

| Parameter     | Description                            | Default/Max         |
|---------------|----------------------------------------|---------------------|
| MAX_STATUS_NUMBER | The maximum number of an HTTP response status.<br>Note: "NETWORK_AUTHENTICATION_REQUIRED" is the highest standard HTTP response status = 511. | 65534 |
| MAX_REASON_LENGTH | The maximum length of a response reason string. | 65534 |

## HTTP Request/Response Common Parameters

| Parameter     | Description                            | Request Default     |
|---------------|----------------------------------------|---------------------|
| MAX_HEADER_NUMBER | The maximum number of HTTP header field lines: max 65534. | 100 |
| MAX_HEADER_LENGTH | The maximum cumulative length the HTTP header fields: max 4 billion. | 65534 |
| MAX_LINE_LENGTH | The maximum length of an HTTP header field line: max 65534. | 1024 |
| MAX_WHITESPACE_CHARS | The maximum number of consecutive whitespace characters permitted: max 254.| 8 |
| STRICT_CRLF | Enforce strict parsing of CRLF in requests/responses. | false |

### MAX_HEADER_NUMBER

The maximum number of header fields allowed in a request

### MAX_HEADER_LENGTH

The maximum total size of the header fields for each request/response message.

### MAX_LINE_LENGTH

This is the maximum length of a header field line, the default is 1024.  
Note: cookie field lines may be longer than this.

### MAX_WHITESPACE_CHARS

rfc7230 allows "optional" whitespace characters, see: [rfc7230](https://tools.ietf.org/html/rfc7230) section 3.2.3.  
This value sets the maximum number of optional whitespace characters between HTTP elements:
default 8, 1 is the minimum.

### STRICT_CRLF

A strictly valid HTTP line must end with CRLF pair, however rfc7230 recommends
that servers are tolerant so allow an LF without the CR (the default).

Enabling this value enforces strict CRLF parsing.
