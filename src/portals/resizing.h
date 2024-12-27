#pragma once
#include "../all.h"

// The size of the resize area in pixels.
#define PORTAL_RESIZE_AREA_SIZE 20

// The minimum time (in milliseconds) between portal size updates during
// resizing (approximately 60 FPS at 16ms).
#define PORTAL_RESIZE_THROTTLE_MS 16

#ifdef STATIC

/**
 * Starts resizing a portal.
 * 
 * @param portal The portal to resize.
 * @param mouse_root_x The x coordinate of the mouse, relative to the root window.
 * @param mouse_root_y The y coordinate of the mouse, relative to the root window.
 */
static void start_resizing_portal(Portal *portal, int mouse_root_x, int mouse_root_y);

/**
 * Updates the size of the portal that's currently being resized.
 * 
 * @param mouse_root_x The x coordinate of the mouse, relative to the root window.
 * @param mouse_root_y The y coordinate of the mouse, relative to the root window.
 * @param event_time The time of the event.
 */
static void update_resizing_portal(int mouse_root_x, int mouse_root_y, Time event_time);

/**
 * Stops resizing the portal that's currently being resized.
 */
static void stop_resizing_portal();

/**
 * Checks if the mouse is within the resize area, which in the case of our
 * window manager is the bottom-right corner of the portal.
 * 
 * @param portal The portal to check the resize area for.
 * @param mouse_rel_x The x coordinate of the mouse, relative to the portal.
 * @param mouse_rel_y The y coordinate of the mouse, relative to the portal.
 */
static bool is_portal_resize_area(Portal *portal, int mouse_rel_x, int mouse_rel_y);

#endif
