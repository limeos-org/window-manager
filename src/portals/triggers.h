#pragma once
#include "../all.h"

/**
 * Draws all portal triggers (E.g. close, arrange).
 *
 * @param portal The portal to draw the triggers for.
 *
 * @note Intended to be used by `draw_portal_frame()`.
 */
void draw_portal_triggers(Portal *portal);

/**
 * Checks if the given coordinates are within any trigger area.
 *
 * @param portal The portal to check.
 * @param rel_x X coordinate relative to the portal.
 * @param rel_y Y coordinate relative to the portal.
 *
 * @return True if the coordinates are within a trigger area.
 */
bool is_portal_triggers_area(Portal *portal, int rel_x, int rel_y);
