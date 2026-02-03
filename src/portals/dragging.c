#include "../all.h"

static Portal *dragged_portal = NULL;
static bool is_dragging = false;

static int mouse_start_root_x = 0, mouse_start_root_y = 0;
static int portal_start_x = 0, portal_start_y = 0;

static int throttle_ms = 0;
static Time last_drag_time = 0;

void start_dragging_portal(Portal *portal, int mouse_root_x, int mouse_root_y)
{
    is_dragging = true;
    dragged_portal = portal;
    portal_start_x = portal->geometry.x_root;
    portal_start_y = portal->geometry.y_root;
    mouse_start_root_x = mouse_root_x;
    mouse_start_root_y = mouse_root_y;

    // Show the dragging cursor.
    add_marker(common.string_to_id("dragging_portal"), XC_fleur, true);
}

void update_dragging_portal(int mouse_root_x, int mouse_root_y, Time event_time)
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

void stop_dragging_portal(void)
{
    // Disable dragging.
    is_dragging = false;
    dragged_portal = NULL;

    // Hide the dragging cursor.
    remove_marker(common.string_to_id("dragging_portal"));
}

bool is_portal_dragging()
{
    return is_dragging;
}

HANDLE(Initialize)
{
    int framerate;
    common.get_config_int(&framerate, CFG_KEY_FRAMERATE, CFG_DEFAULT_FRAMERATE);
    throttle_ms = framerate_to_throttle_ms(framerate);
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
