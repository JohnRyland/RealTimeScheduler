///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    logger.hpp
//!
//! @brief
//!    Logs diagnostics messages.
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
//!    Formats and logs messages, prepending file and line numbers.
//!    There are 3 levels, debug, warning and error.
//!
//!    Debug logging is disabled in release builds.
//!
//!    Error logging will log the error and then the program will be aborted.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "formatter.hpp"
#include "system.hpp"
#include "configuration.hpp"
#include "debug_logger.h"

enum LogLevel
{
  LOG_VERBOSE,
  LOG_DEBUG,
  LOG_WARNING,
  LOG_ERR
};

/// @cond document_details
namespace details
{
  template <int LEVEL, size_t N1, size_t N2, typename... Ts>
  static inline constexpr
  void LogMessage(const char (&a_sourceFile)[N1], int a_sourceLine, const char (&a_formatString)[N2], Ts&&... a_params)
  {
    k_dbg<LOGGER>("step-1\n");
    if constexpr(buildType == BuildType::Debug || LEVEL != LOG_DEBUG)
    {
      k_dbg<LOGGER>("step-2\n");
      const auto message = Formatter(a_formatString, a_params...);
      k_dbg<LOGGER>("step-3: message: -%s- len: %i \n", message.c_str(), message.length());
      auto formattedString = Formatter("{1}:{2}: {3}", a_sourceFile, a_sourceLine, message);
      k_dbg<LOGGER>("step-4: formattedString: -%s- len: %i \n", formattedString.c_str(), formattedString.length());
      sysLog(formattedString.c_str(), formattedString.length());
      k_dbg<LOGGER>("step-5\n");
      if constexpr(LEVEL == LOG_ERR)
      {
        sysAbort();
      }
      k_dbg<LOGGER>("step-6\n");
    }
  }
} // end namespace details
/// @endcond

/// @brief Formats and logs the message if the program build type is a debug build.
///        Source file and line number information are prepended to the message.
/// @param ... The first parameter is the format string which uses the Formatter.
/// @see Formatter
#define LogDebug(...)   details::LogMessage<LOG_DEBUG>  (__FILE__, __LINE__, __VA_ARGS__)

/// @brief Formats and logs the message.
///        Source file and line number information are prepended to the message.
/// @param ... The first parameter is the format string which uses the Formatter.
/// @see Formatter
#define LogWarning(...) details::LogMessage<LOG_WARNING>(__FILE__, __LINE__, __VA_ARGS__)

/// @brief Formats and logs the message and then aborts the program.
///        Source file and line number information are prepended to the message.
/// @param ... The first parameter is the format string which uses the Formatter.
/// @see Formatter
#define LogError(...)   details::LogMessage<LOG_ERR>    (__FILE__, __LINE__, __VA_ARGS__)
