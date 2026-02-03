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

/**
 * Starts resizing a portal.
 *
 * @param portal The portal to resize.
 * @param mouse_root_x The initial mouse X position relative to root.
 * @param mouse_root_y The initial mouse Y position relative to root.
 */
void start_resizing_portal(Portal *portal, int mouse_root_x, int mouse_root_y);

/**
 * Updates the portal size during resizing.
 *
 * @param mouse_root_x The current mouse X position relative to root.
 * @param mouse_root_y The current mouse Y position relative to root.
 * @param event_time The event timestamp for throttling.
 */
void update_resizing_portal(int mouse_root_x, int mouse_root_y, Time event_time);

/**
 * Stops resizing the current portal.
 */
void stop_resizing_portal(void);
