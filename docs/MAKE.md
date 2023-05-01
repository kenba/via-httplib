# Build Guide

The library is header only, so the library `include` directory just needs to be
added to the include path.

## Boost / Standalone `asio` Library

The library uses [boost asio](http://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio.html) by default.  
To use [standalone asio](http://think-async.com/):

+ set the environment variable `$ASIO_DIR` to the path of the `asio` root directory,
+ or add `$ASIO_DIR/include` to your include path and  define the macro `ASIO_STANDALONE`.

Portability between `boost asio` and `standalone asio` is provided by the macros:

+ ASIO,
+ ASIO_ERROR_CODE and
+ ASIO_TIMER.

defined in [socket_adaptor.hpp](include/via/comms/socket_adaptor.hpp):

```C++
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
```

It is hoped that they can continue to provide portability when `asio` becomes a standard C++ library:
see: [Networking Library Proposal](http://open-std.org/JTC1/SC22/WG21/docs/papers/2015/n4478.html).

## Recommended Windows build steps

Set the environment variable OPENSSL\_ROOT\_DIR to the location of where **OpenSSL** has been installed.

Set the environment variable BOOST\_ROOT to the location of where **Boost** has been installed,  
or set the environment variable ASIO\DIR to the location of where **Asio** has been installed.

Set the environment variable VIAHTTPLIB to the location of where **via-httplib** has been installed.

## Tools, and their versions, currently built against

+ Asio version 1.28.0
+ Boost version 1.82.0
+ CMake version 3.26.3
+ GCC version 12.1.0
+ MinGW version 11.2.0
+ OpenSSL version 3.1.0
+ Visual Studio 2022
