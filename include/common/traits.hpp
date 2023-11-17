///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    traits.hpp
//!
//! @brief
//!    C++ Type Traits helpers
//!
//! @author
//!    Copyright (c) 2019-2023, John Ryland,
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
//!    Similar but a bit different to the C++20 ranges library.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "integers.hpp"

/// @cond documention_todo

/////////////////
// TYPE TRAITS //
/////////////////

// Type modifiers /////////////////////////////////////////////////////////////

template<class T> struct remove_cv { typedef T type; };
template<class T> struct remove_cv<const T> { typedef T type; };
template<class T> struct remove_cv<volatile T> { typedef T type; };
template<class T> struct remove_cv<const volatile T> { typedef T type; };
template<class T> struct remove_const { typedef T type; };
template<class T> struct remove_const<const T> { typedef T type; };
template<class T> struct remove_volatile { typedef T type; };
template<class T> struct remove_volatile<volatile T> { typedef T type; };
template<class T> struct remove_reference { typedef T type; };
template<class T> struct remove_reference<T&> { typedef T type; };
template<class T> struct remove_reference<T&&> { typedef T type; };

template<class T> using  remove_cv_t = typename remove_cv<T>::type;
template<class T> using  remove_const_t = typename remove_const<T>::type;
template<class T> using  remove_volatile_t = typename remove_volatile<T>::type;
template<class T> using  remove_reference_t = typename remove_reference<T>::type;

// Integral Constants /////////////////////////////////////////////////////////

template<class T, T v>
struct integral_constant
{
  static constexpr T value = v;
  using value_type = T;
  using type = integral_constant;
  constexpr operator value_type() const noexcept { return value; }
  constexpr value_type operator()() const noexcept { return value; }
};

template<bool B> using bool_constant = integral_constant<bool, B>;

using true_type	 = bool_constant<true>;
using false_type = bool_constant<false>;

// Enable If //////////////////////////////////////////////////////////////////

template<bool B, class T = void> struct enable_if {};
template<class T >               struct enable_if<true, T> { typedef T type; };

template<bool B, class T = void> using  enable_if_t = typename enable_if<B,T>::type;

// Traits /////////////////////////////////////////////////////////////////////

template<class T, class U>  struct is_same       : false_type {};
template<class T>           struct is_same<T, T> : true_type {};
template<class T> struct is_lvalue_reference     : false_type {};
template<class T> struct is_lvalue_reference<T&> : true_type {};
template<class T> constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;
template<class> struct is_integral_base: false_type {};

template<> struct is_integral_base<bool>:     true_type {};
template<> struct is_integral_base<int8_t>:   true_type {};
template<> struct is_integral_base<uint8_t>:  true_type {};
template<> struct is_integral_base<int16_t>:  true_type {};
template<> struct is_integral_base<uint16_t>: true_type {};
template<> struct is_integral_base<int32_t>:  true_type {};
template<> struct is_integral_base<uint32_t>: true_type {};
template<> struct is_integral_base<int64_t>:  true_type {};
template<> struct is_integral_base<uint64_t>: true_type {};
template<> struct is_integral_base<k_size_t>: true_type {};

template<class T> struct is_integral : is_integral_base<remove_cv_t<T>> {};

template<class T>
struct is_floating_point
     : integral_constant<
         bool,
         is_same<float, typename remove_cv<T>::type>::value
         || is_same<double, typename remove_cv<T>::type>::value
         || is_same<long double, typename remove_cv<T>::type>::value
     > {};

template<class T>
struct is_arithmetic : integral_constant<bool,
                                              is_integral<T>::value ||
                                              is_floating_point<T>::value> {};

// Forward ////////////////////////////////////////////////////////////////////

template< class T > constexpr T&& forward( remove_reference_t<T>& t ) noexcept
{
    return static_cast<T&&>(t);
}

template< class T > constexpr T&& forward( remove_reference_t<T>&& t ) noexcept
{
    static_assert(!is_lvalue_reference_v<T>);
    return static_cast<T&&>(t);
}

// Generic Traits Helpers /////////////////////////////////////////////////////

///
/// Example usage of the below traits helpers:  
///
///  template <typename T>
///  using is_container_t = has_traits_t<
///    // list of traits to check for:
///    typename T::value_type,
///    typename T::const_iterator,
///    decltype(std::declval<T>().cbegin()),
///    decltype(std::declval<T>().cend())
///  >;
///
///  // Similar to enable_if, uses SFINEA to enable if BaseType matches the set of traits
///  template <typename Container, typename BaseType, is_container_t<BaseType> = true>
///  class container_wrapper_view
///  {
///    // ...
///  };
///

template <class... Ts>
struct has_traits_helper
{
};

template <class Traits, bool B>
struct has_traits
{
};

template <class Traits>
struct has_traits<Traits, true>
{
  using type = bool;
};

template <class... Ts>
using has_traits_t = typename has_traits<has_traits_helper<Ts...>, true>::type;

/// @endcond
