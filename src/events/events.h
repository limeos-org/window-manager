#pragma once
#include "../all.h"

/**
 * A custom event type. This event is invoked before the `Initialize` event, and
 * is intended to be used for any necessary setup before the event loop begins.
 * 
 * @note When the `Prepare` event is invoked, configuration has not yet been loaded.
 * @note Only handle the `Prepare` event if absolutely necessary. Use the
 * `Initialize` event for most setup tasks.
 */
#define Prepare 128

/**
 * A custom event type. This event is invoked after the `Prepare` event, and
 * is intended to be used for any necessary setup after the event loop begins.
 * 
 * @note When the `Initialize` event is invoked, configuration has been loaded.
 */
#define Initialize 129

/**
 * A custom event type. Similar to the `ButtonPress` event, with the exception
 * that it is invoked when a button is pressed anywhere on the screen, no matter
 * what events are being listened for on the target window.
 */
#define GlobalButtonPress 130

/**
 * A custom event type. Similar to the `ButtonRelease` event, with the exception
 * that it is invoked when a button is released anywhere on the screen, no matter
 * what events are being listened for on the target window.
 */
#define GlobalButtonRelease 131

/**
 * A custom event type. Similar to the `MotionNotify` event, with the exception
 * that it is invoked when a pointer is moved anywhere on the screen, no matter
 * what events are being listened for on the target window.
 */
#define GlobalMotionNotify 132

/**
 * Initiates an infinite loop, handling X11/XInput2 events as they come in, and
 * invoking the appropriate registered event handlers.
 * 
 * @param display The X11 display.
 * @param root_window The X11 root window.
 * 
 * @note This function initiates an infinite loop and does not return under
 * normal circumstances. Any code that follows this function call will be
 * unreachable.
 */
void initialize_event_loop(Display *display, Window root_window);
