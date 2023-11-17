///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    string_tests.hpp
//!
//! @brief
//!    Unit tests for string class and utilities.
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
//!    Test cases for String<N>.

///////////////////////////////////////////////////////////////////////////////
// Includes

#include <cstdio>
#include "string_utilities.hpp"

// For code coverage, instantiate these templates in this translation unit
#include "array.hpp"

template class Array<int, 1>;

#define TEST(x) \
  if consteval { \
    static_assert(x, #x); \
  } else { \
    puts("running test of " #x); \
    assertion(x); \
    puts("passed test of " #x); \
  }

constexpr void run_tests()
{
  constexpr String<10> s;
  TEST(s.length() == 0);
  TEST(s.data()[0] == 0);

  constexpr auto s2 = to_string("1234");
  TEST(s2.length() == 4);
  TEST(s2.data()[4] == 0);

  constexpr auto s3 = String("1234", 3);
  TEST(s3.length() == 3);
  TEST(s3.data()[3] == 0);

  constexpr auto s4 = s2;
  TEST(s4.length() == 4);
  TEST(s4.data()[4] == 0);

  constexpr auto s5 = s + s2;
  TEST(s5.length() == 4);
  TEST(s5.data()[4] == 0);

  constexpr auto s6 = s2.template startLengthSubString<2>(1,2);
  TEST(s6.length() == 2);
  TEST(s6.data()[0] == '2');
  TEST(s6.data()[1] == '3');
  TEST(s6.data()[2] == 0);

  constexpr auto s7 = s2.template startEndSubString<4>(1,3);
  TEST(s7.length() == 3);
  TEST(s7.data()[0] == '2');
  TEST(s7.data()[1] == '3');
  TEST(s7.data()[2] == '4');
  TEST(s7.data()[3] == 0);

  TEST(s2.compare(to_string("23")) == false);
  TEST(s3.compare(to_string("23")) == false);
  TEST(s2.compare(to_string("1234")) == true);
  TEST(s3.compare(to_string("123")) == true);

  TEST(s2.find(to_string("23")) == 1);
  TEST(s3.find(to_string("23")) == 1);
  TEST(s2.find(to_string("xy")) == -1);
  TEST(s3.find(to_string("xy")) == -1);

  TEST(s2.find('3') == 2);
  TEST(s3.find('3') == 2);
  TEST(s2.find('3', 1) == 2);
  TEST(s3.find('3', 1) == 2);
  TEST(s2.find('3', 2) == 2);
  TEST(s3.find('3', 2) == 2);
  TEST(s2.find('3', 3) == -1);
  TEST(s3.find('3', 3) == -1);

  TEST(strCmp(s2.data(), "1234") == true);
  TEST(strCmp(s3.data(), "123") == true);

  TEST(s.empty() == true);
  TEST(s2.empty() == false);
  TEST(s3.empty() == false);
}

consteval void run_tests_compile_time()
{
  run_tests();
}

void run_tests_run_time()
{
  run_tests();
}

int main()
{
  run_tests_compile_time();
  run_tests_run_time();
}

