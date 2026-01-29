#include "../all.h"

static cairo_t *cr = NULL;
static cairo_surface_t *xlib_surface = NULL;
static cairo_surface_t *png_surface = NULL;

static char cfg_background_mode[16];
static unsigned long cfg_background_color;
static char cfg_background_image_path[MAX_PATH];

static cairo_surface_t *load_background_image(Display *display, const char *filename)
{
    // Create a surface to hold the original image.
    cairo_surface_t *original_image = cairo_image_surface_create_from_png(filename);
    if (cairo_surface_status(original_image) != CAIRO_STATUS_SUCCESS)
    {
        LOG_ERROR("Failed to load background image (%s).", filename);
        return NULL;
    }

    // Get screen and image dimensions.
    int screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);
    int image_width = cairo_image_surface_get_width(original_image);
    int image_height = cairo_image_surface_get_height(original_image);

    // Create a surface to hold the scaled image.
    cairo_surface_t *scaled_image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, screen_width, screen_height);
    cairo_t *scale_cr = cairo_create(scaled_image);
    cairo_scale(scale_cr, (double)screen_width / image_width, (double)screen_height / image_height);
    cairo_set_source_surface(scale_cr, original_image, 0, 0);
    cairo_paint(scale_cr);
    cairo_destroy(scale_cr);

    // Free the original image surface from memory.
    cairo_surface_destroy(original_image);

    return scaled_image;
}

void draw_background(cairo_t *cr)
{
    // Ensure the Cairo context is valid.
    if (cr == NULL)
        return;

    // Draw the background image if configured.
    if (strcmp(cfg_background_mode, "image") == 0 && png_surface != NULL)
    {
        cairo_set_source_surface(cr, png_surface, 0, 0);
        cairo_paint(cr);
    }
    else
    {
        // Fall back to solid color background.
        double r, g, b;
        common.hex_to_rgb(cfg_background_color, &r, &g, &b);
        cairo_set_source_rgb(cr, r, g, b);
        cairo_paint(cr);
    }
}

HANDLE(Initialize)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Get configuration values.
    common.get_config_str(cfg_background_mode, sizeof(cfg_background_mode), CFG_KEY_BACKGROUND_MODE, CFG_DEFAULT_BACKGROUND_MODE);
    common.get_config_hex(&cfg_background_color, CFG_KEY_BACKGROUND_COLOR, CFG_DEFAULT_BACKGROUND_COLOR);
    common.get_config_path(cfg_background_image_path, sizeof(cfg_background_image_path), CFG_KEY_BACKGROUND_IMAGE_PATH, CFG_DEFAULT_BACKGROUND_IMAGE_PATH);

    // Prepare xlib surface.
    int screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);
    xlib_surface = cairo_xlib_surface_create(display, root_window, DefaultVisual(display, screen), screen_width, screen_height);

    // Prepare png surface if neccessary.
    if (strcmp(cfg_background_mode, "image") == 0)
    {
        char expanded_path[MAX_PATH];
        common.expand_path(cfg_background_image_path, expanded_path, sizeof(expanded_path));
        png_surface = load_background_image(display, expanded_path);
    }

    cr = cairo_create(xlib_surface);
}

HANDLE(Expose)
{
    XExposeEvent *_event = &event->xexpose;
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    if (_event->window == root_window && _event->count == 0)
    {
        draw_background(cr);
    }
}
