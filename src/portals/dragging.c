#include "../all.h"

static Portal *dragged_portal = NULL;
static bool is_dragging = false;

static int mouse_start_root_x = 0, mouse_start_root_y = 0;
static int portal_start_x = 0, portal_start_y = 0;

static int throttle_ms = 0;
static Time last_drag_time = 0;

static void start_dragging_portal(Portal *portal, int mouse_root_x, int mouse_root_y)
{
    is_dragging = true;
    dragged_portal = portal;
    portal_start_x = portal->x_root;
    portal_start_y = portal->y_root;
    mouse_start_root_x = mouse_root_x;
    mouse_start_root_y = mouse_root_y;

    // Show the dragging cursor.
    add_marker(string_to_id("dragging_portal"), XC_fleur, true);
}

static void update_dragging_portal(int mouse_root_x, int mouse_root_y, Time event_time)
{
    // Throttle the dragging to prevent excessive updates.
    if (event_time - last_drag_time < (Time)throttle_ms) return;

    // Calculate the new portal position.
    int new_portal_x = portal_start_x + (mouse_root_x - mouse_start_root_x);
    int new_portal_y = portal_start_y + (mouse_root_y - mouse_start_root_y);

    // Move the portal using the existing function.
    move_portal(dragged_portal, new_portal_x, new_portal_y);

    // Update the last drag time, so we can throttle the next update.
    last_drag_time = event_time;
}

static void stop_dragging_portal()
{
    // Disable dragging.
    is_dragging = false;
    dragged_portal = NULL;

    // Hide the dragging cursor.
    remove_marker(string_to_id("dragging_portal"));
}

bool is_portal_dragging()
{
    return is_dragging;
}

HANDLE(Initialize)
{
    int framerate;
    get_config_int(&framerate, CFG_KEY_FRAMERATE, CFG_DEFAULT_FRAMERATE);
    throttle_ms = framerate_to_throttle_ms(framerate);
}

HANDLE(PortalButtonPress)
{
    PortalButtonPressEvent *_event = &event->portal_button_press;

    if (_event->button != Button1) return;
    if (is_dragging == true) return;

    Portal *portal = _event->portal;
    if (portal == NULL) return;

    // Ensure we're clicking on the frame area (title bar).
    if (is_portal_frame_area(portal, _event->x_portal, _event->y_portal) == false) return;

    // Don't start dragging if clicking on a trigger (close button, etc.).
    if (is_portal_triggers_area(portal, _event->x_portal, _event->y_portal)) return;

    start_dragging_portal(portal, _event->x_root, _event->y_root);
}

HANDLE(RawButtonRelease)
{
    RawButtonReleaseEvent *_event = &event->raw_button_release;

    if (_event->button != Button1) return;
    if (is_dragging == false) return;

    stop_dragging_portal();
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
    if (is_dragging)
    {
        Time current_time = x_get_current_time();
        update_dragging_portal(pointer_x_root, pointer_y_root, current_time);
        return;
    }

    // Handle hover cursor for frame/title bar area.
    Portal *portal = find_portal_by_window(child_window);
    bool in_frame_area = false;

    if (portal != NULL && is_portal_frame_valid(portal))
    {
        int rel_x = pointer_x_root - portal->x_root;
        int rel_y = pointer_y_root - portal->y_root;

        // Only show frame cursor if not in resize area (resize takes priority).
        if (!is_portal_resize_area(portal, rel_x, rel_y) &&
            is_portal_frame_area(portal, rel_x, rel_y))
        {
            in_frame_area = true;
        }
    }

    // Use markers - add_marker is safe to call repeatedly (no-op if exists),
    // remove_marker is safe to call if not exists.
    if (in_frame_area)
    {
        add_marker(string_to_id("hover_frame"), XC_hand2, false);
    }
    else
    {
        remove_marker(string_to_id("hover_frame"));
    }
}

HANDLE(PortalDestroyed)
{
    PortalDestroyedEvent *_event = &event->portal_destroyed;

    // If the destroyed portal was being dragged, stop dragging.
    if (is_dragging && _event->portal == dragged_portal)
    {
        stop_dragging_portal();
    }
}
