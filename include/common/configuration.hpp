///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    configuration.hpp
//!
//! @brief
//!    Detect various configuration options.
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
//!    Determines if a release or debug build.

#pragma once

/// @brief Build type configuration.
enum class BuildType
{
  Debug,
  Release
};

/// @brief Build type configuration.
#ifdef NDEBUG // Release builds
constexpr BuildType buildType = BuildType::Release;
#else          // Debug builds
constexpr BuildType buildType = BuildType::Debug;
#endif
