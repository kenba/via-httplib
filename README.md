via-httplib: A C++ HTTP Library
===============================

A library for embedding an HTTP or HTTPS server in C++ applications.

The library provides an asynchronous C++ HTTP server that attempts to comply with
[rfc2616](www.w3.org/Protocols/rfc2616/rfc2616.html) wherever possible.

`via::http_server` is a class template requiring a `socket_adaptor` to instantiate it.
For example the following code declares an plain HTTP server that passes data in a
`std::vector<char>` (the default).

    #include "via/comms/tcp_adaptor.hpp"
    #include "via/http_server.hpp"
    
    typedef via::http_server<via::comms::tcp_adaptor> http_server_type;
    
Whilst the example below declares an HTTPS server that passes data in a `std::string`.

    #include "via/comms/ssl/ssl_tcp_adaptor.hpp"
    #include "via/http_server.hpp"
    
    typedef via::http_server<via::comms::ssl::ssl_tcp_adaptor, std::string> https_server_type;

The library also contains C++ HTTP and HTTPS client code see: [Clients](docs/CLIENT.md).
    
Requirements
------------

+ The `boost` C++ library, especially `asio`, see [boost](http://www.boost.org/). Tested with version 1.57.

+ A C++11 compiler, it's been tested with `MSVC 2013`, `GCC 4.9.1` and `MinGw 4.9.1`.
If you need a C++03 version then download tag 1.0.3 or from the C++03 branch, but please be aware of this warning: [old compilers](http://www.boost.org/users/news/old_compilers.html)  
Note: versions of GCC/MinGW prior to 4.9 didn't fully implement std::regex, see:
[GCC Bug 53631](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631)

+ For HTTPS, the `OpenSSL` library, see [openssl](http://www.openssl.org/).

Structure
---------
  
An outline HTTP Server class diagram:
  
![HTTP Server Classes](uml/http_server_classes.png)

An outline HTTP Client class diagram:

![HTTP Client Classes](uml/http_client_classes.png)

Directory structure and contents:

| Directory            | Contents                                                                 |
|----------------------|--------------------------------------------------------------------------|
| [via](via)           | The `via-httplib` API classes: [http_server](via/http_server.hpp), and [http_client](http_client.hpp). |
| `via/comms`          | The TCP/IP communications software.                                      |
| `via/comms/ssl`      | The SSL specific TCP/IP communications software.                         |
| `via/http`           | HTTP parsers and encoders.                                               |
| `examples`           | Sample SSL certificates for the SSL examples.                            |
| [examples/server](examples/server) | Example HTTP servers.                                      |
| [examples/client](examples/client) | Example HTTP clients.                                      |
| `tests`              | A main function for the `boost Test` library.                            |
| `tests/http`         | HTTP parser and encoder tests.                                           |
| `docs`               | [Make](docs/MAKE.md), [User Guide](docs/USE.md), [Server Security](docs/SECURITY.md) and [Design Notes](docs/DESIGN.md) |
| `uml`                | UML diagrams in [PlantUML](http://plantuml.sourceforge.net/index.html) format. |

Installing
----------

Download the latest tagged version of `via-httplib` from
[Github](https://github.com/kenba/via-httplib)
and follow the instructions here: [Make](docs/MAKE.md).  
Or simply build the .cpp files into your application (there are only 8 of them).

`via-http` lib depends on the `boost` libraries.
If `boost` is not installed on your machine then download the latest package from
[boost](http://www.boost.org/) and follow the instructions here:
[boost getting started](http://www.boost.org/doc/libs/1_57_0/more/getting_started/index.html).

The `boost asio` library (and hence `via-httplib`) depends upon the
`OpenSSL` library to implement SSL/TLS sockets.
If you require an **HTTPS** server or client then you'll need to install the
`OpenSSL` library as well.
Please note that a plain HTTP server should *not* require `OpenSLL`.

If `OpenSLL` is not installed on your machine then download the latest stable
package from [openssl source](http://www.openssl.org/source/) and build it.
Note: a binary distribution may be available for your machine,
see: [OpenSSL binaries](http://www.openssl.org/related/binaries.html),
which could save you a lot of trouble, since building the `OpenSSL` binaries can
be a long-winded process...  

Example
-------

A simple HTTP server ([`simple_http_server.cpp`](examples/server/simple_http_server.cpp)):  

	#include "via/comms/tcp_adaptor.hpp"
	#include "via/http_server.hpp"
	#include <iostream>
	
	/// Define an HTTP server using std::string to store message bodies
	typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
	typedef http_server_type::http_connection_type http_connection;
	
	namespace
	{
	  /// The handler for incoming HTTP requests.
	  /// Prints the request and responds with 200 OK.
	  void request_handler(http_connection::weak_pointer weak_ptr,
	                       via::http::rx_request const& request,
	                       std::string const& body)
	  {
	    std::cout << "Rx request: " << request.to_string();
	    std::cout << "Rx headers: " << request.headers().to_string();
	    std::cout << "Rx body: "    << body << std::endl;
	
	    via::http::tx_response response(via::http::response_status::code::OK);
	    response.add_server_header();
	    response.add_date_header();
	    weak_ptr.lock()->send(response);
	  }
	}
    
    int main(int argc, char *argv[])
	{
	  try
	  {
	    // The asio io_service.
	    boost::asio::io_service io_service;
	
	    // Create the HTTP server, attach the request handler
	    // and accept IPV4 connections on the default port (80)
	    http_server_type http_server(io_service);
	    http_server.request_received_event(request_handler);
	    boost::system::error_code error(http_server.accept_connections());
	    if (error)
	    {
	      std::cerr << "Error: "  << error.message() << std::endl;
	      return 1;
	    }
	
	    // Start the server
	    io_service.run();
	  }
	  catch (std::exception& e)
	  {
	    std::cerr << "Exception:"  << e.what() << std::endl;
	    return 1;
	  }
	
	  return 0;
	}
    
This server will output all of the requests that it receives to std::cout and respond with a `200 OK` response to each of them.
e.g. a request from Google Chrome on a Windows 7 PC:

    Rx request: GET /hello HTTP/1.1
    Rx headers: accept: */*
    accept-encoding: gzip,deflate,sdch
    accept-language: en-GB,en-US;q=0.8,en;q=0.6
    connection: keep-alive
    content-type: text/plain; charset=utf-8
    host: 127.0.0.1
    user-agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36

    Rx body:

e.g. a request from Internet Explorer on a Windows 7 PC:

    Rx request: GET /hello HTTP/1.1
    Rx headers: accept: text/html, application/xhtml+xml, */*
    accept-encoding: gzip, deflate
    accept-language: en-GB
    connection: Keep-Alive
    dnt: 1
    host: 127.0.0.1
    user-agent: Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0)

    Rx body:

An equivalent simple HTTPS server can be found here:[`simple_https_server.cpp`](examples/server/simple_https_server.cpp)

Further Information
-------------------

[Make](docs/MAKE.md)

[User Guide](docs/USE.md)

[Server Security](docs/SECURITY.md)

[Design Notes](docs/DESIGN.md)
