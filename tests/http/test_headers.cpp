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
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(field.parse(next, header_data.end()));
  BOOST_CHECK(header_data.end() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a vector of unsigned chars.
BOOST_AUTO_TEST_CASE(ValidSingleVectorUnsignedhar1)
{
  std::string HEADER_LINE("Content: abcdefgh\r\n");
  std::vector<unsigned char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<unsigned char>::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(field.parse(next, header_data.end()));
  BOOST_CHECK(header_data.end() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string.
BOOST_AUTO_TEST_CASE(ValidSingleString1)
{
  std::string header_data("Content: abcdefgh\n");
  std::string::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(field.parse(next, header_data.end()));
  BOOST_CHECK(header_data.end() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string with an extra character.
BOOST_AUTO_TEST_CASE(ValidSingleString2)
{
  std::string header_data("Content: abcdefgh\r\nA");
  std::string::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(field.parse(next, header_data.end()));
  BOOST_CHECK(header_data.end() != next);
  BOOST_CHECK_EQUAL('A', *next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string without a space after the :.
BOOST_AUTO_TEST_CASE(ValidSingleLine3)
{
  std::string header_data("Content:abcdefgh\r\n");
  std::string::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(field.parse(next, header_data.end()));
  BOOST_CHECK(header_data.end() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A standard single http header line in a string.
BOOST_AUTO_TEST_CASE(ValidSingleLine4)
{
  std::string header_data("Accept-Charset: abcdefgh\r\n");
  std::string::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(field.parse(next, header_data.end()));
  BOOST_CHECK(header_data.end() == next);
  BOOST_CHECK_EQUAL(header_field::lowercase_name
                     (header_field::id::ACCEPT_CHARSET).c_str(),
                      field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string, but starting with a space.
BOOST_AUTO_TEST_CASE(InValidSingleLine1)
{
  std::string header_data(" Content:abcdefgh\r\n");
  std::string::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(!field.parse(next, header_data.end()));
}

// A single http header line in a string, but without a :.
BOOST_AUTO_TEST_CASE(InValidSingleLine2)
{
  std::string header_data("Content abcdefgh\r\n");
  std::string::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(!field.parse(next, header_data.end()));
}

// A single http header line in a string.
BOOST_AUTO_TEST_CASE(InValidSingleLine3)
{
  std::string header_data("Content: abcdefgh\r\r");
  std::string::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(!field.parse(next, header_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidSingleLine4)
{
  std::string header_data("Content: abcdefgh\r\n");
  std::string::iterator next(header_data.begin());

  field_line field(false, 8, 16);
  BOOST_CHECK(!field.parse(next, header_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidSingleLine5)
{
  std::string header_data("Content:             abcdefgh\r\r");
  std::string::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(!field.parse(next, header_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidSingleLine6)
{
  std::string header_data("Content: abcdefgh\n");
  std::string::iterator next(header_data.begin());

  field_line field(true, 8, 1024);
  BOOST_CHECK(!field.parse(next, header_data.end()));
}

// A multiple http header line in a string
BOOST_AUTO_TEST_CASE(ValidMultiString1)
{
  std::string header_data("Content: ab\r\n cd\r\n  ef\r\n\tgh\r\n");
  std::string::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(field.parse(next, header_data.end()));
  BOOST_CHECK(header_data.end() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("ab cd ef gh", field.value().c_str());
}

// A multiple http header line in a vector of chars
BOOST_AUTO_TEST_CASE(ValidMultiLine1)
{
  std::string HEADER_LINE("Content: ab\r\n cd\r\n  ef\r\n\tgh\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator next(header_data.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(field.parse(next, header_data.end()));
  BOOST_CHECK(header_data.end() == next);
  BOOST_CHECK_EQUAL("content",  field.name().c_str());
  BOOST_CHECK_EQUAL("ab cd ef gh", field.value().c_str());
}

// A standard single http header line in two strings.
BOOST_AUTO_TEST_CASE(ValidMultiMsg1)
{
  std::string header_data1("Accept-Char");
  std::string::iterator next(header_data1.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(!field.parse(next, header_data1.end()));
  BOOST_CHECK(header_data1.end() == next);

  std::string header_data2("set: abcdefgh\r\n");
  next = header_data2.begin();
  BOOST_CHECK(field.parse(next, header_data2.end()));
  BOOST_CHECK(header_data2.end() == next);
  BOOST_CHECK_EQUAL(header_field::lowercase_name
                    (header_field::id::ACCEPT_CHARSET).c_str(),
                    field.name().c_str());
  BOOST_CHECK_EQUAL("abcdefgh", field.value().c_str());
}

BOOST_AUTO_TEST_CASE(ValidMultiMsg2)
{
  std::string header_data1("Accept-Charset: abcd");
  std::string::iterator next(header_data1.begin());

  field_line field(false, 8, 1024);
  BOOST_CHECK(!field.parse(next, header_data1.end()));
  BOOST_CHECK(header_data1.end() == next);

  std::string header_data2("efgh\r\n");
  next = header_data2.begin();
  BOOST_CHECK(field.parse(next, header_data2.end()));
  BOOST_CHECK(header_data2.end() == next);
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
  std::string::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);
  BOOST_CHECK(the_headers.valid());
//  std::cout << the_headers.to_string() << std::endl;
}

BOOST_AUTO_TEST_CASE(ValidSingleHeaderString1)
{
  std::string header_data("Content: abcdefgh\r\n\r\n");
  std::string::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);
//  std::cout << the_headers.to_string() << std::endl;
}

BOOST_AUTO_TEST_CASE(ValidSingleHeader1)
{
  std::string HEADER_LINE("Content: abcdefgh\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);
//  std::cout << the_headers.to_string() << std::endl;
}

BOOST_AUTO_TEST_CASE(ValidMultipleHeader1)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n\r\n";
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);
}

BOOST_AUTO_TEST_CASE(ValidMultipleHeader2)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n\r\nA";
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() != header_next);
  BOOST_CHECK_EQUAL('A', *header_next);
}

BOOST_AUTO_TEST_CASE(ValidMultipleHeaderMultiLine1)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Enco";
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(!the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);

  std::string HEADER_LINE2("ding: \t Chunked\r\n\r\n");
  std::vector<char> header_data2(HEADER_LINE2.begin(), HEADER_LINE2.end());
  header_next = header_data2.begin();
  BOOST_CHECK(the_headers.parse(header_next, header_data2.end()));
  BOOST_CHECK(header_data2.end() == header_next);

  BOOST_CHECK_EQUAL("Chunked",
                    the_headers.find
                    (header_field::id::TRANSFER_ENCODING).c_str());
}

BOOST_AUTO_TEST_CASE(InValidSingleHeaderString1)
{
  std::string header_data("Content: abcdefgh\r\n\r\r");
  std::string::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(!the_headers.parse(header_next, header_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidMultipleHeaderString2)
{
  // Two headers in message but parser only accepts one

  std::string header_data("Host: localhost\r\n");
  header_data += ("Transfer-Encoding: Chunked\r\n");
  std::string::iterator header_next(header_data.begin());

  message_headers the_headers(false, 1, 1024, 1, 8190);
  BOOST_CHECK(!the_headers.parse(header_next, header_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidMultipleHeaderString3)
{
  // Parser set to only parse 32 bytes of header.
  std::string header_data("Host: localhost\r\n");
  header_data += ("Transfer-Encoding: Chunked\r\n");
  std::string::iterator header_next(header_data.begin());

  message_headers the_headers(false, 1, 1024, 100, 32);
  BOOST_CHECK(!the_headers.parse(header_next, header_data.end()));
}
BOOST_AUTO_TEST_CASE(ValidContentLength1)
{
  // Simple number
  std::string HEADER_LINE("Content-Length: 4\n\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);

  BOOST_CHECK_EQUAL(4, the_headers.content_length());
  BOOST_CHECK(!the_headers.close_connection());
  BOOST_CHECK(!the_headers.expect_continue());
}

// A invalid content length http header line.
BOOST_AUTO_TEST_CASE(InValidContentLength1)
{
  // Alpha before number
  std::string HEADER_LINE("Content-Length: z4\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);

  BOOST_CHECK_EQUAL(-1, the_headers.content_length());
}

BOOST_AUTO_TEST_CASE(InValidContentLength2)
{
  // Alpha after number
  std::string HEADER_LINE("Content-Length: 4z\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);

  BOOST_CHECK_EQUAL(-1, the_headers.content_length());
}

BOOST_AUTO_TEST_CASE(InValidContentLength3)
{
  // Number is too big
  std::string HEADER_LINE("Content-Length: 999999999999999999999\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);

  BOOST_CHECK_EQUAL(-1, the_headers.content_length());
}

BOOST_AUTO_TEST_CASE(ValidCloseConnection1)
{
  // Simple number
  std::string HEADER_LINE("Connection: close\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);

  BOOST_CHECK(the_headers.close_connection());
}

BOOST_AUTO_TEST_CASE(ValidExpectContinue1)
{
  // Simple number
  std::string HEADER_LINE("Expect: 100-continue\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);

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

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestRepeatedHeaders)

BOOST_AUTO_TEST_CASE(ValidCookieHeader1)
{
  // Single cookie
  std::string HEADER_LINE("Set-Cookie: abcdefg hijkl\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n\r\n";
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);

  std::string COOKIE_STR("abcdefg hijkl");
  const std::string& cookies(the_headers.find("set-cookie"));
  BOOST_CHECK_EQUAL(COOKIE_STR, cookies);

  BOOST_CHECK_EQUAL("Chunked",
    the_headers.find
    (header_field::id::TRANSFER_ENCODING).c_str());

  std::string HEADER_STRING("set-cookie: abcdefg hijkl\r\n");
  HEADER_STRING += "transfer-encoding: Chunked\r\n";
  auto header_str(the_headers.to_string());
  BOOST_CHECK_EQUAL(HEADER_STRING, header_str);
}

BOOST_AUTO_TEST_CASE(ValidCookieHeader2)
{
  // Two cookies
  std::string HEADER_LINE("Set-Cookie: abcdefg hijkl\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n";
  HEADER_LINE += "Set-Cookie: ijklm nopq\r\n\r\n";
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);

  std::string COOKIE_STR("abcdefg hijkl;ijklm nopq");
  const std::string& cookies(the_headers.find("set-cookie"));
  BOOST_CHECK_EQUAL(COOKIE_STR, cookies);

  BOOST_CHECK_EQUAL("Chunked",
    the_headers.find
    (header_field::id::TRANSFER_ENCODING).c_str());
}

BOOST_AUTO_TEST_CASE(ValidRepeatedHeader1)
{
  // Two Content-Languages
  std::string HEADER_LINE("Content-Language: abcdefg hijkl\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n";
  HEADER_LINE += "Content-Language: ijklm nopq\r\n\r\n";
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::iterator header_next(header_data.begin());

  message_headers the_headers(false, 8, 1024, 100, 8190);
  BOOST_CHECK(the_headers.parse(header_next, header_data.end()));
  BOOST_CHECK(header_data.end() == header_next);

  std::string CONTENT_STR("abcdefg hijkl,ijklm nopq");
  const std::string& content(the_headers.find("content-language"));
  BOOST_CHECK_EQUAL(CONTENT_STR, content);

  BOOST_CHECK_EQUAL("Chunked",
    the_headers.find
    (header_field::id::TRANSFER_ENCODING).c_str());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
