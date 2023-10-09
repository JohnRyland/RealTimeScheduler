/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

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
