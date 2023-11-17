///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    formatter.hpp
//!
//! @brief
//!    C++ Positional String Formatter.
//!
//! @author
//!    Copyright (c) 2023, John Ryland,
//!    All rights reserved.
//!
//! License
//!    BSD-2-Clause License. See included LICENSE file for details.
//!    If LICENSE file is missing, see:
//!    https://opensource.org/licenses/BSD-2-Clause
//!
//!    Other licensing terms available.
//!    See LICENSE.Commercial for details.
//!
//! @details
//!    Provides similar functionality to sprintf, however with some improvements.
//!   
//!    The first improvement is the safety because an output buffer is not
//!    provided by the caller which needs to be large enough, instead the result
//!    is returned on the stack.
//!   
//!    This is the second improvement which is that there are no heap or dynamic
//!    memory allocations. Instead everything is using the stack only. This is
//!    acheieved by using the String<N> class.
//!   
//!    The last improvement is positional replacement of the parameters in to the
//!    format string. A format string such as "Hi {1}, today is {2} of {3}." can
//!    be translated in to other languages where the word order is different and
//!    the code doesn't need to be changed to accomodate such a change, only the
//!    format strings need to be selected based on the current language.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "string.hpp"
#include "string_utilities.hpp"
#include "debug_logger.h"

/// @cond document_details
namespace details
{
  template <size_t ParamIndex, size_t N>
  static inline constexpr
  auto FormatterHelper(const String<N>& a_formatString)
  {
    k_dbg<FORMATTER>("helper  fmt: -%s- len: %i  N: %i\n", a_formatString.c_str(), a_formatString.length(), N);
    const auto str = a_formatString;
    k_dbg<FORMATTER>("helper  str: -%s- len: %i  N: %i\n", str.c_str(), str.length(), N);
    return str;
  }
  
  template <size_t ParamIndex, size_t N, typename T, typename... Ts>
  static inline constexpr
  auto FormatterHelper(const String<N>& a_formatString, T&& a_param, Ts&&... a_params)
  {
    k_dbg<FORMATTER>("step-1\n");
    const auto paramIndexString = to_string("{") + to_string(ParamIndex) + to_string("}");
    k_dbg<FORMATTER>("step-2\n");
    const auto paramIndexStringLength = paramIndexString.length();
    k_dbg<FORMATTER>("step-3\n");
    const auto paramPosition = a_formatString.find(paramIndexString);
    k_dbg<FORMATTER>("step-4: paramIndexString: -%s- len: %i  pos: %i\n", paramIndexString.c_str(), paramIndexStringLength, paramPosition);
    const auto before = paramPosition ? a_formatString.template startEndSubString<N>(0, paramPosition - 1) : String<N>("");
    k_dbg<FORMATTER>("step-5\n");
    const auto after = a_formatString.template startEndSubString<N>(paramPosition + paramIndexStringLength); 
    k_dbg<FORMATTER>("step-6:  b4: -%s- len: %i  aft: -%s- len: %i\n", before.c_str(), before.length(), after.c_str(), after.length());
    const auto paramString = to_string(a_param);
    k_dbg<FORMATTER>("step-7: paramStr: -%s- len: %i \n", paramString.c_str(), paramString.length());
    const auto beforeWithParamString = before + paramString;
    k_dbg<FORMATTER>("step-8\n");
    const auto newFormatString = beforeWithParamString + after;
    k_dbg<FORMATTER>("step-9\n");
    //const auto newFormatString = before + to_string(a_param) + after;
//    const auto ret = FormatterHelper<ParamIndex + 1>(newFormatString, forward<Ts>(a_params)...);
//    return ret;
    return FormatterHelper<ParamIndex + 1>(newFormatString, forward<Ts>(a_params)...);
  }
} // end namespace details
/// @endcond

/// @brief Creates a fixed size string that is formatted by a_formatString and where the
///        parameters are substituted in to the positional places of the format string.
///        For example a format string of "Hi {2}, you are {1}." and with parameters
///        of age and name which make a new string of "Hi <name>, you are <age>".
/// @tparam ...Ts are types of a_params.
/// @tparam N is the size of the format string.
/// @param a_formatString is the string describing where a_params are to be expanded to.
/// @param ...a_params are the parmeters to expand as strings and insert in to the format string.
/// @return Returns a fixed sized string.
template <size_t N, typename... Ts>
static inline constexpr
auto Formatter(const char (&a_formatString)[N], Ts&&... a_params)
{
  k_dbg<FORMATTER>("entry\n");
  const auto str = to_string(a_formatString);
  k_dbg<FORMATTER>("entry2\n");
  return details::FormatterHelper<1>(str, forward<Ts>(a_params)...);
}
