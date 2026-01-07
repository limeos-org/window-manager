#pragma once
#include "../all.h"

/**
 * The left padding for the title text in pixels.
 */
#define PORTAL_TITLE_PADDING_LEFT 8

/**
 * The right padding for the title text in pixels.
 * This should account for the trigger buttons.
 */
#define PORTAL_TITLE_PADDING_RIGHT 50

/**
 * Draws the portal title text within the title bar.
 *
 * @param portal The portal to draw the title for.
 *
 * @note Intended to be called from `draw_portal_frame()`.
 */
void draw_portal_title(Portal *portal);

/**
 * Updates the portal title from the client window's name property.
 *
 * @param portal The portal to update the title for.
 */
void update_portal_title(Portal *portal);
