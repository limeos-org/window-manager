/**
 * Central dispatcher for portal interaction events.
 *
 * This module routes portal interaction events to the appropriate handlers
 * (dragging, resizing, triggers). It consolidates event handling, eliminates
 * duplicate XQueryPointer calls, and centralizes cursor management.
 */

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

    // Find the portal that owns the window under the cursor.
    Portal *portal = find_portal_by_window(child_window);
    if (portal == NULL) return;

    // Skip override-redirect portals (popups, dropdowns, menus).
    if (portal->override_redirect) return;

    // Handle focus for all button clicks.
    handle_portal_focus_click(portal);

    // Only route left button clicks to interaction handlers.
    if (_event->button != Button1) return;

    // Calculate the position of the pointer relative to the portal.
    int rel_x = pointer_x_root - portal->geometry.x_root;
    int rel_y = pointer_y_root - portal->geometry.y_root;

    // Check triggers area first (close button, etc.).
    if (is_portal_triggers_area(portal, rel_x, rel_y))
    {
        handle_trigger_click(portal, rel_x, rel_y);
        return;
    }

    // Check resize area (bottom-right corner).
    if (is_portal_resize_area(portal, rel_x, rel_y))
    {
        if (!is_portal_resizing())
        {
            start_resizing_portal(portal, pointer_x_root, pointer_y_root);
        }
        return;
    }

    // Check frame area (title bar) for dragging.
    if (is_portal_frame_area(portal, rel_x, rel_y))
    {
        if (!is_portal_dragging())
        {
            start_dragging_portal(portal, pointer_x_root, pointer_y_root);
        }
        return;
    }
}

HANDLE(RawButtonRelease)
{
    RawButtonReleaseEvent *_event = &event->raw_button_release;

    if (_event->button != Button1) return;

    // Stop dragging if active.
    if (is_portal_dragging())
    {
        stop_dragging_portal();
    }

    // Stop resizing if active.
    if (is_portal_resizing())
    {
        stop_resizing_portal();
    }
}

HANDLE(RawMotionNotify)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Get the current pointer position since RawMotionNotify doesn't include it.
    int pointer_x_root = 0, pointer_y_root = 0;
    Window child_window = None;
    XQueryPointer(
        display,            // Display
        root_window,        // Window
        &(Window){0},       // Root (Unused)
        &child_window,      // Child
        &pointer_x_root,    // Pointer X (Relative to root)
        &pointer_y_root,    // Pointer Y (Relative to root)
        &(int){0},          // Window X (Unused)
        &(int){0},          // Window Y (Unused)
        &(unsigned int){0}  // Mask (Unused)
    );

    // Handle active dragging.
    if (is_portal_dragging())
    {
        Time current_time = x_get_current_time();
        update_dragging_portal(pointer_x_root, pointer_y_root, current_time);
        return;
    }

    // Handle active resizing.
    if (is_portal_resizing())
    {
        Time current_time = x_get_current_time();
        update_resizing_portal(pointer_x_root, pointer_y_root, current_time);
        return;
    }

    // Handle hover cursors.
    Portal *portal = find_portal_by_window(child_window);
    bool in_resize_area = false;
    bool in_frame_area = false;

    if (portal != NULL && is_portal_frame_valid(portal))
    {
        int rel_x = pointer_x_root - portal->geometry.x_root;
        int rel_y = pointer_y_root - portal->geometry.y_root;

        // Check resize area first (takes priority over frame area).
        if (is_portal_resize_area(portal, rel_x, rel_y))
        {
            in_resize_area = true;
        }
        // Check frame area (title bar) only if not in resize area.
        else if (is_portal_frame_area(portal, rel_x, rel_y))
        {
            in_frame_area = true;
        }
    }

    // Update hover markers.
    if (in_resize_area)
    {
        add_marker(common.string_to_id("hover_resize"), XC_bottom_right_corner, true);
    }
    else
    {
        remove_marker(common.string_to_id("hover_resize"));
    }

    if (in_frame_area)
    {
        add_marker(common.string_to_id("hover_frame"), XC_hand2, false);
    }
    else
    {
        remove_marker(common.string_to_id("hover_frame"));
    }
}
