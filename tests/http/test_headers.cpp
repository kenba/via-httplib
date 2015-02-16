//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//////////////////////////////////////////////////////////////////////////////
#include "via/http/headers.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestFieldLineParser)

// A single http header line in a vector of chars.
BOOST_AUTO_TEST_CASE(ValidSingleVectorChar1)
{
  std::string HEADER_LINE("Content: abcdefgh\r\n");
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(field.parse(next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a vector of unsigned chars.
BOOST_AUTO_TEST_CASE(ValidSingleVectorUnsignedhar1)
{
  std::string HEADER_LINE("Content: abcdefgh\r\n");
  std::vector<unsigned char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(field.parse(next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string.
BOOST_AUTO_TEST_CASE(ValidSingleString1)
{
  std::string header_data("Content: abcdefgh\n");
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(field.parse(next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string with an extra character.
BOOST_AUTO_TEST_CASE(ValidSingleString2)
{
  std::string header_data("Content: abcdefgh\r\nA");
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(field.parse(next, header_data.cend()));
  BOOST_CHECK(header_data.cend() != next);
  BOOST_CHECK_EQUAL('A', *next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string without a space after the :.
BOOST_AUTO_TEST_CASE(ValidSingleLine3)
{
  std::string header_data("Content:abcdefgh\r\n");
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(field.parse(next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A standard single http header line in a string.
BOOST_AUTO_TEST_CASE(ValidSingleLine4)
{
  std::string header_data("Accept-Charset: abcdefgh\r\n");
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(field.parse(next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == next);
  BOOST_CHECK_EQUAL(header_field::lowercase_name
                     (header_field::id::ACCEPT_CHARSET).c_str(),
                      field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string, but starting with a space.
BOOST_AUTO_TEST_CASE(InValidSingleLine1)
{
  std::string header_data(" Content:abcdefgh\r\n");
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(!field.parse(next, header_data.cend()));
}

// A single http header line in a string, but without a :.
BOOST_AUTO_TEST_CASE(InValidSingleLine2)
{
  std::string header_data("Content abcdefgh\r\n");
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(!field.parse(next, header_data.cend()));
}

// A single http header line in a string.
BOOST_AUTO_TEST_CASE(InValidSingleLine3)
{
  std::string header_data("Content: abcdefgh\r\r");
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(!field.parse(next, header_data.cend()));
}

BOOST_AUTO_TEST_CASE(InValidSingleLine4)
{
  std::string header_data("Content: abcdefgh\r\n");
  auto next(header_data.cbegin());

  field_line::max_length_s = 16;
  field_line field;
  BOOST_CHECK(!field.parse(next, header_data.cend()));
  field_line::max_length_s = 1024;
}

BOOST_AUTO_TEST_CASE(InValidSingleLine5)
{
  std::string header_data("Content:             abcdefgh\r\r");
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(!field.parse(next, header_data.cend()));
}

BOOST_AUTO_TEST_CASE(InValidSingleLine6)
{
  std::string header_data("Content: abcdefgh\n");
  auto next(header_data.cbegin());

  field_line::strict_crlf_s = true;
  field_line field;
  BOOST_CHECK(!field.parse(next, header_data.cend()));
  field_line::strict_crlf_s = false;
}

// A multiple http header line in a string
BOOST_AUTO_TEST_CASE(ValidMultiString1)
{
  std::string header_data("Content: ab\r\n cd\r\n  ef\r\n\tgh\r\n");
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(field.parse(next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("ab cd ef gh", field.value().c_str());
}

// A multiple http header line in a vector of chars
BOOST_AUTO_TEST_CASE(ValidMultiLine1)
{
  std::string HEADER_LINE("Content: ab\r\n cd\r\n  ef\r\n\tgh\r\n");
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto next(header_data.cbegin());

  field_line field;
  BOOST_CHECK(field.parse(next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("ab cd ef gh", field.value().c_str());
}

// A standard single http header line in two strings.
BOOST_AUTO_TEST_CASE(ValidMultiMsg1)
{
  std::string header_data1("Accept-Char");
  auto next(header_data1.cbegin());

  field_line field;
  BOOST_CHECK(!field.parse(next, header_data1.cend()));
  BOOST_CHECK(header_data1.cend() == next);

  std::string header_data2("set: abcdefgh\r\n");
  next = header_data2.cbegin();
  BOOST_CHECK(field.parse(next, header_data2.cend()));
  BOOST_CHECK(header_data2.cend() == next);
  BOOST_CHECK_EQUAL(header_field::lowercase_name
                    (header_field::id::ACCEPT_CHARSET).c_str(),
                    field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

BOOST_AUTO_TEST_CASE(ValidMultiMsg2)
{
  std::string header_data1("Accept-Charset: abcd");
  auto next(header_data1.cbegin());

  field_line field;
  BOOST_CHECK(!field.parse(next, header_data1.cend()));
  BOOST_CHECK(header_data1.cend() == next);

  std::string header_data2("efgh\r\n");
  next = header_data2.cbegin();
  BOOST_CHECK(field.parse(next, header_data2.cend()));
  BOOST_CHECK(header_data2.cend() == next);
  BOOST_CHECK_EQUAL(header_field::lowercase_name
                    (header_field::id::ACCEPT_CHARSET).c_str(),
                     field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestHeadersParser)

BOOST_AUTO_TEST_CASE(ValidEmptyHeaderString)
{
  std::string header_data("\r\n");
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);
  BOOST_CHECK(the_headers.valid());
//  std::cout << the_headers.to_string() << std::endl;
}

BOOST_AUTO_TEST_CASE(ValidSingleHeaderString1)
{
  std::string header_data("Content: abcdefgh\r\n\r\n");
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);
//  std::cout << the_headers.to_string() << std::endl;
}

BOOST_AUTO_TEST_CASE(ValidSingleHeader1)
{
  std::string HEADER_LINE("Content: abcdefgh\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);
//  std::cout << the_headers.to_string() << std::endl;
}

BOOST_AUTO_TEST_CASE(ValidMultipleHeader1)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n\r\n";
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);
}

BOOST_AUTO_TEST_CASE(ValidMultipleHeader2)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n\r\nA";
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() != header_next);
  BOOST_CHECK_EQUAL('A', *header_next);
}

BOOST_AUTO_TEST_CASE(ValidMultipleHeaderMultiLine1)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Enco";
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(!the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);

  std::string HEADER_LINE2("ding: \t Chunked\r\n\r\n");
  std::vector<char> header_data2(HEADER_LINE2.cbegin(), HEADER_LINE2.cend());
  header_next = header_data2.cbegin();
  BOOST_CHECK(the_headers.parse(header_next, header_data2.cend()));
  BOOST_CHECK(header_data2.cend() == header_next);

  BOOST_CHECK_EQUAL("Chunked",
                    the_headers.find
                    (header_field::id::TRANSFER_ENCODING).c_str());

  std::string output(the_headers.to_string());
//  BOOST_CHECK_EQUAL(output, HEADER_LINE);
}

BOOST_AUTO_TEST_CASE(InValidSingleHeaderString1)
{
  std::string header_data("Content: abcdefgh\r\n\r\r");
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(!the_headers.parse(header_next, header_data.cend()));
}

BOOST_AUTO_TEST_CASE(ValidContentLength1)
{
  // Simple number
  std::string HEADER_LINE("Content-Length: 4\n\n");
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);

  BOOST_CHECK_EQUAL(4U, the_headers.content_length());
  BOOST_CHECK(!the_headers.close_connection());
  BOOST_CHECK(!the_headers.expect_continue());
}

// A invalid content length http header line.
BOOST_AUTO_TEST_CASE(InValidContentLength1)
{
  // Alpha before number
  std::string HEADER_LINE("Content-Length: z4\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);

  BOOST_CHECK_EQUAL(std::numeric_limits<size_t>::max(), the_headers.content_length());
}

BOOST_AUTO_TEST_CASE(InValidContentLength2)
{
  // Alpha after number
  std::string HEADER_LINE("Content-Length: 4z\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);

  BOOST_CHECK_EQUAL(std::numeric_limits<size_t>::max(), the_headers.content_length());
}

BOOST_AUTO_TEST_CASE(InValidContentLength3)
{
  // Number is too big
  std::string HEADER_LINE("Content-Length: 999999999999999999999\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);

  BOOST_CHECK_EQUAL(std::numeric_limits<size_t>::max(), the_headers.content_length());
}

BOOST_AUTO_TEST_CASE(ValidCloseConnection1)
{
  // Simple number
  std::string HEADER_LINE("Connection: close\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);

  BOOST_CHECK(the_headers.close_connection());
}

BOOST_AUTO_TEST_CASE(ValidExpectContinue1)
{
  // Simple number
  std::string HEADER_LINE("Expect: 100-continue\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.cbegin(), HEADER_LINE.cend());
  auto header_next(header_data.cbegin());

  message_headers the_headers;
  BOOST_CHECK(the_headers.parse(header_next, header_data.cend()));
  BOOST_CHECK(header_data.cend() == header_next);

  BOOST_CHECK(the_headers.expect_continue());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestSplitHeaders)

BOOST_AUTO_TEST_CASE(ValidSingleHeader1)
{
  std::string HEADER_LINE("Content: abcdefgh\r\n");
  BOOST_CHECK(!are_headers_split(HEADER_LINE));
}

BOOST_AUTO_TEST_CASE(ValidMultipleHeader1)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n";
  BOOST_CHECK(!are_headers_split(HEADER_LINE));
}

BOOST_AUTO_TEST_CASE(InValidSingleHeader1)
{
  std::string HEADER_LINE("Content: abcdefgh\n\r\n");
  BOOST_CHECK(are_headers_split(HEADER_LINE));
}

BOOST_AUTO_TEST_CASE(InValidSingleHeader2)
{
  std::string HEADER_LINE("Content: abcdefgh\n\n");
  BOOST_CHECK(are_headers_split(HEADER_LINE));
}

BOOST_AUTO_TEST_CASE(InValidMultipleHeader1)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n";
  BOOST_CHECK(are_headers_split(HEADER_LINE));
}

BOOST_AUTO_TEST_CASE(InValidMultipleHeader2)
{
  std::string HEADER_LINE("Content-Length: \t4\n\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n";
  BOOST_CHECK(are_headers_split(HEADER_LINE));
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
