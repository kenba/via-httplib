//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "../../via/http/chunk.hpp"
#include <vector>
#include <iostream>

#ifdef _MSC_VER
// cpputest\MemoryLeakWarningPlugin.h: C++11 exception specification deprecated
#pragma warning (disable:4290)
#endif
#include "CppUTest/TestHarness.h"

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestChunkLineParser)
{
};

TEST(TestChunkLineParser, EmptyChunk1)
{
  std::string chunk_data("0\r\n");
  std::string::const_iterator next(chunk_data.begin());

  chunk_header the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() == next);
  STRCMP_EQUAL("0",  the_chunk.hex_size().c_str());
  STRCMP_EQUAL("", the_chunk.extension().c_str());
  CHECK_EQUAL(0, the_chunk.size());
  CHECK(the_chunk.is_last());
}

TEST(TestChunkLineParser, EmptyChunk2)
{
  std::string chunk_data("0;\r\n");
  std::string::const_iterator next(chunk_data.begin());

  chunk_header the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() == next);
  STRCMP_EQUAL("0",  the_chunk.hex_size().c_str());
  STRCMP_EQUAL("", the_chunk.extension().c_str());
  CHECK_EQUAL(0, the_chunk.size());
  CHECK(the_chunk.is_last());
}

TEST(TestChunkLineParser, ValidString1)
{
  std::string chunk_data("f; some rubbish\r\n");
  std::string::const_iterator next(chunk_data.begin());

  chunk_header the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() == next);
  STRCMP_EQUAL("f",  the_chunk.hex_size().c_str());
  STRCMP_EQUAL("some rubbish", the_chunk.extension().c_str());
  CHECK_EQUAL(15, the_chunk.size());
  CHECK(!the_chunk.is_last());
}

TEST(TestChunkLineParser, ValidString2)
{
  std::string chunk_data("f\r\nA");
  std::string::const_iterator next(chunk_data.begin());

  chunk_header the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() != next);
  BYTES_EQUAL('A', *next);
  STRCMP_EQUAL("f",  the_chunk.hex_size().c_str());
  CHECK_EQUAL(15, the_chunk.size());
  CHECK(!the_chunk.is_last());
}

TEST(TestChunkLineParser, ValidString3)
{
  std::string chunk_data("f; some rubbish\r\nA");
  std::string::const_iterator next(chunk_data.begin());

  chunk_header the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() != next);
  BYTES_EQUAL('A', *next);
  STRCMP_EQUAL("f",  the_chunk.hex_size().c_str());
  STRCMP_EQUAL("some rubbish", the_chunk.extension().c_str());
  CHECK_EQUAL(15, the_chunk.size());
  CHECK(!the_chunk.is_last());
}

TEST(TestChunkLineParser, MultipleString1)
{
  std::string chunk_data("2f; some rubbish\r\n");
  std::string::const_iterator next(chunk_data.begin());

  chunk_header the_chunk;
  CHECK(!the_chunk.parse(next, chunk_data.begin() +1));
  CHECK(chunk_data.end() != next);
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() == next);
  STRCMP_EQUAL("2f",  the_chunk.hex_size().c_str());
  STRCMP_EQUAL("some rubbish", the_chunk.extension().c_str());
  CHECK_EQUAL(47, the_chunk.size());
}

TEST(TestChunkLineParser, InValidString1)
{
  std::string chunk_data("g;\r\n");
  std::string::const_iterator next(chunk_data.begin());

  chunk_header the_chunk;
  CHECK(!the_chunk.parse(next, chunk_data.end()));
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestChunkEncoder)
{
};

TEST(TestChunkEncoder, EmptyChunk1)
{
  chunk_header the_chunk(0);
  std::string chunk_string(the_chunk.to_string());

  STRCMP_EQUAL("0\r\n",  chunk_string.c_str());
}

TEST(TestChunkEncoder, ValidChunk1)
{
  chunk_header the_chunk(15);
  std::string chunk_string(the_chunk.to_string());

  STRCMP_EQUAL("f\r\n",  chunk_string.c_str());
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestLastChunkEncoder)
{
};

TEST(TestLastChunkEncoder, EmptyChunk1)
{
  std::string empty_string("");
  last_chunk the_chunk(empty_string, empty_string);
  std::string chunk_string(the_chunk.message());

  STRCMP_EQUAL("0\r\n\r\n", chunk_string.c_str());
}

TEST(TestLastChunkEncoder, EmptyChunk2)
{
  std::string empty_string("");
  last_chunk the_chunk("extension", empty_string);
  std::string chunk_string(the_chunk.message());

  STRCMP_EQUAL("0; extension\r\n\r\n", chunk_string.c_str());
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestChunkParser)
{
};

TEST(TestChunkParser, ValidChunk1)
{
  std::string chunk_data("f;\r\n");
  chunk_data += "123456789abcdef\r\n";
  std::string::const_iterator next(chunk_data.begin());

  rx_chunk<std::string> the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK_EQUAL(15, the_chunk.size());
  BYTES_EQUAL('1', the_chunk.data().front());
  BYTES_EQUAL('f', the_chunk.data().back());
}

TEST(TestChunkParser, ValidChunk2)
{
  std::string chunk_data("f;\n");
  chunk_data += "123456789abcdef\n";
  std::string::const_iterator next(chunk_data.begin());

  rx_chunk<std::string> the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK_EQUAL(15, the_chunk.size());
  BYTES_EQUAL('1', the_chunk.data().front());
  BYTES_EQUAL('f', the_chunk.data().back());
}

TEST(TestChunkParser, ValidChunk3)
{
  std::string chunk_data("f;\r\n");
  std::string::const_iterator next(chunk_data.begin());

  rx_chunk<std::string> the_chunk;
  CHECK(!the_chunk.parse(next, chunk_data.end()));

  std::string chunk_data1("123456789abcdef\r\n");
  next = chunk_data1.begin();
  CHECK(the_chunk.parse(next, chunk_data1.end()));

  CHECK_EQUAL(15, the_chunk.size());
  BYTES_EQUAL('1', the_chunk.data().front());
  BYTES_EQUAL('f', the_chunk.data().back());
}

TEST(TestChunkParser, ValidChunk4)
{
  std::string chunk_data("f");
  std::string::const_iterator next(chunk_data.begin());

  rx_chunk<std::string> the_chunk;
  CHECK(!the_chunk.parse(next, chunk_data.end()));

  std::string chunk_data1(";\r\n123456789abcdef\r\n");
  next = chunk_data1.begin();
  CHECK(the_chunk.parse(next, chunk_data1.end()));

  CHECK_EQUAL(15, the_chunk.size());
  BYTES_EQUAL('1', the_chunk.data().front());
  BYTES_EQUAL('f', the_chunk.data().back());
}

TEST(TestChunkParser, ValidLastChunk1)
{
  std::string chunk_data("0\r\n\r\n");
  std::string::const_iterator next(chunk_data.begin());

  rx_chunk<std::string> the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK_EQUAL(0, the_chunk.size());
  CHECK(the_chunk.valid());
  CHECK(the_chunk.is_last());
}

TEST(TestChunkParser, ValidChunkTrailer1)
{
  std::string chunk_data("0\r\n");
  chunk_data += "Accept-Encoding: gzip\r\n\r\n";
  std::string::const_iterator next(chunk_data.begin());

  rx_chunk<std::string> the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK_EQUAL(0, the_chunk.size());
  CHECK(the_chunk.is_last());
}
//////////////////////////////////////////////////////////////////////////////

