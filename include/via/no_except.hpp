#ifndef NOEXCEPT_HPP_VIA_HTTPLIB_
#define NOEXCEPT_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file no_except.hpp
/// @brief Contains the NOEXCEPT macro for C++11 noexcept / C++03 throw()
//////////////////////////////////////////////////////////////////////////////


#ifndef NOEXCEPT
  // if C++11 and Visual Studio
  #if ((__cplusplus >= 201103L) && (!_MSC_VER))
  /// C++03/C++11 compatibility, noexcept available in C++11
  #define NOEXCEPT noexcept
  #else
  /// C++03/C++11 compatibility, throw() to indicate noexcept in C++03
  #define NOEXCEPT throw()
  #endif
#endif

#endif // NOEXCEPT_HPP
