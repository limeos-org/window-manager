/**
 * This code is responsible for window compositing using the XComposite
 * extension combined with Cairo for rendering.
 *
 * It redirects all window rendering off-screen and then composites them back
 * to the root window. Double-buffering is used to prevent flicker: all drawing
 * is done to an off-screen X11 pixmap first, then copied to the root window in
 * one operation.
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

static Portal *find_fullscreen_portal()
{
    unsigned int count = 0;
    Portal **sorted = get_sorted_portals(&count);

    for (int i = count - 1; i >= 0; i--)
    {
        Portal *portal = sorted[i];
        if (portal != NULL && portal->fullscreen && portal->mapped)
        {
            return portal;
        }
    }
    return NULL;
}

static void draw_fullscreen_portal(Portal *portal)
{
    if (!compositor_enabled) return;

    Display *display = DefaultDisplay;

    // Grab the server to prevent window destruction during pixmap operations.
    XGrabServer(display);

    // Verify the client window is viewable.
    XWindowAttributes attrs;
    if (!XGetWindowAttributes(display, portal->client_window, &attrs) ||
        attrs.map_state != IsViewable)
    {
        XUngrabServer(display);
        return;
    }

    // Get the client window pixmap directly (bypass frame).
    Pixmap pixmap = XCompositeNameWindowPixmap(display, portal->client_window);
    if (pixmap == None)
    {
        XUngrabServer(display);
        return;
    }

    // Release the server grab now that we have the pixmap.
    XUngrabServer(display);

    // Create surface with screen dimensions.
    cairo_surface_t *window_surface = cairo_xlib_surface_create(
        display,
        pixmap,
        portal->client_visual,
        screen_width,
        screen_height
    );
    if (cairo_surface_status(window_surface) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(window_surface);
        XFreePixmap(display, pixmap);
        return;
    }

    // Draw at (0,0) covering the entire screen.
    cairo_set_source_surface(buffer_cr, window_surface, 0, 0);
    cairo_paint(buffer_cr);

    // Release Cairo surface and free the pixmap.
    cairo_set_source_rgb(buffer_cr, 0, 0, 0);
    cairo_surface_destroy(window_surface);
    XFreePixmap(display, pixmap);
}

static void draw_portal(Portal *portal)
{
    if (!compositor_enabled) return;
    if (portal == NULL) return;
    if (portal->mapped == false) return;
    if (portal->initialized == false) return;

    Display *display = DefaultDisplay;
    bool has_frame = is_portal_frame_valid(portal);
    Visual *visual = has_frame ? portal->frame_visual : portal->client_visual;

    // Get the window to composite (frame if it exists, otherwise client).
    Window target_window = has_frame ?
        portal->frame_window : portal->client_window;

    // Grab the server to prevent window destruction during pixmap operations.
    XGrabServer(display);

    // Verify override-redirect windows are viewable before getting their pixmap.
    // These windows are controlled by clients and can change state rapidly.
    // Framed portals are controlled by us, so we trust portal->mapped.
    if (portal->override_redirect)
    {
        XWindowAttributes attrs;
        if (!XGetWindowAttributes(display, target_window, &attrs) ||
            attrs.map_state != IsViewable)
        {
            XUngrabServer(display);
            return;
        }
    }

    // Get the window pixmap.
    Pixmap pixmap = XCompositeNameWindowPixmap(display, target_window);
    if (pixmap == None)
    {
        XUngrabServer(display);
        return;
    }

    // Release the server grab now that we have the pixmap.
    XUngrabServer(display);

    // Create a Cairo surface from the pixmap using cached dimensions.
    cairo_surface_t *window_surface = cairo_xlib_surface_create(
        display,
        pixmap,
        visual,
        portal->geometry.width,
        portal->geometry.height
    );

    // Check if the surface was created successfully.
    if (cairo_surface_status(window_surface) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(window_surface);
        XFreePixmap(display, pixmap);
        return;
    }

    // Draw the window surface to the off-screen buffer based on decoration kind.
    DecorationKind kind = get_portal_decoration_kind(portal);
    if (kind == DECORATION_FRAMED || kind == DECORATION_FRAMELESS)
    {
        // Select decoration parameters based on kind.
        int shadow_layers;
        double shadow_spread, shadow_opacity, corner_radius;
        void (*draw_border)(cairo_t *, Portal *, Pixmap);
        if (kind == DECORATION_FRAMED)
        {
            shadow_layers = 4;
            shadow_spread = 20;
            shadow_opacity = 0.1;
            corner_radius = PORTAL_CORNER_RADIUS;
            draw_border = draw_framed_border;
        }
        else
        {
            shadow_layers = 3;
            shadow_spread = 12;
            shadow_opacity = 0.08;
            corner_radius = PORTAL_FRAMELESS_CORNER_RADIUS;
            draw_border = draw_frameless_border;
        }

        // Draw drop shadow.
        draw_shadow(
            buffer_cr, portal, shadow_layers,
            shadow_spread, shadow_opacity, corner_radius
        );

        // Clip to rounded corners and paint portal content.
        cairo_save(buffer_cr);
        cairo_rounded_rectangle(
            buffer_cr,
            portal->geometry.x_root,
            portal->geometry.y_root,
            portal->geometry.width,
            portal->geometry.height,
            corner_radius
        );
        cairo_clip(buffer_cr);
        cairo_set_source_surface(
            buffer_cr,
            window_surface,
            portal->geometry.x_root,
            portal->geometry.y_root
        );
        cairo_paint(buffer_cr);
        cairo_restore(buffer_cr);

        // Draw border.
        draw_border(buffer_cr, portal, pixmap);
    }
    else
    {
        // Paint the window content directly.
        cairo_set_source_surface(
            buffer_cr,
            window_surface,
            portal->geometry.x_root,
            portal->geometry.y_root
        );
        cairo_paint(buffer_cr);
    }

    // Clear the source to release Cairo's reference to window_surface.
    cairo_set_source_rgb(buffer_cr, 0, 0, 0);

    // Cleanup.
    cairo_surface_destroy(window_surface);
    XFreePixmap(display, pixmap);
}

static void compositor_redraw()
{
    if (!compositor_enabled) return;

    Display *display = DefaultDisplay;

    // Draw all portals, or just the fullscreen one to the off-screen buffer.
    Portal *fullscreen = find_fullscreen_portal();
    if (fullscreen == NULL)
    {
        draw_background(buffer_cr);

        unsigned int portal_count = 0;
        Portal **portals = get_sorted_portals(&portal_count);
        for (unsigned int i = 0; i < portal_count; i++)
        {
            if (portals[i] != NULL) draw_portal(portals[i]);
        }
    }
    else
    {
        draw_fullscreen_portal(fullscreen);
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
