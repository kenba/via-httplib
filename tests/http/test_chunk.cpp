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
