# Chunked Transfer Encoding #

All HTTP/1.1 applications **MUST** be able to receive and decode "chunked"
transfer-coding, see [rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html)
section 3.6.1.  
Chunked encoding modifies the body of a message in order to transfer
it as a series of chunks. This allows dynamically produced content to be transferred.

## Receiving Chunks ##

`via-httplib` Server and Client applications may receive HTTP chunks by registering
a handler for the `chunk_received_event` from `http_connection` and `http_client`
respectively, see [Servers](Server.md) and [Clients](CLIENT.md).

If a server application doesn't register a handler for this event then `http_server`
will concatenate the received chunks into the body of the request and send a
`request_received_event` when the last chunk is received.
This way, a server may receive chunked HTTP PUT or POST requests from a
client without handling the chunks itself.

If a client application doesn't register a handler for this event then all the
chunks received in a chunked message will be lost. The application cannot be
considered to be a compliant HTTP/1.1 application without a chunk handler,
so it should send all requests with the minor HTTP version set to zero.
I.e. they should be HTTP/1.0 requests.

## Sending Chunks ##

An HTTP/1.1 Server or Client may send chunks after the HTTP `response` or `request`
message. However, the `response` or `request` **must** contain a Transfer-Encoding
header field containing anything other than "identity`.
See [rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html) section 4.4 para 2.
E.g.: Transfer-Encoding: Chunked

Servers and Clients may send chunks using the `send_chunk` and `last_chunk` methods
of `http_connection` and `http_client`.

### Server ###

`http_connection` chunk functions, see `<via/http_connection.hpp>`:

    bool send_chunk(Container chunk, std::string extension = "");

    bool last_chunk(std::string extension = "", std::string trailer_string = "");

The `extension` and `trailer_string` are defined in
[rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html) section 3.6.1.

A server's chunks must be sent via an `http_connection` pointer, like requests.
So the application must store the `http_connection::weak_pointer` from the
request handler to that it may send chunks later.

Whenever a server application stores `http_connection` pointers in order to send
asynchronous responses or chunks it should always store them as
`http_connection::weak_pointer` **NOT** `http_connection::shared_pointer`.
Otherwise it will become a co-owner of the connection and `http_server` will not
be able to delete it.

### Client ###

`http_client` chunk functions, see `<via/http_client.hpp>`:

    bool send_chunk(Container chunk, std::string extension = "");

    bool last_chunk(std::string extension = "", std::string trailer_string = "");

A client's chunks must be sent via an `http_client` pointer.
However, since the client owns the pointer, it can store the pointer however it wishes.

## Examples ##

An HTTP Server that sends a chunked response to GET /hello:
[`chunked_http_server.cpp`](../examples/server/chunked_http_server.cpp)

An HTTP Client that sends a chunked request to PUT /hello:
[`chunked_http_client.cpp`](../examples/client/chunked_http_client.cpp)