#pragma once
#include "../all.h"

/**
 * Checks if a portal is currently being dragged.
 *
 * @return - `true` A portal is being dragged.
 * @return - `false` No portal is being dragged.
 */
bool is_portal_dragging();

/**
 * Starts dragging a portal.
 *
 * @param portal The portal to drag.
 * @param mouse_root_x The initial mouse X position relative to root.
 * @param mouse_root_y The initial mouse Y position relative to root.
 */
void start_dragging_portal(Portal *portal, int mouse_root_x, int mouse_root_y);

/**
 * Updates the portal position during dragging.
 *
 * @param mouse_root_x The current mouse X position relative to root.
 * @param mouse_root_y The current mouse Y position relative to root.
 * @param event_time The event timestamp for throttling.
 */
void update_dragging_portal(int mouse_root_x, int mouse_root_y, Time event_time);

/**
 * Stops dragging the current portal.
 */
void stop_dragging_portal(void);
