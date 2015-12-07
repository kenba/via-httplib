via-httplib: A C++ HTTP Library
===============================

A library for embedding **HTTP** or **HTTPS**, **IPV6** and **IPV4** servers in C++ applications.

`via-httplib` is an asynchronous C++ HTTP server built upon `boost asio` that
aims to provide a simple, secure and efficient server that complies with the
requirements of [rfc7230](https://tools.ietf.org/html/rfc7230)
wherever possible.

### SSL / TLS Configuration

The server is `via::http_server`, a class template requiring a
`SocketAdaptor` to instantiate it:

 + a `tcp_adaptor` for a plain **HTTP** server
 + an `ssl_tcp_adaptor` for an **HTTPS** server  
 
### IPV6 / IPV4 Configuration

Whether the server accepts IPV6 and IPV4 connections or just IPV4 connections
depends upon how the port is configured:

 + **IPV6**, (the default) the server accepts both IPV6 and IPV4 connections
 + **IPV4 only**, the server only accepts IPV4 connections  
 
### Data / Text Configuration

The server can be configured to pass HTTP message bodies in different types of
containers, e.g.:

   + `std::vector<char>` (the default) for handing binary data, e.g. images, files, etc.
   + `std::string` for handing textural data, e.g. HTML, JSON, etc.
  
| Socket Adaptor    | Container         | Description                   |
|-------------------|-------------------|-------------------------------|
| `tcp_adaptor`     | `std::vector<char>`   | An HTTP data server.  |
| `tcp_adaptor`     | `std::string`     | An HTTP text server.          |
| `ssl_tcp_adaptor` | `std::vector<char>`   | An HTTPS data server. |
| `ssl_tcp_adaptor` | `std::string`     | An HTTPS text server.         |

The HTTP message bodies can be sent using **buffered** or **unbuffered** methods.  
The unbuffered methods use "scatter-gather" writes to avoid copying data.

Requirements
------------

+ A C++11 compiler.   
This version requires a complier that supports:  lambdas, enum classes, member function delete
and std::functional. It's been tested with `MSVC 2015`, `MSVC 2013`, `GCC 4.9.1` and `MinGw 4.9.1`.  

+ The `boost` C++ library, especially `asio`, see [boost](http://www.boost.org/).

+ For HTTPS, the `OpenSSL` library, see [openssl](http://www.openssl.org/).

+ For C++ code documentation, Doxygen, see [Doxygen](http://www.stack.nl/~dimitri/doxygen/)

Getting Started
---------------

Download the latest tagged version of `via-httplib` from
[Github](https://github.com/kenba/via-httplib)
and follow the instructions here: [Make](docs/MAKE.md).  
Or simply build the .cpp files into your application (there are 8 of them).

`via-http` lib depends on the `boost` libraries.
If `boost` is not installed on your machine then download the latest package from
[boost](http://www.boost.org/) and follow the instructions here:
[boost getting started](http://www.boost.org/doc/libs/1_59_0/more/getting_started/index.html).

The `boost asio` library (and hence `via-httplib`) depends upon the
`OpenSSL` library to implement SSL/TLS sockets.
If you require an **HTTPS** server or client then you'll need to install the
`OpenSSL` library as well.
Please note that a plain HTTP server should *not* require `OpenSLL`.

If `OpenSLL` is not installed on your machine then you may download the latest stable
package from [openssl source](http://www.openssl.org/source/) and build it.
Note: a binary distribution may be available for your machine,
see: [OpenSSL binaries](https://wiki.openssl.org/index.php/Binaries).

| Document | Description |
|----------|-------------|
| [Build Guide](docs/MAKE.md) | How to build the library. |
| [Server User Guide](docs/Server.md) | How to use the library to create HTTP servers. |
| [Client User Guide](docs/Client.md) | How to use the library to create HTTP clients. |
| [Security Guide](docs/Server_Security.md) | How to configure the library securely. |
| [Design](docs/Design_Top.md) | The library design. |
| [examples/server](examples/server) | Example HTTP & HTTPS servers. |
| [examples/client](examples/client) | Example HTTP & HTTPS clients. |

Namespace Structure
-------------------

![Via Namespaces](docs/images/via_namespaces.png)

Directory Structure
-------------------

| Directory            | Contents                                                                 |
|----------------------|--------------------------------------------------------------------------|
| [via](include/via)           | The `via-httplib` API classes: [http_server](include/via/http_server.hpp), [http_connection](include/via/http_connection.hpp) and [http_client](include/via/http_client.hpp). |
| [examples/server](examples/server) | Example HTTP & HTTPS servers.                              |
| [examples/client](examples/client) | Example HTTP & HTTPS clients.                              |
| `tests`              | A unit tests for the HTTP parsers and encoders.                          |
| [docs](docs)         | The User Guides and design documents.                                    |
| `docs/html`          | [Doxygen](http://www.stack.nl/~dimitri/doxygen/) output directory. Created by running `doxygen Doxyfile` in the [docs](docs) directory. | 

Acknowledgements
----------------

Thanks to:
 + **Neil Tisdale** for encouraging and inspiring me to create the library
 + **Louis Nayegon** for helping to develop it and recommending GitHub  
 + **Adam Leggett** for helping to identify and fix security, efficiency and CMake issues
 + **Christopher Kohlhoff** for the `asio` library, without which, this library wouldn't exist.
 