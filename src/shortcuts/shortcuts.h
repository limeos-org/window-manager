#pragma once
#include "../all.h"

/**
 * The maximum number of shortcuts that can be registered.
 */
#define SHORTCUT_MAX_COUNT 32

/**
 * The maximum length of a shortcut name.
 */
#define SHORTCUT_MAX_NAME_LENGTH 64

/**
 * A type representing a registered keyboard shortcut.
 */
typedef struct {
    char name[SHORTCUT_MAX_NAME_LENGTH];
    unsigned int modifiers;
    KeySym keysym;
} Shortcut;

/**
 * Parses a shortcut string into a modifier mask and keysym.
 *
 * The shortcut string format is "modifier+modifier+key", where modifiers
 * can be: super, shift, ctrl, control, alt.
 *
 * @param shortcut_string The shortcut string to parse (e.g., "super+enter").
 * @param out_modifiers Pointer to store the resulting modifier mask.
 * @param out_keysym Pointer to store the resulting keysym.
 *
 * @return - `0` The shortcut was parsed successfully.
 * @return - `-1` The shortcut string could not be parsed.
 */
int parse_shortcut(const char *shortcut_string, unsigned int *out_modifiers, KeySym *out_keysym);

/**
 * Registers a named shortcut for detection.
 *
 * @param name The name identifier for the shortcut (e.g., "terminal").
 * @param shortcut_string The shortcut string (e.g., "super+enter").
 *
 * @return - `0` The shortcut was registered successfully.
 * @return - `-1` Registration failed (invalid string or max shortcuts reached).
 */
int register_shortcut(const char *name, const char *shortcut_string);

/**
 * Checks if a key event matches any registered shortcut.
 *
 * @param modifiers The current modifier mask.
 * @param keysym The keysym of the pressed key.
 *
 * @return A pointer to the matching Shortcut, or NULL if no match.
 */
Shortcut *find_matching_shortcut(unsigned int modifiers, KeySym keysym);
