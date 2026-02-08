/**
 * This code is responsible for portal frame management.
 * It handles creating, drawing, and destroying decorative frames for portals.
 */

#include "../all.h"

bool should_portal_be_framed(Portal *portal)
{
    Display *display = DefaultDisplay;
    Window client_window = portal->client_window;

    // Check if portal is a managed top-level window (ICCCM).
    if (!portal->top_level)
    {
        return false;
    }

    // Check Motif hints for decoration preferences.
    if (!x_window_wants_decorations_motif(display, client_window))
    {
        return false;
    }

    // Check EWMH window type for decoration preferences.
    if (!x_window_wants_decorations_ewmh(display, portal->client_window_type))
    {
        return false;
    }

    return true;
}

void create_portal_frame(Portal *portal)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
    int x_root = portal->geometry.x_root;
    int y_root = portal->geometry.y_root;
    unsigned int width = portal->geometry.width;
    unsigned int height = portal->geometry.height;

    // Create the frame window.
    Window frame_window = x_create_simple_window(
        display,        // Display
        root_window,    // Parent window
        x_root,         // X (Relative to parent)
        y_root,         // Y (Relative to parent)
        width,          // Width
        height,         // Height
        0,              // Border width
        0,              // Border color
        0xFFFFFF        // Background color
    );

    // Choose which frame window events we should listen for.
    XSelectInput(display, frame_window, ExposureMask | SubstructureNotifyMask);

    // Create the Cairo context for the frame window.
    Visual *visual = DefaultVisual(display, DefaultScreen(display));
    cairo_surface_t *surface = cairo_xlib_surface_create(
        display,
        frame_window,
        visual,
        width, height
    );
    cairo_t *cr = cairo_create(surface);

    // Assign the frame window and Cairo context to the portal.
    portal->frame_window = frame_window;
    portal->frame_cr = cr;
    portal->frame_visual = visual;

    // Add the client window to our save-set so it survives if the WM exits.
    XAddToSaveSet(display, portal->client_window);

    // Reparent the client window to the frame window.
    XReparentWindow(
        display,                // Display
        portal->client_window,  // Window
        portal->frame_window,   // Parent window
        0,                      // X (Relative to parent)
        PORTAL_TITLE_BAR_HEIGHT // Y (Relative to parent)
    );

    // Set _NET_FRAME_EXTENTS to inform the client about decoration sizes.
    // This is needed for applications to correctly calculate coordinates
    // (e.g., for drag and drop operations).
    Atom _NET_FRAME_EXTENTS = XInternAtom(display, "_NET_FRAME_EXTENTS", False);
    unsigned long extents[4] = {
        0,                        // Left
        0,                        // Right
        PORTAL_TITLE_BAR_HEIGHT,  // Top
        0                         // Bottom
    };
    XChangeProperty(
        display,
        portal->client_window,
        _NET_FRAME_EXTENTS,
        XA_CARDINAL,
        32,
        PropModeReplace,
        (unsigned char *)extents,
        4
    );
}

void draw_portal_frame(Portal *portal)
{
    const Theme *theme = get_portal_theme(portal);
    cairo_t *cr = portal->frame_cr;
    unsigned int width = portal->geometry.width;
    unsigned int height = portal->geometry.height;

    // Match the frame window background to the theme so that
    // unpainted areas (e.g., during resize) blend with the titlebar.
    unsigned long bg = (theme->variant == THEME_VARIANT_DARK) ? 0x000000 : 0xFFFFFF;
    XSetWindowBackground(DefaultDisplay, portal->frame_window, bg);

    // Resize the Cairo surface.
    cairo_xlib_surface_set_size(cairo_get_target(cr), width, height);

    // Clear the frame with transparency to avoid artifacts.
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    // Draw title bar. Corner rounding is handled by the compositor's
    // clip path, so a plain rectangle is sufficient here.
    cairo_set_source_rgb(cr, theme->titlebar_bg.r, theme->titlebar_bg.g, theme->titlebar_bg.b);
    cairo_rectangle(cr, 0, 0, width, PORTAL_TITLE_BAR_HEIGHT);
    cairo_fill(cr);

    // Draw focus indicator: filled if focused, outlined if not.
    cairo_set_source_rgba(cr,
        theme->titlebar_text.r,
        theme->titlebar_text.g,
        theme->titlebar_text.b,
        0.5
    );
    double indicator_radius = 3.0;
    double indicator_x = 10.0 + indicator_radius;
    double indicator_y = PORTAL_TITLE_BAR_HEIGHT / 2.0;
    cairo_arc(cr, indicator_x, indicator_y, indicator_radius, 0, 2 * PI);
    if (portal == get_focused_portal())
    {
        cairo_fill(cr);
    }
    else
    {
        cairo_set_line_width(cr, 1.0);
        cairo_stroke(cr);
    }

    // Draw title within the title bar.
    draw_portal_title(portal);

    // Draw triggers within the title bar.
    draw_portal_triggers(portal);
}

bool is_portal_frame_valid(Portal *portal)
{
    return (
        portal != NULL &&
        portal->frame_window != 0 &&
        x_window_exists(DefaultDisplay, portal->frame_window)
    );
}

int destroy_portal_frame(Portal *portal)
{
    // Destroy the Cairo context and surface.
    cairo_surface_t *surface = cairo_get_target(portal->frame_cr);
    cairo_destroy(portal->frame_cr);
    cairo_surface_destroy(surface);
    portal->frame_cr = NULL;

    // Destroy the frame window.
    int status = XDestroyWindow(DefaultDisplay, portal->frame_window);
    if (status == 0) return -1;
    portal->frame_window = 0;

    return 0;
}

bool is_portal_frame_area(Portal *portal, int rel_x, int rel_y)
{
    // Check if the position is within the title bar area.
    return (rel_x >= 0 &&
            rel_x < (int)portal->geometry.width &&
            rel_y >= 0 &&
            rel_y < PORTAL_TITLE_BAR_HEIGHT);
}

HANDLE(PortalFocused)
{
    // Redraw each portal's frame to update focus indicator.
    unsigned int count;
    Portal **portals = get_sorted_portals(&count);
    for (unsigned int i = 0; i < count; i++)
    {
        if (portals[i] == NULL) continue;
        if (is_portal_frame_valid(portals[i]))
        {
            draw_portal_frame(portals[i]);
        }
    }
}

HANDLE(Expose)
{
    XExposeEvent *_event = &event->xexpose;

    // Ensure this is the last expose event in the sequence.
    if (_event->count > 0) return;

    // Find the portal associated with the event window.
    Portal *portal = find_portal_by_window(_event->window);
    if (portal == NULL) return;

    // Ensure the event window is a portal frame window.
    if (_event->window != portal->frame_window) return;

    // Redraw the portal frame.
    draw_portal_frame(portal);
}
