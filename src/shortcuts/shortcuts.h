#pragma once
#include "../all.h"

/**
 * Finds a shortcut in the shortcut registry.
 *
 * @param keys The key combination to search for.
 * @param keys_size The number of keys in the keys array.
 * @param out_name The buffer where the shortcut name will be stored.
 * @param name_size The size of the `out_name` buffer.
 *
 * @return - `0` The shortcut was found.
 * @return - `-1` The shortcut could not be found.
 */
int find_shortcut(int *keys, int keys_size, char *out_name, int name_size);
