#include "../all.h"

typedef struct {
    char name[MAX_SHORTCUT_NAME];
    int keys[MAX_SHORTCUT_KEYS];
} Shortcut;

static Shortcut shortcuts[MAX_SHORTCUTS];

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
    }
}

int find_shortcut(int *keys, int keys_size, char *out_name, int name_size)
{
    for (int i = 0; i < MAX_SHORTCUTS; i++)
    {
        // Skip empty/unregistered shortcut slots.
        if (shortcuts[i].name[0] == '\0') continue;

        // Skip if the first key doesn't match (quick rejection).
        if (shortcuts[i].keys[0] != keys[0]) continue;

        // Compare all keys.
        bool match = true;
        for (int j = 0; j < keys_size; j++)
        {
            if (shortcuts[i].keys[j] != keys[j])
            {
                match = false;
                break;
            }
        }

        // If the keys match, assign the name of the shortcut to the `out_name`
        // parameter.
        if (match && name_size > 0)
        {
            strncpy(out_name, shortcuts[i].name, name_size - 1);
            out_name[name_size - 1] = '\0';  // Ensure null termination.
            return 0;
        }
    }

    return -1;
}

HANDLE(Initialize)
{
    char config_value[CFG_MAX_VALUE_LENGTH];
    int keys[MAX_SHORTCUT_KEYS];

    // Register the terminal shortcut.
    GET_CONFIG(config_value, CFG_MAX_VALUE_LENGTH, CFG_BUNDLE_TERMINAL_SHORTCUT);
    x_key_names_to_symbols(config_value, '+', keys, MAX_SHORTCUT_KEYS);
    register_shortcut(CFG_KEY_TERMINAL_SHORTCUT, keys, MAX_SHORTCUT_KEYS);

    // Register the exit shortcut.
    GET_CONFIG(config_value, CFG_MAX_VALUE_LENGTH, CFG_BUNDLE_EXIT_SHORTCUT);
    x_key_names_to_symbols(config_value, '+', keys, MAX_SHORTCUT_KEYS);
    register_shortcut(CFG_KEY_EXIT_SHORTCUT, keys, MAX_SHORTCUT_KEYS);

    // Register the close shortcut.
    GET_CONFIG(config_value, CFG_MAX_VALUE_LENGTH, CFG_BUNDLE_CLOSE_SHORTCUT);
    x_key_names_to_symbols(config_value, '+', keys, MAX_SHORTCUT_KEYS);
    register_shortcut(CFG_KEY_CLOSE_SHORTCUT, keys, MAX_SHORTCUT_KEYS);
}
