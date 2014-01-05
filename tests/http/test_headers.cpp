//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//////////////////////////////////////////////////////////////////////////////
#include "../../via/http/headers.hpp"
#include <vector>
#include <iostream>

#ifdef _MSC_VER
// cpputest\MemoryLeakWarningPlugin.h: C++11 exception specification deprecated
#pragma warning (disable:4290)
#endif
#include "CppUTest/TestHarness.h"

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestFieldLineParser)
{
};

// A single http header line in a vector of chars.
TEST(TestFieldLineParser, ValidSingleVectorChar1)
{
  std::string HEADER_LINE("Content: abcdefgh\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::const_iterator next(header_data.begin());

  field_line field;
  CHECK(field.parse(next, header_data.end()));
  CHECK(header_data.end() == next);
  STRCMP_EQUAL("content",  field.name().c_str());
  STRCMP_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a vector of unsigned chars.
TEST(TestFieldLineParser, ValidSingleVectorUnsignedhar1)
{
  std::string HEADER_LINE("Content: abcdefgh\r\n");
  std::vector<unsigned char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<unsigned char>::const_iterator next(header_data.begin());

  field_line field;
  CHECK(field.parse(next, header_data.end()));
  CHECK(header_data.end() == next);
  STRCMP_EQUAL("content",  field.name().c_str());
  STRCMP_EQUAL("abcdefgh", field.value().c_str());
}


// A single http header line in a string.
TEST(TestFieldLineParser, ValidSingleString1)
{
  std::string header_data("Content: abcdefgh\r\n");
  std::string::const_iterator next(header_data.begin());

  field_line field;
  CHECK(field.parse(next, header_data.end()));
  CHECK(header_data.end() == next);
  STRCMP_EQUAL("content",  field.name().c_str());
  STRCMP_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string with an extra character.
TEST(TestFieldLineParser, ValidSingleString2)
{
  std::string header_data("Content: abcdefgh\r\nA");
  std::string::const_iterator next(header_data.begin());

  field_line field;
  CHECK(field.parse(next, header_data.end()));
  CHECK(header_data.end() != next);
  BYTES_EQUAL('A', *next);
  STRCMP_EQUAL("content",  field.name().c_str());
  STRCMP_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string without a space after the :.
TEST(TestFieldLineParser, ValidSingleLine3)
{
  std::string header_data("Content:abcdefgh\r\n");
  std::string::const_iterator next(header_data.begin());

  field_line field;
  CHECK(field.parse(next, header_data.end()));
  CHECK(header_data.end() == next);
  STRCMP_EQUAL("content",  field.name().c_str());
  STRCMP_EQUAL("abcdefgh", field.value().c_str());
}

// A standard single http header line in a string.
TEST(TestFieldLineParser, ValidSingleLine4)
{
  std::string header_data("Accept-Charset: abcdefgh\r\n");
  std::string::const_iterator next(header_data.begin());

  field_line field;
  CHECK(field.parse(next, header_data.end()));
  CHECK(header_data.end() == next);
  STRCMP_EQUAL(header_field::lowercase_name(header_field::ACCEPT_CHARSET).c_str(),
               field.name().c_str());
  STRCMP_EQUAL("abcdefgh", field.value().c_str());
}

// A single http header line in a string, but starting with a space.
TEST(TestFieldLineParser, InValidSingleLine1)
{
  std::string header_data(" Content:abcdefgh\r\n");
  std::string::const_iterator next(header_data.begin());

  field_line field;
  CHECK(!field.parse(next, header_data.end()));
}

// A single http header line in a string, but without a :.
TEST(TestFieldLineParser, InValidSingleLine2)
{
  std::string header_data("Content abcdefgh\r\n");
  std::string::const_iterator next(header_data.begin());

  field_line field;
  CHECK(!field.parse(next, header_data.end()));
}

// A multiple http header line in a string
TEST(TestFieldLineParser, ValidMultiString1)
{
  std::string header_data("Content: ab\r\n cd\r\n  ef\r\n\tgh\r\n");
  std::string::const_iterator next(header_data.begin());

  field_line field;
  CHECK(field.parse(next, header_data.end()));
  CHECK(header_data.end() == next);
  STRCMP_EQUAL("content",  field.name().c_str());
  STRCMP_EQUAL("ab cd ef gh", field.value().c_str());
}

// A multiple http header line in a vector of chars
TEST(TestFieldLineParser, ValidMultiLine1)
{
  std::string HEADER_LINE("Content: ab\r\n cd\r\n  ef\r\n\tgh\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::const_iterator next(header_data.begin());

  field_line field;
  CHECK(field.parse(next, header_data.end()));
  CHECK(header_data.end() == next);
  STRCMP_EQUAL("content",  field.name().c_str());
  STRCMP_EQUAL("ab cd ef gh", field.value().c_str());
}

// A standard single http header line in two strings.
TEST(TestFieldLineParser, ValidMultiMsg1)
{
  std::string header_data1("Accept-Char");
  std::string::const_iterator next(header_data1.begin());

  field_line field;
  CHECK(!field.parse(next, header_data1.end()));
  CHECK(header_data1.end() == next);

  std::string header_data2("set: abcdefgh\r\n");
  next = header_data2.begin();
  CHECK(field.parse(next, header_data2.end()));
  CHECK(header_data2.end() == next);
  STRCMP_EQUAL(header_field::lowercase_name(header_field::ACCEPT_CHARSET).c_str(),
               field.name().c_str());
  STRCMP_EQUAL("abcdefgh", field.value().c_str());
}

TEST(TestFieldLineParser, ValidMultiMsg2)
{
  std::string header_data1("Accept-Charset: abcd");
  std::string::const_iterator next(header_data1.begin());

  field_line field;
  CHECK(!field.parse(next, header_data1.end()));
  CHECK(header_data1.end() == next);

  std::string header_data2("efgh\r\n");
  next = header_data2.begin();
  CHECK(field.parse(next, header_data2.end()));
  CHECK(header_data2.end() == next);
  STRCMP_EQUAL(header_field::lowercase_name(header_field::ACCEPT_CHARSET).c_str(),
               field.name().c_str());
  STRCMP_EQUAL("abcdefgh", field.value().c_str());
}
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestHeadersParser)
{
};

TEST(TestHeadersParser, ValidEmptyHeaderString)
{
  std::string header_data("\r\n");
  std::string::const_iterator header_next(header_data.begin());

  message_headers the_headers;
  CHECK(the_headers.parse(header_next, header_data.end()));
  CHECK(header_data.end() == header_next);
  CHECK(the_headers.valid());
//  std::cout << the_headers.to_string() << std::endl;
}

TEST(TestHeadersParser, ValidSingleHeaderString1)
{
  std::string header_data("Content: abcdefgh\r\n\r\n");
  std::string::const_iterator header_next(header_data.begin());

  message_headers the_headers;
  CHECK(the_headers.parse(header_next, header_data.end()));
  CHECK(header_data.end() == header_next);
//  std::cout << the_headers.to_string() << std::endl;
}

TEST(TestHeadersParser, ValidSingleHeader1)
{
  std::string HEADER_LINE("Content: abcdefgh\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::const_iterator header_next(header_data.begin());

  message_headers the_headers;
  CHECK(the_headers.parse(header_next, header_data.end()));
  CHECK(header_data.end() == header_next);
//  std::cout << the_headers.to_string() << std::endl;
}

TEST(TestHeadersParser, ValidMultipleHeader1)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n\r\n";
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::const_iterator header_next(header_data.begin());

  message_headers the_headers;
  CHECK(the_headers.parse(header_next, header_data.end()));
  CHECK(header_data.end() == header_next);
}

TEST(TestHeadersParser, ValidMultipleHeader2)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n\r\nA";
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::const_iterator header_next(header_data.begin());

  message_headers the_headers;
  CHECK(the_headers.parse(header_next, header_data.end()));
  CHECK(header_data.end() != header_next);
  BYTES_EQUAL('A', *header_next);
}

TEST(TestHeadersParser, ValidMultipleHeaderMultiLine1)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Enco";
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::const_iterator header_next(header_data.begin());

  message_headers the_headers;
  CHECK(!the_headers.parse(header_next, header_data.end()));
  CHECK(header_data.end() == header_next);

  std::string HEADER_LINE2("ding: \t Chunked\r\n\r\n");
  std::vector<char> header_data2(HEADER_LINE2.begin(), HEADER_LINE2.end());
  header_next = header_data2.begin();
  CHECK(the_headers.parse(header_next, header_data2.end()));
  CHECK(header_data2.end() == header_next);

  STRCMP_EQUAL("Chunked",
               the_headers.find(header_field::TRANSFER_ENCODING).c_str());
}
//////////////////////////////////////////////////////////////////////////////
