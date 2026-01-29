/**
 * This code is responsible for managing the shortcut registry, including
 * registration, key grabbing, and lookup of keyboard shortcuts.
 */

#include "../all.h"

/** A registered keyboard shortcut with its name and key combination. */
typedef struct {
    char name[MAX_SHORTCUT_NAME];
    int keys[MAX_SHORTCUT_KEYS];
} Shortcut;

static Shortcut shortcuts[MAX_SHORTCUTS];
static char terminal_command[CONFIG_MAX_VALUE_LENGTH];

static void grab_shortcut_keys(int *keys, int keys_size)
{
    Display *display = DefaultDisplay;
    Window root = DefaultRootWindow(display);

    // Separate modifiers from non-modifier keys.
    unsigned int modifiers = 0;
    KeyCode keycodes[MAX_SHORTCUT_KEYS] = {0};
    int keycode_count = 0;

    for (int i = 0; i < keys_size; i++)
    {
        if (keys[i] == 0 || keys[i] == NoSymbol) continue;
        unsigned int mod = x_keysym_to_modifier(keys[i]);
        if (mod != 0)
        {
            modifiers |= mod;
        }
        else
        {
            keycodes[keycode_count++] = XKeysymToKeycode(display, keys[i]);
        }
    }

    // Grab each non-modifier key with the modifier mask. This ensures all keys
    // in multi-key shortcuts are grabbed and won't propagate to clients.
    // We grab with all four NumLock/CapsLock combinations so shortcuts work
    // whether these locks are on or off.
    unsigned int locks[] = {0, Mod2Mask, LockMask, Mod2Mask | LockMask};
    for (int i = 0; i < keycode_count; i++)
    {
        if (keycodes[i] == 0) continue;
        for (int j = 0; j < 4; j++)
        {
            XGrabKey(display, keycodes[i], modifiers | locks[j], root, True, GrabModeAsync, GrabModeAsync);
        }
    }
}

static void register_shortcut(const char *name, int *keys, int keys_size)
{
    // Ensure the parameters are within the expected limits.
    if (strlen(name) > MAX_SHORTCUT_NAME)
    {
        LOG_WARNING("Shortcut name \"%s\" is too long! Maximum is %d.", name, MAX_SHORTCUT_NAME);
        return;
    }
    if (keys_size > MAX_SHORTCUT_KEYS)
    {
        LOG_WARNING("Shortcut \"%s\" has too many keys! Maximum is %d.", name, MAX_SHORTCUT_KEYS);
        return;
    }

    // Register the shortcut.
    bool registered = false;
    for (int i = 0; i < MAX_SHORTCUTS; i++)
    {
        // Find a free slot in the shortcuts registry.
        if (shortcuts[i].name[0] != '\0')
        {
            continue;
        }

        // Copy the name and keys into the shortcut entry.
        strcpy(shortcuts[i].name, name);
        for (int j = 0; j < keys_size; j++)
        {
            shortcuts[i].keys[j] = keys[j];
        }

        registered = true;
        break;
    }
    if (!registered)
    {
        LOG_WARNING("Shortcut registry is full! Could not register \"%s\".", name);
        return;
    }

    // Grab the key combination to prevent it from propagating to clients.
    grab_shortcut_keys(keys, keys_size);
}

/** Check if a shortcut key array contains a specific key. */
static bool shortcut_contains_key(int key, int *array, int array_size)
{
    for (int i = 0; i < array_size; i++)
    {
        if (array[i] == key) return true;
    }
    return false;
}

/** Count non-zero keys in a shortcut key array. */
static int count_shortcut_keys(int *array, int array_size)
{
    int count = 0;
    for (int i = 0; i < array_size; i++)
    {
        if (array[i] != 0) count++;
    }
    return count;
}

const char *get_terminal_command()
{
    return terminal_command;
}

int find_shortcut(int *keys, int keys_size, char *out_name, int out_name_size)
{
    int input_key_count = count_shortcut_keys(keys, keys_size);

    for (int i = 0; i < MAX_SHORTCUTS; i++)
    {
        // Skip empty/unregistered shortcut slots.
        if (shortcuts[i].name[0] == '\0') continue;

        // Quick rejection: key counts must match.
        int shortcut_key_count = count_shortcut_keys(shortcuts[i].keys, MAX_SHORTCUT_KEYS);
        if (shortcut_key_count != input_key_count) continue;

        // Order-independent comparison: check that all shortcut keys are
        // present in the input. Since counts match, this ensures an exact
        // set match.
        bool match = true;
        for (int j = 0; j < MAX_SHORTCUT_KEYS; j++)
        {
            int shortcut_key = shortcuts[i].keys[j];
            if (shortcut_key == 0) continue;

            if (!shortcut_contains_key(shortcut_key, keys, keys_size))
            {
                match = false;
                break;
            }
        }

        // If the keys match, assign the name of the shortcut to the
        // `out_name` parameter.
        if (match && out_name_size > 0)
        {
            strncpy(out_name, shortcuts[i].name, out_name_size - 1);
            out_name[out_name_size - 1] = '\0';  // Ensure null termination.
            return 0;
        }
    }

    return -1;
}

HANDLE(Initialize)
{
    char config_value[CONFIG_MAX_VALUE_LENGTH];
    int keys[MAX_SHORTCUT_KEYS];

    // Load the terminal command.
    common.get_config_str(
        terminal_command, sizeof(terminal_command),
        CFG_KEY_TERMINAL_COMMAND, CFG_DEFAULT_TERMINAL_COMMAND
    );

    // Register the terminal shortcut.
    common.get_config_str(
        config_value, sizeof(config_value),
        CFG_KEY_TERMINAL_SHORTCUT, CFG_DEFAULT_TERMINAL_SHORTCUT
    );
    x_key_names_to_symbols(config_value, '+', keys, MAX_SHORTCUT_KEYS);
    register_shortcut(CFG_KEY_TERMINAL_SHORTCUT, keys, MAX_SHORTCUT_KEYS);

    // Register the exit shortcut.
    common.get_config_str(
        config_value, sizeof(config_value),
        CFG_KEY_EXIT_SHORTCUT, CFG_DEFAULT_EXIT_SHORTCUT
    );
    x_key_names_to_symbols(config_value, '+', keys, MAX_SHORTCUT_KEYS);
    register_shortcut(CFG_KEY_EXIT_SHORTCUT, keys, MAX_SHORTCUT_KEYS);

    // Register the close shortcut.
    common.get_config_str(
        config_value, sizeof(config_value),
        CFG_KEY_CLOSE_SHORTCUT, CFG_DEFAULT_CLOSE_SHORTCUT
    );
    x_key_names_to_symbols(config_value, '+', keys, MAX_SHORTCUT_KEYS);
    register_shortcut(CFG_KEY_CLOSE_SHORTCUT, keys, MAX_SHORTCUT_KEYS);
}
