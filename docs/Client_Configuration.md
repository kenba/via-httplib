# HTTP Client Configuration

## Response Parser Parameters

These parameters affect how the parser identifies invalid HTTP response.
They are very similar to the request parser parameters used by the server.
However, they are not currently configurable, the default response values
are as tolerant as they can be.  

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

## TCP Client Option Parameters

Access using `connection().set_`, e.g.:

    http_client->connection().set_receive_buffer_size(16384);

| Parameter           | Description                                         |
|---------------------|-----------------------------------------------------|
| timeout             | The tcp send and receive timeout values (in mS).    |
| keep_alive          | The tcp keep alive status.                          |
| rx_buffer_size      | The maximum size of the connection receive buffer (default 8192).  |
| receive_buffer_size | The size of the tcp socket's receive buffer.        |
| send_buffer_size    | The size of the tcp socket's send buffer.           |
