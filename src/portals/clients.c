#include "../all.h"

bool is_portal_client_valid(Portal *portal)
{
    return (
        portal != NULL &&
        portal->client_window != 0 &&
        x_window_exists(DefaultDisplay, portal->client_window)
    );
}

int destroy_portal_client(Portal *portal)
{
    Display *display = DefaultDisplay;
    Window client_window = portal->client_window;

    // Try to gracefully close via the `WM_DELETE_WINDOW` protocol (Newer),
    // fallback to `XDestroyWindow()` if protocol is unsupported (Older).
    Atom WM_PROTOCOLS = XInternAtom(display, "WM_PROTOCOLS", False);
    Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
    if(x_window_supports_protocol(display, client_window, WM_DELETE_WINDOW))
    {
        int status = XSendEvent(display, client_window, False, NoEventMask, (XEvent*)&(XClientMessageEvent) {
            .type = ClientMessage,
            .window = client_window,
            .message_type = WM_PROTOCOLS,
            .format = 32,
            .data.l[0] = WM_DELETE_WINDOW,
            .data.l[1] = CurrentTime
        });
        if (status == 0) return -1;
    }
    else
    {
        int status = XDestroyWindow(display, client_window);
        if (status == 0) return -2;
        portal->client_window = 0;
    }

    return 0;
}

HANDLE(CreateNotify)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
    XCreateWindowEvent *_event = &event->xcreatewindow;

    // Ensure the window isn't an off-screen dummy window.
    if (_event->x < 0 || _event->y < 0) return;

    // Ensure the window wasn't created by ourselves.
    pid_t pid = x_get_window_pid(display, _event->window);
    if (pid == getpid()) return;

    // Only create portals for top-level windows (direct children of root).
    // Per ICCCM Section 4.1.1, top-level windows are direct children of root.
    // Child windows of applications should not become portals.
    Window parent_window = x_get_window_parent(display, _event->window);
    if (parent_window != root_window) return;

    // Create a portal for the window.
    create_portal(_event->window);
}

HANDLE(DestroyNotify)
{
    XDestroyWindowEvent *_event = &event->xdestroywindow;

    // Ensure the event came from a portal client window.
    Portal *portal = find_portal_by_window(_event->window);
    if (portal == NULL || portal->client_window != _event->window) return;

    // Destroy the portal.
    destroy_portal(portal);
}

HANDLE(MapRequest)
{
    XMapRequestEvent *_event = &event->xmaprequest;

    // Ensure the event came from a portal client window.
    Portal *portal = find_portal_by_window(_event->window);
    if (portal == NULL || portal->client_window != _event->window) return;

    // Map all portal windows.
    map_portal(portal);
}

HANDLE(MapNotify)
{
    XMapEvent *_event = &event->xmap;

    // Ensure the event came from a portal client window.
    Portal *portal = find_portal_by_window(_event->window);
    if (portal == NULL || portal->client_window != _event->window) return;

    // Map all portal windows.
    map_portal(portal);
}

HANDLE(UnmapNotify)
{
    XUnmapEvent *_event = &event->xunmap;

    // Ensure the event came from a portal client window.
    Portal *portal = find_portal_by_window(_event->window);
    if (portal == NULL || portal->client_window != _event->window) return;

    // Unmap all portal windows.
    unmap_portal(portal);
}

HANDLE(ConfigureRequest)
{
    XConfigureRequestEvent *_event = &event->xconfigurerequest;
    Display *display = DefaultDisplay;
    Window client_window = _event->window;

    // Either apply the configuration to the portal, or directly to the
    // client window, depending on whether the portal is framed.
    Portal *portal = find_portal_by_window(client_window);
    if (portal != NULL && is_portal_frame_valid(portal))
    {
        // For framed portals, the client window position is fixed within the
        // frame. Only allow resize requests, and resize the whole portal.
        if (_event->value_mask & (CWWidth | CWHeight))
        {
            unsigned int new_width = (_event->value_mask & CWWidth) ?
                _event->width : portal->geometry.width;
            unsigned int new_height = (_event->value_mask & CWHeight) ?
                _event->height + PORTAL_TITLE_BAR_HEIGHT : portal->geometry.height;
            resize_portal(portal, new_width, new_height);
        }

        // Send a synthetic ConfigureNotify to inform the client of its actual
        // geometry, as required by ICCCM.
        XSendEvent(display, client_window, False, StructureNotifyMask, (XEvent*)&(XConfigureEvent) {
            .type = ConfigureNotify,
            .display = display,
            .event = client_window,
            .window = client_window,
            .x = portal->geometry.x_root,
            .y = portal->geometry.y_root + PORTAL_TITLE_BAR_HEIGHT,
            .width = common.int_max(1, portal->geometry.width),
            .height = common.int_max(1, portal->geometry.height - PORTAL_TITLE_BAR_HEIGHT),
            .border_width = 0,
            .above = None,
            .override_redirect = False
        });
    }
    else
    {
        // For non-framed windows, apply configuration changes as requested.
        XConfigureWindow(
            display,
            client_window,
            _event->value_mask,
            &(XWindowChanges) {
                .x = _event->x,
                .y = _event->y,
                .width = _event->width,
                .height = _event->height,
                .border_width = _event->border_width,
                .sibling = _event->above,
                .stack_mode = _event->detail
            }
        );
    }
}

HANDLE(ConfigureNotify)
{
    XConfigureEvent *_event = &event->xconfigure;
    Display *display = DefaultDisplay;

    // Ensure the event came from a portal client window.
    Portal *portal = find_portal_by_window(_event->window);
    if (portal == NULL || _event->window != portal->client_window) return;

    // Skip geometry enforcement and synchronization for fullscreen portals.
    // Fullscreen portals have their geometry managed by fullscreen.c.
    if (portal->fullscreen) return;

    // Enforce client position within the frame for framed portals.
    // Some clients try to move themselves even after being reparented.
    if (is_portal_frame_valid(portal))
    {
        // Move back only if the position is incorrect.
        if (_event->x != 0 || _event->y != PORTAL_TITLE_BAR_HEIGHT)
        {
            XMoveWindow(display, portal->client_window, 0, PORTAL_TITLE_BAR_HEIGHT);
        }
    }

    // Synchronize the portal geometry.
    synchronize_portal(portal);
}
