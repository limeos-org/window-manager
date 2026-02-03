/**
 * This code is responsible for managing the `_NET_CLIENT_LIST` property on
 * the root window, which is an array of top-level client windows that are
 * managed by the window manager. EWMH specification requires every window 
 * manager to advertise this list so that other applications can query it.
 * 
 * https://specifications.freedesktop.org/wm-spec/1.5/ar01s03.html#id-1.4.4
 */

#include "../all.h"

static void update_ewmh_client_list(Portal *exclude)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Allocate memory for the client list.
    Window *client_list = malloc(MAX_PORTALS * sizeof(Window));
    if (client_list == NULL)
    {
        LOG_ERROR("Could not update EWMH client list, memory allocation failed.");
        return;
    }

    // Build the client list from valid, initialized, top-level portals.
    // Per EWMH spec, _NET_CLIENT_LIST contains only top-level application
    // windows (for taskbars/switchers), excluding popups, tooltips, etc.
    Portal *portals = get_unsorted_portals();
    int clients_added = 0;
    for (int i = 0; i < MAX_PORTALS; i++)
    {
        Portal *portal = &portals[i];
        if (!portal->active) continue;
        if (portal == exclude) continue;
        if (!portal->initialized) continue;
        if (!portal->top_level) continue;

        client_list[clients_added] = portal->client_window;
        clients_added++;
    }

    // Update the `_NET_CLIENT_LIST` property on the root window.
    unsigned char *cast_client_list = (unsigned char *)client_list;
    Atom _NET_CLIENT_LIST = XInternAtom(display, "_NET_CLIENT_LIST", False);
    XChangeProperty(
        display,            // Display
        root_window,        // Window
        _NET_CLIENT_LIST,   // Property
        XA_WINDOW,          // Type
        32,                 // Format (32-bit)
        PropModeReplace,    // Mode
        cast_client_list,   // Data
        clients_added       // Element Count
    );

    // Free the client list.
    free(client_list);
}

HANDLE(PortalMapped)
{
    update_ewmh_client_list(NULL);
}

HANDLE(PortalDestroyed)
{
    PortalDestroyedEvent *_event = &event->portal_destroyed;
    update_ewmh_client_list(_event->portal);
}
