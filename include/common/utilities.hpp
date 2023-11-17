///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    utilities.hpp
//!
//! @brief
//!    C++ Utility functions.
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
//!    Miscellaneous functions.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "integers.hpp"
#include "assertion.hpp"

/// @brief Returns the smaller of the two values.
/// @tparam T is the type of values to compare.
/// @param a_value1 is the first value to compare with a_value2.
/// @param a_value2 is the second value to compare with a_value1.
/// @return Returns the value which is smaller between a_value1 and a_value2.
template <typename T>
static inline constexpr
T min(const T& a_value1, const T& a_value2)
{
  return (a_value1 < a_value2) ? a_value1 : a_value2;
}

/// @brief Returns the larger of the two values.
/// @tparam T is the type of values to compare.
/// @param a_value1 is the first value to compare with a_value2.
/// @param a_value2 is the second value to compare with a_value1.
/// @return Returns the value which is larger between a_value1 and a_value2.
template <typename T>
static inline constexpr
T max(const T& a_value1, const T& a_value2)
{
  return (a_value1 > a_value2) ? a_value1 : a_value2;
}

/// @brief Returns a_value if it falls between a_min and a_max,
///        otherwise returns the closest value that is between them.
/// @tparam T is the type of values to compare.
/// @param a_value is the value to clamp to be between a_min and a_max.
/// @param a_min is the first value to compare with a_value.
/// @param a_max is the second value to compare with a_value.
/// @return Returns a_value if it is between a_min and a_max, otherwise the closest value that is.
template <typename T>
static inline constexpr
T clamp(const T& a_value, const T& a_min, const T& a_max)
{
  return min(max(a_value, a_min), a_max);
}

template <typename T, k_size_t N>
static inline constexpr
k_size_t Size(T (& /*a_string*/ )[N])
{
  return N;
}

template <size_t DstLen = 0, size_t SrcLen = 0>
static inline constexpr
void memCpy(char (&a_dst)[DstLen], const char (&a_src)[SrcLen], size_t a_length, size_t a_srcOffset = 0UL, size_t a_dstOffset = 0UL)
{
  assertion(a_length <= (SrcLen - a_srcOffset));
  assertion(a_length <= (DstLen - a_dstOffset));
  for (size_t i = 0; i < a_length; ++i)
    a_dst[i + a_dstOffset] = a_src[i + a_srcOffset];
}

template <size_t DstLen>
static inline constexpr
void memZero(char (&a_dst)[DstLen], size_t a_length, size_t a_offset = 0UL)
{
  assertion(a_offset < DstLen);
  assertion(a_length <= (DstLen - a_offset));
  for (size_t i = 0; i < a_length; ++i)
    a_dst[i + a_offset] = 0;
}

template <size_t SrcLen = 0>
static inline constexpr
size_t strLen(const char (&a_src)[SrcLen], size_t a_length, size_t a_srcOffset)
{
  assertion(a_length <= (SrcLen - a_srcOffset));
  for (size_t i = 0; i < a_length; ++i)
    if (!a_src[i + a_srcOffset])
      return i;
  return a_length;
}

template <size_t SrcLen = 0>
static inline constexpr
size_t strLen(const char (&a_src)[SrcLen], size_t a_length)
{
  return strLen(a_src, a_length, 0);
}

template <size_t SrcLen = 0>
static inline constexpr
size_t strLen(const char (&a_src)[SrcLen])
{
  return strLen(a_src, SrcLen, 0);
}

template <size_t SrcLen = 0, size_t DstLen = 0>
static inline constexpr
bool strCmp(const char (&a_src)[SrcLen], const char (&a_dst)[DstLen], size_t a_length = -1, size_t a_srcOffset = 0, size_t a_dstOffset = 0)
{
  a_length = (a_length == -1) ? ((SrcLen < DstLen) ? SrcLen : DstLen) : a_length;
  assertion(a_length <= (SrcLen - a_srcOffset));
  assertion(a_length <= (DstLen - a_dstOffset));
  assertion(a_src[SrcLen - 1] == 0);
  assertion(a_dst[DstLen - 1] == 0);
  size_t i;
  for (i = 0; i < a_length; ++i)
  {
    if (!a_dst[i + a_dstOffset] || !a_src[i + a_srcOffset])
      return !a_dst[i + a_dstOffset] && !a_src[i + a_srcOffset];
    if (a_dst[i + a_dstOffset] != a_src[i + a_srcOffset])
      return false;
  }
  return true;
}
