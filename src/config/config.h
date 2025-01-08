#pragma once
#include "../all.h"

// The maximum length of a configuration file line.
#define CFG_MAX_LINE_LENGTH 256

// The maximum length of a configuration key.
#define CFG_MAX_KEY_LENGTH 64

// The maximum length of a configuration value.
#define CFG_MAX_VALUE_LENGTH 64

// The maximum number of configuration entries that can be loaded.
#define CFG_MAX_ENTRIES 64

// Macros for retrieving configuration values.
#define GET_CONFIG_IMPL(dest, dest_size, type, key, fallback) \
    get_config_value_##type(dest, dest_size, key, fallback)
#define GET_CONFIG(dest, dest_size, bundle) \
    GET_CONFIG_IMPL(dest, dest_size, bundle)

// Configuration field constants (throttle_ms).
#define CFG_TYPE_THROTTLE_MS int
#define CFG_KEY_THROTTLE_MS "throttle_ms"
#define CFG_DEFAULT_THROTTLE_MS "16"
#define CFG_BUNDLE_THROTTLE_MS \
        CFG_TYPE_THROTTLE_MS, \
        CFG_KEY_THROTTLE_MS, \
        CFG_DEFAULT_THROTTLE_MS

// Configuration field constants (terminal_shortcut).
#define CFG_TYPE_TERMINAL_SHORTCUT str
#define CFG_KEY_TERMINAL_SHORTCUT "terminal_shortcut"
#define CFG_DEFAULT_TERMINAL_SHORTCUT "super+enter"
#define CFG_BUNDLE_TERMINAL_SHORTCUT \
        CFG_TYPE_TERMINAL_SHORTCUT, \
        CFG_KEY_TERMINAL_SHORTCUT, \
        CFG_DEFAULT_TERMINAL_SHORTCUT

// Configuration field constants (terminal_command).
#define CFG_TYPE_TERMINAL_COMMAND str
#define CFG_KEY_TERMINAL_COMMAND "terminal_command"
#define CFG_DEFAULT_TERMINAL_COMMAND "xterm"
#define CFG_BUNDLE_TERMINAL_COMMAND \
        CFG_TYPE_TERMINAL_COMMAND, \
        CFG_KEY_TERMINAL_COMMAND, \
        CFG_DEFAULT_TERMINAL_COMMAND

// Configuration field constants (exit_shortcut).
#define CFG_TYPE_EXIT_SHORTCUT str
#define CFG_KEY_EXIT_SHORTCUT "exit_shortcut"
#define CFG_DEFAULT_EXIT_SHORTCUT "super+escape"
#define CFG_BUNDLE_EXIT_SHORTCUT \
        CFG_TYPE_EXIT_SHORTCUT, \
        CFG_KEY_EXIT_SHORTCUT, \
        CFG_DEFAULT_EXIT_SHORTCUT

// Configuration field constants (background_mode).
#define CFG_TYPE_BACKGROUND_MODE str
#define CFG_KEY_BACKGROUND_MODE "background_mode"
#define CFG_DEFAULT_BACKGROUND_MODE "solid"
#define CFG_BUNDLE_BACKGROUND_MODE \
        CFG_TYPE_BACKGROUND_MODE, \
        CFG_KEY_BACKGROUND_MODE, \
        CFG_DEFAULT_BACKGROUND_MODE

// Configuration field constants (background_color).
#define CFG_TYPE_BACKGROUND_COLOR hex
#define CFG_KEY_BACKGROUND_COLOR "background_color"
#define CFG_DEFAULT_BACKGROUND_COLOR "0x1C1C1C"
#define CFG_BUNDLE_BACKGROUND_COLOR \
        CFG_TYPE_BACKGROUND_COLOR, \
        CFG_KEY_BACKGROUND_COLOR, \
        CFG_DEFAULT_BACKGROUND_COLOR

// Configuration field constants (background_image_path).
#define CFG_TYPE_BACKGROUND_IMAGE_PATH path
#define CFG_KEY_BACKGROUND_IMAGE_PATH "background_image_path"
#define CFG_DEFAULT_BACKGROUND_IMAGE_PATH "~/background.png"
#define CFG_BUNDLE_BACKGROUND_IMAGE_PATH \
        CFG_TYPE_BACKGROUND_IMAGE_PATH, \
        CFG_KEY_BACKGROUND_IMAGE_PATH, \
        CFG_DEFAULT_BACKGROUND_IMAGE_PATH

/**
 * Retrieves a configuration value from the loaded configuration entries.
 * Intended to be used for configuration values of type `str`.
 *
 * @param dest Buffer where the value will be stored.
 * @param dest_size Size of the destination buffer.
 * @param key Configuration key to look up.
 * @param fallback Default value to use if key is not found.
 *
 * @warning Don't use directly! Use the `GET_CONFIG` macro instead.
 */
void get_config_value_str(char *dest, size_t dest_size, char *key, char *fallback);

/**
 * Retrieves a configuration value from the loaded configuration entries.
 * Intended to be used for configuration values of type `path`.
 *
 * @param dest Buffer where the value will be stored.
 * @param dest_size Size of the destination buffer.
 * @param key Configuration key to look up.
 * @param fallback Default value to use if key is not found.
 *
 * @warning Don't use directly! Use the `GET_CONFIG` macro instead.
 */
void get_config_value_path(char *dest, size_t dest_size, char *key, char *fallback);

/**
 * Retrieves a configuration value from the loaded configuration entries.
 * Intended to be used for configuration values of type `hex`.
 *
 * @param dest Buffer where the value will be stored.
 * @param dest_size Size of the destination buffer, may be `NULL`.
 * @param key Configuration key to look up.
 * @param fallback Default value to use if key is not found.
 *
 * @warning Don't use directly! Use the `GET_CONFIG` macro instead.
 */
void get_config_value_hex(unsigned long *dest, size_t dest_size, char *key, char *fallback);

/**
 * Retrieves a configuration value from the loaded configuration entries.
 * Intended to be used for configuration values of type `int`.
 *
 * @param dest Buffer where the value will be stored.
 * @param dest_size Size of the destination buffer, may be `NULL`.
 * @param key Configuration key to look up.
 * @param fallback Default value to use if key is not found.
 *
 * @warning Don't use directly! Use the `GET_CONFIG` macro instead.
 */
void get_config_value_int(int *dest, size_t dest_size, char *key, char *fallback);
