#pragma once
#include "../all.h"

/**
 * Creates a frame window and its Cairo context.
 * 
 * @param portal The portal to create the frame window for. It is assumed that
 * the portal struct is already populated with the necessary information
 * (Such as `x`, `y`, `width` and `height` etc.) to create the frame.
 * @param out_window The output parameter to store the created frame window.
 * @param out_cr The output parameter to store the Cairo context for the frame.
 */
void create_portal_frame(Portal *portal, Window *out_window, cairo_t **out_cr);

/**
 * Draws all frame decorations for the portal. This includes the title bar,
 * title text and buttons (E.g. close, arrange).
 * 
 * @param portal The portal to draw the frame decorations for.
 */
void draw_portal_frame(Portal *portal);

/**
 * Checks if the provided coordinates are within the frame area of the portal.
 * 
 * @param portal The portal to check the frame area for.
 * @param rel_x The x coordinate, relative to the portal.
 * @param rel_y The y coordinate, relative to the portal.
 * 
 * @return - `True (1)` The coordinates are within the frame area.
 * @return - `False (0)` The coordinates are not within the frame area.
 */
bool is_portal_frame_area(Portal *portal, int rel_x, int rel_y);

/**
 * Checks if the portal frame is valid.
 * 
 * @param portal The portal containing the frame window.
 * 
 * @return - `True (1)` The portal frame is valid.
 * @return - `False (0)` The portal frame is invalid.
 */
bool is_portal_frame_valid(Portal *portal);

/**
 * Destroys a frame window and performs neccessary cleanups.
 * 
 * @param portal The portal containing the frame window.
 * 
 * @return - `0` The frame window was destroyed successfully.
 * @return - `-1` The portal frame is invalid.
 * @return - `-2` Internal `XDestroyWindow()` call failed.
 * 
 * @note - Can safely be called without checking if the frame is valid first.
 */
int destroy_portal_frame(Portal *portal);
