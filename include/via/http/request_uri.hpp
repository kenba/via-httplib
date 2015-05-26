#ifndef URI_HPP_VIA_HTTPLIB_
#define URI_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file request_uri.hpp
/// @brief The HTTP request_uri class.
//////////////////////////////////////////////////////////////////////////////
#include <string>

namespace via
{
  namespace http
  {
    /// @class request_uri
    /// The class splits a uri contained in an HTTP request into it's parts:
    /// path, query and fragment. See: https://tools.ietf.org/pdf/rfc3986.pdf
    class request_uri
    {
      std::string path_;     ///< The path of the uri.
      std::string query_;    ///< The (optional) query of the uri.
      std::string fragment_; ///< The (optional) fragment of the uri.

    public:

      /// Constructor
      /// It initialises the path with the uri.
      /// It then searches for the query and fragment deleimiters.
      /// If either is found it modifies the path accordingly and sets the
      /// query and fragment with the appropriate text from the uri.
      /// @param uri the uri from an HTTP request
      explicit request_uri(std::string const& uri)
        : path_(uri)
        , query_()
        , fragment_()
      {
        auto query_start(uri.find('?'));
        auto fragment_start(uri.find('#'));

        bool has_fragment(fragment_start != std::string::npos);
        bool has_query(query_start < fragment_start);
        if (has_query || has_fragment)
        {
          if (has_query)
          {
            path_.erase(query_start++);
            query_ = uri.substr(query_start, fragment_start - query_start);
          }
          else
            path_.erase(fragment_start);

          if (has_fragment)
            fragment_ = uri.substr(++fragment_start);
        }
      }

      /// Accessor for the uri path.
      std::string const& path() const
      { return path_; }

      /// Accessor for the (optional) query.
      std::string const& query() const
      { return query_; }

      /// Accessor for the (optional) fragment.
      std::string const& fragment() const
      { return fragment_; }
    };
  }
}

#endif // URI_HPP_VIA_HTTPLIB_
