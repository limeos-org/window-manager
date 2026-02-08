#include "../all.h"

static Portal *focused_portal = NULL;

Portal *get_focused_portal()
{
    return focused_portal;
}

void focus_portal(Portal *portal)
{
    if (portal->visibility != PORTAL_VISIBLE) return;

    // Always set keyboard focus (external events can steal it).
    x_focus_window(DefaultDisplay, portal->client_window);

    // Raise the portal if not already on top.
    if (get_top_portal() != portal)
    {
        raise_portal(portal);
    }

    // Notify if the portal is not already focused.
    if (get_focused_portal() != portal)
    {
        call_event_handlers((Event*)&(PortalFocusedEvent){
            .type = PortalFocused,
            .portal = portal
        });
    }
}

HANDLE(PortalFocused)
{
    focused_portal = event->portal_focused.portal;
}

HANDLE(PortalDestroyed)
{
    PortalDestroyedEvent *_event = &event->portal_destroyed;
    Portal *destroyed = _event->portal;

    // Clear stale focus if the destroyed portal was focused.
    if (focused_portal == destroyed) focused_portal = NULL;

    // Skip focus transfer if the destroyed portal was not focused.
    if (focused_portal != NULL) return;

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
    PortalMappedEvent *_event = &event->portal_mapped;
    Portal *portal = _event->portal;

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
