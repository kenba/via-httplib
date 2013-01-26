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

  chunk_line the_chunk;
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

  chunk_line the_chunk;
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

  chunk_line the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() == next);
  STRCMP_EQUAL("f",  the_chunk.hex_size().c_str());
  STRCMP_EQUAL("some rubbish", the_chunk.extension().c_str());
  CHECK_EQUAL(15, the_chunk.size());
}

TEST(TestChunkLineParser, InValidString1)
{
  std::string chunk_data("g;\r\n");
  std::string::const_iterator next(chunk_data.begin());

  chunk_line the_chunk;
  CHECK(!the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.begin() == next);
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestChunkEncoder)
{
};

TEST(TestChunkEncoder, EmptyChunk1)
{
  chunk_line the_chunk(0);
  std::string chunk_string(the_chunk.to_string());

  STRCMP_EQUAL("0\r\n",  chunk_string.c_str());
}

TEST(TestChunkEncoder, ValidChunk1)
{
  chunk_line the_chunk(15);
  std::string chunk_string(the_chunk.to_string());

  STRCMP_EQUAL("f\r\n",  chunk_string.c_str());
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestChunkParser)
{
};

TEST(TestChunkParser, ValidChunkVectorChar1)
{
  std::string CHUNK("f\r\n123456789abcdef");
  std::vector<char> chunk_data(CHUNK.begin(), CHUNK.end());
  std::vector<char>::const_iterator next(chunk_data.begin());

  chunk<std::vector<char> > the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() == next);
  CHECK_EQUAL(15, the_chunk.size());
  STRCMP_EQUAL("", the_chunk.extension().c_str());
  BYTES_EQUAL('1', the_chunk.data().front());
  BYTES_EQUAL('f', the_chunk.data().back());
}

TEST(TestChunkParser, ValidChunkVectorUnsignedChar1)
{
  std::string CHUNK("f\r\n123456789abcdef");
  std::vector<unsigned char> chunk_data(CHUNK.begin(), CHUNK.end());
  std::vector<unsigned char>::const_iterator next(chunk_data.begin());

  chunk<std::vector<unsigned char> > the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() == next);
  CHECK_EQUAL(15, the_chunk.size());
  STRCMP_EQUAL("", the_chunk.extension().c_str());
  BYTES_EQUAL('1', the_chunk.data().front());
  BYTES_EQUAL('f', the_chunk.data().back());
}

TEST(TestChunkParser, ValidChunkString1)
{
  std::string chunk_data("f\r\n123456789abcdef");
  std::string::const_iterator next(chunk_data.begin());

  chunk<std::string> the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() == next);
  CHECK_EQUAL(15, the_chunk.size());
  STRCMP_EQUAL("", the_chunk.extension().c_str());
  // BYTES_EQUAL('1', the_chunk.data().front()); // MinGw4.4
  // BYTES_EQUAL('f', the_chunk.data().back()); // MinGw4.4
}

TEST(TestChunkParser, ValidChunkString2)
{
  std::string chunk_data("f; some rubbish\n123456789abcdef");
  std::string::const_iterator next(chunk_data.begin());

  chunk<std::string> the_chunk;
  CHECK(the_chunk.parse(next, chunk_data.end()));
  CHECK(chunk_data.end() == next);
  CHECK_EQUAL(15, the_chunk.size());
  STRCMP_EQUAL("some rubbish", the_chunk.extension().c_str());
  // BYTES_EQUAL('1', the_chunk.data().front()); // MinGw4.4
  // BYTES_EQUAL('f', the_chunk.data().back()); // MinGw4.4
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
TEST_GROUP(TestChunkEncoders)
{
};

TEST(TestChunkEncoders, ValidChunkVectorChar1)
{
  std::string CHUNK("123456789abcdef");
  std::vector<char> data(CHUNK.begin(), CHUNK.end());
  std::vector<char>::const_iterator next(data.begin());

  chunk<std::vector<char> > the_chunk(data);
  std::vector<char> chunk_data(the_chunk.message());
  std::string chunk_string(chunk_data.begin(), chunk_data.end());
//  std::cout << std::endl << chunk_string << std::endl;
  STRCMP_EQUAL("f\r\n123456789abcdef\r\n", chunk_string.c_str());
}

TEST(TestChunkEncoders, ValidChunkVectorUnsignedChar1)
{
  std::string CHUNK("123456789abcdef");
  std::vector<unsigned char> data(CHUNK.begin(), CHUNK.end());
  std::vector<unsigned char>::const_iterator next(data.begin());

  chunk<std::vector<unsigned char> > the_chunk(data);
  std::vector<unsigned char> chunk_data(the_chunk.message());
  std::string chunk_string(chunk_data.begin(), chunk_data.end());
//  std::cout << std::endl << chunk_string << std::endl;
  STRCMP_EQUAL("f\r\n123456789abcdef\r\n", chunk_string.c_str());
}

TEST(TestChunkEncoders, ValidChunkString1)
{
  std::string data("123456789abcdef");
  std::string::const_iterator next(data.begin());

  chunk<std::string> the_chunk(data);
  std::string chunk_string(the_chunk.message());
//  std::cout << std::endl << chunk_string << std::endl;
  STRCMP_EQUAL("f\r\n123456789abcdef\r\n", chunk_string.c_str());
}
//////////////////////////////////////////////////////////////////////////////
