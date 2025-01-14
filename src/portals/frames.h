#pragma once
#include "../all.h"

/**
 * Determines whether a portal should have a decorative frame.
 *
 * Checks ICCCM top-level status, Motif decoration hints, and EWMH window type
 * to decide if the portal should be framed.
 *
 * @param portal The portal to check.
 *
 * @return - `true` The portal should be framed.
 * @return - `false` The portal should not be framed.
 *
 * @note The portal's `top_level` and `client_window_type` fields must be populated
 * before calling this function.
 */
bool should_portal_be_framed(Portal *portal);

/**
 * Creates a decorative frame window for a portal.
 *
 * Allocates a frame window, reparents the client window into it, sets up Cairo
 * rendering context, and configures EWMH frame extents.
 *
 * @param portal The portal to create the frame for.
 *
 * @note The portal's `frame_window` and `frame_cr` fields will be populated.
 */
void create_portal_frame(Portal *portal);

/**
 * Draws all frame decorations for the portal. This includes the title bar,
 * title text and buttons (E.g. close, arrange).
 * 
 * @param portal The portal to draw the frame decorations for.
 */
void draw_portal_frame(Portal *portal);

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
 * @return - `-1` Internal `XDestroyWindow()` call failed.
 *
 * @warning - Ensure the portal frame is valid before calling this function.
 */
int destroy_portal_frame(Portal *portal);

/**
 * Checks if a position is within the frame area (title bar) of a portal.
 *
 * @param portal The portal to check.
 * @param rel_x The X coordinate relative to the portal.
 * @param rel_y The Y coordinate relative to the portal.
 *
 * @return - `true` The position is within the frame area.
 * @return - `false` The position is not within the frame area.
 */
bool is_portal_frame_area(Portal *portal, int rel_x, int rel_y);
