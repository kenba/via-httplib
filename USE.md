Use
===

The purpose of this document is to help you understand how to use the library
to implement the HTTP protocol in your applications.

The library implements the HTTP 1.1 communications protocol as defined in
RFC2616 and some of the terms used (e.g. chunk) are taken directly from RFC2616.
If you don't know the protocol then I recommend that you read a good
introduction to HTTP e.g.: [HTTP Made Really Easy](http://www.jmarshall.com/easy/http/)
or if your suffer from insomnia: [RFC2616](http://www.ietf.org/rfc/rfc2616.txt)
itself, before using the library.

Instantiating the Classes
-------------------------

The library is designed using the "templated bridge" design pattern,
i.e. a bridge design pattern (see GoF pg 151), but implemented in templates.

The key classes for creating HTTP servers and clients are: http\_server and
http\_client respectively:

    template <typename SocketAdaptor, typename Container = std::vector<char> >
    class http_server
	
	template <typename SocketAdaptor, typename Container = std::vector<char> >
    class http_client
	
To use these template classes they must first be instantiated with a 
SocketAdaptor.

There are two types of SocketAdaptor:  
* tcp\_adaptor - supports TCP/IP communicatons for plain connections.  
* ssl\_tcp\_adaptor - supports SSL/TLS communicatons for encrypted connections.

It is recommended to use typedefs to define the instantiated classes. So a
plain HTTP server would be:

     typedef via::http_server<via::comms::tcp_adaptor> http_server_type;  
And an encrypted HTTPS client would be:

     typedef via::http_client<via::comms::ssl::ssl_tcp_adaptor> http_server_type;

Also note that a server creates connections whenever a client connects to it.
So the plain HTTP server above would create plain HTTP connections:

	typedef via::http_connection<via::comms::tcp_adaptor> http_connection_type;
	
Constructing the Classes
------------------------

The library uses the boost::asio library to implement it's communications.
The asio library bases it's communications around a io\_service object and so
both the server and client require a boost::asio::io\_service in their
constructors:

    explicit http_server(boost::asio::io_service& io_service,
                         unsigned short port = SocketAdaptor::DEFAULT_HTTP_PORT);
				
	explicit http_client(boost::asio::io_service& io_service);
    
Normally, the io\_service is created before the http\_server or http\_client
and the io\_service.run() function is called after the http\_server or
http\_client have been set up. However, multiple instances of the io_service
can be run for better performance. See the boost asio documentation at
[www.boost.org](http://www.boost.org)

Event Callbacks
---------------

The library uses the boost::signals library to implement it's callbacks.
The signals library implements function callbacks in terms of signals and
slots:  
* signals are a callback function signature, defined in the library.  
* slots are the actual callback functions, defined in the user's code.  

The http\_server and http\_client classes have functions to connect the
user's slots to the signals. Note: that not all of the signals need to have
slots defined. However, it's vitally important to get the function
signature right for the slots otherwise the compilier errors are almost
indecipherable!

The http\_server code contains following signal signal definitions:

    /// The signal sent when a request is received.
    typedef boost::signal<void (const boost::weak_ptr<http_connection_type>,
                                http::rx_request const&,
                                Container const&)> http_request_signal;
								
	/// The signal sent when a chunk is received.							
	typedef boost::signal<void (const boost::weak_ptr<http_connection_type>,
                                http::rx_chunk const&,
                                Container const&)> http_chunk_signal;
								
And the following functions to connect slots to the signals.
								
	/// Connect the request received slot.							
	void request_received_event(http_request_signal_slot const& slot);
	
	/// Connect the chunk received slot.
    void chunk_received_event(http_chunk_signal_slot const& slot)

The http\_client code contains the following signal definitions:

    /// The signal sent when a response is received.
    typedef boost::signal<void (http::rx_response const&,
                                Container const&)> http_response_signal;
								
	/// The signal sent when a chunk is received.							
    typedef boost::signal<void (http::rx_chunk const&,
                                Container const&)> http_chunk_signal;
								
	/// The signal sent when a socket is disconnected.
    typedef boost::signal<void (void)> http_disconnected_signal;
	
And the following functions to connect slots to the signals.

    /// Connect the response received slot.
    void response_received_event(http_response_signal_slot const& slot);

    /// Connect the chunk received slot.
    void chunk_received_event(http_chunk_signal_slot const& slot);

    /// Connect the disconnected slot.
    void disconnected_event(http_disconnected_signal_slot const& slot);
	
Server Connections
------------------

HTTP client communications is relatively simple: create an instance of an
http\_client, send a message get a response, send another, etc.

However, for the http\_server things are a bit more complicated...

Everytime that a new client connects to the server, the server creates a new
http\_connection. When the http\_server receives a request, the response must
be sent back on the same connection. Which is why the http\_server's signals
contain:

    const boost::weak_ptr<http_connection_type>
	
so that the user's code can send it's response on the right connection.

Note: the signals sent to the server have weak_ptr's to the http\_connections.
The weak\_ptr must be converted to a shared\_ptr (by calling  lock() on the
weak\_ptr) before any of the http\_connections functions can be called.

This is a key design feature of the http\_server. The http\_server manages a
collection of shared pointers to the http\_connections and sends weak pointers
in it's signals. If an http\_connection goes out of scope (e.g. by a client
disconnect) then the http\_server will delete it's shared pointer to the
http\_connection and the call to lock() on the weak pointer will return NULL.

So if the user code stores the pointer to the http\_connection so that it can
send a response later, then it must be stored as a weak pointer and the return
value of the call to lock() must be tested to ensure that the http\_connection
has not been deleted by the server in the meanwhile, e.g.:

    http_connection_type::shared_pointer pointer(connection.lock());
    if (pointer)
    {
      via::http::tx_response response(via::http::response_status::OK);
      pointer->send(response);
    }
    else
      std::cerr << "Failed to lock connection weak_pointer" << std::endl;
	  
SSL/TLS Support
---------------

The library uses the OpenSSL library to implement the SSL/TLS protocol, see: [www.openssl.org](http://www.openssl.org).

The library has been designed so that if SSL support is not needed then the
OpenSSL library does not need to be present. The OpenSSL library is only
required if the file ssl\_tcp\_adaptor.hpp is included in the application.

If SSL support is required then in application it must have OpenSSL in the
include path and link with the OpenSSL libraries. It must also define the
macro: HTTP_SSL. This macro is used in http\_server.hpp and defined in
ssl\_tcp\_adaptor.hpp. So simply including ssl\_tcp\_adaptor.hpp before
http\_server.hpp works. However, if you don't want to reply on the file
inclusion order then the macro should be defined in the Makefile / project
settings.

Examples
========

An example HTTP Server:
[simple\_http\_server.cpp](examples/server/simple_http_server.cpp)

The example code above creates an HTTP server on the given port that will
print requests to the console and send a HTTP OK (200) response to each
request.

An example HTTPS Server:
[simple\_https\_server.cpp](examples/server/simple_https_server.cpp)

The example code above creates an HTTPS server on the given port that will
print requests to the console and send a HTTP OK (200) response to each
request.

