#
# MagicCore x86 OS Kernel Loader
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

# Macro to add an exported label (both with an without an underscore so it is accessible in C with either gcc or clang)
#define GLOBAL(id) \
  .globl id ;\
  id: ;\
  .globl _##id ;\
  _##id

# Macro to define the init_array section name, it depends if using gcc or clang as to what they call it
# was using: (__clang__) and (__GNUC__), but now based on explicitly set flags
#if defined(macho_fmt)
#  define  init_array_section    __TEXT,__constructor
#  define  init_early_section    __TEXT,.init_early
#elif defined(elf_fmt)
#  define  init_array_section    .init_array
#  define  init_early_section    .init_early
#endif
