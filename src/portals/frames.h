#pragma once
#include "../all.h"

/**
 * TODO: Document this function.
 */
bool should_portal_be_framed(Portal *portal);

/**
 * TODO: Document this function.
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
