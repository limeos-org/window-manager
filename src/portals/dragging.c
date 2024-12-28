#include "../all.h"

#define PORTAL_DRAG_THROTTLE_MS 16

static Portal *dragged_portal = NULL;
static bool is_dragging = false;

static int mouse_start_root_x = 0, mouse_start_root_y = 0;
static int portal_start_x = 0, portal_start_y = 0;

static Time last_drag_time = 0;

static void start_dragging_portal(Portal *portal, int mouse_root_x, int mouse_root_y)
{
    is_dragging = true;
    dragged_portal = portal;
    portal_start_x = portal->x;
    portal_start_y = portal->y;
    mouse_start_root_x = mouse_root_x;
    mouse_start_root_y = mouse_root_y;
}

static void update_dragging_portal(int mouse_root_x, int mouse_root_y, Time event_time)
{
    // Throttle the dragging to prevent excessive updates.
    if (event_time - last_drag_time < PORTAL_DRAG_THROTTLE_MS) return;

    // Calculate the new portal position.
    int new_portal_x = portal_start_x + (mouse_root_x - mouse_start_root_x);
    int new_portal_y = portal_start_y + (mouse_root_y - mouse_start_root_y);

    // Update the portal position in memory.
    dragged_portal->x = new_portal_x;
    dragged_portal->y = new_portal_y;

    // Move the portal frame window.
    XMoveWindow(
        dragged_portal->display,
        dragged_portal->frame_window,
        new_portal_x,
        new_portal_y
    );

    // Update the last drag time, so we can throttle the next update.
    last_drag_time = event_time;
}

static void stop_dragging_portal()
{
    // Disable dragging.
    is_dragging = false;

    // Notify the client window of the new position.
    XConfigureEvent configure_event = {
        .type = ConfigureNotify,
        .display = dragged_portal->display,
        .event = dragged_portal->client_window,
        .window = dragged_portal->client_window,
        .x = dragged_portal->x,
        .y = dragged_portal->y,
        .width = dragged_portal->width,
        .height = dragged_portal->height - PORTAL_TITLE_BAR_HEIGHT,
        .border_width = 0,
        .above = None,
        .override_redirect = False
    };
    XSendEvent(
        dragged_portal->display,
        dragged_portal->client_window,
        False,
        StructureNotifyMask,
        (XEvent *)&configure_event
    );
}

HANDLE(GlobalButtonPress)
{
    XButtonEvent *_event = &event->xbutton;

    if (_event->button != Button1) return;
    if (is_dragging == true) return;

    Portal *portal = find_portal(_event->window);
    if(portal == NULL) return;

    if (is_portal_frame_area(portal, _event->x, _event->y) == false) return;

    start_dragging_portal(portal, _event->x_root, _event->y_root);
}

HANDLE(GlobalButtonRelease)
{
    XButtonEvent *_event = &event->xbutton;

    if (_event->button != Button1) return;
    if (is_dragging == false) return;

    stop_dragging_portal();
}

HANDLE(GlobalMotionNotify)
{
    XMotionEvent *_event = &event->xmotion;

    if (is_dragging == false) return;

    update_dragging_portal(_event->x_root, _event->y_root, _event->time);
}
