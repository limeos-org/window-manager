#pragma once
#include "../all.h"

/** The maximum number of workspaces supported by the window manager. */
#define MAX_WORKSPACES 6

/** Returns the current workspace index. */
int get_current_workspace();

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
