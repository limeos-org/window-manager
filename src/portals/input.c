#include "../all.h"

HANDLE(RawButtonPress)
{
    RawButtonPressEvent *_event = &event->raw_button_press;
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Get the pointer's current position and the window under the cursor.
    int pointer_x_root = 0, pointer_y_root = 0;
    Window child_window = None;
    XQueryPointer(
        display,            // Display
        root_window,        // Window
        &(Window){0},       // Root (Unused)
        &child_window,      // Child (the window under cursor)
        &pointer_x_root,    // Pointer X (Relative to root)
        &pointer_y_root,    // Pointer Y (Relative to root)
        &(int){0},          // Window X (Unused)
        &(int){0},          // Window Y (Unused)
        &(unsigned int){0}  // Mask (Unused)
    );

    // If the click is on an override-redirect window (popup/dropdown/menu),
    // don't interfere - let the application handle it directly.
    if (child_window != None)
    {
        XWindowAttributes attrs;
        if (XGetWindowAttributes(display, child_window, &attrs) &&
            attrs.override_redirect == True)
        {
            return;
        }
    }

    // Find the portal that owns the window under the cursor.
    Portal *clicked_portal = find_portal_by_window(child_window);
    if (clicked_portal == NULL) return;

    // Calculate the position of the pointer relative to the portal.
    int pointer_x_portal = pointer_x_root - clicked_portal->x_root;
    int pointer_y_portal = pointer_y_root - clicked_portal->y_root;

    // Call all event handlers of the PortalButtonPress event.
    call_event_handlers((Event*)&(PortalButtonPressEvent) {
        .type = PortalButtonPress,
        .portal = clicked_portal,
        .x_root = pointer_x_root,
        .y_root = pointer_y_root,
        .x_portal = pointer_x_portal,
        .y_portal = pointer_y_portal,
        .button = _event->button
    });
}

HANDLE(RawButtonRelease)
{
    RawButtonReleaseEvent *_event = &event->raw_button_release;
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Get the pointer's current position and the window under the cursor.
    int pointer_x_root = 0, pointer_y_root = 0;
    Window child_window = None;
    XQueryPointer(
        display,            // Display
        root_window,        // Window
        &(Window){0},       // Root (Unused)
        &child_window,      // Child (the window under cursor)
        &pointer_x_root,    // Pointer X (Relative to root)
        &pointer_y_root,    // Pointer Y (Relative to root)
        &(int){0},          // Window X (Unused)
        &(int){0},          // Window Y (Unused)
        &(unsigned int){0}  // Mask (Unused)
    );

    // Find the portal that owns the window under the cursor.
    Portal *portal = find_portal_by_window(child_window);
    if (portal == NULL) return;

    // Calculate the position of the pointer relative to the portal.
    int pointer_x_portal = pointer_x_root - portal->x_root;
    int pointer_y_portal = pointer_y_root - portal->y_root;

    // Call all event handlers of the PortalButtonRelease event.
    call_event_handlers((Event*)&(PortalButtonReleaseEvent) {
        .type = PortalButtonRelease,
        .portal = portal,
        .x_root = pointer_x_root,
        .y_root = pointer_y_root,
        .x_portal = pointer_x_portal,
        .y_portal = pointer_y_portal,
        .button = _event->button
    });
}

HANDLE(RawMotionNotify)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Get the pointer's current position and the window under the cursor.
    int pointer_x_root = 0, pointer_y_root = 0;
    Window child_window = None;
    XQueryPointer(
        display,            // Display
        root_window,        // Window
        &(Window){0},       // Root (Unused)
        &child_window,      // Child (the window under cursor)
        &pointer_x_root,    // Pointer X (Relative to root)
        &pointer_y_root,    // Pointer Y (Relative to root)
        &(int){0},          // Window X (Unused)
        &(int){0},          // Window Y (Unused)
        &(unsigned int){0}  // Mask (Unused)
    );

    // Find the portal that owns the window under the cursor.
    Portal *portal = find_portal_by_window(child_window);
    if (portal == NULL) return;

    // Calculate the position of the pointer relative to the portal.
    int pointer_x_portal = pointer_x_root - portal->x_root;
    int pointer_y_portal = pointer_y_root - portal->y_root;

    // Call all event handlers of the PortalMotionNotify event.
    call_event_handlers((Event*)&(PortalMotionNotifyEvent) {
        .type = PortalMotionNotify,
        .portal = portal,
        .x_root = pointer_x_root,
        .y_root = pointer_y_root,
        .x_portal = pointer_x_portal,
        .y_portal = pointer_y_portal
    });
}
