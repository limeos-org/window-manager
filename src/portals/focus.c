#include "../all.h"

static Portal *last_focused_portal = NULL;

Portal *get_focused_portal()
{
    return last_focused_portal;
}

HANDLE(PortalButtonPress)
{
    PortalButtonPressEvent *_event = &event->portal_button_press;
    Portal *portal = _event->portal;

    // Ensure the portal hasn't already been focused, preventing
    // unnecessary code execution.
    if (last_focused_portal == portal) return;

    // Ensure the portal is mapped before trying to focus it.
    // XSetInputFocus returns BadMatch if the window is not viewable.
    if (!portal->mapped) return;

    // Set input focus on the portal client window.
    XSetInputFocus(
        DefaultDisplay,         // Display
        portal->client_window,  // Window
        RevertToPointerRoot,    // Revert To
        CurrentTime             // Time
    );

    // Raise the portal windows.
    raise_portal(portal);

    // Store the last focused portal.
    last_focused_portal = portal;

    // Call all event handlers of the PortalFocused event.
    call_event_handlers((Event*)&(PortalFocusedEvent){
        .type = PortalFocused,
        .portal = portal
    });
}
