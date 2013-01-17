//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "chunk.hpp"

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    bool chunk_line::parse_char(char c, parsing_state& state)
    {
      switch (state)
      {
      case CHUNK_SIZE_LS:
        // Ignore leading whitespace 
        if (is_space_or_tab(c))
          break;
        else
          state = CHUNK_SIZE;
        // intentional fall-through
      case CHUNK_SIZE:
        if (std::isxdigit (c))
          hex_size_.push_back(c);
        else
        {
          if (is_end_of_line(c) || (';' == c))
          {
            if (';' == c)
              state = CHUNK_EXTENSION_LS;
            else if ('\r' == c)
              state = CHUNK_LF;
            else // ('\n' == c)
              state = CHUNK_END;
          }
          else
            return false;
        }
        break;

      case CHUNK_EXTENSION_LS:
        // Ignore leading whitespace 
        if (is_space_or_tab(c))
          break;
        else
          state = CHUNK_EXTENSION;
        // intentional fall-through
      case CHUNK_EXTENSION:
        if (!is_end_of_line(c))
          extension_.push_back(c);
        else if ('\r' == c)
          state = CHUNK_LF;
        else // ('\n' == c) \\ but permit just \n
          state = CHUNK_END;
        break;

      case CHUNK_LF:
        if ('\n' == c)
          state = CHUNK_END;
        else 
          return false;
        break;

      default:
        return false;
      }

      return true; 
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string chunk_line::to_string() const
    {
      std::string output(hex_size_);
      if (!extension_.empty())
        output += "; " + extension_;

      output += "\r\n";
      return output;
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
