# via-httplib: A C++ HTTP Library

[![C/C++ CI](https://github.com/kenba/via-httplib/workflows/C/C++%20CI/badge.svg)](https://github.com/kenba/via-httplib/actions)
[![License](https://img.shields.io/badge/License-Boost%201.0-blue)](https://opensource.org/licenses/BSL-1.0)
[![Coverage Status](https://coveralls.io/repos/github/kenba/via-httplib/badge.svg?branch=main)](https://coveralls.io/github/kenba/via-httplib?branch=main)

A header-only library for embedding **HTTP** or **HTTPS**, **IPV6** and **IPV4** servers in C++ applications.

`via-httplib` is an asynchronous communications library built upon `asio` (either
[boost](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio.html) or
[standalone](http://think-async.com/)) to enable simple, secure and efficient
HTTP/HTTPS servers to be built that comply with
[rfc7230](https://tools.ietf.org/html/rfc7230) wherever possible.

## Requirements

+ A C++17 compliant compiler.  
This version requires a complier that supports string_view.  
It's been tested with `MSVC 2022`, `Clang 14.0.0`, `GCC 11.3.0` and `MinGw 11.2.0`.  
C++11 versions are available in tags `1.6.0` and lower.  

+ The `asio` C++ library.  
Either [standalone asio](http://think-async.com/) or [boost asio](http://www.boost.org/).  

+ For: multithreading, authentication and/or the request router, the relevant [boost](http://www.boost.org/) libraries are required.  
However, if they are not required, the library can use [standalone asio](http://think-async.com/) on its own.

+ For HTTPS: the `OpenSSL` library, see [openssl](http://www.openssl.org/).  

+ For C++ code documentation: [Doxygen](https://doxygen.nl/)

## Getting Started

Download the latest tagged version of `via-httplib` from
[Github](https://github.com/kenba/via-httplib)
and follow the instructions here: [Make](docs/MAKE.md).  

`via-http` lib depends on the `standalone asio` or `boost` libraries.  
If `boost` is not installed on your machine then download the latest package from
[boost](http://www.boost.org/) and follow the instructions here:
[boost getting started](http://www.boost.org/doc/libs/1_76_0/more/getting_started/index.html).
Otherwise `standalone asio` can be downloaded from: [asio](http://think-async.com/).

If you require an **HTTPS** server or client then you must also install the
[OpenSSL](https://www.openssl.org/) library.  
Please note that a plain **HTTP** server does **not** require `OpenSLL`.

The [Server User Guide](docs/Server.md) explains how to use the library to create HTTP servers and
the [Client User Guide](docs/Client.md) explains how to use the library to create HTTP clients.

An example https server [docker](https://www.docker.com/) image can be built using
the following commands from this directory:

```shell
docker build . -t via-http-server:1.10.0
```

## Example HTTP Server

The following code implements an HTTP server that responds to GET requests to the
`/hello` and `/hello/:name` endpoints:

```C++
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_server.hpp"
#include "via/http/request_router.hpp"
#include <iostream>

/// Define an HTTP server using std::string to store message bodies
typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
typedef http_server_type::http_connection_type http_connection;
typedef http_server_type::http_request http_request;

namespace
{
  using namespace via::http;

  /// /hello request handler
  tx_response get_hello_handler(http_request const&, //request,
                                Parameters const&, //parameters,
                                std::string const&, // data,
                                std::string &response_body)
  {
    response_body += "Hello, whoever you are?!";
    return tx_response(response_status::code::OK);
  }

  /// /hello/:name request handler
  tx_response get_hello_name_handler(http_request const&, //request,
                                     Parameters const& parameters,
                                     std::string const&, // data,
                                     std::string &response_body)
  {
    response_body += "Hello, ";
    auto iter(parameters.find("name"));
    if (iter != parameters.end())
      response_body += iter->second;

    return tx_response(response_status::code::OK);
  }
}

int main(int /* argc */, char *argv[])
{
  std::string app_name(argv[0]);
  unsigned short port_number(via::comms::tcp_adaptor::DEFAULT_HTTP_PORT);
  std::cout << app_name << ": " << port_number << std::endl;

  try
  {
    // The asio io_context.
    boost::asio::io_context io_context;

    // Create the HTTP server
    http_server_type http_server(io_context);

    // Attach the request method handlers
    http_server.request_router().add_method("GET", "/hello", get_hello_handler);
    http_server.request_router().add_method(request_method::GET, "/hello/:name",
                                            get_hello_name_handler);

    // Accept connections (both IPV4 and IPV6) on the default port (80)
    boost::system::error_code error(http_server.accept_connections());
    if (error)
    {
      std::cerr << "Error: "  << error.message() << std::endl;
      return 1;
    }

    // Start the server
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception:"  << e.what() << std::endl;
    return 1;
  }

  return 0;
}
```

## Documentation

| Document | Description |
|----------|-------------|
| [Build Guide](docs/MAKE.md) | How to build the library. |
| [Server User Guide](docs/Server.md) | How to use the library to create HTTP servers. |
| [Client User Guide](docs/Client.md) | How to use the library to create HTTP clients. |
| [Security Guide](docs/Server_Security.md) | HTTP Server security considerations. |
| [Design](docs/Design_Top.md) | The library design. |
| [examples/server](examples/server) | Example HTTP & HTTPS servers. |
| [examples/client](examples/client) | Example HTTP & HTTPS clients. |

## Directory Structure

| Directory            | Contents                                                                 |
|----------------------|--------------------------------------------------------------------------|
| [via](include/via)           | The `via-httplib` API classes: [http_server](include/via/http_server.hpp), [http_connection](include/via/http_connection.hpp) and [http_client](include/via/http_client.hpp). |
| [examples/server](examples/server) | Example HTTP & HTTPS servers.                              |
| [examples/client](examples/client) | Example HTTP & HTTPS clients.                              |
| [tests](tests)       | Unit tests for the HTTP parsers and encoders.                            |
| [docs](docs)         | The User Guides and design documents.                                    |
| `docs/html`          | [Doxygen](https://doxygen.nl/) output directory. Created by running `doxygen Doxyfile` in the [docs](docs) directory. |

## License

Licensed under the Boost License, Version 1.0, as per Boost.  
You may obtain a copy of the License at: <https://www.boost.org/LICENSE_1_0.txt>

## Acknowledgements

Thanks to:

+ **Neil Tisdale** for encouraging and inspiring me to create the library
+ **Louis Nayegon** for helping to develop it and recommending GitHub  
+ **Adam Leggett** for helping to identify and fix security, efficiency and CMake issues
+ **Christopher Kohlhoff** for the `asio` library, without which, this library wouldn't exist.
