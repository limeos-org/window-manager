#include "../all.h"

/**
 * This code is responsible for handling the exit shortcut action,
 * performing a graceful shutdown of the window manager.
 */

void exit_window_manager()
{
    LOG_INFO("Exit shortcut triggered, shutting down...");

    // Exit the window manager.
    exit(EXIT_SUCCESS);
}

HANDLE(ShortcutPressed)
{
    ShortcutPressedEvent *_event = &event->shortcut_pressed;

    // Check if this is the exit shortcut.
    if (strcmp(_event->name, "exit") == 0)
    {
        exit_window_manager();
    }
}
