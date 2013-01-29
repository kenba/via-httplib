via-httplib
===========

A C++ HTTP server library written in C++03 using the TR1, Boost and OpenSSL
libraries.

It parses and encodes HTTP requests and responses. It can be configured to
work over plain TCP/IP or SSL connections. 

The parser attempts to parse HTTP requests and tolerantly, accepting multiple
whitespace characters and just LF for line termination. 

The parsed data is held within class member variables and can be read via
the appropriate accessor functions.

Note: message header field names (parsed by the headers class) are converted
into lower case when they are parsed. This is because although the field
names in RFC2616 are written in mixed case, the document states that field
names are "case-insensitive". It also states that the order of field names is
not important. So the parsed message fields are stored in a map using the
parsed lowercase field name as the key.

The library was originally developed to support a simple RESTful controller
for a client. It is currently working in that capacity. However, further
development work is planned to make it fully compliant with RFC2616.
