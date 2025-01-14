#include "../all.h"

HANDLE(ShortcutPressed)
{
    ShortcutPressedEvent *_event = &event->shortcut_pressed;

    // Ensure we're handling the exit shortcut.
    if (strcmp(_event->name, CFG_KEY_EXIT_SHORTCUT) != 0) return;

    exit(EXIT_SUCCESS);
}
