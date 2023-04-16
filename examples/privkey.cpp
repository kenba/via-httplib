//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file privkey.cpp
/// @brief An example private key in memory.
//////////////////////////////////////////////////////////////////////////////
#include "privkey.hpp"

const std::string PRIVKEY = R"delim(
-----BEGIN PRIVATE KEY-----
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
CXkDjDZQ3nxpbzsHVqvOMVdHSsldoqI=
-----END PRIVATE KEY-----
)delim";
