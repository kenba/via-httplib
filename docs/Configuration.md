# Software Configuration

## Boost / Standalone `asio` Configuration

The library uses [boost asio](http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio.html) by default.  
To use [standalone asio](http://think-async.com/):

   + set the environment variable `$ASIO_ROOT` to the path of the `asio` root directory;
   + add `$ASIO_ROOT/include` to your include path;
   + define the macro `ASIO_STANDALONE`.
   
Note: if you use `qmake` file and include the file [via-httplib.pri](via-httplib.pri) then you just
need to set the `$ASIO_ROOT` environment variable.

Portability between `bost asio` and `standalone asio` is provided by the macros:

   + ASIO,
   + ASIO_ERROR_CODE and
   + ASIO_TIMER.

They are defined in [socket_adaptor.hpp](include/via/comms/socket_adaptor.hpp):
   
	#ifdef ASIO_STANDALONE
	  #include <asio.hpp>
	  #define ASIO asio
	  #define ASIO_ERROR_CODE asio::error_code
	  #define ASIO_TIMER asio::steady_timer
	#else
	  #include <boost/asio.hpp>
	  #define ASIO boost::asio
	  #define ASIO_ERROR_CODE boost::system::error_code
	  #define ASIO_TIMER boost::asio::deadline_timer
	#endif
	
It is hoped that they can continue to provide portability when `asio` becomes a standard C++ library:
see: [Networking Library Proposal](http://open-std.org/JTC1/SC22/WG21/docs/papers/2015/n4478.html).

## HTTP Server Configuration

An HTTP Server is created from a template class:

    template <typename SocketAdaptor, typename Container = std::vector<char>,
              bool use_strand = false>
    class http_server
    .
    .
    .
    
The `SocketAdaptor`, `Container` and `use_strand` template parameters configure:
SSL/TLS comms, data/text data bodies and multithreading.

### SSL / TLS Configuration

A `via::http_server`, a requires a `SocketAdaptor` to instantiate it:

 + a `tcp_adaptor` for a plain **HTTP** server
 + an `ssl_tcp_adaptor` for an **HTTPS** server  
 
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

E.g.

    // An HTTP data server.
    typedef via::http_server<via::comms::tcp_adaptor> http_data_server_type;
    
    // An HTTP text server.
    typedef via::http_server<via::comms::tcp_adaptor, std::string> http_text_server_type;
    
    // An HTTPS data server.
    typedef via::http_server<via::comms::ssl_tcp_adaptor> https_data_server_type;
    
    // An HTTPS text server.
    typedef via::http_server<via::comms::ssl_tcp_adaptor, std::string> https_text_server_type;
    

### Multithreading Configuration

The server can be configured to use run the `asio::io_service` in multiple threads
(in a thread pool) by setting `use_strand`, e.g.:

    // A multithreading HTTP text server.
    typedef via::http_server<via::comms::tcp_adaptor, std::string, true> http_server_type;
    .
    .
    .

    // Create a thread pool for the threads and run the asio io_service
    // in each of the threads.
    std::vector<std::shared_ptr<std::thread>> threads;
    for (std::size_t i = 0; i < no_of_threads; ++i)
    {
      std::shared_ptr<std::thread> thread(std::make_shared<std::thread>
                           ([&io_service](){ io_service.run(); }));
      threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (std::size_t i(0); i < threads.size(); ++i)
      threads[i]->join();

 
## IPV6 / IPV4 Configuration

Whether a server accepts IPV6 and IPV4 connections or just IPV4 connections
depends upon whether `http_server.accept_connections` is called with the
`ipv4_only` parameter set:

 + **false**, (the default) the server accepts both IPV6 and IPV4 connections
 + **true**, the server only accepts IPV4 connections  
