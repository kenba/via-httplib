# HTTP Request Routing #

The `http_server` contains an internal `request_router` for routing
HTTP requests to their relevant handler functions, e.g.:

    tx_response get_hello_handler(rx_request const&, //request,
                              Parameters const&, //parameters,
                              std::string const&, // data,
                              std::string &response_body)
    {
      response_body += "Hello, whoever you are?!";
      return tx_response(response_status::code::OK);
    }

    http_server.request_router().add_method("GET", "/hello", get_hello_handler);
    
In the example above, `get_hello_handler` will be called whenever an HTTP GET
request is received for the resource at "/hello".

## Request Handlers

The signature of a request handler function is:

    typedef std::function<tx_response (rx_request const& request,
                                       Parameters const& parameters,
                                       Container const& data,
                                       Container& response_body)> Handler;
                                       
where:
- request is the received HTTP request containing the request line and headers
- parameters are the parameters extracted from the uri, see below
- data is the body (if any) contained in the request
- response_body is the body (if any) to be sent in the response.
- tx_response is the HTTP response header

## Registering Handlers

A handler is registered for each HTTP method/uri combination by calling add_method, e.g.:

    http_server.request_router().add_method("GET", "/hello", get_hello_handler);
    http_server.request_router().add_method("DELETE", "/hello", delete_hello_handler);

The declaration of the `add_method` function is:

    bool add_method(std::string const& method, std::string const& path,
                    Handler handler,
                    authentication::authentication const* auth_ptr = nullptr);

Note: the method takes an optional authentication pointer for registering
an authentication object with the handler. Currently there is only a `basic`
authentication class avaailable (in namespace `authentication`) however, basic`
authentication can be made secure when used over SSL/TLS connections.

## URI Path Parameters

The `path` in the `add_method` call can be a simple uri path, e.g.: /hello/world
or it may contain parameters that can be extracted by the router,
e.g.: /hello/:name

Each parameter name starts with a colon (:). The request router will match the
parameter to anything and copy whaterver it finds into a map paired with it's
parameter name.

## Example

See: [`routing_http_server.cpp`](../examples/server/routing_http_server.cpp)
