# HTTP Server Security

Since RFC2616 was written a number of vulnerabilities in the HTTP protocol have
been discovered and exploited, see
[WASC Threat Classification](http://projects.webappsec.org/w/page/13246978/Threat%20Classification)

## Denial of Service Attacks

The best known is a Denial of Service (aka DoS) attack where a number of
clients flood an HTTP server with requests denying the service to legitimate
users, see [Denial of service attack](http://en.wikipedia.org/wiki/Denial-of-service_attack)

An excellent article on how to protect against DoS attacks can be found here:
[How to Protect Against Slow HTTP Attacks](https://community.qualys.com/blogs/securitylabs/2011/11/02/how-to-protect-against-slow-http-attacks).
`via-httplib` has been designed to support it's recommended Protection Strategies as follows:

 + It checks the lengths of HTTP methods against a maximum limit. It will class
 a request is invalid if the methods is too long.  
 The application can choose how to respond to any methods that are not
 supported by the URL.
 
 + It has size limits for the sizes of the headers, the message body and any chunks.
 It will class a request is invalid if any of the size limits are exceeded. Also
 the library insists on a content-length header for any request with a message body.  
 The application can read the content-length header to apply URL-specific limits.

 + The connection timeout is configurable.
 
 + The application can register a callback function to be notified whenever
 an invalid HTTP request is received.
 
 + The application can register a callback function to be notified whenever
 a client connects to the server.
 
 + The application can read the address of the client from the http_connection
 pointer provided in all of the callback functions.
 
 + The application can disconnect a client by using the http_connection pointer
 provided in all of the callback functions.
 
With regards to the backlog of pending connections and minimum incoming data rate,
`via-httplib` does not maintain any pending connections nor does it measure the
incoming data rate.
 
## HTTP Response Splitting

An HTTP Response Splitting attack exploits a security hole in an HTTP server
which causes it to form an output stream that is interpreted by the target
as two HTTP responses instead of one, 
see: [HTTP Response Splitting](http://projects.webappsec.org/w/page/13246931/HTTP%20Response%20Splitting)

The security hole in the HTTP server that is exploited is an vulnerability 
of the HTTP protocol itself. The HTTP protocol defines a response header as
complete when two pairs of carriage return & line feed characters are received.
By including two pairs of carriage return & line feed characters in an HTTP
response header the client can be fooled into seeing two responses.

The solution is simply for the HTTP server to ensure that the headers of all
response messages do not contain two pairs of carriage return & line feed
characters.

`via-httplib` contains functions to validate HTTP response headers and it
will not send a response with a split header.

## TLS Truncation Attack

A TLS truncation attack blocks a victim's account logout requests so that
the user unknowingly remains logged into a web service, see: [Truncation Attack](http://en.wikipedia.org/wiki/Transport_Layer_Security#Truncation_attack)

This is a client vulnerability. However it shows the importance of shutting down
SSL/TLS connections correctly. Simply, closing the TCP socket without 
initiating an SSL shutdown makes the connection vulnerable, see: [Boost SSL async shutdown](http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error/25703699#25703699)

