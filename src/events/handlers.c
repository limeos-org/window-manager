#include "../all.h"

typedef struct {
    int type;
    EventCallback *callback;
} EventHandler;

typedef struct {
    EventHandler *handlers;
    int count;
    int capacity;
} EventHandlers;

static EventHandlers event_handlers = {
    .handlers = NULL,
    .count = 0,
    .capacity = 0,
};

void register_event_handler(int type, EventCallback *callback)
{
    // Allocate additional memory for the event handlers if necessary.
    if (event_handlers.count >= event_handlers.capacity)
    {
        int new_capacity = event_handlers.capacity == 0 ? 2 : event_handlers.capacity * 2;
        EventHandler *handlers = realloc(event_handlers.handlers, new_capacity * sizeof(EventHandler));
        if (handlers == NULL)
        {
            LOG_ERROR("Failed to allocate memory for event handlers.");
            exit(EXIT_FAILURE);
        }
        event_handlers.handlers = handlers;
        event_handlers.capacity = new_capacity;
    }

    // Register the event handler.
    event_handlers.handlers[event_handlers.count] = (EventHandler){
        .type = type,
        .callback = callback,
    };
    event_handlers.count++;
}

void call_event_handlers(Event *event)
{
    // Iterate through all event handlers, calling the callback of each event
    // handler that has a matching event type.
    for (int i = 0; i < event_handlers.count; i++)
    {
        if (event_handlers.handlers[i].type == event->type)
        {
            event_handlers.handlers[i].callback(event);
        }
    }
}
