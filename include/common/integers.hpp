///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    integers.hpp
//!
//! @brief
//!    C++ StdInt like types.
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
//!    Various sized integer types for use in our freestanding environment.

#pragma once

typedef unsigned char       uint8_t;
static_assert( 1 ==  sizeof(uint8_t), "adjust uint8_t typedef");
typedef unsigned short      uint16_t;
static_assert( 2 ==  sizeof(uint16_t), "adjust uint16_t typedef");
typedef unsigned int        uint32_t;
static_assert( 4 ==  sizeof(uint32_t), "adjust uint32_t typedef");
typedef unsigned long long  uint64_t;
static_assert( 8 ==  sizeof(uint64_t), "adjust uint64_t typedef");
typedef unsigned long       k_size_t;
static_assert(sizeof(void*) == sizeof(k_size_t), "adjust k_size_t typedef");

typedef signed char         int8_t;
static_assert( 1 ==  sizeof(int8_t), "adjust int8_t typedef");
typedef signed short        int16_t;
static_assert( 2 ==  sizeof(int16_t), "adjust int16_t typedef");
typedef signed int          int32_t;
static_assert( 4 ==  sizeof(int32_t), "adjust int32_t typedef");
typedef signed long long    int64_t;
static_assert( 8 ==  sizeof(int64_t), "adjust int64_t typedef");
typedef signed long         k_ssize_t;
static_assert(sizeof(void*) == sizeof(k_ssize_t), "adjust k_ssize_t typedef");

// Abbreviated versions of the above. Avoid using these in APIs and headers,
// instead preferring the above types. The ones below can be used inside
// implementation specific files.
typedef uint8_t   u8;
typedef int8_t    i8;
typedef uint16_t  u16;
typedef int16_t   i16;
typedef uint32_t  u32;
typedef int32_t   i32;
typedef uint64_t  u64;
typedef int64_t   i64;
typedef k_size_t  sz;
typedef k_ssize_t ssz;
typedef k_size_t  size_t;
typedef k_ssize_t ssize_t;
