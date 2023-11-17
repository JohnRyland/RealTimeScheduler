///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    string.hpp
//!
//! @brief
//!    C++ String class without dynamic memory.
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
//!    Aims to provide similar functionality to std::string, however without
//!    any dynamic memory allocations. It should be almost a drop-in replacement
//!    for most common use cases, but currently only implements a small number
//!    of the functions of std::string, some deliberately avoided for safety or
//!    because they would require dynamic memory allocations.
//!   
//!    Consider the std::string compare function. Provided in String is a single
//!    compare function but in std::string the API has overloads for comparing
//!    sub-strings of either or both of the comparing string and/or compared
//!    string. It could be easy to get this wrong, yet the alternative is the
//!    caller chains this together with calls to substr of either or both the
//!    strings to achieve the same with code which is more self documenting and
//!    obvious. I would guess std::string has these overloads for efficiency
//!    because the std::string returned by substr would require dynamic memory
//!    allocations that are avoided which is not something String<N> has to
//!    worry about.
//!   
//!    Because this class uses an internal fixed length buffer to store the
//!    characters and we want to avoid dynamic memory allocations, it is not
//!    possible to grow the string, so appending character or strings to the
//!    string is not possible, however concatenating two strings together to
//!    a new object is possible. The lack of the ability to append to a given
//!    string is usually able to be worked around by creating a new variable.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "debug_logger.h"
#include "utilities.hpp"

/// @brief
/// A string class that doesn't use any dynamic memory allocations. Aims to be
/// a replacement for std::string in many cases, but because it doesn't make
/// any allocations it can never be API compatible.
/// @todo
/// This is not UTF-8 aware or able to be used with wide-strings. These are
/// possible future enhancements.
template <size_t N>
class String
{
public:
  /// @brief This is the type of the string data the class stores.
  typedef char const DataArrayType[N+1];

  inline constexpr
  void check_valid() const
  {
    assertion(m_data[N] == 0);
    assertion(m_len <= N);
  }

  /// @brief Default constructor.
  inline constexpr
  String()
  {
    initData();
    check_valid();
  }

  /// @brief Copy constructor.
  /// @param a_other is other string of same type that will be copy constructed from.
  inline constexpr
  String(const String<N>& a_other)
  {
    initData(a_other.m_data, a_other.m_len);
    check_valid();
  }

  /// @brief Constant string constructor.
  /// @param a_data is a constant string that will be copied.
  inline constexpr
  String(const char (&a_data)[N])
  {
    initData(a_data, N);
    check_valid();
  }

  /// @brief Constant string constructor.
  /// @param a_data is a constant string that will be copied.
  /// @param a_length is the size of the string (excluding the null terminator).
  inline constexpr
  String(const char (&a_data)[N], size_t a_length)
  {
    initData(a_data, a_length);
    check_valid();
  }

/*
  /// @brief Copy from other string of another size.
  /// @tparam OtherLen is the size of the other String<N> object.
  /// @param a_other is the other String<N> object to be copied.
  template <size_t OtherLen>
  inline constexpr
  String(const String<OtherLen>& a_other)
  {
    k_dbg<STRING>("Copy constructor from different size. From -%s- with N=%i OtherLen: %i other.m_len: %i\n", a_other.m_data, N, OtherLen, a_other.m_len);
    static_assert(OtherLen <= N, "Can only copy to a string same or smaller size.");
    initData(a_other.m_data, a_other.m_len);
    check_valid();
  }
*/

  /// @brief Copy constructor from a constant string.
  /// @tparam OtherLen is the size of the string to be copied.
  /// @param a_other is the constant string that will be copied.
  template <size_t OtherLen>
  inline constexpr
  String(const char (&a_other)[OtherLen])
  {
    static_assert(OtherLen-1 <= N, "Can only copy to a string same or smaller size.");
    /// @todo check if this should be OtherLen-1 as it may include the nul-terminator
    initData(a_other, OtherLen-1);
    check_valid();
  }

  /// @brief Assignment operator from a same sized string.
  /// @param a_other is the other string that will be copied.
  inline constexpr
  String<N>& operator=(const String<N>& a_other)
  {
    k_dbg<STRING>("assignment\n");
    memCpy(m_data, a_other.m_data, N);
    m_len = a_other.m_len;
    return *this;
  }

  /// @brief Concatenate this string with another string to create a new string.
  /// @tparam N2 is the size of the second string.
  /// @param a_other is the second string to be concatenated with this string.
  /// @return Returns a new string which is this string with a_other appended to it.
  template <size_t N2>
  inline constexpr
  String<N+N2> operator+(const String<N2>& a_other) const
  {
    String<N+N2> newArray;
    size_t len = min(m_len, N);
    assertion(len <= N);
    size_t olen = min(a_other.m_len, N2);
    assertion(olen <= N2);
    newArray.m_len = len + olen;
    assertion(newArray.m_len <= (N+N2));
    assertion(sizeof(newArray.m_data) == (N+N2+1));
    memCpy(newArray.m_data, m_data, len);
    memCpy(newArray.m_data, a_other.m_data, olen, 0, len);
    memZero(newArray.m_data, N + N2 + 1 - newArray.m_len, newArray.m_len);
    assertion(newArray.m_data[N+N2] == 0);

    newArray.check_valid();
    return newArray;
  }

  /// @brief Similar to substr in std::string, it gets a sub-section of this string.
  /// @tparam N2 is the maximum size of the resulting sub-string.
  /// @param a_offset is the position in this string that the sub-string starts from.
  /// @param a_length is the length of the sub-string.
  /// @return Returns a new string which is a sub-section of this string from a_offset and a_length long.
  template <size_t N2>
  inline constexpr
  String<N2> startLengthSubString(size_t a_offset, size_t a_length = npos) const
  {
    assertion(a_offset <= m_len);
    a_length = (a_length == npos) ? m_len - a_offset : a_length;
    assertion(a_offset + a_length <= N);
    assertion(a_length <= N2);
    String<N2> newArray;
    newArray.initData(m_data, a_length, a_offset);
    assertion(newArray.m_data[N2] == 0);
    assertion(m_len <= N);

    newArray.check_valid();
    return newArray;
  }

  template <size_t N2>
  inline constexpr
  String<N2> startEndSubString(size_t a_start, size_t a_end = npos) const
  {
    assertion(m_len);
    a_end = (a_end == npos) ? m_len : a_end;
    assertion(a_end <= N);
    assertion(a_end - a_start <= N2);
    String<N2> newArray;
    newArray.initData(m_data, a_end - a_start + 1, a_start);
    assertion(newArray.m_data[N2] == 0);
    assertion(m_len <= N);

    newArray.check_valid();
    return newArray;
  }

  /// @brief compares this string with a_other for equality
  /// @param a_other is a string to be compared against this string
  /// @return returns true if the strings have the same length and up to that length the characters are the same
  template <size_t N2>
  inline constexpr
  bool compare(const String<N2>& a_other) const
  {
    check_valid();
    a_other.check_valid();
    if (m_len != a_other.m_len)
      return false;
    for (size_t j = 0; j < m_len; ++j)
      if (a_other.m_data[j] != m_data[j])
        return false;
    return true;
  }

  /// @brief Searches in this string for the position of a matching sub-string (needle).
  /// @tparam N2 is the size of a_str.
  /// @param a_str is our needle sub-string we are using to search the haystack (this string).
  /// @return Returns the offset position where the needle was found, or returns npos (-1) if not found.
  template <size_t N2>
  inline constexpr
  size_t find(const String<N2>& a_str) const
  {
    k_dbg<STRING>("searching for %s in %s.  sizes: %zu,%zu  lengths: %zu %zu\n", a_str.data(), data(), N2,N, a_str.length(), length());
    assertion(m_len <= N);
    if (m_len < a_str.m_len)
      return npos;
    size_t last = m_len - a_str.m_len;
    for (size_t i = 0; i <= last; ++i)
      if (a_str.compare(startLengthSubString<N2>(i, a_str.m_len)))
        return i;
    return npos;
  }

  /// @brief Searches in this string starting at offset for the first position of a matching character.
  /// @param a_char is the character to search this string for.
  /// @param a_offset is the starting position in this string to begin searching from.
  /// @return Returns the offset position where the character was found, or returns npos (-1) if it was not found.
  inline constexpr
  size_t find(char a_char, size_t a_offset = 0UL) const
  {
    assertion(m_len <= N);
    if (a_offset <= N && a_offset <= m_len)
      for (size_t i = a_offset; i < m_len; ++i)
        if (m_data[i] == a_char)
          return i;
    return npos;
  }

  /// @brief Gets the length.
  /// @return Returns the length of the string.
  inline constexpr
  size_t length() const
  {
    assertion(m_len <= N);
    return m_len;
  }

  /// @brief Gets the data.
  /// @return Returns a raw pointer to the string data.
  inline constexpr
  const DataArrayType& data() const
  {
    assertion(m_len <= N);
    return m_data;
  }

  /// @brief std::string compatibility.
  /// @return Returns a raw pointer to the string data which is guarenteed to be null-terminated.
  inline constexpr
  const char* c_str() const
  {
    assertion(m_len <= N);
    assertion(m_data[N] == 0);
    return m_data;
  }

  /// @brief Checks if the string is empty.
  /// @return Returns true if the string starts with a null-terminator, otherwise returns false.
  inline constexpr
  bool empty() const
  {
    assertion(m_len <= N);
    assertion(!m_len == !m_data[0]);
    return 0 == m_data[0];
  }

  /// @brief Sentinel value that means an invalid position.
  static const size_t npos = ~0UL;

private:
  template <size_t N2>
  friend class String;

  // Null padded but not necessarily null terminated
  // I've added an extra byte so can ensure null terminated
  // to avoid needing to append a null when needed
  // but this defeats using slices which means extra copying
  char     m_data[N+1];

  // The N+1 for null and this m_len make this String<N> not
  // as nice as it could be for including in structs for
  // doing serialization and deserialization.
  size_t   m_len;

  template <size_t DataLen>
  inline constexpr
  void initData(const char (&a_data)[DataLen], size_t a_length, size_t a_srcOffset)
  {
    k_dbg<STRING>("init data from -%s- with N=%i DataLen=%i a_len=%i\n", a_data, N, DataLen, a_length);
    //assertion(a_length <= N);
    m_len = strLen(a_data, a_length, a_srcOffset);
    m_len = min(m_len, N);
    //assertion(m_len <= N);
    /// @todo Convert m_data to Array so can pick a char type and
    /// avoid calling memcpy/memset and use Array implemented ones.
    memCpy(m_data, a_data, m_len, a_srcOffset);
    memZero(m_data, N + 1 - m_len, m_len);
    //assertion(m_len <= N);
  }

  template <size_t DataLen>
  inline constexpr
  void initData(const char (&a_data)[DataLen], size_t a_length)
  {
    initData(a_data, a_length, 0UL);
  }

  template <size_t DataLen>
  inline constexpr
  void initData(const char (&a_data)[DataLen])
  {
    initData(a_data, 0UL, 0UL);
  }

  inline constexpr
  void initData()
  {
    initData("", 0UL, 0UL);
  }
};
