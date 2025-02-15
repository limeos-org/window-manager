#include "../all.h"

HANDLE(RawButtonPress)
{
    RawButtonPressEvent *_event = &event->raw_button_press;
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Get the pointers current position.
    int pointer_x_root = 0, pointer_y_root = 0;
    XQueryPointer(
        display,            // Display
        root_window,        // Window
        &(Window){0},       // Root (Unused)
        &(Window){0},       // Child (Unused)
        &pointer_x_root,    // Pointer X (Relative to root)
        &pointer_y_root,    // Pointer Y (Relative to root)
        &(int){0},          // Window X (Unused)
        &(int){0},          // Window Y (Unused)
        &(unsigned int){0}  // Mask (Unused)
    );

    // Find the portal located at the pointer position.
    Portal *portal = find_portal_at_pos(pointer_x_root, pointer_y_root);
    if (portal == NULL) return;

    // Calculate the position of the pointer relative to the portal.
    int pointer_x_portal = pointer_x_root - portal->x_root;
    int pointer_y_portal = pointer_y_root - portal->y_root;

    // Call all event handlers of the PortalButtonPress event.
    call_event_handlers((Event*)&(PortalButtonPressEvent) {
        .type = PortalButtonPress,
        .portal = portal,
        .x_root = pointer_x_root,
        .y_root = pointer_y_root,
        .x_portal = pointer_x_portal,
        .y_portal = pointer_y_portal,
        .button = _event->button
    });
}

// TEST
HANDLE(PortalButtonPress)
{
    PortalButtonPressEvent *_event = &event->portal_button_press;

    printf(
        "PortalButtonPress:\n"
        "  - Portal: %p\n"
        "  - Client Window: 0x%lx\n"
        "  - Frame Window: 0x%lx\n"
        "  - Root Coordinates: (%d, %d)\n"
        "  - Portal Coordinates: (%d, %d)\n"
        "  - Button: %d\n",
        _event->portal,
        _event->portal->client_window,
        _event->portal->frame_window,
        _event->x_root, _event->y_root,
        _event->x_portal, _event->y_portal,
        _event->button
    );
}