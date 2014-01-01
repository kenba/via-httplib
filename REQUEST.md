# Request Handlers #

Every application must register a handler for the `request_received_event` so that it
can respond to HTTP requests from clients.

The request handler will be a function like the one shown below:

    void request_handler(http_connection::weak_pointer weak_ptr,
                         via::http::rx_request const& request,
                         std::string const& body);

It takes three parameters:

 1. A weak pointer to an `http_server::http_connection_type`: `weak_ptr`.  
    The request handler shall send the response via this pointer, either within the
    handler or by storing the weak pointer for sending the response later.

 2. A constant reference to the received HTTP request: `request`.  
    The `rx_request` class contains information about the HTTP request, e.g.:  
    the method string `request.method()`, the uri string `request.uri()`and
    the HTTP headers `request.headers()`.  
    
 3. A constant reference to the body in the HTTP request: `body`.  
    The type of body is determined by the Container parameter used to create an
    instance of `http_server`.
    Note: in the example above, the `http_server Container` parameter is a `std::string`.

The request handler should query the `rx_request` class to determine how to handle the
request and then send it's response via the connection pointer, e.g:

    void request_handler(http_connection::weak_pointer weak_ptr,
                         via::http::rx_request const& request,
                         std::string const& body)
    {
      // The default response is 404 Not Found
      via::http::tx_response response(via::http::response_status::NOT_FOUND);
      if (request.uri() == "/hello")
      {
         if (request.method() == "GET")
           response.set_status(via::http::response_status::OK);
         else
         {
           response.set_status(via::http::response_status::METHOD_NOT_ALLOWED);
           response.add_header(via::http::header_field::ALLOW, "GET");
         }
      }

      weak_ptr.lock()->send(response);

      http_connection::shared_pointer connection(weak_ptr.lock());
      if (connection)
        connection->send(response);
      else
        std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
    }

In the example above, the `request_handler` will only send a "200 OK" response to a
"GET" request to the uri: "/hello".
A request to any other uri will receive a "404 Not Found" response, whilst a
request to the "/hello" uri with any other method will receive a
"405 Method Not Allowed" response with the header "Allow: GET".

## Responses ##

### `tx_response` ###

All responses must contain an instance of `via::http::tx_response`, which has two
Constructors:

    explicit tx_response(response_status::status_code status,
                         std::string header_string = "",
                         bool is_chunked = false,
                         int minor_version = 1,
                         int major_version = 1);

and

    explicit tx_response(std::string reason_phrase,
                         int status,
                         std::string header_string = "",
                         bool is_chunked = false,
                         int minor_version = 1,
                         int major_version = 1);

The first constructor just requires a `via::http::response_status::status_code`,
this is an enumeration of the standard HTTP response status codes, see:
`<via/http/response_status.hpp>`

The second constructor requires a string and an integer.
It enables the application to send a non-standard HTTP response status.

The other parameters are the same for both constructors and described below:  

 + `header_string`: a string containing one or more response headers.
    HTTP headers are just strings of the form "Header: Value\r\n" the application can
    set any number of headers in this string.
 + `is_chunked`: a flag indicating whether the response body will be sent in "chunks",
    default: false.
 + `minor_version`: the minor HTTP version number, set to zero for HTTP/1.0.
    Default 1 for HTTP/1.1.  
    Set to zero for HTTP/1.0
 + `major_version`: the major HTTP version number. Default 1 for HTTP/1.1.


There are also "set" and "add" functions in the `tx_response` class so that the 
status, headers, etc. can be set after the class has been constructed, i.e.:  

 + `void set_status(response_status::status_code status)`  
    Set a standard HTTP response status (defined in `<via/http/response_status.hpp>`)
 + `void set_status_and_reason(int status, const std::string& reason_phrase)`  
    Set a non- standard response status and the reason phrase associated with it.
 + `void add_header(header_field::field_id id, const std::string& value)`  
    Add a standard HTTP header field (defined in `<via/http/header_field.hpp>`)
    to the response.
 + `void add_header(std::string const& field, const std::string& value)`  
    Add a non-standard header field  to the response.
 + `void set_minor_version(int minor_version)` Set the HTTP minor version.
 + `void set_major_version(int major_version)` Set the HTTP major version.


Note: The `tx_response` class will set the following HTTP headers when required.
The application should **NOT** set the following headers:  

 + Content-Length
 + Date
 + Transfer-Encoding: Chunked
 + Server

### Sending Responses ###

All responses are sent via an `http_connection::weak_pointer`, i.e. the first 
parameter in the request handler.

The `http_connection::weak_pointer` is a *non-owning* pointer to an `http_connection`.
It must be converted to an `http_connection::shared_pointer` in order to access the
`http_connection`. This is achieved by calling the `lock()` function to assume
temporary ownership of the `http_connection`.  
Note: the `lock()` function may return NULL if there is no longer an *owning*
pointer to the `http_connection`.

When the response is sent from the request handler, the `weak_pointer` will be valid
and there is no no need to test whether the call to `lock()` succeeded, e.g.:

      weak_ptr.lock()->send(response);

However, if the response is sent asynchronously by storing the `weak_pointer` and
calling it later. Then the call to `lock()` may fail and the conversion to an
`http_connection::shared_pointer` should be tested, e.g.:

    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
      connection->send(response);
    else
      std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;

All responses are sent using one of the `http_connection send` functions.
There are two types of `send` functions:

 + bool send(http::tx_response& response);
 + bool send(http::tx_response& response, Container const& body);

The first sends a response without a body, the second sends a response with a body.
However, in the second case, the body may be of zero length, in which case the body
will not be sent in the HTTP response like the first type.

There are other variations of send functions taking move parameters and iterators,
but they are all either for simple responses or responses with bodies.

All `http_connection send` functions return a boolean which is true if the connection
is left open, false if the connection will be closed after the response has been sent.

### Sending Chunks ###

All chunks are sent via an `http_connection::weak_pointer`, like requests above.

Chunks are a way of sending very large bodies or continuous responses.
The `http_connection send_chunk` functions all take a chunk parameter and an
optional chunk extension, e.g.:

    bool send_chunk(Container const& chunk, std::string extension = "");
  
Like the `http_connection send` functions above they return a boolean about the state
of the connection. After the last chunk of a response has been sent, the `last_chunk`
function should be called, e.g.:

    bool last_chunk(std::string extension = "", std::string trailer_string = "");

### Note on storing `http_connection` pointers ###

Whenever the application stores `http_connection` pointers in order to send
asynchronous responses or chunks it should always store them as
`http_connection::weak_pointer` **NOT** `http_connection::shared_pointer`.
 
The conversion to `http_connection::shared_pointer` promotes it to an *owning*
pointer, it is only intended to be used temporarily.
If the `http_connection::shared_pointer` is stored, it becomes a co-owner of the
`http_connection` and `http_server` can no longer delete the `http_connection`
whenever it's disconnected.