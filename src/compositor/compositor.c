/**
 * XComposite-based compositor for the window manager.
 *
 * This module handles window compositing using the XComposite extension
 * combined with Cairo for rendering. It redirects all window rendering
 * off-screen and then composites them back to the root window.
 *
 * Double-buffering is used to prevent flicker: all drawing is done to an
 * off-screen X11 pixmap first, then copied to the root window in one operation.
 */

#include "../all.h"

static cairo_t *root_cr = NULL;
static cairo_surface_t *root_surface = NULL;
static cairo_t *buffer_cr = NULL;
static cairo_surface_t *buffer_surface = NULL;
static Pixmap buffer_pixmap = None;
static bool compositor_enabled = false;

static int screen_width = 0;
static int screen_height = 0;

static void compositor_init()
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
    int screen = DefaultScreen(display);

    // Check if XComposite extension is available.
    int event_base, error_base;
    if (!XCompositeQueryExtension(display, &event_base, &error_base))
    {
        LOG_WARNING("XComposite extension not available, compositor disabled.");
        return;
    }

    // Check XComposite version (need at least 0.2 for NameWindowPixmap).
    int major = 0, minor = 0;
    XCompositeQueryVersion(display, &major, &minor);
    if (major == 0 && minor < 2)
    {
        LOG_WARNING("XComposite version too old (need 0.2+), compositor disabled.");
        return;
    }

    // Get screen dimensions.
    screen_width = DisplayWidth(display, screen);
    screen_height = DisplayHeight(display, screen);

    // Redirect all subwindows of the root window for manual compositing.
    XCompositeRedirectSubwindows(display, root_window, CompositeRedirectManual);

    // Create a Cairo surface for drawing to the root window.
    Visual *visual = DefaultVisual(display, screen);
    int depth = DefaultDepth(display, screen);
    root_surface = cairo_xlib_surface_create(
        display,
        root_window,
        visual,
        screen_width,
        screen_height
    );
    root_cr = cairo_create(root_surface);

    // Create an off-screen X11 pixmap for double-buffering.
    buffer_pixmap = XCreatePixmap(display, root_window, screen_width, screen_height, depth);
    buffer_surface = cairo_xlib_surface_create(
        display,
        buffer_pixmap,
        visual,
        screen_width,
        screen_height
    );
    buffer_cr = cairo_create(buffer_surface);

    compositor_enabled = true;
}

static bool is_window_viewable(Display *display, Window window)
{
    XWindowAttributes attrs;
    if (XGetWindowAttributes(display, window, &attrs) == 0)
    {
        return false;
    }
    return attrs.map_state == IsViewable;
}

static void draw_portal(Portal *portal)
{
    if (!compositor_enabled) return;
    if (portal == NULL) return;
    if (portal->mapped == false) return;
    if (portal->initialized == false) return;

    Display *display = DefaultDisplay;

    // Get the window to composite (frame if it exists, otherwise client).
    Window target_window = is_portal_frame_valid(portal) ?
        portal->frame_window : portal->client_window;

    // Grab the server to prevent window destruction during pixmap operations.
    // This ensures atomicity between checking window validity and using its 
    // pixmap.
    XGrabServer(display);

    // Check if the window is viewable before trying to get its pixmap.
    if (!is_window_viewable(display, target_window))
    {
        XUngrabServer(display);
        return;
    }

    // Get the window pixmap.
    Pixmap pixmap = XCompositeNameWindowPixmap(display, target_window);
    if (pixmap == None)
    {
        XUngrabServer(display);
        return;
    }

    // Get the actual window attributes for the correct visual and dimensions.
    XWindowAttributes window_attrs;
    if (XGetWindowAttributes(display, target_window, &window_attrs) == 0)
    {
        XFreePixmap(display, pixmap);
        XUngrabServer(display);
        return;
    }

    // Create a Cairo surface from the pixmap using the window's visual.
    cairo_surface_t *window_surface = cairo_xlib_surface_create(
        display,
        pixmap,
        window_attrs.visual,
        window_attrs.width,
        window_attrs.height
    );

    // Release the server grab - we have all the data we need.
    XUngrabServer(display);

    // Check if the surface was created successfully.
    if (cairo_surface_status(window_surface) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(window_surface);
        XFreePixmap(display, pixmap);
        return;
    }

    // Draw the window surface to the off-screen buffer.
    cairo_set_source_surface(buffer_cr, window_surface, portal->x_root, portal->y_root);
    cairo_paint(buffer_cr);

    // Cleanup.
    cairo_surface_destroy(window_surface);
    XFreePixmap(display, pixmap);
}

static void compositor_redraw()
{
    if (!compositor_enabled) return;

    Display *display = DefaultDisplay;

    // Clear the off-screen buffer with the background color.
    unsigned long bg_color;
    GET_CONFIG(&bg_color, sizeof(bg_color), CFG_BUNDLE_BACKGROUND_COLOR);
    double r, g, b;
    hex_to_rgb(bg_color, &r, &g, &b);
    cairo_set_source_rgb(buffer_cr, r, g, b);
    cairo_paint(buffer_cr);

    // Get sorted portals and draw them to the buffer (back to front).
    unsigned int portal_count = 0;
    Portal **portals = get_sorted_portals(&portal_count);
    for (unsigned int i = 0; i < portal_count; i++)
    {
        if (portals[i] != NULL)
        {
            draw_portal(portals[i]);
        }
    }

    // Copy the completed buffer to the root window in one operation.
    cairo_set_source_surface(root_cr, buffer_surface, 0, 0);
    cairo_paint(root_cr);

    // Flush to ensure drawing is displayed.
    XFlush(display);
}

HANDLE(Initialize)
{
    compositor_init();
}

HANDLE(Update)
{
    compositor_redraw();
}
