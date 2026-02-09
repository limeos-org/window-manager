#pragma once
#include "../all.h"

/** Checks whether a portal is eligible for tiling. */
bool is_tiling_eligible(Portal *portal);

/**
 * Appends a portal to the end of a workspace's tile order.
 *
 * @param portal The portal to append.
 * @param workspace The workspace index (0 to MAX_WORKSPACES - 1).
 *
 * @note Does nothing if the portal is already in the tile order.
 */
void append_to_tile_order(Portal *portal, int workspace);

/**
 * Removes a portal from a workspace's tile order.
 *
 * @param portal The portal to remove.
 * @param workspace The workspace index (0 to MAX_WORKSPACES - 1).
 */
void remove_from_tile_order(Portal *portal, int workspace);

/** Returns the number of portals in the tile order for a workspace. */
int get_tile_order_count(int workspace);

/** Clears the tile order for a workspace. */
void clear_tile_order(int workspace);

/**
 * Computes and applies tile geometries to all portals in the tile order.
 *
 * Divides the viewport into slots based on the number of portals and the
 * configured tile gap, then moves and resizes each portal to fill its slot.
 *
 * @param workspace The workspace index (0 to MAX_WORKSPACES - 1).
 */
void apply_tiling_layout(int workspace);

/** Returns whether a tiling layout is currently being applied. */
bool is_applying_tiling_layout();

/**
 * Arranges portals in a diagonal cascade for the tiling-to-floating
 * transition.
 *
 * Sizes each portal to the median of their last floating dimensions,
 * capped at WORKSPACE_VIEWPORT_THRESHOLD_PERCENT of the viewport, and
 * offsets each by WORKSPACE_CASCADE_OFFSET_PX pixels diagonally.
 *
 * @param workspace The workspace index (0 to MAX_WORKSPACES - 1).
 */
void cascade_tiled_portals(int workspace);
