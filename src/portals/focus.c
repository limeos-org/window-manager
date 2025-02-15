#include "../all.h"

Portal *last_focused_portal = NULL;

HANDLE(PortalButtonPress)
{
    PortalButtonPressEvent *_event = &event->portal_button_press;

    // Ensure the portal hasn't already been focused.
    if (last_focused_portal == _event->portal) return;

    // Raise the portal.
    raise_portal(_event->portal);
    last_focused_portal = _event->portal;
}
