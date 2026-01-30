/**
 * This code is responsible for portal shadow drawing.
 *
 * It handles rendering drop shadows for portals, creating a multi-layer soft
 * shadow effect for visual depth.
 */

#include "../all.h"

void draw_portal_shadow(cairo_t *cr, Portal *portal)
{
    // Define the shadow parameters.
    int shadow_layers = 4;
    double shadow_offset_x = 0;
    double shadow_offset_y = 0;
    double shadow_spread = 20;
    double shadow_opacity = 0.1;

    // Draw each shadow layer from outermost to innermost.
    for (int layer = shadow_layers; layer > 0; layer--)
    {
        // Calculate the layer-specific values.
        double factor = (double)layer / shadow_layers;
        double offset_x = shadow_offset_x * factor;
        double offset_y = shadow_offset_y * factor;
        double spread = shadow_spread * factor;
        double opacity = (shadow_opacity / shadow_layers) * (1.0 - factor * 0.5);

        // Draw the shadow layer.
        cairo_set_source_rgba(cr, 0, 0, 0, opacity);
        cairo_rounded_rectangle(cr,
            portal->geometry.x_root + offset_x - spread / 2,
            portal->geometry.y_root + offset_y - spread / 2,
            portal->geometry.width + spread,
            portal->geometry.height + spread,
            PORTAL_CORNER_RADIUS + spread / 2);
        cairo_fill(cr);
    }
}
