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
    return (rel_x >= (int)portal->width - PORTAL_RESIZE_AREA_SIZE &&
            rel_x <= (int)portal->width &&
            rel_y >= (int)portal->height - PORTAL_RESIZE_AREA_SIZE &&
            rel_y <= (int)portal->height);
}

static void start_resizing_portal(Portal *portal, int mouse_root_x, int mouse_root_y)
{
    is_resizing = true;
    resized_portal = portal;
    portal_start_width = portal->width;
    portal_start_height = portal->height;
    mouse_start_root_x = mouse_root_x;
    mouse_start_root_y = mouse_root_y;

    // Show the resizing cursor.
    add_marker(string_to_id("resizing_portal"), XC_bottom_right_corner, true);
}

static void update_resizing_portal(int mouse_root_x, int mouse_root_y, Time event_time)
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
            min_width = int_max(MINIMUM_PORTAL_WIDTH, hints.min_width);
            min_height = int_max(MINIMUM_PORTAL_HEIGHT, hints.min_height);
            if (is_portal_frame_valid(resized_portal))
            {
                min_height += PORTAL_TITLE_BAR_HEIGHT;
            }
        }
    }

    // Calculate new portal width and height.
    int new_portal_width = int_max(
        min_width,
        portal_start_width + (mouse_root_x - mouse_start_root_x)
    );
    int new_portal_height = int_max(
        min_height,
        portal_start_height + (mouse_root_y - mouse_start_root_y)
    );

    // Resize the portal using the existing function.
    resize_portal(resized_portal, new_portal_width, new_portal_height);

    // Update the last resize time, so we can throttle the next update.
    last_resize_time = event_time;
}

static void stop_resizing_portal()
{
    // Disable resizing.
    is_resizing = false;
    resized_portal = NULL;

    // Hide the resizing cursor.
    remove_marker(string_to_id("resizing_portal"));
}

bool is_portal_resizing()
{
    return is_resizing;
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
    if (is_resizing == true) return;

    Portal *portal = _event->portal;
    if (portal == NULL) return;

    // Ensure we're clicking on the resize area.
    if (is_portal_resize_area(portal, _event->x_portal, _event->y_portal) == false) return;

    start_resizing_portal(portal, _event->x_root, _event->y_root);
}

HANDLE(RawButtonRelease)
{
    RawButtonReleaseEvent *_event = &event->raw_button_release;

    if (_event->button != Button1) return;
    if (is_resizing == false) return;

    stop_resizing_portal();
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

    // Handle active resizing.
    if (is_resizing)
    {
        Time current_time = x_get_current_time();
        update_resizing_portal(pointer_x_root, pointer_y_root, current_time);
        return;
    }

    // Handle hover cursor for resize area.
    Portal *portal = find_portal_by_window(child_window);
    bool in_resize_area = false;
    if (portal != NULL && is_portal_frame_valid(portal))
    {
        int rel_x = pointer_x_root - portal->x_root;
        int rel_y = pointer_y_root - portal->y_root;
        in_resize_area = is_portal_resize_area(portal, rel_x, rel_y);
    }

    // Create or remove the resize marker as needed.
    if (in_resize_area)
    {
        add_marker(string_to_id("hover_resize"), XC_bottom_right_corner, true);
    }
    else
    {
        remove_marker(string_to_id("hover_resize"));
    }
}
