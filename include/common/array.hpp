///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    array.hpp
//!
//! @brief
//!    C++ Array Template.
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
//!    Generic fixed length array.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "utilities.hpp"

/// @brief A generic fixed length array.
/// @tparam T is the type of elements.
/// @tparam N is the size.
template <typename T, size_t N>
class Array
{
public:
  /// @brief Copies a_value to the whole array.
  /// @param a_value is the value to set the array to.
  /// @param a_len is the number of items in the array to copy a_value to.
  void Set(T a_value, size_t a_len)
  {
    for (size_t i = 0; i < min(a_len, N); ++i)
      m_data[i] = a_value;
  }

  /// @brief Copies from a_other a_len elements in to this array.
  /// @tparam N2 is the size of the other array.
  /// @tparam OFFSET is the position in this array to copy to.
  /// @param a_other is the other array to be copied from.
  /// @param a_len is the number of elements to copy.
  template <size_t N2, size_t OFFSET=0>
  void Copy(const T (&a_other)[N2], size_t a_len)
  {
    static_assert(N2 <= (N - OFFSET), "Too big to copy.");
    for (size_t i = 0; i < min(a_len, N2); ++i)
      m_data[i + OFFSET] = a_other[i];
  }

  /// @brief Joins two arrays together and returns the combined array.
  /// @tparam N2 is the size of the second array.
  /// @param a_other is the second array.
  /// @return Returns the combined array of this array and a_other.
  template <size_t N2>
  Array<T,N+N2> Concatenate(const Array<T,N2>& a_other)
  {
    Array<T,N+N2> newArray;
    newArray.Copy<N,0>(m_data, N);
    newArray.Copy<N2,N>(a_other.m_data, N2);
    return newArray;
  }

  /// @brief Returns a raw pointer to the array's data.
  T* data()
  {
    return m_data;
  }

  /// @brief Returns a const raw pointer to the array's data.
  const T* data() const
  {
    return m_data;
  }

private:
  T m_data[N];
};

/// @brief A fixed length array of bytes.
/// @tparam N is the size.
template <size_t N>
using ByteArray = Array<uint8_t, N>;
