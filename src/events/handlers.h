#pragma once
#include "../all.h"

// Macros for declaring and registering event handlers.
#define EVENT_HANDLER_PARAMS \
    __attribute__((unused)) XEvent *event, \
    __attribute__((unused)) Display *display, \
    __attribute__((unused)) Window root_window
#define HANDLE_IMPLEMENTATION(type, line, count) \
    void handler_##type##line##count(EVENT_HANDLER_PARAMS); \
    void register_handler_##type##line##count() __attribute__((constructor)); \
    void register_handler_##type##line##count() { \
        register_event_handler(type, &handler_##type##line##count); \
    } \
    void handler_##type##line##count(EVENT_HANDLER_PARAMS)
#define HANDLE_EXPANDED(type, line, count) HANDLE_IMPLEMENTATION(type, line, count)
#define HANDLE(type) HANDLE_EXPANDED(type, __LINE__, __COUNTER__)

// Event handler callback function signature.
typedef void EventHandler(XEvent *event, Display *display, Window root_window);

/**
 * Registers an event handler for a given event type.
 * 
 * @param event_type The type of event to register the handler for.
 * @param event_handler The event handler callback function.
 * 
 * @warning Don't use directly! Use the `HANDLE` macro instead.
 */
void register_event_handler(int event_type, EventHandler *event_handler);

/**
 * Invokes all registered event handlers for a given event type.
 * 
 * @param display The X11 display.
 * @param window The X11 window where the event occurred.
 * @param event_type The type of event that occurred.
 * @param event The event data.
 * 
 * @note Even though the event data contains the event type, it is passed as a
 * separate parameter anyways to allow for custom event types.
 */
void invoke_event_handlers(Display *display, Window window, int event_type, XEvent *event);
