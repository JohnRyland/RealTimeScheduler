///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    string_utilities.hpp
//!
//! @brief
//!    Common string utilities.
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
//!    String helper functions for common string manipulations.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "utilities.hpp"
#include "string.hpp"
#include "traits.hpp"

/// @cond document_details
namespace details
{
  // Worse case is base-2 where we output a character for each bit of the number
  template <typename T, size_t BASE>
  static inline constexpr
  auto makeNumberStringHelper(T a_number, uint8_t a_padding = 0, char a_paddingChar = ' ')
  {
    static_assert(BASE >= 2 && BASE <= 16,  "Base must be between 2 and 16.");
  
    // Calculate the space needed as a string for the size of the number in the given base
    const size_t B = ((BASE < 4) ? 8 : ((BASE <= 7) ? 4 : (BASE < 16) ? 3 : 2));
    const size_t N = sizeof(T) * B;
  
    constexpr char NumChar[] = "0123456789ABCDEF";
    char data[N + 1];
    data[N] = 0;
    size_t idx = N;
    size_t len = 0;
    a_padding = min<uint8_t>(a_padding, N);
  
    // Convert number to string
    do {
      idx--;
      len++;
      data[idx] = NumChar[((int)a_number % BASE)];
      a_number /= BASE;
    }
    while ((int)a_number && idx); // TODO: handle double/float and decimal places
  
    // Add padding
    while (len < a_padding && idx)
    {
      idx--;
      len++;
      data[idx] = a_paddingChar;
    }
  
    // Move data from start of characters in to the String object to return
    char data2[N + 1];
    data2[N] = 0;
    memCpy(data2, data, len + 1, idx);
    return String<N + 1>(data2, len);
    //return String<N>(data + idx, len);
  }

  // Convert string to number
  template <typename T, size_t BASE, size_t N>
  static inline constexpr
  T stringToNumber(const String<N>& str, size_t& lastIndex)
  {
    static_assert(BASE >= 2 && BASE <= 16,  "Base must be between 2 and 16.");
    static_assert(N <= 65,  "String longer than expected. Execution should be bounded with a small N value.");
    T number = 0;
    for (lastIndex = 0; lastIndex < N; ++lastIndex)
    {
      // get next char from string
      char ch = str.data()[lastIndex];
      // convert character to value in the given BASE
      if (BASE == 16 && ch >= 'A' && ch <= 'F')
        ch -= 'A' - 10;
      else if (BASE == 16 && ch >= 'a' && ch <= 'f')
        ch -= 'a' - 10;
      else if (ch >= '0' && ch <= '9')
        ch -= '0';
      else
        // stop processign when the character is not a valid number character (0-9 or a-f or A-F)
        return number;
      // also check the character is valid for the given BASE, eg: BASE 2 or binary only accepts 0s and 1s
      if (ch < 0 || ch >= BASE)
        return number;
      // apply the character's value to the number
      number = number * BASE + ch;
    }
    return number;
  }

  template <size_t N>
  static inline constexpr
  int atoi(const String<N>& a_string)
  {
    size_t lastIndex;
    // TODO: check if signed and there is a '+' or '-' sign prepended or not to the number
    return stringToNumber<int, 10>(a_string, lastIndex);
  }

  template <size_t N>
  static inline constexpr
  double atof(const String<N>& a_string)
  {
    // TODO: check if signed and there is a '+' or '-' sign prepended or not to the number
    bool negative = false;
    size_t lastIndex;
    double wholeValue = stringToNumber<int, 10>(a_string, lastIndex);
    double fraction = 0.0;
    if (a_string.data()[lastIndex] == '.')
      fraction = stringToNumber<int, 10>(a_string.startEndSubString(lastIndex + 1), lastIndex);
    while (lastIndex)
      fraction *= 0.1;
    return ((negative) ? -1.0 : 1.0) * (wholeValue + fraction);
  }

} // end namespace details
/// @endcond

/// @brief Constructs a String<N> from the char array.
/// @tparam N is the length of the string.
/// @param a_arg is the string.
/// @return Returns the string as a new String<N>.
template <size_t N>
static inline constexpr
auto to_string(const char (&a_arg)[N])
{
  return String<N>(a_arg);
}

/// @brief Converts a string to a string. Useful for templates that call to_string on a variable type.
/// @tparam N is the length of the string.
/// @param a_arg is the string.
/// @return Returns the string.
template <size_t N>
static inline constexpr
auto to_string(const String<N>& a_arg)
{
  return a_arg;
}

/// @brief Converts numbers to strings.
/// @tparam T has to be a number type.
/// @param a_arg is the number to convert to a string.
/// @return Returns a String<N> object of the number as a string.
/// @note Only converts the whole number part of floating point numbers.
template <typename T, enable_if_t< is_arithmetic<T>::value, bool > = true>
static inline constexpr
auto to_string(const T& a_arg)
{
  return details::makeNumberStringHelper<T, 10>(a_arg, 0);
}

/// @brief Converts a number to a string using base-16 (hex).
/// @tparam T has to be a number type.
/// @param a_arg is the number to convert to a string.
/// @return Returns a String<N> object of the number as a string in hex with 0x prefix.
template <typename T>
static inline constexpr
auto makeHexString(T a_arg)
{
  return to_string("0x") + details::makeNumberStringHelper<T, 16>(a_arg, sizeof(T) * 2, '0');
}

template<typename T, typename type>
using templated_by_return_type = enable_if_t< is_same< T, type >::value, T >;

template <typename T, size_t N>
static inline constexpr
templated_by_return_type<T, int> from_string(const String<N>& a_string)
{
    return atoi(a_string);
}

/// @brief WIP
template <typename T, size_t N>
static inline constexpr
templated_by_return_type<T, float> from_string(const String<N>& a_string)
{
    return atof(a_string);
}

template <size_t N, typename Functor>
static inline constexpr
size_t find_first(const String<N>& a_str, Functor&& a_functor, size_t a_offset = 0)
{
  size_t len = a_str.length();
  assertion(a_offset < N && a_offset <= len);
  for (size_t i = a_offset; i < len; ++i)
    if (a_functor(a_str.data()[i]))
      return i;
  return String<N>::npos;
}

template <size_t N, typename Functor>
static inline constexpr
size_t find_last(const String<N>& a_str, Functor&& a_functor, size_t a_offset = -1)
{
  size_t len = a_str.length();
  a_offset = (a_offset == (size_t)-1) ? len - 1 : a_offset;
  assertion(a_offset < N && a_offset < len);
  for (size_t i = a_offset; (int)i >= 0; --i)
    if (a_functor(a_str.data()[i]))
      return i;
  return String<N>::npos;
}

static inline constexpr
bool isSpace(char ch)
{
  char matches[] = { ' ', '\n', '\r', '\f', '\v', '\t' };
  for (size_t i = 0; i < sizeof(matches); ++i)
    if (ch == matches[i])
        return true;
  return false;
}

template <size_t N>
static inline constexpr
String<N> trim(const String<N>& a_inputString)
{
  String<N> ret;
  size_t begin = find_first(a_inputString, [](char ch){ return !isSpace(ch); });
  if (begin == String<N>::npos)
    return ret;
  size_t end = find_last(a_inputString, [](char ch){ return !isSpace(ch); });
  assertion(end != String<N>::npos); // If we found first non-space, there are non-space characters in the string
  ret = a_inputString.template startEndSubString<N>(begin, end);
  return ret;
}

// Potentially can run at compile-time
template <size_t N>
static inline constexpr
size_t LineCount(const String<N>& a_string)
{
  size_t currentLine = 0;
  size_t lineEnd = 0;
  while (lineEnd != a_string.npos)
  {
    lineEnd = a_string.find('\n', lineEnd + 1);
    ++currentLine;
  }
  return currentLine;
}

// Runs at compile-time
template <size_t N, const char (&STRING)[N]>
static inline constexpr
size_t LineCount()
{
  return LineCount(to_string(STRING));
}

