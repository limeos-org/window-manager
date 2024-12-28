#pragma once
#include "../all.h"

/**
 * Draws the portal title text within the title bar area.
 * 
 * @param portal The portal to draw the title for.
 * 
 * @note Intended to be used by `draw_portal_frame()`. Invoking this function
 * directly, without also redrawing other elements, will simply cause the new 
 * title text to overlap the old title text.
 */
void draw_portal_title(Portal *portal);
