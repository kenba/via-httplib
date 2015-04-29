//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/chunk.hpp"

namespace via
{
  namespace http
  {
    //////////////////////////////////////////////////////////////////////////
    bool chunk_header::parse_char(char c)
    {
      static const size_t MAX_SIZE_DIGITS(16); // enough for a 64 bit number

      // Ensure that the overall header length is within limits
      if (++length_ > max_line_length_)
        state_ = CHUNK_ERROR_LENGTH;

      switch (state_)
      {
      case CHUNK_SIZE_LS:
        // Ignore leading whitespace 
        if (is_space_or_tab(c))
        {
          // but only upto to a limit!
          if (++ws_count_ > max_whitespace_)
          {
            state_ = CHUNK_ERROR_WS;
            return false;
          }
          else
          break;
        }
        else
          state_ = CHUNK_SIZE;
        // intentional fall-through
      case CHUNK_SIZE:
        if (std::isxdigit (c))
        {
          hex_size_.push_back(c);
          // limit the length of the hex string
          if (hex_size_.size() > MAX_SIZE_DIGITS)
          {
            state_ = CHUNK_ERROR_SIZE;
            return false;
          }
        }
        else
        {
          if (is_end_of_line(c) || (';' == c))
          {
            size_ = from_hex_string(hex_size_);
            size_read_ = true;
            if (size_ > max_chunk_size_)
            {
              state_ = CHUNK_ERROR_SIZE;
              return false;
            }

            if (';' == c)
            {
              ws_count_ = 0;
              state_ = CHUNK_EXTENSION_LS;
            }
            else
            {
              if ('\r' == c)
              state_ = CHUNK_LF;
            else // ('\n' == c)
              {
                if (strict_crlf_)
                  return false;
          else
                  state_ = CHUNK_VALID;
              }
            }
          }
          else
            return false;
        }
        break;

      case CHUNK_EXTENSION_LS:
        // Ignore leading whitespace 
        if (is_space_or_tab(c))
        {
          // but only upto to a limit!
          if (++ws_count_ > max_whitespace_)
            return false;
          else
          break;
        }
        else
          state_ = CHUNK_EXTENSION;
        // intentional fall-through
      case CHUNK_EXTENSION:
        if (!is_end_of_line(c))
          extension_.push_back(c);
        else if ('\r' == c)
          state_ = CHUNK_LF;
        else // ('\n' == c)
        {
          if (strict_crlf_)
          {
            state_ = CHUNK_ERROR_CRLF;
            return false;
          }
          else
            state_ = CHUNK_VALID;
        }
        break;

      case CHUNK_LF:
        if ('\n' == c)
          state_ = CHUNK_VALID;
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
    std::string chunk_header::to_string() const
    {
      std::string output(hex_size_);
      if (!extension_.empty())
        output += "; " + extension_;

      output += CRLF;
      return output;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    std::string last_chunk::to_string() const
    {
      std::string output("0");
      if (!extension_.empty())
        output += "; " + extension_;

      output += CRLF;
      output += trailer_string_;

      output += CRLF;
      return output;
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
