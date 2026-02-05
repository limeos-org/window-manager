/**
 * This code is responsible for portal border drawing, where the border color 
 * adapts per-pixel based on the luminance of adjacent content.
 */

#include "../all.h"

/**
 * Reads luminance from an already-fetched XImage at (x, y).
 *
 * Uses BT.601 coefficients: 0.299*R + 0.587*G + 0.114*B.
 *
 * @param image The XImage to read from.
 * @param x The x coordinate within the image.
 * @param y The y coordinate within the image.
 *
 * @return Luminance value from 0.0 (dark) to 1.0 (light).
 */
static float pixel_luminance(XImage *image, int x, int y)
{
    // Retrieve the pixel value at the given coordinates.
    unsigned long pixel = XGetPixel(image, x, y);

    // Extract RGB components and compute luminance.
    unsigned char r = (pixel >> 16) & 0xFF;
    unsigned char g = (pixel >> 8) & 0xFF;
    unsigned char b = pixel & 0xFF;
    return (float)(0.299 * r + 0.587 * g + 0.114 * b) / 255.0f;
}

/**
 * Draws a straight border line with per-pixel adaptive coloring.
 *
 * Walks the strip pixel by pixel, groups consecutive same-color
 * (black/white, threshold at 0.5 luminance) pixels into runs,
 * and draws each run as a single cairo line segment.
 *
 * @param cr Cairo context to draw on.
 * @param strip XImage containing the 1-pixel-wide edge strip.
 * @param length Number of pixels in the strip.
 * @param vertical Whether the line runs vertically.
 * @param start_x Starting x coordinate in cairo space.
 * @param start_y Starting y coordinate in cairo space.
 * @param alpha Border alpha value.
 */
static void draw_adaptive_line(
    cairo_t *cr, XImage *strip, int length, bool vertical,
    double start_x, double start_y, double alpha
) {
    // Skip empty strips.
    if (length <= 0)
    {
        return;
    }

    // Derive per-pixel increments from the axis direction.
    double dx = vertical ? 0.0 : 1.0;
    double dy = vertical ? 1.0 : 0.0;

    // Initialize run tracking with the first pixel's luminance.
    int run_start = 0;
    float luminance = pixel_luminance(strip, 0, 0);
    bool run_dark = (luminance > 0.5f);

    // Walk the strip and flush runs on color transitions.
    for (int i = 1; i <= length; i++)
    {
        // Default to the current run color so the final iteration
        // (i == length) flushes without a false color change.
        bool current_dark = run_dark;
        if (i < length)
        {
            int strip_x = vertical ? 0 : i;
            int strip_y = vertical ? i : 0;
            float current_luminance = pixel_luminance(strip, strip_x, strip_y);
            current_dark = (current_luminance > 0.5f);
        }

        // Flush the current run on color change or at the end.
        if (current_dark != run_dark || i == length)
        {
            double run_color = run_dark ? 0.0 : 1.0;
            cairo_set_source_rgba(cr, run_color, run_color, run_color, alpha);
            double x0 = start_x + run_start * dx;
            double y0 = start_y + run_start * dy;
            double x1 = start_x + i * dx;
            double y1 = start_y + i * dy;
            cairo_move_to(cr, x0, y0);
            cairo_line_to(cr, x1, y1);
            cairo_stroke(cr);
            run_start = i;
            run_dark = current_dark;
        }
    }
}

/**
 * Returns the border color for a given pixel in a strip image.
 *
 * @param strip The strip image to sample from.
 * @param index The pixel index within the strip.
 * @param vertical Whether the strip is oriented vertically.
 *
 * @return - `0.0` - Black, for light content.
 * @return - `1.0` - White, for dark content.
 */
static double border_color_at(XImage *strip, int index, bool vertical)
{
    // Compute the strip coordinates for the given index.
    int strip_x = vertical ? 0 : index;
    int strip_y = vertical ? index : 0;

    // Sample luminance and return the contrasting border color.
    float luminance = pixel_luminance(strip, strip_x, strip_y);
    return (luminance > 0.5f) ? 0.0 : 1.0;
}

/**
 * Resolves the arc color from the nearest strip endpoint.
 *
 * Tries the primary strip first; falls back to the fallback strip.
 * Returns 1.0 (white) if neither strip is available.
 *
 * @param primary Primary strip image to sample from.
 * @param primary_length Length of the primary strip.
 * @param primary_index Pixel index to sample in the primary strip.
 * @param primary_vertical Whether the primary strip is vertical.
 * @param fallback Fallback strip image if primary is unavailable.
 * @param fallback_length Length of the fallback strip.
 * @param fallback_index Pixel index to sample in the fallback strip.
 * @param fallback_vertical Whether the fallback strip is vertical.
 *
 * @return Border color: 0.0 (black) or 1.0 (white).
 */
static double resolve_arc_color(
    XImage *primary, int primary_length,
    int primary_index, bool primary_vertical,
    XImage *fallback, int fallback_length,
    int fallback_index, bool fallback_vertical)
{
    if (primary && primary_length > 0)
    {
        return border_color_at(primary, primary_index, primary_vertical);
    }
    if (fallback && fallback_length > 0)
    {
        return border_color_at(
            fallback, fallback_index, fallback_vertical
        );
    }
    return 1.0;
}

void draw_framed_border(cairo_t *cr, Portal *portal, Pixmap pixmap)
{
    // Retrieve display, theme, and geometry values.
    Display *display = DefaultDisplay;
    const Theme *theme = get_current_theme();
    double x = portal->geometry.x_root;
    double y = portal->geometry.y_root;
    double width = portal->geometry.width;
    double height = portal->geometry.height;
    double radius = PORTAL_CORNER_RADIUS;
    double title_height = PORTAL_TITLE_BAR_HEIGHT;

    // Draw inner border around title bar using the theme color.
    cairo_set_source_rgba(cr,
        theme->titlebar_border.r,
        theme->titlebar_border.g,
        theme->titlebar_border.b,
        theme->titlebar_border.a
    );
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, x + 0.5, y + title_height);
    cairo_line_to(cr, x + 0.5, y + radius);
    cairo_arc(cr, x + radius, y + radius, radius - 0.5, PI, 3 * PI / 2);
    cairo_line_to(cr, x + width - radius, y + 0.5);
    cairo_arc(cr, x + width - radius, y + radius, radius - 0.5, -PI / 2, 0);
    cairo_line_to(cr, x + width - 0.5, y + title_height);
    cairo_stroke(cr);

    // Compute edge lengths for the adaptive border.
    cairo_set_line_width(cr, 1);
    double alpha = theme->titlebar_border.a;
    int edge_height = (int)(height - title_height - radius);
    int edge_width = (int)(width - 2 * radius);

    // Grab 1-pixel-wide strips from the pixmap along each edge.
    XImage *strip_left = NULL;
    XImage *strip_right = NULL;
    XImage *strip_bottom = NULL;
    if (edge_height > 0)
    {
        strip_left = XGetImage(
            display, pixmap,
            PORTAL_BORDER_WIDTH, (int)title_height,
            1, edge_height, AllPlanes, ZPixmap
        );
        strip_right = XGetImage(
            display, pixmap,
            (int)width - PORTAL_BORDER_WIDTH - 1,
            (int)title_height, 1, edge_height,
            AllPlanes, ZPixmap
        );
    }
    if (edge_width > 0)
    {
        strip_bottom = XGetImage(
            display, pixmap,
            (int)radius,
            (int)height - PORTAL_BORDER_WIDTH - 1,
            edge_width, 1, AllPlanes, ZPixmap
        );
    }

    // Declare arc color for reuse across corner arcs.
    double arc_color;

    // Draw left edge from top to bottom.
    if (strip_left && edge_height > 0)
    {
        draw_adaptive_line(
            cr, strip_left, edge_height,
            true, x + 0.5, y + title_height, alpha
        );
    }

    // Draw bottom-left arc with the nearest strip pixel color.
    arc_color = resolve_arc_color(
        strip_left, edge_height, edge_height - 1, true,
        strip_bottom, edge_width, 0, false
    );
    cairo_set_source_rgba(cr,
        arc_color, arc_color, arc_color, alpha
    );
    cairo_arc_negative(cr,
        x + radius, y + height - radius,
        radius - 0.5, PI, PI / 2
    );
    cairo_stroke(cr);

    // Draw bottom edge from left to right.
    if (strip_bottom && edge_width > 0)
    {
        draw_adaptive_line(cr,
            strip_bottom, edge_width,
            false, x + radius, y + height - 0.5, alpha
        );
    }

    // Draw bottom-right arc with the nearest strip pixel color.
    arc_color = resolve_arc_color(
        strip_right, edge_height, edge_height - 1, true,
        strip_bottom, edge_width, edge_width - 1, false
    );
    cairo_set_source_rgba(cr,
        arc_color, arc_color, arc_color, alpha
    );
    cairo_arc_negative(cr,
        x + width - radius, y + height - radius,
        radius - 0.5, PI / 2, 0
    );
    cairo_stroke(cr);

    // Draw right edge from top to bottom.
    if (strip_right && edge_height > 0)
    {
        draw_adaptive_line(cr,
            strip_right, edge_height,
            true, x + width - 0.5, y + title_height, alpha
        );
    }

    // Destroy the fetched strip images.
    if (strip_left)
    {
        XDestroyImage(strip_left);
    }
    if (strip_right)
    {
        XDestroyImage(strip_right);
    }
    if (strip_bottom)
    {
        XDestroyImage(strip_bottom);
    }

    // Draw title bar separator line.
    ThemeColorRGBA separator = theme->titlebar_separator;
    cairo_set_source_rgba(cr,
        separator.r, separator.g, separator.b, separator.a
    );
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, x, y + title_height - 0.5);
    cairo_line_to(cr, x + width, y + title_height - 0.5);
    cairo_stroke(cr);
}

void draw_frameless_border(cairo_t *cr, Portal *portal, Pixmap pixmap)
{
    // Retrieve display, theme, and geometry values.
    Display *display = DefaultDisplay;
    const Theme *theme = get_current_theme();
    double x = portal->geometry.x_root;
    double y = portal->geometry.y_root;
    double width = portal->geometry.width;
    double height = portal->geometry.height;
    double radius = PORTAL_FRAMELESS_CORNER_RADIUS;
    double alpha = theme->titlebar_border.a;

    // Compute straight-edge lengths and line width for drawing.
    int edge_width = (int)(width - 2 * radius);
    int edge_height = (int)(height - 2 * radius);
    cairo_set_line_width(cr, 1);

    // Grab 4 edge strips one pixel inside each border edge.
    XImage *strip_top = NULL;
    XImage *strip_bottom = NULL;
    XImage *strip_left = NULL;
    XImage *strip_right = NULL;
    if (edge_width > 0)
    {
        strip_top = XGetImage(
            display, pixmap,
            (int)radius, 1, edge_width, 1,
            AllPlanes, ZPixmap
        );
        strip_bottom = XGetImage(
            display, pixmap,
            (int)radius, (int)height - 2, edge_width, 1,
            AllPlanes, ZPixmap
        );
    }
    if (edge_height > 0)
    {
        strip_left = XGetImage(
            display, pixmap,
            1, (int)radius, 1, edge_height,
            AllPlanes, ZPixmap
        );
        strip_right = XGetImage(
            display, pixmap,
            (int)width - 2, (int)radius, 1, edge_height,
            AllPlanes, ZPixmap
        );
    }

    // Declare arc color for reuse across corner arcs.
    double arc_color;

    // Draw top edge from left to right.
    if (strip_top && edge_width > 0)
    {
        draw_adaptive_line(cr,
            strip_top, edge_width,
            false, x + radius + 0.5, y + 0.5, alpha
        );
    }

    // Draw top-right arc with the nearest strip pixel color.
    arc_color = resolve_arc_color(
        strip_top, edge_width, edge_width - 1, false,
        strip_right, edge_height, 0, true
    );
    cairo_set_source_rgba(cr, arc_color, arc_color, arc_color, alpha);
    cairo_arc(cr,
        x + width - radius - 0.5, y + radius + 0.5,
        radius, -PI / 2, 0
    );
    cairo_stroke(cr);

    // Draw right edge from top to bottom.
    if (strip_right && edge_height > 0)
    {
        draw_adaptive_line(cr,
            strip_right, edge_height,
            true, x + width - 0.5, y + radius + 0.5, alpha
        );
    }

    // Draw bottom-right arc with the nearest strip pixel color.
    arc_color = resolve_arc_color(
        strip_right, edge_height, edge_height - 1, true,
        strip_bottom, edge_width, edge_width - 1, false
    );
    cairo_set_source_rgba(cr, arc_color, arc_color, arc_color, alpha);
    cairo_arc(cr,
        x + width - radius - 0.5,
        y + height - radius - 0.5,
        radius, 0, PI / 2
    );
    cairo_stroke(cr);

    // Draw bottom edge from left to right.
    if (strip_bottom && edge_width > 0)
    {
        draw_adaptive_line(cr,
            strip_bottom, edge_width,
            false, x + radius + 0.5, y + height - 0.5, alpha
        );
    }

    // Draw bottom-left arc with the nearest strip pixel color.
    arc_color = resolve_arc_color(
        strip_bottom, edge_width, 0, false,
        strip_left, edge_height, edge_height - 1, true
    );
    cairo_set_source_rgba(cr,
        arc_color, arc_color, arc_color, alpha
    );
    cairo_arc(cr,
        x + radius + 0.5, y + height - radius - 0.5,
        radius, PI / 2, PI
    );
    cairo_stroke(cr);

    // Draw left edge from top to bottom.
    if (strip_left && edge_height > 0)
    {
        draw_adaptive_line(cr,
            strip_left, edge_height,
            true, x + 0.5, y + radius + 0.5, alpha
        );
    }

    // Draw top-left arc with the nearest strip pixel color.
    arc_color = resolve_arc_color(
        strip_left, edge_height, 0, true,
        strip_top, edge_width, 0, false
    );
    cairo_set_source_rgba(cr,
        arc_color, arc_color, arc_color, alpha
    );
    cairo_arc(cr,
        x + radius + 0.5, y + radius + 0.5,
        radius, PI, 3 * PI / 2
    );
    cairo_stroke(cr);

    // Destroy the fetched strip images.
    if (strip_top)
    {
        XDestroyImage(strip_top);
    }
    if (strip_bottom)
    {
        XDestroyImage(strip_bottom);
    }
    if (strip_left)
    {
        XDestroyImage(strip_left);
    }
    if (strip_right)
    {
        XDestroyImage(strip_right);
    }
}
