/**
 * This code is responsible for portal border drawing.
 *
 * It handles rendering borders for portals, including luminance-based
 * adaptive coloring for the client area border and title bar separator.
 */

#include "../all.h"

/**
 * Samples the client area of a portal to determine content luminance.
 *
 * @param display The X display connection.
 * @param pixmap The window pixmap to sample from.
 * @param portal The portal containing geometry information.
 *
 * @return Luminance value from 0.0 (dark) to 1.0 (light).
 */
static float sample_client_luminance(Display *display, Pixmap pixmap, Portal *portal)
{
    // Calculate client area bounds within the frame.
    int client_x = PORTAL_BORDER_WIDTH;
    int client_y = PORTAL_TITLE_BAR_HEIGHT;
    int client_width = portal->geometry.width - 2 * PORTAL_BORDER_WIDTH;
    int client_height = portal->geometry.height - PORTAL_TITLE_BAR_HEIGHT - PORTAL_BORDER_WIDTH;

    // Ensure client area is valid.
    if (client_width <= 0 || client_height <= 0)
    {
        return 0.0f;
    }

    // Define sample points within the client area.
    int margin = 10;
    int sample_points[][2] = {
        { client_x + client_width / 2, client_y + client_height / 2 },           // Center
        { client_x + margin, client_y + margin },                                // Top-left
        { client_x + client_width - margin, client_y + margin },                 // Top-right
        { client_x + margin, client_y + client_height - margin },                // Bottom-left
        { client_x + client_width - margin, client_y + client_height - margin }, // Bottom-right
    };
    int num_samples = sizeof(sample_points) / sizeof(sample_points[0]);

    // Sample pixels and calculate average luminance.
    double total_luminance = 0.0;
    int valid_samples = 0;
    for (int i = 0; i < num_samples; i++)
    {
        int x = sample_points[i][0];
        int y = sample_points[i][1];

        // Clamp to valid coordinates.
        if (x < client_x) x = client_x;
        if (x >= client_x + client_width) x = client_x + client_width - 1;
        if (y < client_y) y = client_y;
        if (y >= client_y + client_height) y = client_y + client_height - 1;

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

void draw_portal_border(cairo_t *cr, Portal *portal, Pixmap pixmap)
{
    Display *display = DefaultDisplay;
    const Theme *theme = get_current_theme();

    double x = portal->geometry.x_root;
    double y = portal->geometry.y_root;
    double width = portal->geometry.width;
    double height = portal->geometry.height;
    double radius = PORTAL_CORNER_RADIUS;
    double title_height = PORTAL_TITLE_BAR_HEIGHT;

    // Sample luminance to determine border colors.
    float luminance = sample_client_luminance(display, pixmap, portal);

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

    // Draw title bar separator / divider. Alpha increases with title bar and
    // content contrast so the separator remains visible at high-contrast
    // boundaries.
    ThemeColorRGB separator = theme->titlebar_separator;
    bool high_contrast;
    if (theme->variant == THEME_VARIANT_LIGHT)
    {
        high_contrast = luminance <= 0.5f;
    }
    else
    {
        high_contrast = luminance > 0.5f;
    }
    double separator_a = high_contrast ? 0.5 : 0.05;
    cairo_set_source_rgba(cr, separator.r, separator.g, separator.b, separator_a);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, x, y + title_height - 0.5);
    cairo_line_to(cr, x + width, y + title_height - 0.5);
    cairo_stroke(cr);
}
