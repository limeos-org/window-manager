#pragma once
#include "../all.h"

/**
 * A custom event type. This event is invoked before the `Initialize` event, and
 * is intended to be used for any necessary setup before the event loop begins.
 * 
 * @note - When the `Prepare` event is invoked, configuration has not yet been
 * loaded.
 * @note - When using within a `HANDLE()` macro, the event is always `NULL`.
 * @note - Only handle the `Prepare` event if absolutely necessary. Use the
 * `Initialize` event for most setup tasks.
 */
#define Prepare 128

/**
 * A custom event type. This event is invoked after the `Prepare` event, and
 * is intended to be used for any necessary setup after the event loop begins.
 * 
 * @note - When the `Initialize` event is invoked, configuration has been
 * loaded.
 * @note - When using within a `HANDLE()` macro, the event is always `NULL`.
 */
#define Initialize 129

/**
 * A custom event type. Similar to the `ButtonPress` event, with the exception
 * that it is invoked when a button is pressed anywhere on the screen, no matter
 * what events are being listened for on the target window.
 * 
 * @note - When using within a `HANDLE()` macro, the event presents itself as a 
 * standard Xlib `xbutton` event.
 */
#define GlobalButtonPress 130

/**
 * A custom event type. Similar to the `ButtonRelease` event, with the exception
 * that it is invoked when a button is released anywhere on the screen, no matter
 * what events are being listened for on the target window.
 * 
 * @note - When using within a `HANDLE()` macro, the event presents itself as a 
 * standard Xlib `xbutton` event.
 */
#define GlobalButtonRelease 131

/**
 * A custom event type. Similar to the `MotionNotify` event, with the exception
 * that it is invoked when a pointer is moved anywhere on the screen, no matter
 * what events are being listened for on the target window.
 * 
 * @note - When using within a `HANDLE()` macro, the event presents itself as a 
 * standard Xlib `xmotion` event.
 */
#define GlobalMotionNotify 132

/**
 * A custom event type. Similar to the `KeyPress` event, with the exception
 * that it is invoked when a key is pressed anywhere on the screen, no matter
 * what events are being listened for on the target window.
 * 
 * @note - When using within a `HANDLE()` macro, the event presents itself as a 
 * standard Xlib `xkey` event.
 */
#define GlobalKeyPress 133

/**
 * A custom event type. Similar to the `KeyRelease` event, with the exception
 * that it is invoked when a key is released anywhere on the screen, no matter
 * what events are being listened for on the target window.
 * 
 * @note - When using within a `HANDLE()` macro, the event presents itself as a 
 * standard Xlib `xkey` event.
 */
#define GlobalKeyRelease 134

/**
 * A custom event type. Invoked when a portal is created.
 * 
 * @note - When using within a `HANDLE()` macro, the event presents itself as a
 * standard Xlib `xclient` event.
 * @note - A pointer to the created window can be found in: 
 * `(Portal*)xclient->data.l[0]`.
 */
#define PortalCreated 135

/**
 * A custom event type. Invoked when a portal is destroyed.
 * 
 * @note - When using within a `HANDLE()` macro, the event presents itself as a
 * standard Xlib `xclient` event.
 * @note - The client window ID of the destroyed portal can be found in:
 * `(Window)xclient->data.l[0]`.
 * @note - The frame window ID of the destroyed portal can be found in:
 * `(Window)xclient->data.l[1]`.
 * @note - Both windows are destroyed, but their ID's may still be used to 
 * perform any necessary cleanup.
 */
#define PortalDestroyed 136

/**
 * A custom event type. Invoked when a portal is focused.
 * 
 * @note - When using within a `HANDLE()` macro, the event presents itself as a
 * standard Xlib `xclient` event.
 * @note - A pointer to the focused window can be found in:
 * `(Portal*)xclient->data.l[0]`.
 */
#define PortalFocused 137

/**
 * Initiates an infinite loop, handling X11/XInput2 events as they come in, and
 * invoking the appropriate registered event handlers.
 * 
 * @param display The X11 display.
 * @param root_window The X11 root window.
 */
void initialize_event_loop(Display *display, Window root_window);
