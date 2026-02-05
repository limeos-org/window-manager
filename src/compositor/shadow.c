/**
 * This code is responsible for portal shadow drawing.
 *
 * It handles rendering drop shadows for portals, creating a multi-layer soft
 * shadow effect for visual depth.
 */

#include "../all.h"

void draw_shadow(
    cairo_t *cr, Portal *portal, int layers,
    double spread, double opacity, double corner_radius
) {
    // Draw each shadow layer from outermost to innermost.
    for (int layer = layers; layer > 0; layer--)
    {
        // Calculate the layer-specific values.
        double factor = (double)layer / layers;
        double layer_spread = spread * factor;
        double layer_opacity = (opacity / layers) * (1.0 - factor * 0.5);

        // Draw the shadow layer.
        cairo_set_source_rgba(cr, 0, 0, 0, layer_opacity);
        cairo_rounded_rectangle(cr,
            portal->geometry.x_root - layer_spread / 2,
            portal->geometry.y_root - layer_spread / 2,
            portal->geometry.width + layer_spread,
            portal->geometry.height + layer_spread,
            corner_radius + layer_spread / 2
        );
        cairo_fill(cr);
    }
}
