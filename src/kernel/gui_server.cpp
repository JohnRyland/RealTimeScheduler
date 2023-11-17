// John Ryland, 2023
#pragma once


// Gist:

// event queue <- input, mouse, keys, touch, requests etc  (so that IRQs don't block for long, lock-free hopefully)

// update <- RTOS called at some interval which processes the event queue  (part of the schedule and bg processing)

// update can do some drawing or might dispatch events to apps

// what happens if event queue is full? fixed event queue, but might need to calculate some constraints on events
// perhaps can try to dynamically schedule based on queue size. Eventually it will need to drop or compress events.
// mouse events like movement could be skipped, don't have to do each one for example. But needs some consistency, eg:
// mouse events intermingled with key events, then the mouse events need to be calculated until the key event, then
// that processed, and then the rest of the mouse events, we can't just pull out all the mouse events and process at once.

// raw events from drivers
enum gui_event_type
{
    mouse_move,
    mouse_button_down,
    mouse_button_up,
    key_down,
    key_up,
    touch_event,
    create_window,
    draw_command
};

struct gui_event
{
    gui_event_type  ev_type;
    // event data
};

// apps

// queue<gui_event> gui_event_queue;

void gui_add_event(gui_event& ev)
{
  // gui_event_queue.add(ev);
}

void gui_process()
{
    // while (!gui_event_queue.empty())
    //  process_event();
}

void gui_dispatch_event()
{

}




