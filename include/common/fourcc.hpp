///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    fourcc.hpp
//!
//! @brief
//!    Checking a 4 byte header signature called a 'four-cc'.
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
//!    Check a 4 byte signature value matches a pre-determined 4 byte pattern.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "integers.hpp"

/// \brief
/// Construct from a string such as "BLAH" or from an array of uint8_t such
/// as { 0x01, 0x02, 0x03, 0x04 }, or from a 32bit value such as 0x01020304.
/// The array constructor may be useful for the input being compared to the
/// known.
class FourCC
{
public:
    inline constexpr
    FourCC(const char fourCC[5])
        : _fourCC(fourCC[0] | (fourCC[1] << 8) | (fourCC[2] << 16) | (fourCC[3] << 24))
    {
    }

    inline constexpr
    FourCC(const uint8_t fourCC[4])
        : _fourCC(fourCC[0] | (fourCC[1] << 8) | (fourCC[2] << 16) | (fourCC[3] << 24))
    {
    }

    inline constexpr
    FourCC(const uint32_t fourCC)
        : _fourCC(fourCC)
    {
    }

    /// compare two fourCC values
    static inline constexpr
    bool matches(const FourCC fourCC1, const FourCC fourCC2)
    {
        return fourCC1._fourCC == fourCC2._fourCC;
    }

private:
    u32 _fourCC;
};
