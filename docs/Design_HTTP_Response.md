# HTTP Responses #

The library has different classes for receiving and transmitting responses. 
The `response.hpp` header file contains:

+ class `tx_response` for constructing an HTTP response for an HTTP server to send. 
+ class `rx_response` for parsing an HTTP response for an HTTP client.  

![Response Parsing Classes](images/http_response_parser.png)

## Response Parsing ##

A **response** message must always start with a response start line
followed by any number of message header lines and blank line. 

![Response Line Parsing States](images/response_line_parsing_state_diagram.png)

Where a response (or request) message contains data in the body of the message,
it must have a **Content-Length** message header containing the size in octets
(another word for bytes) of the data. Otherwise, if the data is to be sent in
chunks, the request or response message must have a **Transfer-Encoding** message
header containing the word **Chunked**.

![Response Parsing States](images/response_parsing_state_diagram.png)

Where a request or response message contains chunked data, each chunk of data
must be preceded by a **chunk** header, which is just a line before the data with
the size of the data (in a hex string).
