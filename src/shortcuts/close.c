#include "../all.h"

HANDLE(ShortcutPressed)
{
    ShortcutPressedEvent *_event = &event->shortcut_pressed;

    // Ensure we're handling the close shortcut.
    if (strcmp(_event->name, CFG_KEY_CLOSE_SHORTCUT) != 0) return;

    // Get the currently focused portal.
    Portal *portal = get_focused_portal();
    if (portal == NULL) return;

    // Close the portal (ICCCM compliant - uses WM_DELETE_WINDOW if supported).
    destroy_portal(portal);
}
