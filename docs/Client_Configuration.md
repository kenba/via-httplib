# HTTP Client Configuration

## Response Parser Parameters

These parameters affect how the parser identifies invalid HTTP response.
Note: the default values for responses are as tolerant as they can be.  

| Parameter         | Default | Description                                         |
|-------------------|---------|-----------------------------------------------------|
| strict_crlf       | false   | Enforce strict parsing of CRLF.                     |
| max_whitespace    | 254     | The maximum number of consectutive whitespace characters. |
| max_status_no     | 65534   | The maximum number of a response status.            |
| max_reason_length | 65534   | The maximum length of a response reason.            |
| max_line_length   | 65534   | The maximum length of a response header field line. |
| max_header_number | 65534   | The maximum number of header fields in a response.  |
| max_header_length | 8190    | The maximum length of characters in the headers.    |
| max_body_size     | 4Gb     | The maximum size of a request body.                 |
| max_chunk_size    | 4Gb     | The maximum size of each request chunk.             |

### strict_crlf

A strictly valid HTTP line must end with CRLF pair, however RFC2616 recommends
that servers and clients are tolerant so allow an LF without the CR (the default).

Enabling this value enforces strict CRLF parsing.

### max_whitespace

RFC2616 recommends that severs should allow "any amount of" whitespace characters.  
This value set the maximum number of whitespace characters between HTTP elements: 
default 254, 1 is the minimum.

### max_status_no

RFC2616 does not specify the maximum status number.

### max_reason_length

RFC2616 does not specify the length of the status reason.

### max_line_length

This is the maximum length of a header field line.

### max_header_number

The maximum number of header fields allowed in a response

### max_header_length

The maximum total size of the header fields for each response message.

### max_body_size

The maximum size of a response body.

### max_chunk_size

The maximum size of a response chunk.

