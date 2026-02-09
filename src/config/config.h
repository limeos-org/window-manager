#pragma once
#include "../all.h"

/** Configuration key for the target framerate. */
#define CFG_KEY_FRAMERATE "framerate"
#define CFG_DEFAULT_FRAMERATE "60"

/** Configuration key for the window theme. */
#define CFG_KEY_THEME "theme"
#define CFG_DEFAULT_THEME "adaptive"

/** Configuration key for the gap between tiled portals. */
#define CFG_KEY_TILE_GAP "tile_gap"
#define CFG_DEFAULT_TILE_GAP "6"

/** Configuration key for the background mode. */
#define CFG_KEY_BACKGROUND_MODE "background_mode"
#define CFG_DEFAULT_BACKGROUND_MODE "solid"

/** Configuration key for the background color. */
#define CFG_KEY_BACKGROUND_COLOR "background_color"
#define CFG_DEFAULT_BACKGROUND_COLOR "0x1C1C1C"

/** Configuration key for the background image path. */
#define CFG_KEY_BACKGROUND_IMAGE_PATH "background_image_path"
#define CFG_DEFAULT_BACKGROUND_IMAGE_PATH "~/background.png"

/** Configuration key for the terminal shortcut. */
#define CFG_KEY_TERMINAL_SHORTCUT "terminal_shortcut"
#define CFG_DEFAULT_TERMINAL_SHORTCUT "super+t"

/** Configuration key for the terminal command. */
#define CFG_KEY_TERMINAL_COMMAND "terminal_command"
#define CFG_DEFAULT_TERMINAL_COMMAND "xterm"

/** Configuration key for the exit shortcut. */
#define CFG_KEY_EXIT_SHORTCUT "exit_shortcut"
#define CFG_DEFAULT_EXIT_SHORTCUT "super+shift+e"

/** Configuration key for the close shortcut. */
#define CFG_KEY_CLOSE_SHORTCUT "close_shortcut"
#define CFG_DEFAULT_CLOSE_SHORTCUT "super+shift+q"

/** Format string for workspace shortcut config keys (use with snprintf). */
#define CFG_KEY_WORKSPACE_SHORTCUT_FMT "go_to_workspace_%d_shortcut"

/** Format string for workspace shortcut defaults (use with snprintf). */
#define CFG_DEFAULT_WORKSPACE_SHORTCUT_FMT "super+%d"

/** Configuration key for the 1st workspace shortcut. */
#define CFG_KEY_WORKSPACE_1_SHORTCUT "go_to_workspace_1_shortcut"
#define CFG_DEFAULT_WORKSPACE_1_SHORTCUT "super+1"

/** Configuration key for the 2nd workspace shortcut. */
#define CFG_KEY_WORKSPACE_2_SHORTCUT "go_to_workspace_2_shortcut"
#define CFG_DEFAULT_WORKSPACE_2_SHORTCUT "super+2"

/** Configuration key for the 3rd workspace shortcut. */
#define CFG_KEY_WORKSPACE_3_SHORTCUT "go_to_workspace_3_shortcut"
#define CFG_DEFAULT_WORKSPACE_3_SHORTCUT "super+3"

/** Configuration key for the 4th workspace shortcut. */
#define CFG_KEY_WORKSPACE_4_SHORTCUT "go_to_workspace_4_shortcut"
#define CFG_DEFAULT_WORKSPACE_4_SHORTCUT "super+4"

/** Configuration key for the 5th workspace shortcut. */
#define CFG_KEY_WORKSPACE_5_SHORTCUT "go_to_workspace_5_shortcut"
#define CFG_DEFAULT_WORKSPACE_5_SHORTCUT "super+5"

/** Configuration key for the 6th workspace shortcut. */
#define CFG_KEY_WORKSPACE_6_SHORTCUT "go_to_workspace_6_shortcut"
#define CFG_DEFAULT_WORKSPACE_6_SHORTCUT "super+6"

/** Format string for move-to-workspace shortcut config keys (use with snprintf). */
#define CFG_KEY_MOVE_WORKSPACE_SHORTCUT_FMT "move_to_workspace_%d_shortcut"

/** Format string for move-to-workspace shortcut defaults (use with snprintf). */
#define CFG_DEFAULT_MOVE_WORKSPACE_SHORTCUT_FMT "super+shift+%d"

/** Configuration key for moving to the 1st workspace. */
#define CFG_KEY_MOVE_WORKSPACE_1_SHORTCUT "move_to_workspace_1_shortcut"
#define CFG_DEFAULT_MOVE_WORKSPACE_1_SHORTCUT "super+shift+1"

/** Configuration key for moving to the 2nd workspace. */
#define CFG_KEY_MOVE_WORKSPACE_2_SHORTCUT "move_to_workspace_2_shortcut"
#define CFG_DEFAULT_MOVE_WORKSPACE_2_SHORTCUT "super+shift+2"

/** Configuration key for moving to the 3rd workspace. */
#define CFG_KEY_MOVE_WORKSPACE_3_SHORTCUT "move_to_workspace_3_shortcut"
#define CFG_DEFAULT_MOVE_WORKSPACE_3_SHORTCUT "super+shift+3"

/** Configuration key for moving to the 4th workspace. */
#define CFG_KEY_MOVE_WORKSPACE_4_SHORTCUT "move_to_workspace_4_shortcut"
#define CFG_DEFAULT_MOVE_WORKSPACE_4_SHORTCUT "super+shift+4"

/** Configuration key for moving to the 5th workspace. */
#define CFG_KEY_MOVE_WORKSPACE_5_SHORTCUT "move_to_workspace_5_shortcut"
#define CFG_DEFAULT_MOVE_WORKSPACE_5_SHORTCUT "super+shift+5"

/** Configuration key for moving to the 6th workspace. */
#define CFG_KEY_MOVE_WORKSPACE_6_SHORTCUT "move_to_workspace_6_shortcut"
#define CFG_DEFAULT_MOVE_WORKSPACE_6_SHORTCUT "super+shift+6"

/** Configuration key for the arrange shortcut. */
#define CFG_KEY_ARRANGE_SHORTCUT "arrange_shortcut"
#define CFG_DEFAULT_ARRANGE_SHORTCUT "super+a"
