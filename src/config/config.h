#pragma once
#include "../all.h"

#define CFG_DIRECTORY "~/.config/lime-os-wm"
#define CFG_FILE_PATH "~/.config/lime-os-wm/config"

#define CFG_MAX_LINE_LENGTH 256
#define CFG_MAX_KEY_LENGTH 64
#define CFG_MAX_VALUE_LENGTH 64
#define CFG_MAX_ENTRIES 64

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

// Key-value pair struct for storing configuration entries.
typedef struct {
    char key[CFG_MAX_KEY_LENGTH];
    char value[CFG_MAX_VALUE_LENGTH];
} ConfigEntry;

#ifdef STATIC

/**
 * Creates the configuration directory and all its parent directories if they
 * don't exist.
 *
 * @param path Full path of the configuration directory to create.
 */
static void create_config_directory(const char *path);

/**
 * Creates a new configuration file with default settings.
 *
 * @param path Path where the configuration file should be created.
 */
static void create_config_file(const char *path);

/**
 * Parses a configuration file and stores the key-value pairs in the
 * configuration entries array.
 *
 * @param path Path to the configuration file.
 */
static void parse_config_file(const char *path);

#endif

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
 * @param dest_size Size of the destination buffer.
 * @param key Configuration key to look up.
 * @param fallback Default value to use if key is not found.
 *
 * @warning Don't use directly! Use the `GET_CONFIG` macro instead.
 */
void get_config_value_hex(unsigned long *dest, size_t dest_size, char *key, char *fallback);

#define GET_CONFIG_IMPL(dest, dest_size, type, key, fallback) \
    get_config_value_##type(dest, dest_size, key, fallback)

#define GET_CONFIG(dest, dest_size, bundle) \
    GET_CONFIG_IMPL(dest, dest_size, bundle)
