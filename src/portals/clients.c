#include "../all.h"

static const long client_event_mask =
    PropertyChangeMask;

static void handle_portal_client_config(Portal *portal, XConfigureRequestEvent *event)
{
    // Only handle initial client configuration requests. Once the client
    // is mapped, thus belonging to a portal, we ignore further requests.
    if(portal != NULL) return;

    // Apply the configuration changes exactly as requested by the client.
    XWindowChanges changes;
    changes.x = event->x;
    changes.y = event->y;
    changes.width = event->width;
    changes.height = event->height;
    changes.border_width = event->border_width;
    changes.sibling = event->above;
    changes.stack_mode = event->detail;
    XConfigureWindow(
        event->display,
        event->window,
        event->value_mask,
        &changes
    );
}

bool is_portal_client_area(Portal *portal, int rel_x, int rel_y)
{
    (void)portal, (void)rel_x;
    return rel_y > PORTAL_TITLE_BAR_HEIGHT;
}

bool is_portal_client_valid(Portal *portal)
{
    return (
        portal != NULL &&
        portal->client_window != 0 &&
        x_window_exists(portal->display, portal->client_window)
    );
}

int destroy_portal_client(Portal *portal)
{
    if (!is_portal_client_valid(portal))
    {
        return -1;
    }

    Display *display = portal->display;
    Window client_window = portal->client_window;

    // Try to gracefully close via the `WM_DELETE_WINDOW` protocol, fallback to
    // `XDestroyWindow()` if protocol is unsupported.
    Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    if(x_window_supports_protocol(display, client_window, wm_delete_window))
    {
        XEvent event;
        event.xclient.type = ClientMessage;
        event.xclient.window = client_window;
        event.xclient.message_type = wm_protocols;
        event.xclient.format = 32;
        event.xclient.data.l[0] = wm_delete_window;
        event.xclient.data.l[1] = CurrentTime;

        int status = XSendEvent(display, client_window, False, NoEventMask, &event);
        if (status == 0) return -2;
    }
    else
    {
        int status = XDestroyWindow(display, client_window);
        if (status == 0) return -3;

        // Client was destroyed immediately, so we can clear the client window
        // reference from the portal.
        portal->client_window = 0;
    }

    return 0;
}

HANDLE(MapRequest)
{
    XMapRequestEvent *_event = &event->xmaprequest;

    // Considering a MapRequest event can only be sent by clients, we can
    // safely assume that the window is a client window.
    Window client_window = _event->window;

    // Choose which client window events we should listen for.
    XSelectInput(display, client_window, client_event_mask);

    // Create a portal for the client window.
    create_portal(display, client_window);
}

HANDLE(DestroyNotify)
{
    XDestroyWindowEvent *_event = &event->xdestroywindow;

    // Ensure the event came from a portal client window.
    Portal *portal = find_portal(_event->window);
    if (portal == NULL || _event->window != portal->client_window) return;

    // Destroy the portal.
    destroy_portal(portal);
}

HANDLE(MapNotify)
{
    XMapEvent *_event = &event->xmap;

    // Ensure the event came from a portal client window.
    Portal *portal = find_portal(_event->window);
    if (portal == NULL || _event->window != portal->client_window) return;

    // Map all portal windows.
    map_portal(portal);
}

HANDLE(UnmapNotify)
{
    XUnmapEvent *_event = &event->xunmap;

    // Ensure the event came from a portal client window.
    Portal *portal = find_portal(_event->window);
    if (portal == NULL || _event->window != portal->client_window) return;

    // Unmap all portal windows.
    unmap_portal(portal);
}

HANDLE(ConfigureRequest)
{
    XConfigureRequestEvent *_event = &event->xconfigurerequest;

    // Considering a ConfigureRequest event can only be sent by clients, we can
    // safely assume that the window is a client window.
    Window client_window = _event->window;

    // Handle the client configuration request.
    Portal *portal = find_portal(client_window);
    handle_portal_client_config(portal, _event);
}
