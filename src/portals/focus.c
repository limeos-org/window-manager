#include "../all.h"

void handle_portal_focus_click(Portal *portal)
{
    if (portal->visibility != PORTAL_VISIBLE) return;

    // Always set keyboard focus (external events can steal it).
    x_focus_window(DefaultDisplay, portal->client_window);

    // Raise and notify if not already on top.
    if (get_top_portal() == portal) return;
    raise_portal(portal);
    call_event_handlers((Event*)&(PortalFocusedEvent){
        .type = PortalFocused,
        .portal = portal
    });
}

HANDLE(PortalDestroyed)
{
    Portal *destroyed = event->portal_destroyed.portal;

    // Prefer focusing back to the transient parent if available.
    Portal *next_portal = NULL;
    if (destroyed->transient_for != NULL &&
        destroyed->transient_for->initialized &&
        destroyed->transient_for->visibility == PORTAL_VISIBLE &&
        destroyed->transient_for->workspace == get_current_workspace())
    {
        next_portal = destroyed->transient_for;
    }

    // Otherwise find the next topmost portal to put focus on (sorted array is 
    // bottom-to-top).
    if (next_portal == NULL)
    {
        unsigned int count = 0;
        Portal **sorted = get_sorted_portals(&count);
        for (int i = (int)count - 1; i >= 0; i--)
        {
            Portal *portal = sorted[i];
            if (portal == NULL) continue;
            if (portal == destroyed) continue;
            if (!portal->initialized || portal->visibility != PORTAL_VISIBLE) continue;
            if (portal->workspace != get_current_workspace()) continue;
            next_portal = portal;
            break;
        }
    }

    if (next_portal == NULL) return;

    // Set keyboard focus to the next portal.
    x_focus_window(DefaultDisplay, next_portal->client_window);

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

    // Skip override-redirect windows (popups, dropdowns, menus).
    // These manage their own focus and stealing it breaks app behavior.
    if (portal->override_redirect) return;

    // Set keyboard focus to the newly mapped portal.
    x_focus_window(DefaultDisplay, portal->client_window);

    // Notify.
    call_event_handlers((Event*)&(PortalFocusedEvent){
        .type = PortalFocused,
        .portal = portal
    });
}
