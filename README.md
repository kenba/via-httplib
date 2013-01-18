via-httplib
===========

A C++ HTTP header parsing and encoding library written in C++03 using the TR1
and boost libraries. It parses and encodes both requests and responses.
It attempts to parse requests and responses tolerantly, accepting multiple
whitespace characters and just LF for line termination. 

All of the parser functions use a pair of forward iterators to refer to the
input message header. If the HTTP header was parsed successfully, then the
parsing function will return true and the “next” iterator will now point to
the next character to be parsed. If the parser did not recognise the input
as a valid HTTP header, then the “next” iterator will not be changed when the
function returns.

The parsed data is held within class member variables and can be read via
the appropriate accessor function.

Note: message header field names (parsed by the headers class) are converted
into lower case when they are parsed. This is because although the field
names in RFC2616 are written in mixed case, the document states that field
names are "case-insensitive". It also states that the order of field names is
not important. So the headers class stores the parsed message fields in a map
using the parsed lowercase field name as the key.

So it is vital to use lower case field names when searching for a message
header field value. E.g. to find the value of the "Content-Type" header field
you should call headers.find("content-type"). However, for standard headers,
like content type, it is recommend to use the enums provided and call
headers.find(header_field::CONTENT_TYPE) instead.
