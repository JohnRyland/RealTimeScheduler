
# Real-time Scheduler
### Small RTOS for MCUs

Copyright (c) 2022, John Ryland
All rights reserved.


## Introduction

This code requires a modern C++ compiler but is mostly C and avoids using
classes.

The intention is that this can be made to work on a variety of embedded
devices. This isn't currently working on any devices yet, but the Pi Pico
might be the first. It is the plan that any device where you program to
bare metal could have this run on it, provided a small piece of code
is written as a driver for installing a timer interrupt handler on the
target device.


## Background

The original version of this scheduler I wrote about 25 years ago while
at university and was a DOS application compiled with a Borland Turbo-C
compiler. It made use of the conio library for visualizing the schedule
and installed a timer ISR to the x86 platform.

The code has been tidied up and modernised to C++11 code, and replacements
for the conio functions and installing a timer event have been made for macOS.

The visualization code is to help develop and debug the scheduler, but isn't
intended as being required on any target device.


## Drivers

The timer driver is the critical piece of code which allows the scheduler
to maintain a tick variable and to run code at regular intervals. It also
allows the scheduler to preempt tasks which have been running longer than
their allocated time.


## Tasks

The main purpose of a scheduler is to run tasks. These tasks might be
periodic (repeating) or aperiodic (single non-repeating tasks).

In a real-time scheduler, a task may have time constraint criteria, such
as not being run earlier than a given time or not being started or finishing
later than a given time. For the scheduler to be able to make any kind of
guarentees that a given task can be executed within these constraints, the tasks
themselves mustn't be unconstrained in how long they run for. They need to
request how long they will run for each time they are scheduled (this is their
execution bound). If they break this constraint, then the scheduling of other
tasks could be broken. In this implementation there is a preemptor which if a
task runs longer than its execution bounds, then the scheduler exits. This
might be able to be changed to make it a callback or policy setting.

The kind of use cases that a real-time task might be used for are things that
are time critical, such as re-filling a buffer which the hardware is continuously
draining, such as an audio device where the audio buffer needs constant refilling
otherwise audible clicks and noises are heard when it isn't refilled in time.
With a fixed sized buffer and the hardware draining it at a constant rate, the
task needed to keep filling it is going to be a periodic task with a period
at least as often as how long it takes for the buffer to be drained. The task has
a simple well defined scope of fetching data and putting it in the buffer.
This should be fairly deterministic in how long it will take, and the
execution bounds can be calculated based on the worst case for how
long this takes.


## Ports

Currently only the macOS port is tested as working. Soon the Linux, Windows
and DOS ports will be updated, and the plan is to support the Pi Pico.


## Goals

Goal is to have this work on the Pi Pico. On the Pi Pico it will be then
the basis for a small RTOS that will be able to help in the refresh of a
LCD panel connects via SPI.

Another goal is to add non-real-time tasks which are able to be run in
the free time of the CPU and can be preempted (interrupted) to then
run a real-time task or to switch to another non-real-time task if it's
quanta of time is up.

Other goals are to be able to switch between kernel and user space when
executing a task, and to switch back when the task is done. This is to
allow for better memory and system protection. Currently everything is
running with the same permissions and protection level. It is essentially
calling a function to execute a task. The alternative will be port/device
specific, so this will need to be abstracted to allow replacing the
implmentation details of this feature.

Other goals are implementing more general purpose drviers and exposing
APIs to these in the tasks to make this more like an actual OS and not
just a scheduler.



