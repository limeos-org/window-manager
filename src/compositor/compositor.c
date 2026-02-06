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

static bool compositor_enabled = false;

static cairo_t *root_cr = NULL;
static cairo_surface_t *root_surface = NULL;

static cairo_t *buffer_cr = NULL;
static cairo_surface_t *buffer_surface = NULL;
static Pixmap buffer_pixmap = None;

/**
 * Tracks which client windows have been composite-redirected for the purpose
 * of split rendering misaligned framed portals. Indexed by portal index.
 */
static Window redirected_clients[MAX_PORTALS] = {0};

static int screen_width = 0;
static int screen_height = 0;

static void init_compositor()
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
        if (portal != NULL &&
            portal->fullscreen &&
            portal->visibility == PORTAL_VISIBLE
        ) {
            return portal;
        }
    }
    return NULL;
}

/**
 * Acquires a window's composite pixmap and wraps it in a Cairo surface.
 *
 * @param window The window to acquire.
 * @param visual The visual for the Cairo surface.
 * @param width The surface width.
 * @param height The surface height.
 * @param check_viewable Whether to verify the window is viewable.
 * @param out_pixmap Receives the acquired pixmap on success.
 *
 * @return - `cairo_surface_t*` On success.
 * @return - `NULL` If the window is not viewable or acquisition failed.
 *
 * @note Caller owns both the returned surface and *out_pixmap.
 */
static cairo_surface_t *acquire_window_surface(
    Window window,
    Visual *visual,
    unsigned int width,
    unsigned int height,
    bool check_viewable,
    Pixmap *out_pixmap
)
{
    Display *display = DefaultDisplay;
    *out_pixmap = None;

    // Grab the server to prevent window changes during acquisition.
    XGrabServer(display);

    // Verify the window is viewable if requested.
    if (check_viewable)
    {
        XWindowAttributes attrs;
        if (!XGetWindowAttributes(display, window, &attrs)
            || attrs.map_state != IsViewable)
        {
            XUngrabServer(display);
            return NULL;
        }
    }

    // Get the composite pixmap.
    Pixmap pixmap = XCompositeNameWindowPixmap(display, window);
    if (pixmap == None)
    {
        XUngrabServer(display);
        return NULL;
    }

    // Release the server grab.
    XUngrabServer(display);

    // Create a Cairo surface from the pixmap.
    cairo_surface_t *surface = cairo_xlib_surface_create(
        display, pixmap, visual, width, height
    );
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(surface);
        XFreePixmap(display, pixmap);
        return NULL;
    }

    *out_pixmap = pixmap;
    return surface;
}

/** Composites a fullscreen portal to the buffer. */
static void draw_fullscreen_portal(Portal *portal)
{
    if (!compositor_enabled) return;

    // Acquire the client pixmap directly (bypass frame).
    Pixmap pixmap;
    cairo_surface_t *surface = acquire_window_surface(
        portal->client_window, portal->client_visual,
        screen_width, screen_height, true, &pixmap
    );
    if (surface == NULL) return;

    // Draw at (0,0) covering the entire screen.
    cairo_set_source_surface(buffer_cr, surface, 0, 0);
    cairo_paint(buffer_cr);

    // Release Cairo surface and free the pixmap.
    cairo_set_source_rgb(buffer_cr, 0, 0, 0);
    cairo_surface_destroy(surface);
    XFreePixmap(DefaultDisplay, pixmap);
}

/**
 * Renders a misaligned framed portal's content, avoiding
 * position flash from stale client offset in the frame.
 */
static void draw_split_content(
    Portal *portal,
    cairo_surface_t *frame_surface
)
{
    Display *display = DefaultDisplay;

    // Ensure client has its own composite pixmap.
    int portal_index = get_portal_index(portal);
    if (portal_index >= 0
        && redirected_clients[portal_index]
            != portal->client_window)
    {
        XCompositeRedirectWindow(
            display,
            portal->client_window,
            CompositeRedirectAutomatic
        );
        redirected_clients[portal_index] =
            portal->client_window;
    }

    // Paint the title bar from the frame pixmap.
    cairo_save(buffer_cr);
    cairo_rectangle(
        buffer_cr,
        portal->geometry.x_root,
        portal->geometry.y_root,
        portal->geometry.width,
        PORTAL_TITLE_BAR_HEIGHT
    );
    cairo_clip(buffer_cr);
    cairo_set_source_surface(
        buffer_cr,
        frame_surface,
        portal->geometry.x_root,
        portal->geometry.y_root
    );
    cairo_paint(buffer_cr);
    cairo_restore(buffer_cr);

    // Acquire the client pixmap as a Cairo surface.
    unsigned int client_height = portal->geometry.height - PORTAL_TITLE_BAR_HEIGHT;
    Pixmap client_pixmap;
    cairo_surface_t *client_surface = acquire_window_surface(
        portal->client_window, portal->client_visual,
        portal->geometry.width, client_height,
        true, &client_pixmap
    );

    // Paint the client at the WM-controlled offset.
    if (client_surface != NULL)
    {
        cairo_set_source_surface(
            buffer_cr,
            client_surface,
            portal->geometry.x_root,
            portal->geometry.y_root + PORTAL_TITLE_BAR_HEIGHT
        );
        cairo_paint(buffer_cr);
        cairo_set_source_rgb(buffer_cr, 0, 0, 0);

        // Release the client surface and pixmap.
        cairo_surface_destroy(client_surface);
        XFreePixmap(display, client_pixmap);
    }

    // Reset the misalignment flag.
    portal->misaligned = false;
}

static void draw_portal(Portal *portal)
{
    if (!compositor_enabled) return;
    if (portal == NULL) return;
    if (portal->visibility != PORTAL_VISIBLE) return;
    if (portal->initialized == false) return;

    Display *display = DefaultDisplay;
    bool has_frame = is_portal_frame_valid(portal);
    Visual *visual = has_frame ? portal->frame_visual : portal->client_visual;

    // Get the window to composite (frame if it exists, otherwise client).
    Window target_window = has_frame ?
        portal->frame_window : portal->client_window;

    // Acquire the window pixmap as a Cairo surface.
    // Override-redirect windows need viewability checks because clients
    // control them and can change state rapidly. Framed portals are
    // controlled by us, so we trust `portal->visibility`.
    Pixmap pixmap;
    cairo_surface_t *window_surface = acquire_window_surface(
        target_window, visual,
        portal->geometry.width, portal->geometry.height,
        portal->override_redirect, &pixmap
    );
    if (window_surface == NULL) return;

    // Draw the window surface to the off-screen buffer based on decoration kind.
    PortalDecoration kind = get_portal_decoration_kind(portal);
    if (kind == PORTAL_DECORATION_FRAMED || kind == PORTAL_DECORATION_FRAMELESS)
    {
        // Select decoration parameters based on kind.
        int shadow_layers;
        double shadow_spread, shadow_opacity, corner_radius;
        void (*draw_border)(cairo_t *, Portal *, Pixmap);
        if (kind == PORTAL_DECORATION_FRAMED)
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
        // Normally the frame pixmap is painted as a single surface.
        // When the client is misaligned within the frame, split
        // rendering draws the title bar from the frame pixmap and
        // the client from its own pixmap at the WM-controlled
        // offset, bypassing the stale position in the frame.
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
        if (has_frame && portal->misaligned)
        {
            draw_split_content(portal, window_surface);
        }
        else
        {
            cairo_set_source_surface(
                buffer_cr,
                window_surface,
                portal->geometry.x_root,
                portal->geometry.y_root
            );
            cairo_paint(buffer_cr);
        }
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

static void redraw_compositor()
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
    init_compositor();
}

HANDLE(Update)
{
    redraw_compositor();
}

HANDLE(PortalDestroyed)
{
    PortalDestroyedEvent *_event = &event->portal_destroyed;

    // Clear the composite redirect tracking for the destroyed portal.
    int portal_index = get_portal_index(_event->portal);
    if (portal_index >= 0)
    {
        redirected_clients[portal_index] = 0;
    }
}
