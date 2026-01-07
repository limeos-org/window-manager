#include "../all.h"

/**
 * This code is responsible for handling keyboard input, tracking modifier
 * state, and triggering shortcut events when key combinations match.
 */

unsigned int get_current_modifiers()
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Query the current pointer state to get modifiers.
    Window root_return, child_return;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;

    XQueryPointer(
        display,
        root_window,
        &root_return,
        &child_return,
        &root_x, &root_y,
        &win_x, &win_y,
        &mask
    );

    return mask;
}

HANDLE(Initialize)
{
    (void)event;

    // Register the terminal shortcut from config.
    char terminal_shortcut[CFG_MAX_VALUE_LENGTH];
    GET_CONFIG(terminal_shortcut, sizeof(terminal_shortcut), CFG_BUNDLE_TERMINAL_SHORTCUT);
    register_shortcut("terminal", terminal_shortcut);

    // Register the exit shortcut from config.
    char exit_shortcut[CFG_MAX_VALUE_LENGTH];
    GET_CONFIG(exit_shortcut, sizeof(exit_shortcut), CFG_BUNDLE_EXIT_SHORTCUT);
    register_shortcut("exit", exit_shortcut);
}

HANDLE(RawKeyPress)
{
    RawKeyPressEvent *_event = &event->raw_key_press;
    Display *display = DefaultDisplay;

    // Convert keycode to keysym.
    KeySym keysym = XkbKeycodeToKeysym(display, _event->key_code, 0, 0);

    // Get current modifiers.
    unsigned int modifiers = get_current_modifiers();

    // Check if this matches any registered shortcut.
    Shortcut *shortcut = find_matching_shortcut(modifiers, keysym);
    if (shortcut != NULL)
    {
        // Fire the ShortcutPressed event.
        call_event_handlers((Event*)&(ShortcutPressedEvent){
            .type = ShortcutPressed,
            .name = shortcut->name
        });
    }
}
