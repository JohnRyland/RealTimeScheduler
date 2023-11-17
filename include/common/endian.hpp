///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    endian.hpp
//!
//! @brief
//!    Endian and structure packing helpers.
//!
//! @author
//!    Copyright (c) 2022-2023, John Ryland,
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
//!    Allows us to map to/from bytes stored in little or big endian order
//!    (irrespective of host machine endianess).
//!
//!    Provided are _le and _be variations on the uint[N]_t types. The _le store
//!    the bytes in little endian order in memory and the _be version store the
//!    bytes in big endian order.
//!
//!    Also as a side-effect, it allows out of alignment access.
//!
//!    And another side-effect is that structures of these will be tightly packed,
//!    there is no padding as would happen with the normal uint16_t and uint32_t.
//!    The padding is added for the reason of alignment, but because these types
//!    can be accessed without any particular byte alignment, then padding isn't
//!    needed.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#ifndef DISABLE_ENDIAN_TO_STRING
#  include "string_utilities.hpp"
#endif // DISABLE_ENDIAN_TO_STRING

/// \brief
/// Drop in replacement for uint16_t which can help simplify dealing with endian conversions.
/// This is to be used in structs for file formats or protocols that are little-endian.
/// \note
/// Not as efficient as using uint16_t for general purpose use in structs, however it is
/// excellent for use in structs used to handle protocols and file formats for serialization
/// and de-serialization. It has the added benefit of making the structs packed.
class uint16_le
{
public:
  uint16_le()                           { }
  /// Construct from the machine's endianess a 16-bit unsigned int storing it as data in little endian order
  uint16_le(uint16_t a_val)             { operator=(a_val); }
  /// implicitly casts the data which is in little endian converting it to the machine's endianess as a 16-bit unsigned int
  operator uint16_t() const             { return uint16_t(m_bytes[1] << 8U) | m_bytes[0]; }
  /// Assigns from the machine's endianess a 16-bit int storing it as data in little endian order
  uint16_le& operator=(uint16_t a_val)  { m_bytes[1] = a_val >> 8U; m_bytes[0] = a_val & 0xFF; return *this; }
private:
  uint8_t m_bytes[2];
};

/// \brief
/// Drop in replacement for uint32_t which can help simplify dealing with endian conversions.
/// This is to be used in structs for file formats or protocols that are little-endian.
/// \see uint16_le
class uint32_le
{
public:
  uint32_le()                          { }
  /// Construct from the machine's endianess a 32-bit uint storing it as data in little endian order
  uint32_le(uint32_t a_val)            { operator=(a_val); }
  /// implicitly casts the data which is in little endian converting it to the machine's endianess as a 32-bit unsigned int
  operator uint32_t() const            { return uint32_t(m_bytes[3] << 24U) | uint32_t(m_bytes[2] << 16U) | uint32_t(m_bytes[1] << 8U) | m_bytes[0]; }
  /// Assigns from the machine's endianess a 32-bit int storing it as data in little endian order
  uint32_le& operator=(uint32_t a_val) { m_bytes[3] = a_val >> 24U; m_bytes[2] = a_val >> 16U; m_bytes[1] = a_val >> 8U; m_bytes[0] = a_val; return *this; }
private:
  uint8_t m_bytes[4];
};

/// \brief
/// Drop in replacement for uint64_t which can help simplify dealing with endian conversions.
/// This is to be used in structs for file formats or protocols that are little-endian.
/// \see uint16_le
class uint64_le
{
public:
  uint64_le()                          { }
  /// Construct from the machine's endianess a 64-bit unsigned int storing it as data in little endian order
  uint64_le(uint64_t a_val)            { operator=(a_val); }
  /// implicitly casts the data which is in little endian converting it to the machine's endianess as a 64-bit unsigned int
  operator uint64_t() const            { return get<7,56U>() | get<6,48U>() | get<5,40U>() | get<4,32U>()
                                              | get<3,24U>() | get<2,16U>() | get<1,8U>()  | get<0,0U>(); }
  /// Assigns from the machine's endianess a 64-bit int storing it as data in little endian order
  uint64_le& operator=(uint64_t a_val) { m_bytes[7] = a_val >> 56U; m_bytes[6] = a_val >> 48U; m_bytes[5] = a_val >> 40U; m_bytes[4] = a_val >> 32U;
                                         m_bytes[3] = a_val >> 24U; m_bytes[2] = a_val >> 16U; m_bytes[1] = a_val >> 8U; m_bytes[0] = a_val; return *this; }
private:
  template <size_t I, size_t SHIFT>
  inline uint64_t get() const { return uint64_t(uint64_t(m_bytes[I]) << SHIFT); }
  uint8_t m_bytes[8];
};

/// \brief
/// Drop in replacement for uint16_t which can help simplify dealing with endian conversions.
/// This is to be used in structs for file formats or protocols that are big-endian.
/// \see uint16_le
class uint16_be
{
public:
  uint16_be()                          { }
  /// Construct from the machine's endianess a 16-bit unsigned int storing it as data in big endian order
  uint16_be(uint16_t a_val)            { operator=(a_val); }
  /// implicitly casts the data which is in big endian converting it to the machine's endianess as a 16-bit unsigned int
  operator uint16_t() const            { return uint16_t(m_bytes[0] << 8U) | m_bytes[1]; }
  /// Assigns from the machine's endianess a 16-bit int storing it as data in big endian order
  uint16_be& operator=(uint16_t a_val) { m_bytes[0] = a_val >> 8U; m_bytes[1] = a_val & 0xFF; return *this; }
private:
  uint8_t m_bytes[2];
};

/// \brief
/// Drop in replacement for uint32_t which can help simplify dealing with endian conversions.
/// This is to be used in structs for file formats or protocols that are big-endian.
/// \see uint16_le
class uint32_be
{
public:
  uint32_be()                          { }
  /// Construct from the machine's endianess a 32-bit unsigned int storing it as data in big endian order
  uint32_be(uint32_t a_val)            { operator=(a_val); }
  /// implicitly casts the data which is in big endian converting it to the machine's endianess as a 32-bit unsigned int
  operator uint32_t() const            { return uint32_t(m_bytes[0] << 24U) | uint32_t(m_bytes[1] << 16U) | uint32_t(m_bytes[2] << 8U) | m_bytes[3]; }
  /// Assigns from the machine's endianess a 32-bit int storing it as data in big endian order
  uint32_be& operator=(uint32_t a_val) { m_bytes[0] = a_val >> 24U; m_bytes[1] = a_val >> 16U; m_bytes[2] = a_val >> 8U; m_bytes[3] = a_val; return *this; }
private:
  uint8_t m_bytes[4];
};

/// \brief
/// Drop in replacement for uint64_t which can help simplify dealing with endian conversions.
/// This is to be used in structs for file formats or protocols that are big-endian.
/// \see uint16_le
class uint64_be
{
public:
  uint64_be()                          { }
  /// Construct from the machine's endianess a 64-bit unsigned int storing it as data in big endian order
  uint64_be(uint64_t a_val)            { operator=(a_val); }
  /// implicitly casts the data which is in big endian converting it to the machine's endianess as a 64-bit unsigned int
  operator uint64_t() const            { return get<0,56U>() | get<1,48U>() | get<2,40U>() | get<3,32U>()
                                              | get<4,24U>() | get<5,16U>() | get<6,8U>()  | get<7,0U>(); }
  /// Assigns from the machine's endianess a 64-bit int storing it as data in big endian order
  uint64_be& operator=(uint64_t a_val) { m_bytes[0] = a_val >> 56U; m_bytes[1] = a_val >> 48U; m_bytes[2] = a_val >> 40U; m_bytes[3] = a_val >> 32U;
                                         m_bytes[4] = a_val >> 24U; m_bytes[5] = a_val >> 16U; m_bytes[6] = a_val >> 8U; m_bytes[7] = a_val; return *this; }
private:
  template <size_t I, size_t SHIFT>
  inline uint64_t get() const { return uint64_t(uint64_t(m_bytes[I]) << SHIFT); }
  uint8_t m_bytes[8];
};

// Turn off to_string for endian int types
#ifndef DISABLE_ENDIAN_TO_STRING

/// @brief Converts a_arg to a string.
/// @param a_arg is the number to convert to a string.
/// @return Returns a_arg as a string.
inline auto to_string(uint16_le a_arg)
{
  return to_string(a_arg.operator uint16_t());
}

/// @copydoc to_string(uint16_le a_arg)
inline auto to_string(uint32_le a_arg)
{
  return to_string(a_arg.operator uint32_t());
}

/// @copydoc to_string(uint16_le a_arg)
inline auto to_string(uint64_le a_arg)
{
  return to_string(a_arg.operator uint64_t());
}

/// @copydoc to_string(uint16_le a_arg)
inline auto to_string(uint16_be a_arg)
{
  return to_string(a_arg.operator uint16_t());
}

/// @copydoc to_string(uint16_le a_arg)
inline auto to_string(uint32_be a_arg)
{
  return to_string(a_arg.operator uint32_t());
}

/// @copydoc to_string(uint16_le a_arg)
inline auto to_string(uint64_be a_arg)
{
  return to_string(a_arg.operator uint64_t());
}

#endif // DISABLE_ENDIAN_TO_STRING

