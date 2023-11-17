/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

//
// C++ compatibility macros:
//
//   - if using a suitable compiler, use newer language features
//   - otherwise provide fallbacks so that the code can still be compiled with older compilers.
//
// Main assumption should be a C++11 compiler, however this assumption is not valid for older
// targets, for example if targetting a DOS hosted environment, using the TurboC compiler then
// possibilty not even C++03 is supported. I think a pretty minimal expection of what the compiler
// supports would be best, so keeping it simple, however there are some great additions since
// C++11 which improve type safety such as enum classes and static_asserts. If these can be
// emulated then that is ideal.
//

#if __cplusplus >= 201103L
#  define NO_RETURN           [[ noreturn ]]
#  define ENUM(name, type)    enum class name : type {
#  define END_ENUM()          };
#  define STATIC_ASSERT(expr) static_assert(expr, "error") 
#else
#  define NO_RETURN
#  define ENUM(name, type)    namespace name { enum e_##name {
#  define END_ENUM()          }; };
#  define STATIC_ASSERT(expr) typedef int static_assert_##__LINE__[(expr) ? 1 : -1];
#endif

#endif // COMPATIBILITY_H
