///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    system.hpp
//!
//! @brief
//!    Handles logged diagnostics messages.
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
//!    Pre-formatter messages are sent to this API for system specific
//!    handling. For example these could be sent to serial, terminal
//!    or file, or some combination of these.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "integers.hpp"

/// @brief Saves or outputs pre-formatted log message.
/// @param str The first parameter is the pre-formatted string.
/// @param len The second parameter is the length of the pre-formatted string.
/// @see Formatter
void sysLog(const char* str, k_size_t len);

/// @brief Aborts the process, immediately exiting and printing a message and backtrace.
void sysAbort();
