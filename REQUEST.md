# Request Handlers #

Every application that uses `http_server` must register a handler for the 
`request_received_event` so that it can respond to HTTP requests from clients.

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

The request handler should query the `rx_request` class to determine how to handle 
the request and then send it's response via the connection pointer, e.g:

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
    }

In the example above, the `request_handler` will do the following:

+ send a "200 OK" response to a "GET" request to the uri: "/hello".
+ send a "404 Not Found" response to a request to any other uri
+ send a "405 Method Not Allowed" response to any other request to the uri: "/hello"
  with the HTTP header "Allow: GET".

Note: the server may also respond with "200 OK" response to a "HEAD" request to
the uri: "/hello" depending upon how `http_server` is configured.
If the translate_head flag was set in the class template then `http_server`
will pass the "HEAD" request to the `request_handler` as a "GET" request and
send the response without a message body.

## Class `rx_request` ##

The `rx_request` class is defined in `<via/http/request.hpp>`.

It contains all of the information from the HTTP request header. 
It can be read from the `rx_request` class using the following functions:

+ `const std::string& method() const;`  
  The HTTP request method string.
+ `const std::string& uri() const;`  
  The HTTP uri string.
+ `int major_version() const;`  
  The HTTP major version number.
+ `int minor_version() const;`  
  The HTTP minor version number.

The HTTP request header fields are stored in a `message_headers` class.
The function to read the headers in the `rx_request` class is:
`const message_headers& headers() const;`

### Class `message_headers` ###

The `message_headers` class is defined in `<via/http/headers.hpp>`.

It is used to store the received HTTP headers for both requests and responses.
The `message_headers` class provides a couple of `find` functions to search for a
specific header in the received `message_headers`:

+ `const std::string& find(header_field::field_id id) const;`  
  For standard header fields: `header_field::field_id`
  is defined in `<via/http/header_field.hpp>`.
+ `const std::string& find(const std::string& name) const;`  
  For non-standard header fields, although it will find standard header fields
  if given a standard header field name.

Both versions return a string containing the value of the header field.
The string will be empty if the header field wasn't present or was empty.

Note: the header field name given to the `string` version of the `find` function
**must** be in lower case, as `via-httplib` converts all header field names to
lower case whilst parsing the HTTP headers.

The `message_headers` class (and `rx_request` class) also contain functions to
access data for some of the most common headers directly, e.g:

+ Get the size of the message body: `size_t content_length() const;`
+ Is the request chunked?: `bool is_chunked() const;`

## Responses ##

All responses must contain an instance of `via::http::tx_response`.

### Class `tx_response` ###

The `tx_response` class is defined in `<via/http/response.hpp>`.

The class has two Constructors:

    explicit tx_response(response_status::status_code status,
                         std::string header_string = "",
                         bool is_chunked = false,
                         int minor_version = 1,
                         int major_version = 1);

and

    explicit tx_response(const std::string& reason_phrase,
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

### Response Headers ###

An application may set the HTTP response field headers using `header_string` in
the `tx_response` Constructor or the `tx_response::add_header` methods or any 
combination. However, be aware that `via-httplib` will set the following headers as
described below and so the application should **NOT** set the following headers:

 + **Content-Length**  
   Set to the size of **all** message bodies, unless `is_chunked`.  
   Even in response to a HEAD request when the message body is **not** sent to the client.
 + **Date**  
   Set to the current data and time, unless `http_server` is configured with
   `has_clock` = false.
 + **Transfer-Encoding: Chunked**  
   Set whenever `is_chunked` is true in the the `tx_response` Constructor.
 + **Server**  
   Always set to "Via-httplib/tag"

## Sending Responses ##

All responses are sent via an `http_connection::weak_pointer`, i.e. the first 
parameter in the request handler.

The `http_connection` class is defined in `<via/http_connection.hpp>`.

The `http_connection::weak_pointer` is a *non-owning* pointer to an `http_connection`.
It must be converted to an `http_connection::shared_pointer` in order to access the
`http_connection`. This is achieved by calling the `lock()` function to assume
temporary ownership of the `http_connection`.  
Note: the `lock()` function may return NULL if there is no longer an *owning*
pointer to the `http_connection`.

When the response is sent from the request handler, the `weak_pointer` will be valid
and there is no need to test whether the call to `lock()` succeeded, e.g.:

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

+ `bool send(http::tx_response& response);`  
   Sends a response without a body.
+ `bool send(http::tx_response& response, Container const& body);`  
   Send a response with a body. However, the body may be of zero length, in which case the body will not be sent in the HTTP response.
   Note: a body will **never** be sent in response to a HEAD request although it's size may be set in the Content-Length header.

All `http_connection send` functions return a boolean which is true if the connection
is left open, false if the connection will be closed after the response has been sent.

There are other variations of send functions taking move parameters and iterators,
but they are all either for simple responses or responses with bodies, as above.
