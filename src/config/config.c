#include "../all.h"

#define CFG_FILE_PATH "~/.config/limeos-window-manager"

// clang-format off
static const char default_config[] =
    "# ---\n"
    "# General\n"
    "# --- \n"
    "\n"
    "# Redraw frequency of the screen (times per second).\n"
    "# Lowering this value can significantly enhance performance.\n"
    CFG_KEY_FRAMERATE "=" CFG_DEFAULT_FRAMERATE "\n"
    "\n"
    "# The theme of the window manager.\n"
    "# May be 'adaptive', 'light' or 'dark'.\n"
    "# 'adaptive' picks a light or dark titlebar per window by\n"
    "# sampling the client content luminance near the titlebar.\n"
    CFG_KEY_THEME "=" CFG_DEFAULT_THEME "\n"
    "\n"
    "# The gap between tiled portals in pixels.\n"
    CFG_KEY_TILE_GAP "=" CFG_DEFAULT_TILE_GAP "\n"
    "\n"
    "# ---\n"
    "# Background\n"
    "# --- \n"
    "\n"
    "# The rendering mode of the background.\n"
    "# May either be 'solid' or 'image'.\n"
    CFG_KEY_BACKGROUND_MODE "=" CFG_DEFAULT_BACKGROUND_MODE "\n"
    "\n"
    "# A hexadecimal representation of a color.\n"
    "# Used when " CFG_KEY_BACKGROUND_MODE " is set to 'solid'.\n"
    CFG_KEY_BACKGROUND_COLOR "=" CFG_DEFAULT_BACKGROUND_COLOR "\n"
    "\n"
    "# A file path to a PNG background image.\n"
    "# Other file formats such as JPG are not supported.\n"
    "# Used when " CFG_KEY_BACKGROUND_MODE " is set to 'image'.\n"
    CFG_KEY_BACKGROUND_IMAGE_PATH "=" CFG_DEFAULT_BACKGROUND_IMAGE_PATH "\n"
    "\n"
    "# ---\n"
    "# Shortcuts\n"
    "# --- \n"
    "\n"
    "# The shortcut used to open a terminal window.\n"
    CFG_KEY_TERMINAL_SHORTCUT "=" CFG_DEFAULT_TERMINAL_SHORTCUT "\n"
    CFG_KEY_TERMINAL_COMMAND "=" CFG_DEFAULT_TERMINAL_COMMAND "\n"
    "\n"
    "# The shortcut used to exit the window manager.\n"
    CFG_KEY_EXIT_SHORTCUT "=" CFG_DEFAULT_EXIT_SHORTCUT "\n"
    "\n"
    "# The shortcut used to close the focused window.\n"
    CFG_KEY_CLOSE_SHORTCUT "=" CFG_DEFAULT_CLOSE_SHORTCUT "\n"
    "\n"
    "# The shortcuts used to move between workspaces.\n"
    CFG_KEY_WORKSPACE_1_SHORTCUT "=" CFG_DEFAULT_WORKSPACE_1_SHORTCUT "\n"
    CFG_KEY_WORKSPACE_2_SHORTCUT "=" CFG_DEFAULT_WORKSPACE_2_SHORTCUT "\n"
    CFG_KEY_WORKSPACE_3_SHORTCUT "=" CFG_DEFAULT_WORKSPACE_3_SHORTCUT "\n"
    CFG_KEY_WORKSPACE_4_SHORTCUT "=" CFG_DEFAULT_WORKSPACE_4_SHORTCUT "\n"
    CFG_KEY_WORKSPACE_5_SHORTCUT "=" CFG_DEFAULT_WORKSPACE_5_SHORTCUT "\n"
    CFG_KEY_WORKSPACE_6_SHORTCUT "=" CFG_DEFAULT_WORKSPACE_6_SHORTCUT "\n"
    "\n"
    "# The shortcuts used to move windows between workspaces.\n"
    CFG_KEY_MOVE_WORKSPACE_1_SHORTCUT "=" CFG_DEFAULT_MOVE_WORKSPACE_1_SHORTCUT "\n"
    CFG_KEY_MOVE_WORKSPACE_2_SHORTCUT "=" CFG_DEFAULT_MOVE_WORKSPACE_2_SHORTCUT "\n"
    CFG_KEY_MOVE_WORKSPACE_3_SHORTCUT "=" CFG_DEFAULT_MOVE_WORKSPACE_3_SHORTCUT "\n"
    CFG_KEY_MOVE_WORKSPACE_4_SHORTCUT "=" CFG_DEFAULT_MOVE_WORKSPACE_4_SHORTCUT "\n"
    CFG_KEY_MOVE_WORKSPACE_5_SHORTCUT "=" CFG_DEFAULT_MOVE_WORKSPACE_5_SHORTCUT "\n"
    CFG_KEY_MOVE_WORKSPACE_6_SHORTCUT "=" CFG_DEFAULT_MOVE_WORKSPACE_6_SHORTCUT "\n"
    "\n"
    "# The shortcut used to switch between floating and tiling.\n"
    CFG_KEY_ARRANGE_SHORTCUT "=" CFG_DEFAULT_ARRANGE_SHORTCUT "\n";
// clang-format on

HANDLE(Prepare)
{
    // Expand the configuration file path.
    char config_file_path[COMMON_MAX_PATH_LENGTH];
    if (common.expand_path(CFG_FILE_PATH, config_file_path, sizeof(config_file_path)) != 0)
    {
        LOG_WARNING("Failed to expand configuration file path (%s).", CFG_FILE_PATH);
        return;
    }

    // Initialize the configuration system, creating the default config file
    // if it doesn't exist.
    if (common.init_config_with_defaults(config_file_path, default_config) != 0)
    {
        LOG_WARNING("Failed to initialize configuration system.");
    }
}
