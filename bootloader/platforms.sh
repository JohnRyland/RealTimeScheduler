#!/bin/bash


PLATFORMS="
  i386
  x86_64
  arm
  aarch64
  ppc
  ppc64
  mips
  mips64
  riscv32
  riscv64
  m68k
  sh4
"


for PLATFORM in ${PLATFORMS}
do
  mkdir -p ${PLATFORM}
done

