#include "../all.h"

/**
 * This code is responsible for managing the `_NET_CLIENT_LIST` property on
 * the root window, which is an array of top-level client windows that are
 * managed by the window manager. EWMH specification requires every window 
 * manager to advertise this list so that other applications can query it.
 * 
 * https://specifications.freedesktop.org/wm-spec/1.5/ar01s03.html#id-1.4.4
 */

static void update_ewmh_client_list()
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Retrieve all portals.
    unsigned int portal_count = 0;
    Portal *portals = get_unsorted_portals(&portal_count);

    // Allocate memory for the client list, the size being the largest possible
    // number of top-level client windows.
    Window *client_list = malloc(portal_count * sizeof(Window));
    if (client_list == NULL)
    {
        LOG_ERROR("Could not update EWMH client list, memory allocation failed.");
        free(client_list);
        return;
    }

    // Populate the client list with client windows that belong to portals
    // that are initialized and top-level.
    int clients_added = 0;
    for (int i = 0; i < (int)portal_count; i++)
    {
        Portal *portal = &portals[i];

        if (portal == NULL) continue;
        if (portal->initialized == false) continue;
        if (portal->top_level == false) continue;

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
}

HANDLE(PortalMapped)
{
    update_ewmh_client_list();
}

HANDLE(PortalDestroyed)
{
    update_ewmh_client_list();
}
