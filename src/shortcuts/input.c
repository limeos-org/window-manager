#include "../all.h"

static int shortcut_input[MAX_SHORTCUT_KEYS] = {0};

__attribute__((unused))
static void clear_shortcut_input()
{
    for (int i = 0; i < MAX_SHORTCUT_KEYS; i++)
    {
        shortcut_input[i] = 0;
    }
}

static void add_key_to_shortcut_input(int key)
{
    // Ensure the shortcut_input array is not full.
    if (shortcut_input[MAX_SHORTCUT_KEYS - 1] != 0) return;

    // Add the key to the shortcut_input array.
    for (int i = 0; i < MAX_SHORTCUT_KEYS; i++)
    {
        if (shortcut_input[i] == 0)
        {
            shortcut_input[i] = key;
            break;
        }
    }
}

static void remove_key_from_shortcut_input(int key)
{
    for (int i = 0; i < MAX_SHORTCUT_KEYS; i++)
    {
        if (shortcut_input[i] == key)
        {
            shortcut_input[i] = 0;
            return;
        }
    }
}

static bool is_key_in_shortcut_input(int key)
{
    for (int i = 0; i < MAX_SHORTCUT_KEYS; i++)
    {
        if (shortcut_input[i] == key)
        {
            return true;
        }
    }
    return false;
}

static void check_shortcut_input()
{
    // Check if the current shortcut input matches any registered shortcuts.
    char shortcut_name[MAX_SHORTCUT_NAME];
    int result = find_shortcut(shortcut_input, MAX_SHORTCUT_KEYS, shortcut_name, MAX_SHORTCUT_NAME);
    if (result != 0)
    {
        return;
    }

    // Invoke the ShortcutPressed event.
    call_event_handlers((Event*)&(ShortcutPressedEvent){
        .type = ShortcutPressed,
        .name = shortcut_name
    });
}

HANDLE(RawKeyPress)
{
    RawKeyPressEvent *_event = &event->raw_key_press;
    Display *display = DefaultDisplay;

    // Convert keycode to keysym.
    KeySym key = XkbKeycodeToKeysym(display, _event->key_code, 0, 0);

    if (is_key_in_shortcut_input(key)) return;
    add_key_to_shortcut_input(key);
    check_shortcut_input();
}

HANDLE(RawKeyRelease)
{
    RawKeyReleaseEvent *_event = &event->raw_key_release;
    Display *display = DefaultDisplay;

    // Convert keycode to keysym.
    KeySym key = XkbKeycodeToKeysym(display, _event->key_code, 0, 0);

    if (!is_key_in_shortcut_input(key)) return;
    remove_key_from_shortcut_input(key);
}

