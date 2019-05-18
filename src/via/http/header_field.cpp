//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2019 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/header_field.hpp"

#ifdef _MSC_VER // MSVC doesn't like gmtime...
#pragma warning( disable : 4996 )
#endif

namespace via
{
  namespace http
  {
    namespace header_field
    {
      ////////////////////////////////////////////////////////////////////////
      const std::string_view standard_name(id field_id) noexcept
      {
        switch(field_id)
        {
        // General Header Fields
        case id::CACHE_CONTROL:       return HEADER_CACHE_CONTROL;
        case id::CONNECTION:          return HEADER_CONNECTION;
        case id::DATE:                return HEADER_DATE;
        case id::PRAGMA:              return HEADER_PRAGMA;
        case id::TRAILER:             return HEADER_TRAILER;
        case id::TRANSFER_ENCODING:   return HEADER_TRANSFER_ENCODING;
        case id::UPGRADE:             return HEADER_UPGRADE;
        case id::VIA:                 return HEADER_VIA;
        case id::WARNING:             return HEADER_WARNING;

        // Request Header Fields
        case id::ACCEPT:              return HEADER_ACCEPT;
        case id::ACCEPT_CHARSET:      return HEADER_ACCEPT_CHARSET;
        case id::ACCEPT_ENCODING:     return HEADER_ACCEPT_ENCODING;
        case id::ACCEPT_LANGUAGE:     return HEADER_ACCEPT_LANGUAGE;
        case id::AUTHORIZATION:       return HEADER_AUTHORIZATION;
        case id::EXPECT:              return HEADER_EXPECT;
        case id::FROM:                return HEADER_FROM;
        case id::HOST:                return HEADER_HOST;
        case id::IF_MATCH:            return HEADER_IF_MATCH;
        case id::IF_MODIFIED_SINCE:   return HEADER_IF_MODIFIED_SINCE;
        case id::IF_NONE_MATCH:       return HEADER_IF_NONE_MATCH;
        case id::IF_RANGE:            return HEADER_IF_RANGE;
        case id::IF_UNMODIFIED_SINCE: return HEADER_IF_UNMODIFIED_SINCE;
        case id::MAX_FORWARDS:        return HEADER_MAX_FORWARDS;
        case id::PROXY_AUTHORIZATION: return HEADER_PROXY_AUTHORIZATION;
        case id::RANGE:               return HEADER_RANGE;
        case id::REFERER:             return HEADER_REFERER;
        case id::TE:                  return HEADER_TE;
        case id::USER_AGENT:          return HEADER_USER_AGENT;

        // Response Header Fields
        case id::ACCEPT_RANGES:       return HEADER_ACCEPT_RANGES;
        case id::AGE:                 return HEADER_AGE;
        case id::ETAG:                return HEADER_ETAG;
        case id::LOCATION:            return HEADER_LOCATION;
        case id::PROXY_AUTHENTICATE:  return HEADER_PROXY_AUTHENTICATE;
        case id::RETRY_AFTER:         return HEADER_RETRY_AFTER;
        case id::SERVER:              return HEADER_SERVER;
        case id::VARY:                return HEADER_VARY;
        case id::WWW_AUTHENTICATE:    return HEADER_WWW_AUTHENTICATE;

        // Entity Header Fields
        case id::ALLOW:               return HEADER_ALLOW;
        case id::CONTENT_ENCODING:    return HEADER_CONTENT_ENCODING;
        case id::CONTENT_LANGUAGE:    return HEADER_CONTENT_LANGUAGE;
        case id::CONTENT_LENGTH:      return HEADER_CONTENT_LENGTH;
        case id::CONTENT_LOCATION:    return HEADER_CONTENT_LOCATION;
        case id::CONTENT_MD5:         return HEADER_CONTENT_MD5;
        case id::CONTENT_RANGE:       return HEADER_CONTENT_RANGE;
        case id::CONTENT_TYPE:        return HEADER_CONTENT_TYPE;
        case id::EXPIRES:             return HEADER_EXPIRES;
        case id::LAST_MODIFIED:       return HEADER_LAST_MODIFIED;
        case id::EXTENSION_HEADER:    return HEADER_EXTENSION_HEADER;

        default:                      return std::string_view();
        }
      }
      ////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////
      const std::string_view lowercase_name(id field_id) noexcept
      {
        switch(field_id)
        {
        // General Header Fields
        case id::CACHE_CONTROL:       return LC_CACHE_CONTROL;
        case id::CONNECTION:          return LC_CONNECTION;
        case id::DATE:                return LC_DATE;
        case id::PRAGMA:              return LC_PRAGMA;
        case id::TRAILER:             return LC_TRAILER;
        case id::TRANSFER_ENCODING:   return LC_TRANSFER_ENCODING;
        case id::UPGRADE:             return LC_UPGRADE;
        case id::VIA:                 return LC_VIA;
        case id::WARNING:             return LC_WARNING;

        // Request Header Fields
        case id::ACCEPT:              return LC_ACCEPT;
        case id::ACCEPT_CHARSET:      return LC_ACCEPT_CHARSET;
        case id::ACCEPT_ENCODING:     return LC_ACCEPT_ENCODING;
        case id::ACCEPT_LANGUAGE:     return LC_ACCEPT_LANGUAGE;
        case id::AUTHORIZATION:       return LC_AUTHORIZATION;
        case id::EXPECT:              return LC_EXPECT;
        case id::FROM:                return LC_FROM;
        case id::HOST:                return LC_HOST;
        case id::IF_MATCH:            return LC_IF_MATCH;
        case id::IF_MODIFIED_SINCE:   return LC_IF_MODIFIED_SINCE;
        case id::IF_NONE_MATCH:       return LC_IF_NONE_MATCH;
        case id::IF_RANGE:            return LC_IF_RANGE;
        case id::IF_UNMODIFIED_SINCE: return LC_IF_UNMODIFIED_SINCE;
        case id::MAX_FORWARDS:        return LC_MAX_FORWARDS;
        case id::PROXY_AUTHORIZATION: return LC_PROXY_AUTHORIZATION;
        case id::RANGE:               return LC_RANGE;
        case id::REFERER:             return LC_REFERER;
        case id::TE:                  return LC_TE;
        case id::USER_AGENT:          return LC_USER_AGENT;

        // Response Header Fields
        case id::ACCEPT_RANGES:       return LC_ACCEPT_RANGES;
        case id::AGE:                 return LC_AGE;
        case id::ETAG:                return LC_ETAG;
        case id::LOCATION:            return LC_LOCATION;
        case id::PROXY_AUTHENTICATE:  return LC_PROXY_AUTHENTICATE;
        case id::RETRY_AFTER:         return LC_RETRY_AFTER;
        case id::SERVER:              return LC_SERVER;
        case id::VARY:                return LC_VARY;
        case id::WWW_AUTHENTICATE:    return LC_WWW_AUTHENTICATE;

        // Entity Header Fields
        case id::ALLOW:               return LC_ALLOW;
        case id::CONTENT_ENCODING:    return LC_CONTENT_ENCODING;
        case id::CONTENT_LANGUAGE:    return LC_CONTENT_LANGUAGE;
        case id::CONTENT_LENGTH:      return LC_CONTENT_LENGTH;
        case id::CONTENT_LOCATION:    return LC_CONTENT_LOCATION;
        case id::CONTENT_MD5:         return LC_CONTENT_MD5;
        case id::CONTENT_RANGE:       return LC_CONTENT_RANGE;
        case id::CONTENT_TYPE:        return LC_CONTENT_TYPE;
        case id::EXPIRES:             return LC_EXPIRES;
        case id::LAST_MODIFIED:       return LC_LAST_MODIFIED;
        case id::EXTENSION_HEADER:    return LC_EXTENSION_HEADER;

        default:                      return std::string_view();
        }
      }
      ////////////////////////////////////////////////////////////////////////
    }
  }
}
