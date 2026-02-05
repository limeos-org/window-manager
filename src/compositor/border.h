#pragma once
#include "../all.h"

/**
 * Draws borders for a portal.
 *
 * @param cr The Cairo context to draw on.
 * @param portal The portal to draw borders for.
 * @param pixmap The window pixmap to sample luminance from.
 */
void draw_framed_border(cairo_t *cr, Portal *portal, Pixmap pixmap);

/**
 * Draws a border for frameless windows.
 *
 * @param cr The Cairo context to draw on.
 * @param portal The portal to draw the border for.
 * @param pixmap The window pixmap to sample luminance from.
 */
void draw_frameless_border(cairo_t *cr, Portal *portal, Pixmap pixmap);
