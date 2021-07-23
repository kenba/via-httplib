//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2021 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/chunk.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>

using namespace via::http;

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestChunkLineParser)

BOOST_AUTO_TEST_CASE(EmptyChunk1)
{
  std::string chunk_data("0\r\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK(chunk_data.end() == next);
  BOOST_CHECK_EQUAL("0",  the_chunk.hex_size().c_str());
  BOOST_CHECK_EQUAL("", the_chunk.extension().c_str());
  BOOST_CHECK_EQUAL(0U, the_chunk.size());
  BOOST_CHECK(the_chunk.is_last());
  BOOST_CHECK(next == chunk_data.end());
}

BOOST_AUTO_TEST_CASE(EmptyChunk2)
{
  std::string chunk_data("0;\r\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK(chunk_data.end() == next);
  BOOST_CHECK_EQUAL("0",  the_chunk.hex_size().c_str());
  BOOST_CHECK_EQUAL("", the_chunk.extension().c_str());
  BOOST_CHECK_EQUAL(0U, the_chunk.size());
  BOOST_CHECK(the_chunk.is_last());
  BOOST_CHECK(next == chunk_data.end());
}

BOOST_AUTO_TEST_CASE(ValidString1)
{
  std::string chunk_data("f; some rubbish\r\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK(chunk_data.end() == next);
  BOOST_CHECK_EQUAL("f",  the_chunk.hex_size().c_str());
  BOOST_CHECK_EQUAL("some rubbish", the_chunk.extension().c_str());
  BOOST_CHECK_EQUAL(15U, the_chunk.size());
  BOOST_CHECK(!the_chunk.is_last());

  std::string header_string(the_chunk.to_string());
  BOOST_CHECK_EQUAL(chunk_data, header_string);
}

BOOST_AUTO_TEST_CASE(ValidString2)
{
  std::string chunk_data("f\r\nA");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK(chunk_data.end() != next);
  BOOST_CHECK_EQUAL('A', *next);
  BOOST_CHECK_EQUAL("f",  the_chunk.hex_size().c_str());
  BOOST_CHECK_EQUAL(15U, the_chunk.size());
  BOOST_CHECK(!the_chunk.is_last());
}

BOOST_AUTO_TEST_CASE(ValidString3)
{
  std::string chunk_data("f; some rubbish\r\nA");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK(chunk_data.end() != next);
  BOOST_CHECK_EQUAL('A', *next);
  BOOST_CHECK_EQUAL("f",  the_chunk.hex_size().c_str());
  BOOST_CHECK_EQUAL("some rubbish", the_chunk.extension().c_str());
  BOOST_CHECK_EQUAL(15U, the_chunk.size());
  BOOST_CHECK(!the_chunk.is_last());
}

BOOST_AUTO_TEST_CASE(MultipleString1)
{
  std::string chunk_data("2f; some rubbish\r\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.begin() +1));
  BOOST_CHECK(chunk_data.end() != next);
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK(chunk_data.end() == next);
  BOOST_CHECK_EQUAL("2f",  the_chunk.hex_size().c_str());
  BOOST_CHECK_EQUAL("some rubbish", the_chunk.extension().c_str());
  BOOST_CHECK_EQUAL(47U, the_chunk.size());
}

BOOST_AUTO_TEST_CASE(InValidString1)
{
  std::string chunk_data("g;\r\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidString2)
{
  std::string chunk_data("f;\r\r");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidString3)
{
  std::string chunk_data("f\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, true> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidString4)
{
  std::string chunk_data("f;\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, true> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidString5)
{
  std::string chunk_data("2f;                  some rubbish\r\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidString6)
{
  std::string chunk_data("                        2f\r\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidString7)
{
  std::string chunk_data("2f; some rubbish\r\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<10, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidString8)
{
  std::string chunk_data("1234567890abcdef0123456789abcdef012\r\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidString9)
{
  std::string chunk_data("ffff\r\n");
  std::string::iterator next(chunk_data.begin());

  chunk_header<1024, 8, false> the_chunk(1024);
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestChunkEncoder)

BOOST_AUTO_TEST_CASE(EmptyChunk1)
{
  chunk_header<1024, 8, false> the_chunk(0);
  std::string chunk_string(the_chunk.to_string());

  BOOST_CHECK_EQUAL("0\r\n",  chunk_string.c_str());
}

BOOST_AUTO_TEST_CASE(ValidChunk1)
{
  chunk_header<1024, 8, false> the_chunk(15);
  std::string chunk_string(the_chunk.to_string());

  BOOST_CHECK_EQUAL("f\r\n",  chunk_string.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestLastChunkEncoder)

BOOST_AUTO_TEST_CASE(EmptyChunk1)
{
  std::string empty_string("");
  last_chunk the_chunk(empty_string, empty_string);
  std::string chunk_string(the_chunk.to_string());

  BOOST_CHECK_EQUAL("0\r\n\r\n", chunk_string.c_str());
}

BOOST_AUTO_TEST_CASE(EmptyChunk2)
{
  std::string empty_string("");
  last_chunk the_chunk("extension", empty_string);
  std::string chunk_string(the_chunk.to_string());

  BOOST_CHECK_EQUAL("0; extension\r\n\r\n", chunk_string.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(TestChunkParser)

BOOST_AUTO_TEST_CASE(ValidChunk1)
{
  std::string chunk_data("f;\r\n");
  chunk_data += "123456789abcdef\r\n";
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(15U, the_chunk.size());
  BOOST_CHECK_EQUAL('1', the_chunk.data()[0]);
  BOOST_CHECK_EQUAL('f', the_chunk.data()[the_chunk.size()-1]);
}

BOOST_AUTO_TEST_CASE(ValidChunk2)
{
  std::string chunk_data("f;\n");
  chunk_data += "123456789abcdef\n";
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(15U, the_chunk.size());
  BOOST_CHECK_EQUAL('1', the_chunk.data()[0]);
  BOOST_CHECK_EQUAL('f', the_chunk.data()[the_chunk.size()-1]);
}

BOOST_AUTO_TEST_CASE(ValidChunk3)
{
  std::string chunk_data(" f;\r\n");
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));

  std::string chunk_data1("123456789abcdef\r\n");
  next = chunk_data1.begin();
  BOOST_CHECK(the_chunk.parse(next, chunk_data1.end()));

  BOOST_CHECK_EQUAL(15U, the_chunk.size());
  BOOST_CHECK_EQUAL('1', the_chunk.data()[0]);
  BOOST_CHECK_EQUAL('f', the_chunk.data()[the_chunk.size()-1]);
}

BOOST_AUTO_TEST_CASE(ValidChunk4)
{
  std::string chunk_data("f");
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));

  std::string chunk_data1(";\r\n123456789abcdef\r\n");
  next = chunk_data1.begin();
  BOOST_CHECK(the_chunk.parse(next, chunk_data1.end()));

  BOOST_CHECK_EQUAL(15U, the_chunk.size());
  BOOST_CHECK_EQUAL('1', the_chunk.data()[0]);
  BOOST_CHECK_EQUAL('f', the_chunk.data()[the_chunk.size()-1]);
}

BOOST_AUTO_TEST_CASE(ValidChunk5)
{
  std::string chunk_data("f");
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));

  std::string chunk_data1("\n123456789abcdef\n");
  next = chunk_data1.begin();
  BOOST_CHECK(the_chunk.parse(next, chunk_data1.end()));

  BOOST_CHECK_EQUAL(15U, the_chunk.size());
  BOOST_CHECK_EQUAL('1', the_chunk.data()[0]);
  BOOST_CHECK_EQUAL('f', the_chunk.data()[the_chunk.size()-1]);
}

BOOST_AUTO_TEST_CASE(ValidMultipleChunks1)
{
  std::string chunk_data("f;\r\n123456789abcdef\r\n"); // a complete chunk
  chunk_data += "a;\r\n0123456789\r\n"; // and another
  chunk_data += "0;\r\n\r\n"; // last chunk

  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(15U, the_chunk.size());
  BOOST_CHECK_EQUAL('1', the_chunk.data()[0]);
  BOOST_CHECK_EQUAL('f', the_chunk.data()[the_chunk.size()-1]);
  BOOST_CHECK(!the_chunk.is_last());

  the_chunk.clear();
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(10U, the_chunk.size());
  BOOST_CHECK_EQUAL('0', the_chunk.data()[0]);
  BOOST_CHECK_EQUAL('9', the_chunk.data()[the_chunk.size()-1]);
  BOOST_CHECK(!the_chunk.is_last());

  the_chunk.clear();
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(0U, the_chunk.size());
  BOOST_CHECK(the_chunk.is_last());
}

BOOST_AUTO_TEST_CASE(ValidMultipleChunks2)
{
  // As above but without CR's after chunk data
  std::string chunk_data("f;\r\n123456789abcdef\n"); // a complete chunk
  chunk_data += "a;\r\n0123456789\n"; // and another
  chunk_data += "0;\r\n\r\n"; // last chunk

  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(15U, the_chunk.size());
  BOOST_CHECK_EQUAL('1', the_chunk.data()[0]);
  BOOST_CHECK_EQUAL('f', the_chunk.data()[the_chunk.size()-1]);
  BOOST_CHECK(!the_chunk.is_last());

  the_chunk.clear();
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(10U, the_chunk.size());
  BOOST_CHECK_EQUAL('0', the_chunk.data()[0]);
  BOOST_CHECK_EQUAL('9', the_chunk.data()[the_chunk.size()-1]);
  BOOST_CHECK(!the_chunk.is_last());

  the_chunk.clear();
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(0U, the_chunk.size());
  BOOST_CHECK(the_chunk.is_last());
}

BOOST_AUTO_TEST_CASE(InValidChunk1)
{
  std::string chunk_data("g;\r\n");
  chunk_data += "123456789abcdef\r\n";
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidChunk2)
{
  std::string chunk_data("f;\r\r");
  chunk_data += "123456789abcdef\r\n";
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidChunk3)
{
  std::string chunk_data("f;\r\n");
  chunk_data += "123456789abcdef\r\r";
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(InValidChunk4)
{
  // missing \r after chunk dat is strict crlf mode
  std::string chunk_data("f;\r\n");
  chunk_data += "123456789abcdef\n";
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, true> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_CASE(ValidLastChunk1)
{
  std::string chunk_data("0\r\n\r\n");
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(0U, the_chunk.size());
  BOOST_CHECK(the_chunk.valid());
  BOOST_CHECK(the_chunk.is_last());
  BOOST_CHECK(next == chunk_data.end());
}

BOOST_AUTO_TEST_CASE(ValidLastChunk2)
{
  std::string chunk_data("0;\r\n\r\n");
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(0U, the_chunk.size());
  BOOST_CHECK(the_chunk.valid());
  BOOST_CHECK(the_chunk.is_last());
  BOOST_CHECK(next == chunk_data.end());
}

BOOST_AUTO_TEST_CASE(ValidLastChunk3)
{
  std::string extension("");
  std::string trailer_string("");
  last_chunk last_chunk1(extension, trailer_string);
  std::string chunk_data(last_chunk1.to_string());
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(0U, the_chunk.size());
  BOOST_CHECK(the_chunk.valid());
  BOOST_CHECK(the_chunk.is_last());
  BOOST_CHECK(next == chunk_data.end());
}

BOOST_AUTO_TEST_CASE(ValidChunkTrailer1)
{
  std::string chunk_data("0\r\n");
  chunk_data += "Accept-Encoding: gzip\r\n\r\n";
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(the_chunk.parse(next, chunk_data.end()));
  BOOST_CHECK_EQUAL(0U, the_chunk.size());
  BOOST_CHECK(the_chunk.is_last());
}

BOOST_AUTO_TEST_CASE(InValidChunkTrailer1)
{
  std::string chunk_data("0\r\n");
  chunk_data += "Accept-Encoding: gzip\r\r\r\n";
  std::string::iterator next(chunk_data.begin());

  rx_chunk<std::string, 100, 8190, 1024, 8, false> the_chunk;
  BOOST_CHECK(!the_chunk.parse(next, chunk_data.end()));
}

BOOST_AUTO_TEST_SUITE_END()

//////////////////////////////////////////////////////////////////////////////
