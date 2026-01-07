#include "../all.h"

/**
 * This code is responsible for handling portal dragging operations, including
 * detecting title bar clicks, tracking drag state, and updating portal position
 * in response to mouse movements.
 */

// State variables for tracking the current drag operation.
static Portal *dragging_portal = NULL;
static int drag_offset_x = 0;
static int drag_offset_y = 0;

bool is_portal_dragging()
{
    return dragging_portal != NULL;
}

static bool is_in_title_bar(Portal *portal, int x_portal, int y_portal)
{
    // Ensure the portal is valid.
    if (portal == NULL) return false;

    // Check if the click is within the title bar area.
    return (y_portal >= 0 && 
            y_portal < PORTAL_TITLE_BAR_HEIGHT &&
            x_portal >= 0 && 
            x_portal < (int)portal->width);
}

static void start_drag(Portal *portal, int x_portal, int y_portal)
{
    // Store the drag state.
    dragging_portal = portal;
    drag_offset_x = x_portal;
    drag_offset_y = y_portal;
}

static void end_drag()
{
    // Clear the drag state.
    dragging_portal = NULL;
    drag_offset_x = 0;
    drag_offset_y = 0;
}

static void update_drag(int x_root, int y_root)
{
    // Ensure a drag operation is in progress.
    if (dragging_portal == NULL) return;

    // Calculate the new portal position.
    int new_x = x_root - drag_offset_x;
    int new_y = y_root - drag_offset_y;

    // Move the portal to the new position.
    move_portal(dragging_portal, new_x, new_y);
}

HANDLE(PortalButtonPress)
{
    PortalButtonPressEvent *_event = &event->portal_button_press;
    Portal *portal = _event->portal;

    // Ensure the event is a left button press.
    if (_event->button != Button1) return;

    // Ensure no other operation is in progress.
    if (is_portal_dragging()) return;
    if (is_portal_resizing()) return;

    // Ensure the portal is top-level and has a frame.
    if (!portal->top_level) return;
    if (!is_portal_frame_valid(portal)) return;

    // Check if the click is in the title bar area.
    if (!is_in_title_bar(portal, _event->x_portal, _event->y_portal)) return;

    // Ensure we're not clicking on a trigger button.
    PortalResizeEdge edge = get_portal_resize_edge(
        portal,
        _event->x_portal,
        _event->y_portal
    );
    if (edge != PORTAL_RESIZE_EDGE_NONE) return;

    // Start dragging the portal.
    start_drag(portal, _event->x_portal, _event->y_portal);
}

HANDLE(RawMotionNotify)
{
    (void)event;

    // Ensure a drag operation is in progress.
    if (!is_portal_dragging()) return;

    // Get the current pointer position.
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
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

    // Update the drag operation.
    update_drag(pointer_x_root, pointer_y_root);
}

HANDLE(RawButtonRelease)
{
    RawButtonReleaseEvent *_event = &event->raw_button_release;

    // Ensure the event is a left button release.
    if (_event->button != Button1) return;

    // End the drag operation if one is in progress.
    if (is_portal_dragging())
    {
        end_drag();
    }
}
