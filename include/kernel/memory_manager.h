/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

void initialize_memory();

// For real-time the realtime tasks need to have fixed memory requirements and the
// virtual memory is committed to dedicated physical pages so that they can't be
// interrupted or be non-deterministic due to page faults.


// lines
// pages
// ?
// ?
