/**
 * This code is responsible for handling `_NET_WM_MOVERESIZE` and
 * `_NET_CLOSE_WINDOW` ClientMessages.
 *
 * CSD (Client-Side Decorations) applications render their own title bars and
 * window controls. When the user drags a CSD titlebar or clicks a CSD button,
 * the application sends a ClientMessage requesting the WM to perform the action.
 *
 * https://specifications.freedesktop.org/wm/1.5/ar01s04.html#id-1.5.2
 * https://specifications.freedesktop.org/wm/1.5/ar01s04.html#id-1.5.4
 */

#include "../all.h"

// Direction constants from EWMH spec (only those we handle).
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
#define _NET_WM_MOVERESIZE_MOVE              8
#define _NET_WM_MOVERESIZE_MOVE_KEYBOARD     10
#define _NET_WM_MOVERESIZE_CANCEL            11

static Atom _NET_WM_MOVERESIZE = None;
static Atom _NET_CLOSE_WINDOW = None;

HANDLE(Prepare)
{
    Display *display = DefaultDisplay;

    _NET_WM_MOVERESIZE = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
    _NET_CLOSE_WINDOW = XInternAtom(display, "_NET_CLOSE_WINDOW", False);
}

HANDLE(ClientMessage)
{
    XClientMessageEvent *xclient = &event->xclient;

    // Ensure this is a `_NET_WM_MOVERESIZE` message / request.
    if (xclient->message_type != _NET_WM_MOVERESIZE) return;

    // Find the portal associated with the request.
    Portal *portal = find_portal_by_window(xclient->window);
    if (portal == NULL) return;

    // Extract move/resize parameters from the message.
    int x_root = xclient->data.l[0];
    int y_root = xclient->data.l[1];
    int direction = xclient->data.l[2];

    // Handle all supported move/resize directions.
    if (direction == _NET_WM_MOVERESIZE_CANCEL)
    {
        if (is_portal_dragging()) stop_dragging_portal();
        if (is_portal_resizing()) stop_resizing_portal();
        return;
    }
    if (direction == _NET_WM_MOVERESIZE_MOVE ||
        direction == _NET_WM_MOVERESIZE_MOVE_KEYBOARD)
    {
        if (!is_portal_dragging())
        {
            start_dragging_portal(portal, x_root, y_root);
        }
        return;
    }
    if (direction == _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT)
    {
        if (!is_portal_resizing())
        {
            start_resizing_portal(portal, x_root, y_root);
        }
        return;
    }
}

HANDLE(ClientMessage)
{
    XClientMessageEvent *xclient = &event->xclient;

    // Ensure this is a `_NET_CLOSE_WINDOW` message / request.
    if (xclient->message_type != _NET_CLOSE_WINDOW) return;

    // Find the portal associated with the request.
    Portal *portal = find_portal_by_window(xclient->window);
    if (portal == NULL) return;

    // Close the portal.
    destroy_portal(portal);
}
