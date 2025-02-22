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
    XCreateWindowEvent *_event = &event->xcreatewindow;

    // Ensure the window isn't an off-screen dummy window.
    if (_event->x < 0 || _event->y < 0) return;

    // Ensure the window wasn't created by ourselves.
    pid_t pid = x_get_window_pid(display, _event->window);
    if (pid == getpid()) return;

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

    // Considering a ConfigureRequest event can only be sent by clients, we can
    // safely assume that the window is a client window.
    Window client_window = _event->window;

    // Apply the configuration changes exactly as requested by the client.
    XConfigureWindow(
        _event->display,
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

HANDLE(ConfigureNotify)
{
    XConfigureEvent *_event = &event->xconfigure;

    // Ensure the event came from a portal client window.
    Portal *portal = find_portal_by_window(_event->window);
    if (portal == NULL || _event->window != portal->client_window) return;

    // Synchronize the portal geometry.
    synchronize_portal(portal);
}
