#pragma once
#include "../all.h"

/**
 * Draws a drop shadow for a portal.
 *
 * Renders a multi-layer soft shadow effect behind the portal to
 * create depth and visual separation from the background.
 *
 * @param cr The Cairo context to draw on.
 * @param portal The portal to draw the shadow for.
 * @param layers The number of shadow layers to render.
 * @param spread The maximum spread of the outermost layer.
 * @param opacity The base opacity for the shadow.
 * @param corner_radius The corner radius of the portal.
 */
void draw_shadow(
    cairo_t *cr, Portal *portal, int layers,
    double spread, double opacity, double corner_radius
);
