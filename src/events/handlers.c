#include "../all.h"

typedef struct {
    int event_type;
    EventHandler *event_handler;
} EventHandlerMap;

static EventHandlerMap *event_handlers = NULL;
static int event_handlers_count = 0;

void register_event_handler(int event_type, EventHandler *event_handler)
{
    EventHandlerMap *buffer = realloc(event_handlers, (event_handlers_count + 1) * sizeof(EventHandlerMap));
    event_handlers = buffer;

    event_handlers[event_handlers_count].event_type = event_type;
    event_handlers[event_handlers_count].event_handler = event_handler;

    event_handlers_count++;
}

void invoke_event_handlers(Display *display, Window window, int event_type, XEvent *event)
{
    for (int i = 0; i < event_handlers_count; i++)
    {
        if (event_handlers[i].event_type == event_type)
        {
            event_handlers[i].event_handler(event, display, window);
        }
    }
}
