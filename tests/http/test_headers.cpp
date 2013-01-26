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
//  BYTES_EQUAL(header_data.back(), *next); // Not MinGw
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
  CHECK(next == header_data.begin());
}

// A single http header line in a string, but without a :.
TEST(TestFieldLineParser, InValidSingleLine2)
{
  std::string header_data("Content abcdefgh\r\n");
  std::string::const_iterator next(header_data.begin());

  field_line field;
  CHECK(!field.parse(next, header_data.end()));
  CHECK(next == header_data.begin());
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
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestHeadersParser)
{
};

TEST(TestHeadersParser, ValidSingleHeaderString1)
{
  std::string header_data("Content: abcdefgh\r\n\r\n");
  std::string::const_iterator header_next(header_data.begin());

  headers the_headers;
  CHECK(the_headers.parse(header_next, header_data.end()));
//  std::cout << the_headers.to_string() << std::endl;
}

TEST(TestHeadersParser, ValidSingleHeader1)
{
  std::string HEADER_LINE("Content: abcdefgh\r\n\r\n");
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::const_iterator header_next(header_data.begin());

  headers the_headers;
  CHECK(the_headers.parse(header_next, header_data.end()));
//  std::cout << the_headers.to_string() << std::endl;
}

TEST(TestHeadersParser, ValidMultipleHeader1)
{
  std::string HEADER_LINE("Content-Length: \t4\r\n");
  HEADER_LINE += "Transfer-Encoding: \t Chunked\r\n\r\n";
  std::vector<char> header_data(HEADER_LINE.begin(), HEADER_LINE.end());
  std::vector<char>::const_iterator header_next(header_data.begin());

  headers the_headers;
  CHECK(the_headers.parse(header_next, header_data.end()));
}
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestHeadersEncoder)
{
};

/* TODO
TEST(TestHeadersEncoder, ValidChunkTrailer1)
{
  headers the_headers;
  std::string trailer(the_headers.chunk_trailer());
 // std::cout << std::endl << trailer << std::endl;
  STRCMP_EQUAL("0\r\n\r\n", trailer.c_str());
}

TEST(TestHeadersEncoder, ValidChunkTrailer2)
{
  headers the_headers;
  the_headers.add(headers::TRANSFER_ENCODING, headers::CHUNKED_STRING);
  std::string trailer(the_headers.chunk_trailer());
//  std::cout << std::endl << trailer << std::endl;
  STRCMP_EQUAL("0\r\nTransfer-Encoding: Chunked\r\n\r\n", trailer.c_str());
}
*/
//////////////////////////////////////////////////////////////////////////////
