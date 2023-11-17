///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    assertion.hpp
//!
//! @brief
//!    Checks a condition and aborts if the condition is not true.
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
//!    Macro which checks a condition and logs a message and aborts
//!    if the condition evaluates to false.
//!
//!    In debug builds this could be unevaluated, so avoid putting
//!    any condition that produces a side-effect which is depended upon.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "system.hpp"
#include "configuration.hpp"

/// @brief Formats and logs the message if the condition is false.
///        Source file and line number information are prepended to the message.
///        If called in a consteval context, this might be able to give a
///        compile-time error similar to static_assert.
/// @param expression The first parameter is the condition to check.
#define assertion(expression)    ( \
                                   ((expression) || (buildType != BuildType::Debug)) \
                                 ? \
                                   (void)0 \
                                 : \
                                   details::LogAssert(__FILE__ ":" stringize(__LINE__) " Assertion failure: " #expression "\n") \
                                 )

/// @brief Converts arg in to a string.
/// @param arg The expression to convert in to a string.
#define _stringize(arg)          #arg
#define stringize(arg)           _stringize(arg)

/// @cond document_details
namespace details
{
  template <size_t N>
  static inline constexpr
  void LogAssert(const char (&a_assertMessage)[N]) noexcept
  {
    if constexpr(buildType == BuildType::Debug)
    {
      sysLog(a_assertMessage, N);
      sysAbort();
    }
  }
} // end namespace details
/// @endcond
