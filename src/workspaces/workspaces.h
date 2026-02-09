#pragma once
#include "../all.h"

/** The maximum number of workspaces supported by the window manager. */
#define MAX_WORKSPACES 6

/** The maximum number of non-transient portals per workspace. */
#define MAX_WORKSPACE_PORTALS 8

/** The viewport percentage used for auto-tiling thresholds. */
#define WORKSPACE_VIEWPORT_THRESHOLD_PERCENT 75

/** The diagonal pixel offset between cascaded portals. */
#define WORKSPACE_CASCADE_OFFSET_PX 46

/** A type representing the layout mode of a workspace. */
typedef enum
{
    WORKSPACE_LAYOUT_FLOATING,
    WORKSPACE_LAYOUT_TILING
} WorkspaceLayoutMode;

/** Returns the current workspace index. */
int get_current_workspace();

/**
 * Checks whether a portal is currently tiled.
 *
 * @param portal The portal to check.
 *
 * @return True if the portal is non-transient and on a workspace in tiling
 * mode.
 */
bool is_portal_tiled(Portal *portal);

/**
 * Queries the layout mode of a workspace.
 *
 * @param workspace The workspace index (0 to MAX_WORKSPACES - 1).
 *
 * @return The layout mode of the workspace.
 */
WorkspaceLayoutMode get_workspace_layout_mode(int workspace);

/** Toggles the layout mode of the current workspace. */
void toggle_workspace_layout_mode();

/**
 * Recomputes and applies the tiling layout for the specified workspace.
 *
 * @param workspace The workspace index (0 to MAX_WORKSPACES - 1).
 */
void arrange_workspace_portals(int workspace);

/**
 * Determines what the target workspace for a portal should be.
 *
 * @param portal The portal to determine the workspace for.
 *
 * @return The portal's assigned workspace if already set, otherwise the
 * current workspace.
 */
int determine_portal_workspace(Portal *portal);

/**
 * Counts the number of non-transient, non-override-redirect, non-hidden
 * portals on the specified workspace.
 *
 * @param workspace The workspace index (0 to MAX_WORKSPACES - 1).
 *
 * @return The portal count on the workspace.
 */
int count_workspace_portals(int workspace);

/**
 * Switches to the specified workspace.
 *
 * @param workspace The target workspace index (0 to MAX_WORKSPACES - 1).
 *
 * @note Does nothing if the workspace index is invalid or already active.
 */
void switch_workspace(int workspace);

/**
 * Moves the specified portal to a workspace.
 *
 * @param portal The portal to move.
 * @param workspace The target workspace index (0 to MAX_WORKSPACES - 1).
 *
 * @note Does nothing if the portal is NULL, the workspace index is invalid,
 * or the portal is already on the target workspace.
 */
void move_portal_to_workspace(Portal *portal, int workspace);
