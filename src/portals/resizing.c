#include "../all.h"

#define PORTAL_RESIZE_AREA_SIZE 10

static Portal *resized_portal = NULL;
static bool is_resizing = false;

static int mouse_start_root_x = 0, mouse_start_root_y = 0;
static int portal_start_width = 0, portal_start_height = 0;

static int throttle_ms = 0;
static Time last_resize_time = 0;

bool is_portal_resize_area(Portal *portal, int rel_x, int rel_y)
{
    // Check if the position is within the bottom-right corner of the portal.
    return (rel_x >= (int)portal->geometry.width - PORTAL_RESIZE_AREA_SIZE &&
            rel_x <= (int)portal->geometry.width &&
            rel_y >= (int)portal->geometry.height - PORTAL_RESIZE_AREA_SIZE &&
            rel_y <= (int)portal->geometry.height);
}

void start_resizing_portal(Portal *portal, int mouse_root_x, int mouse_root_y)
{
    is_resizing = true;
    resized_portal = portal;
    portal_start_width = portal->geometry.width;
    portal_start_height = portal->geometry.height;
    mouse_start_root_x = mouse_root_x;
    mouse_start_root_y = mouse_root_y;

    // Show the resizing cursor.
    add_marker(common.string_to_id("resizing_portal"), XC_bottom_right_corner, true);
}

void update_resizing_portal(int mouse_root_x, int mouse_root_y, Time event_time)
{
    // Throttle the resizing to prevent excessive updates.
    if (event_time - last_resize_time < (Time)throttle_ms) return;

    // Determine minimum dimensions from client hints or use defaults.
    int min_width = MINIMUM_PORTAL_WIDTH;
    int min_height = MINIMUM_PORTAL_HEIGHT;
    XSizeHints hints;
    if (XGetWMNormalHints(DefaultDisplay, resized_portal->client_window, &hints, &(long){0}))
    {
        if (hints.flags & PMinSize)
        {
            min_width = common.int_max(MINIMUM_PORTAL_WIDTH, hints.min_width);
            min_height = common.int_max(MINIMUM_PORTAL_HEIGHT, hints.min_height);
            if (is_portal_frame_valid(resized_portal))
            {
                min_height += PORTAL_TITLE_BAR_HEIGHT;
            }
        }
    }

    // Calculate new portal width and height.
    int new_portal_width = common.int_max(
        min_width,
        portal_start_width + (mouse_root_x - mouse_start_root_x)
    );
    int new_portal_height = common.int_max(
        min_height,
        portal_start_height + (mouse_root_y - mouse_start_root_y)
    );

    // Resize the portal using the existing function.
    resize_portal(resized_portal, new_portal_width, new_portal_height);

    // Update the last resize time, so we can throttle the next update.
    last_resize_time = event_time;
}

void stop_resizing_portal(void)
{
    // Disable resizing.
    is_resizing = false;
    resized_portal = NULL;

    // Hide the resizing cursor.
    remove_marker(common.string_to_id("resizing_portal"));
}

bool is_portal_resizing()
{
    return is_resizing;
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

    // If the destroyed portal was being resized, stop resizing.
    if (is_resizing && _event->portal == resized_portal)
    {
        stop_resizing_portal();
    }
}
