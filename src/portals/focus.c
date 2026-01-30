#include "../all.h"

HANDLE(PortalDestroyed)
{
    Portal *destroyed = event->portal_destroyed.portal;

    // Find the next topmost portal to focus (sorted array is bottom-to-top).
    unsigned int count = 0;
    Portal **sorted = get_sorted_portals(&count);
    Portal *next_portal = NULL;
    for (int i = (int)count - 1; i >= 0; i--)
    {
        Portal *portal = sorted[i];
        if (portal == NULL) continue;
        if (portal == destroyed) continue;
        if (!portal->initialized || !portal->mapped) continue;
        next_portal = portal;
        break;
    }

    if (next_portal == NULL) return;

    // Set keyboard focus to the next portal.
    XSetInputFocus(
        DefaultDisplay,
        next_portal->client_window,
        RevertToPointerRoot,
        CurrentTime
    );

    // Raise and notify.
    raise_portal(next_portal);
    call_event_handlers((Event*)&(PortalFocusedEvent){
        .type = PortalFocused,
        .portal = next_portal
    });
}

HANDLE(PortalMapped)
{
    Portal *portal = event->portal_mapped.portal;

    // Set keyboard focus to the newly mapped portal.
    XSetInputFocus(
        DefaultDisplay,
        portal->client_window,
        RevertToPointerRoot,
        CurrentTime
    );

    // Notify.
    call_event_handlers((Event*)&(PortalFocusedEvent){
        .type = PortalFocused,
        .portal = portal
    });
}

HANDLE(PortalButtonPress)
{
    Portal *portal = event->portal_button_press.portal;
    if (!portal->mapped) return;

    // Always set keyboard focus (external events can steal it).
    XSetInputFocus(
        DefaultDisplay,         // Display
        portal->client_window,  // Window
        RevertToPointerRoot,    // Revert To
        CurrentTime             // Time
    );

    // Raise and notify if not already on top.
    if (get_top_portal() == portal) return;
    raise_portal(portal);
    call_event_handlers((Event*)&(PortalFocusedEvent){
        .type = PortalFocused,
        .portal = portal
    });
}
