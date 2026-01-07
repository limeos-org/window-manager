#include "../all.h"

/**
 * This code is responsible for parsing shortcut strings, registering named
 * shortcuts, and matching key events to registered shortcuts.
 */

static Shortcut registered_shortcuts[SHORTCUT_MAX_COUNT];
static int shortcut_count = 0;

int parse_shortcut(const char *shortcut_string, unsigned int *out_modifiers, KeySym *out_keysym)
{
    // Ensure the output pointers are valid.
    if (out_modifiers == NULL || out_keysym == NULL) return -1;
    if (shortcut_string == NULL) return -1;

    // Initialize outputs.
    *out_modifiers = 0;
    *out_keysym = NoSymbol;

    // Split the shortcut string by '+'.
    int part_count = 0;
    char **parts = split_string(shortcut_string, "+", &part_count);
    if (parts == NULL || part_count == 0) return -1;

    // Process each part of the shortcut.
    for (int i = 0; i < part_count; i++)
    {
        char *part = parts[i];

        // Convert to lowercase for comparison.
        for (char *p = part; *p; p++)
        {
            *p = tolower((unsigned char)*p);
        }

        // Check for modifier keys.
        if (strcmp(part, "super") == 0 || strcmp(part, "mod4") == 0)
        {
            *out_modifiers |= Mod4Mask;
        }
        else if (strcmp(part, "shift") == 0)
        {
            *out_modifiers |= ShiftMask;
        }
        else if (strcmp(part, "ctrl") == 0 || strcmp(part, "control") == 0)
        {
            *out_modifiers |= ControlMask;
        }
        else if (strcmp(part, "alt") == 0 || strcmp(part, "mod1") == 0)
        {
            *out_modifiers |= Mod1Mask;
        }
        else
        {
            // Not a modifier, treat as the key.
            // Handle special key names.
            if (strcmp(part, "enter") == 0 || strcmp(part, "return") == 0)
            {
                *out_keysym = XK_Return;
            }
            else if (strcmp(part, "space") == 0)
            {
                *out_keysym = XK_space;
            }
            else if (strcmp(part, "tab") == 0)
            {
                *out_keysym = XK_Tab;
            }
            else if (strcmp(part, "escape") == 0 || strcmp(part, "esc") == 0)
            {
                *out_keysym = XK_Escape;
            }
            else if (strcmp(part, "backspace") == 0)
            {
                *out_keysym = XK_BackSpace;
            }
            else if (strcmp(part, "delete") == 0)
            {
                *out_keysym = XK_Delete;
            }
            else if (strlen(part) == 1)
            {
                // Single character key.
                *out_keysym = XStringToKeysym(part);
                if (*out_keysym == NoSymbol)
                {
                    // Try uppercase version.
                    char upper[2] = { toupper(part[0]), '\0' };
                    *out_keysym = XStringToKeysym(upper);
                }
            }
            else
            {
                // Try to convert directly.
                *out_keysym = XStringToKeysym(part);
            }
        }

        // Free each part.
        free(part);
    }

    // Free the parts array.
    free(parts);

    // Verify we got a valid keysym.
    if (*out_keysym == NoSymbol) return -1;

    return 0;
}

int register_shortcut(const char *name, const char *shortcut_string)
{
    // Ensure we have room for another shortcut.
    if (shortcut_count >= SHORTCUT_MAX_COUNT)
    {
        LOG_ERROR("Cannot register shortcut, maximum count reached.");
        return -1;
    }

    // Parse the shortcut string.
    unsigned int modifiers = 0;
    KeySym keysym = NoSymbol;
    if (parse_shortcut(shortcut_string, &modifiers, &keysym) != 0)
    {
        LOG_ERROR("Failed to parse shortcut string: %s", shortcut_string);
        return -1;
    }

    // Store the shortcut.
    Shortcut *shortcut = &registered_shortcuts[shortcut_count];
    strncpy(shortcut->name, name, SHORTCUT_MAX_NAME_LENGTH - 1);
    shortcut->name[SHORTCUT_MAX_NAME_LENGTH - 1] = '\0';
    shortcut->modifiers = modifiers;
    shortcut->keysym = keysym;

    shortcut_count++;

    LOG_INFO("Registered shortcut '%s': %s (modifiers=0x%x, keysym=0x%lx)",
             name, shortcut_string, modifiers, keysym);

    return 0;
}

Shortcut *find_matching_shortcut(unsigned int modifiers, KeySym keysym)
{
    // Mask out lock modifiers (NumLock, CapsLock, etc.)
    unsigned int clean_modifiers = modifiers & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask);

    // Search for a matching shortcut.
    for (int i = 0; i < shortcut_count; i++)
    {
        Shortcut *shortcut = &registered_shortcuts[i];

        if (shortcut->modifiers == clean_modifiers && shortcut->keysym == keysym)
        {
            return shortcut;
        }
    }

    return NULL;
}
