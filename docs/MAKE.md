# Build Guide

The library is header only, so the library `include` directory just needs to be
added to the include path.

## Boost / Standalone `asio` Library

The library uses [boost asio](http://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio.html) by default.  
To use [standalone asio](http://think-async.com/):

+ set the environment variable `$ASIO_ROOT` to the path of the `asio` root directory;
+ add `$ASIO_ROOT/include` to your include path;
+ define the macro `ASIO_STANDALONE`.

Note: if you use `qmake` file and include the file [via-httplib.pri](via-httplib.pri) then you just
need to set the `$ASIO_ROOT` environment variable.

Portability between `boost asio` and `standalone asio` is provided by the macros:

+ ASIO,
+ ASIO_ERROR_CODE and
+ ASIO_TIMER.

They are defined in [socket_adaptor.hpp](include/via/comms/socket_adaptor.hpp):

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
or set the environment variable ASIO\_ROOT to the location of where **Asio** has been installed.

Set the environment variable VIAHTTPLIB to the location of where **via-httplib** has been installed.

## Tools, and their versions, currently built against

+ CMake version 3.20.1
+ OpenSSL version 1.1.1
+ Boost version 1.76.0
+ GCC version 9.3.0
+ MinGW version 8.1.0
+ Visual Studio 2019
