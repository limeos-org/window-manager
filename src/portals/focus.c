#include "../all.h"

Portal *last_focused_portal = NULL;

HANDLE(PortalButtonPress)
{
    PortalButtonPressEvent *_event = &event->portal_button_press;
    Portal *portal = _event->portal;

    // Ensure the portal hasn't already been focused, preventing
    // unnecessary code execution.
    if (last_focused_portal == portal) return;

    // Set input focus on the portal client window.
    XSetInputFocus(
        DefaultDisplay,         // Display
        portal->client_window,  // Window
        RevertToParent,         // Revert To
        CurrentTime             // Time
    );

    // Raise the portal windows.
    raise_portal(portal);

    // Store the last focused portal.
    last_focused_portal = portal;
}
