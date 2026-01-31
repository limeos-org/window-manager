/**
 * This code is responsible for handling workspace switching shortcuts,
 * mapping shortcut names to workspace indices and triggering the switch.
 */

#include "../all.h"

HANDLE(ShortcutPressed)
{
    ShortcutPressedEvent *_event = &event->shortcut_pressed;

    // Check if the shortcut matches any go-to-workspace shortcut.
    for (int i = 0; i < MAX_WORKSPACES; i++)
    {
        // Build the expected shortcut name for this workspace index.
        char expected[MAX_SHORTCUT_NAME];
        snprintf(expected, sizeof(expected), CFG_KEY_WORKSPACE_SHORTCUT_FMT, i + 1);

        // Switch to the workspace if the shortcut name matches.
        if (strcmp(_event->name, expected) == 0)
        {
            switch_workspace(i);
            return;
        }
    }

    // Check if the shortcut matches any move-to-workspace shortcut.
    for (int i = 0; i < MAX_WORKSPACES; i++)
    {
        char expected[MAX_SHORTCUT_NAME];
        snprintf(expected, sizeof(expected), CFG_KEY_MOVE_WORKSPACE_SHORTCUT_FMT, i + 1);

        if (strcmp(_event->name, expected) == 0)
        {
            Portal *portal = get_top_portal();
            if (portal != NULL)
            {
                move_portal_to_workspace(portal, i);
            }
            return;
        }
    }
}
