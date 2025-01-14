#pragma once
#include "../all.h"

/**
 * Checks if a portal is currently being resized.
 *
 * @return - `true` A portal is being resized.
 * @return - `false` No portal is being resized.
 */
bool is_portal_resizing();

/**
 * Checks if a position is within the resize area of a portal.
 *
 * @param portal The portal to check.
 * @param rel_x The X coordinate relative to the portal.
 * @param rel_y The Y coordinate relative to the portal.
 *
 * @return - `true` The position is within the resize area.
 * @return - `false` The position is not within the resize area.
 */
bool is_portal_resize_area(Portal *portal, int rel_x, int rel_y);
