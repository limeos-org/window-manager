#pragma once
#include "../all.h"

/**
 * A macro that acts as the final expansion stage of the `HANDLE()` macro. 
 * 
 * It declares two functions: a registration function with the constructor
 * attribute, making it run automatically without needing to be called, and an 
 * event handler callback function.
 * 
 * It then defines the implementations of these two functions: the registration 
 * function registers the event handler, and the event handler callback function
 * is left empty, to be filled in by the user.
 * 
 * @param type The event type.
 * @param count The counter value.
 * 
 * @warning Don't use directly! Use the `HANDLE()` macro instead.
 */
#define HANDLE_IMPLEMENTATION(type, count) \
    static void register_handler_##type##_##count() __attribute__((constructor)); \
    static void handler_##type##_##count(__attribute__((unused)) Event *event); \
    static void register_handler_##type##_##count() \
    { \
        register_event_handler(type, &handler_##type##_##count); \
    } \
    static void handler_##type##_##count(__attribute__((unused)) Event *event)

/**
 * A macro that acts as the intermediary expansion stage of the `HANDLE()`
 * macro.
 * 
 * It adds the `count` parameter, used to prevent naming collisions.
 * 
 * @param type The event type.
 * @param count The counter value.
 * 
 * @warning Don't use directly! Use the `HANDLE()` macro instead.
 */
#define HANDLE_EXPANDED(type, count) HANDLE_IMPLEMENTATION(type, count)

/**
 * A macro that simplifies the process of creating and registering event
 * handlers.
 * 
 * @param type The event type.
 */
#define HANDLE(type) HANDLE_EXPANDED(type, __COUNTER__)

/**
 * Event handler callback function signature.
 * 
 * @param event The event structure.
 */
typedef void EventCallback(Event *event);

/**
 * Registers an event handler for a given event type.
 * 
 * @param type The event type.
 * @param callback The event handler callback function.
 * 
 * @warning Don't use directly! Use the `HANDLE()` macro instead.
 */
void register_event_handler(int type, EventCallback *callback);

/**
 * Calls all registered event handler callback functions for a given event type.
 * 
 * @param event The event structure, where the `type` field is used to determine
 * which event handlers to call.
 */
void call_event_handlers(Event *event);
