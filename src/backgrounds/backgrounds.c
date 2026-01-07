#include "../all.h"

/**
 * This code is responsible for rendering the desktop background on the root
 * window, supporting solid colors, PNG images, and various scaling modes.
 */

BackgroundScaling parse_background_scaling(const char *scaling_str)
{
    if (strcmp(scaling_str, "fit") == 0) return BACKGROUND_SCALING_FIT;
    if (strcmp(scaling_str, "stretch") == 0) return BACKGROUND_SCALING_STRETCH;
    if (strcmp(scaling_str, "center") == 0) return BACKGROUND_SCALING_CENTER;
    if (strcmp(scaling_str, "tile") == 0) return BACKGROUND_SCALING_TILE;
    return BACKGROUND_SCALING_ZOOM; // Default
}

static void draw_solid_background(cairo_t *cr, int width, int height)
{
    // Get the background color from config.
    unsigned long bg_color = 0;
    GET_CONFIG(&bg_color, 0, CFG_BUNDLE_BACKGROUND_COLOR);

    // Convert hex to RGB.
    double r, g, b;
    hex_to_rgb(bg_color, &r, &g, &b);

    // Fill the entire surface with the color.
    cairo_set_source_rgb(cr, r, g, b);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
}

static void draw_image_background(cairo_t *cr, int screen_width, int screen_height)
{
    // Get the image path from config.
    char image_path[MAX_PATH];
    GET_CONFIG(image_path, sizeof(image_path), CFG_BUNDLE_BACKGROUND_IMAGE_PATH);

    // Get the scaling mode from config.
    char scaling_str[32];
    GET_CONFIG(scaling_str, sizeof(scaling_str), CFG_BUNDLE_BACKGROUND_SCALING);
    BackgroundScaling scaling = parse_background_scaling(scaling_str);

    // Load the PNG image.
    cairo_surface_t *image = cairo_image_surface_create_from_png(image_path);
    cairo_status_t status = cairo_surface_status(image);
    if (status != CAIRO_STATUS_SUCCESS)
    {
        LOG_WARNING("Failed to load background image: %s (%s)", 
                    image_path, cairo_status_to_string(status));
        cairo_surface_destroy(image);
        // Fall back to solid color.
        draw_solid_background(cr, screen_width, screen_height);
        return;
    }

    int img_width = cairo_image_surface_get_width(image);
    int img_height = cairo_image_surface_get_height(image);

    // First, draw a solid background in case image doesn't cover everything.
    draw_solid_background(cr, screen_width, screen_height);

    // Calculate scaling based on mode.
    double scale_x = 1.0, scale_y = 1.0;
    double offset_x = 0.0, offset_y = 0.0;

    switch (scaling)
    {
        case BACKGROUND_SCALING_ZOOM:
        {
            // Scale to fill, maintaining aspect ratio, cropping excess.
            double scale = fmax((double)screen_width / img_width,
                               (double)screen_height / img_height);
            scale_x = scale;
            scale_y = scale;
            offset_x = (screen_width - img_width * scale) / 2;
            offset_y = (screen_height - img_height * scale) / 2;
            break;
        }
        case BACKGROUND_SCALING_FIT:
        {
            // Scale to fit, maintaining aspect ratio, letterboxing.
            double scale = fmin((double)screen_width / img_width,
                               (double)screen_height / img_height);
            scale_x = scale;
            scale_y = scale;
            offset_x = (screen_width - img_width * scale) / 2;
            offset_y = (screen_height - img_height * scale) / 2;
            break;
        }
        case BACKGROUND_SCALING_STRETCH:
        {
            // Stretch to fill exactly.
            scale_x = (double)screen_width / img_width;
            scale_y = (double)screen_height / img_height;
            break;
        }
        case BACKGROUND_SCALING_CENTER:
        {
            // Center at original size.
            offset_x = (screen_width - img_width) / 2;
            offset_y = (screen_height - img_height) / 2;
            break;
        }
        case BACKGROUND_SCALING_TILE:
        {
            // Tile the image.
            cairo_pattern_t *pattern = cairo_pattern_create_for_surface(image);
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
            cairo_set_source(cr, pattern);
            cairo_rectangle(cr, 0, 0, screen_width, screen_height);
            cairo_fill(cr);
            cairo_pattern_destroy(pattern);
            cairo_surface_destroy(image);
            return;
        }
    }

    // Apply transformation and draw.
    cairo_save(cr);
    cairo_translate(cr, offset_x, offset_y);
    cairo_scale(cr, scale_x, scale_y);
    cairo_set_source_surface(cr, image, 0, 0);
    cairo_paint(cr);
    cairo_restore(cr);

    cairo_surface_destroy(image);
}

void draw_background()
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
    int screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);

    // Create a Cairo surface for the root window.
    Visual *visual = DefaultVisual(display, screen);
    cairo_surface_t *surface = cairo_xlib_surface_create(
        display, root_window, visual, screen_width, screen_height
    );
    cairo_t *cr = cairo_create(surface);

    // Get the background mode from config.
    char mode[32];
    GET_CONFIG(mode, sizeof(mode), CFG_BUNDLE_BACKGROUND_MODE);

    // Draw based on mode.
    if (strcmp(mode, "image") == 0)
    {
        draw_image_background(cr, screen_width, screen_height);
    }
    else
    {
        // Default to solid color.
        draw_solid_background(cr, screen_width, screen_height);
    }

    // Cleanup.
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    // Ensure the background is rendered.
    XFlush(display);
}

HANDLE(Initialize)
{
    (void)event;
    
    // Draw the background on startup.
    draw_background();
}
