#pragma once
#include "../all.h"

// The minimum time (in milliseconds) between portal position updates during
// dragging (approximately 60 FPS at 16ms).
#define PORTAL_DRAG_THROTTLE_MS 16

#ifdef STATIC

/**
 * Starts dragging a portal.
 * 
 * @param portal The portal to drag.
 * @param mouse_root_x The x coordinate of the mouse, relative to the root window.
 * @param mouse_root_y The y coordinate of the mouse, relative to the root window.
 */
static void start_dragging_portal(Portal *portal, int mouse_root_x, int mouse_root_y);

/**
 * Updates the position of the portal that's currently being dragged.
 * 
 * @param mouse_root_x The x coordinate of the mouse, relative to the root window.
 * @param mouse_root_y The y coordinate of the mouse, relative to the root window.
 * @param event_time The time of the event.
 */
static void update_dragging_portal(int mouse_root_x, int mouse_root_y, Time event_time);

/**
 * Stops dragging the portal that's currently being dragged.
 */
static void stop_dragging_portal();

#endif
