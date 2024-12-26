#include "../all.h"

static const long client_event_mask =
    PropertyChangeMask;

static void handle_portal_client_config(Portal *portal, XConfigureRequestEvent *event)
{
    // If the portal hasn't been initialized yet, and a client requests its
    // initial configuration, we accept this request and configure the window.
    // Once the portal is initialized, we will ignore all configuration requests,
    // as the clients size and position are managed by the window manager.
    if(portal == NULL)
    {
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
}

int destroy_portal_client(Portal *portal)
{
    if(portal == NULL || portal->client_window == 0)
    {
        LOG_WARNING("Attempted to destroy a non-existent portal client window.");
        return -1;
    }

    XDestroyWindow(portal->display, portal->client_window);
    portal->client_window = 0;

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
}

HANDLE(ConfigureRequest)
{
    XConfigureRequestEvent *_event = &event->xconfigurerequest;

    // Considering a ConfigureRequest event can only be sent by clients, we can
    // safely assume that the window is a client window.
    Window client_window = _event->window;

    Portal *portal = find_portal(client_window);
    handle_portal_client_config(portal, _event);
}
