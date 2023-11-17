
# Requirements
### Copyright (C) 2023, by John Ryland.
### All rights reserved.


## Requirements

This will be a framework for running real-time and non-real-time applications.

The framework can be run either on a host OS or directly on a variety of hardware where it provides the OS.

When hosted on a non-realtime OS, the real-time applications are only simulating that they are real-time, no actual real-time constraints can be guarenteed.

The interface between physical memory and virtual memory also is simulated when run in a hosted way.

The OS should be scalable from hardware without an MMU to ones with an MMU. From single-core to multi-core. Without a GPU to with a GPU. With <1MB to >4GB.
From 32bit to 64bit.

Probably 16bit is out-of-scope.

Low-end spec:

  - 512 kB ROM/Flash
  - 256 kB RAM, no MMU
  - single core ARM 200MHz, 32bit
  - software rendering
  - 240x320

high-end spec:

  - 2TB SSD
  - 8GB RAM
  - MMU
  - 8 core, 3GHz, 64-bit x86
  - GPU (or intel integrated graphics)
  - 4k resolution screen


Some concepts:

 address-spaces
 affinity
 partitioning

kernel
  - scheduler
  - scheduler runs tasks
interrupts
  - call handlers
  - mask, prioritize
  - event queues
memory management
  - address spaces
  - allocate pages

