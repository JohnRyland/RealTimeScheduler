/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#include "interrupts.h"


#include <dispatch/dispatch.h>
#include <stdio.h>
#include <stdlib.h>

dispatch_queue_t queue;
dispatch_source_t timer1;

void sigtrap(int sig)
{
  dispatch_source_cancel(timer1);
  printf("CTRL-C received, exiting program\n");
  exit(EXIT_SUCCESS);
}

void cancel_handler(void* timer)
{
  dispatch_release((dispatch_source_t)timer);
  dispatch_release(queue);
  printf("end\n");
  exit(0);
}

void vector(void* timer)
{
  //  dispatch_source_cancel(timer);
}

int main(int argc, const char* argv[]) {

    signal(SIGINT, &sigtrap);   //catch the cntl-c
    queue = dispatch_queue_create("timerQueue", 0);
    // Create dispatch timer source
    timer1 = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
    // Set block for dispatch source when catched events
    dispatch_source_set_event_handler_f(timer1, vector);
    // Set block for dispatch source when cancel source
    dispatch_source_set_cancel_handler_f(timer1, cancel_handler);

    dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC); // after 1 sec

    // Set timer
    dispatch_source_set_timer(timer1, start, NSEC_PER_SEC / 5, 0);  // 0.2 sec
    printf("start\n");    
    dispatch_resume(timer1);
    while(1)
    {
        ;;
    }
    return 0;
}







void outp(int,int)
{
}


void disable()
{
}


void enable()
{
}


isr_func_t getvect(int)
{
  return 0;
}


void setvect(int, isr_func_t)
{
}


