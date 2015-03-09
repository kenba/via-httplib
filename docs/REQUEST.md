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

+ `const std::string& find(header_field::id field_id) const;`  
  For standard header fields: `header_field::id` is defined in `<via/http/header_field.hpp>`.
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

    explicit tx_response(response_status::code status_code,
                         std::string header_string = "");

and

    explicit tx_response(const std::string& reason_phrase,
                         int status,
                         std::string header_string = "");

The first constructor just requires a `via::http::response_status::code`,
this is an enumeration of the standard HTTP response status codes, see:
`<via/http/response_status.hpp>`

The second constructor requires a string and an integer.
It enables the application to send a non-standard HTTP response status.

The other parameter for both is `header_string`: a string containing one or more
response headers. HTTP headers are just strings of the form "Header: Value\r\n". The application can set any number of HTTP headers in this string, simply by concatenating
them together in this parameter.

There are also "set" and "add" functions in the `tx_response` class so that the 
status and headers can be set after the class has been constructed, i.e.:  

 + `void set_status(response_status::code status)`  
    Set a standard HTTP response status (defined in `<via/http/response_status.hpp>`)
 + `void set_status_and_reason(int status, const std::string& reason_phrase)`  
    Set a non- standard response status and the reason phrase associated with it.
 + `void add_header(header_field::id field_id, const std::string& value)`  
    Add a standard HTTP header field (defined in `<via/http/header_field.hpp>`)
    to the response.
 + `void add_header(std::string const& field, const std::string& value)`  
    Add a non-standard header field  to the response.
 + `void add_date_header()`  
    Add a `Date` header with the current date and time to the response.
 + `void add_server_header()`  
    Add a `Server` header with the current version of `via-httplib` to the response.
 + `add_content_length_header(size_t size)`  
    Add a `Content-Length` header with the given size to the response.

### The `Content-Length` Header ###

Although not required by [rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html),
some browsers do not accept responses without a `Content-Length` Header, so the
`tx_response` class ensures that it (or a `Transfer-Encoding` header, see
[rfc2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html) section 4.4 para 3)
is always present.

In short, this means an application need never set a `Content-Length` header in
`tx_response` unless `translate_head` is disabled (see class `http_server` above)
and it's responding to a `HEAD` request using a send function without a body,
see below.

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
   Note: a body will **never** be sent in response to a HEAD request although it's size may be used in the Content-Length header.

All `http_connection send` functions return a boolean which is true if the connection
is left open, false if the connection will be closed after the response has been sent.

There are other variations of send functions taking move parameters and iterators,
but they are all either for simple responses or responses with bodies, as above.
