#pragma once
#include "../all.h"

/**
 * Draws the portal title text within the title bar area.
 *
 * @param portal The portal to draw the title for.
 *
 * @note - Intended to be used by `draw_portal_frame()`.
 * @note - Direct invocation without redrawing other elements causes text
 * overlap.
 */
void draw_portal_title(Portal *portal);
