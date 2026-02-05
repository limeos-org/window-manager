/**
 * This code is responsible for portal border drawing.
 *
 * It handles rendering borders for portals, including luminance-based
 * adaptive coloring for the client area border.
 */

#include "../all.h"

/**
 * Samples pixel luminance at the given points from a pixmap.
 *
 * @param display The X display connection.
 * @param pixmap The window pixmap to sample from.
 * @param points Array of [x, y] coordinate pairs to sample.
 * @param num_points The number of points to sample.
 *
 * @return Luminance value from 0.0 (dark) to 1.0 (light).
 */
static float sample_luminance(Display *display, Pixmap pixmap, int points[][2], int num_points) {
    // Sample pixels and calculate average luminance.
    double total_luminance = 0.0;
    int valid_samples = 0;
    for (int i = 0; i < num_points; i++)
    {
        int x = points[i][0];
        int y = points[i][1];

        // Get single pixel.
        XImage *img = XGetImage(display, pixmap, x, y, 1, 1, AllPlanes, ZPixmap);
        if (img == NULL)
        {
            continue;
        }

        // Retrieve pixel value.
        unsigned long pixel = XGetPixel(img, 0, 0);
        XDestroyImage(img);

        // Extract RGB components.
        unsigned char r = (pixel >> 16) & 0xFF;
        unsigned char g = (pixel >> 8) & 0xFF;
        unsigned char b = pixel & 0xFF;

        // Calculate relative luminance using standard coefficients.
        double luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0;
        total_luminance += luminance;
        valid_samples++;
    }

    if (valid_samples == 0)
    {
        return 0.0f;
    }

    return (float)(total_luminance / valid_samples);
}

void draw_framed_border(cairo_t *cr, Portal *portal, Pixmap pixmap)
{
    Display *display = DefaultDisplay;
    const Theme *theme = get_current_theme();

    double x = portal->geometry.x_root;
    double y = portal->geometry.y_root;
    double width = portal->geometry.width;
    double height = portal->geometry.height;
    double radius = PORTAL_CORNER_RADIUS;
    double title_height = PORTAL_TITLE_BAR_HEIGHT;

    // Sample the client area below the titlebar to determine
    // border color that contrasts with the adjacent content.
    int cx = PORTAL_BORDER_WIDTH;
    int cy = PORTAL_TITLE_BAR_HEIGHT;
    int cw = portal->geometry.width - 2 * PORTAL_BORDER_WIDTH;
    int ch = portal->geometry.height - PORTAL_TITLE_BAR_HEIGHT - PORTAL_BORDER_WIDTH;
    float luminance = 0.0f;
    if (cw > 0 && ch > 0)
    {
        int margin = 10;
        int points[][2] = {
            { cx + cw / 2, cy + ch / 2 },
            { cx + margin, cy + margin },
            { cx + cw - margin, cy + margin },
            { cx + margin, cy + ch - margin },
            { cx + cw - margin, cy + ch - margin },
        };
        int n = sizeof(points) / sizeof(points[0]);
        for (int i = 0; i < n; i++)
        {
            if (points[i][0] < cx) points[i][0] = cx;
            if (points[i][0] >= cx + cw) points[i][0] = cx + cw - 1;
            if (points[i][1] < cy) points[i][1] = cy;
            if (points[i][1] >= cy + ch) points[i][1] = cy + ch - 1;
        }
        luminance = sample_luminance(display, pixmap, points, n);
    }

    // Draw inner border around title bar.
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

    // Draw inner border around client area. Color contrasts with content,
    // alpha matches the titlebar border.
    double client_border_rgb = (luminance > 0.5f) ? 0.0 : 1.0;
    cairo_set_source_rgba(cr,
        client_border_rgb,
        client_border_rgb,
        client_border_rgb,
        theme->titlebar_border.a
    );
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, x + 0.5, y + title_height);
    cairo_line_to(cr, x + 0.5, y + height - radius);
    cairo_arc_negative(cr, x + radius, y + height - radius, radius - 0.5, PI, PI / 2);
    cairo_line_to(cr, x + width - radius, y + height - 0.5);
    cairo_arc_negative(cr, x + width - radius, y + height - radius, radius - 0.5, PI / 2, 0);
    cairo_line_to(cr, x + width - 0.5, y + title_height);
    cairo_stroke(cr);

    // Draw title bar separator / divider.
    ThemeColorRGBA separator = theme->titlebar_separator;
    cairo_set_source_rgba(cr, separator.r, separator.g, separator.b, separator.a);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, x, y + title_height - 0.5);
    cairo_line_to(cr, x + width, y + title_height - 0.5);
    cairo_stroke(cr);
}

void draw_frameless_border(cairo_t *cr, Portal *portal, Pixmap pixmap)
{
    Display *display = DefaultDisplay;
    const Theme *theme = get_current_theme();

    double x = portal->geometry.x_root;
    double y = portal->geometry.y_root;
    double width = portal->geometry.width;
    double height = portal->geometry.height;
    double radius = PORTAL_FRAMELESS_CORNER_RADIUS;

    // Sample broadly across the full window area since frameless
    // windows have no titlebar, so the border surrounds all content.
    int w = portal->geometry.width;
    int h = portal->geometry.height;
    float luminance = 0.0f;
    if (w > 0 && h > 0)
    {
        int margin = 10;
        int points[][2] = {
            { w / 2, h / 2 },
            { margin, margin },
            { w - margin, margin },
            { margin, h - margin },
            { w - margin, h - margin },
        };
        int n = sizeof(points) / sizeof(points[0]);
        for (int i = 0; i < n; i++)
        {
            if (points[i][0] < 0) points[i][0] = 0;
            if (points[i][0] >= w) points[i][0] = w - 1;
            if (points[i][1] < 0) points[i][1] = 0;
            if (points[i][1] >= h) points[i][1] = h - 1;
        }
        luminance = sample_luminance(display, pixmap, points, n);
    }

    // Draw border: dark content = light border, light content = dark border.
    double border_rgb = (luminance > 0.5f) ? 0.0 : 1.0;
    cairo_set_source_rgba(cr,
        border_rgb,
        border_rgb,
        border_rgb,
        theme->titlebar_border.a
    );
    cairo_set_line_width(cr, 1);

    // Draw full rounded rectangle border.
    cairo_move_to(cr, x + radius + 0.5, y + 0.5);
    cairo_line_to(cr, x + width - radius - 0.5, y + 0.5);
    cairo_arc(cr, x + width - radius - 0.5, y + radius + 0.5, radius, -PI / 2, 0);
    cairo_line_to(cr, x + width - 0.5, y + height - radius - 0.5);
    cairo_arc(cr, x + width - radius - 0.5, y + height - radius - 0.5, radius, 0, PI / 2);
    cairo_line_to(cr, x + radius + 0.5, y + height - 0.5);
    cairo_arc(cr, x + radius + 0.5, y + height - radius - 0.5, radius, PI / 2, PI);
    cairo_line_to(cr, x + 0.5, y + radius + 0.5);
    cairo_arc(cr, x + radius + 0.5, y + radius + 0.5, radius, PI, 3 * PI / 2);
    cairo_close_path(cr);
    cairo_stroke(cr);
}
