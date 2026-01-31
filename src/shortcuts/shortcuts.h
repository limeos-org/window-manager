#pragma once
#include "../all.h"

/** The maximum number of shortcuts that can be registered. */
#define MAX_SHORTCUTS 24

/** The maximum length of a shortcut name. */
#define MAX_SHORTCUT_NAME 32

/** The maximum number of keys that can be used in a shortcut. */
#define MAX_SHORTCUT_KEYS 3

/**
 * Finds a shortcut in the shortcut registry.
 *
 * @param keys The key combination to search for.
 * @param keys_size The number of keys in the keys array.
 * @param out_name The buffer where the shortcut name will be stored.
 * @param out_name_size The size of the `out_name` buffer.
 *
 * @return - `0` The shortcut was found.
 * @return - `-1` The shortcut could not be found.
 */
int find_shortcut(int *keys, int keys_size, char *out_name, int out_name_size);

/** Returns the configured terminal command. */
const char *get_terminal_command();
