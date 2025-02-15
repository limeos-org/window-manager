#include "../all.h"

bool should_portal_be_framed(Portal *portal)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
    Window client_window = portal->client_window;

    // ICCCM (Section 4.1.4) states that window managers should decorate 
    // top-level windows, which are usually direct children of the root 
    // window. ICCCM (Section 4.1.1) defines a top-level window as one that 
    // is a direct child of the root window and does not have the
    // override_redirect attribute set to true.

    // Retrieve the parent window of the client window.
    Window client_parent_window = x_get_window_parent(display, client_window);

    // Retrieve the client window attributes.
    XWindowAttributes client_attributes;
    if (XGetWindowAttributes(display, client_window, &client_attributes) == 0) {
        LOG_WARNING(
            "Could not properly determine whether portal should be framed, "
            "client window (0x%lx) attributes unavailable. Falling back to "
            "`false` (Don't frame).",
            client_window
        );
        return false;
    }

    // Check if the client window meets the criteria for being framed.
    if (client_parent_window == root_window && client_attributes.override_redirect == False) {
        return true;
    }

    return false;
}

void create_portal_frame(Portal *portal)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
    int x_root = portal->x_root;
    int y_root = portal->y_root;
    unsigned int width = portal->width;
    unsigned int height = portal->height;

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
        0               // Background color
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

    // Reparent the client window to the frame window.
    XReparentWindow(
        display,                // Display
        portal->client_window,  // Window
        portal->frame_window,   // Parent window
        0,                      // X (Relative to parent)
        PORTAL_TITLE_BAR_HEIGHT // Y (Relative to parent)
    );
}

void draw_portal_frame(Portal *portal)
{
    cairo_t *cr = portal->frame_cr;
    unsigned int width = portal->width;
    unsigned int height = portal->height;

    // Resize the Cairo surface.
    cairo_xlib_surface_set_size(cairo_get_target(cr), width, height);

    // Draw title bar.
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_rectangle(cr, 0, 0, width, PORTAL_TITLE_BAR_HEIGHT);
    cairo_fill(cr);

    // Draw title within the title bar.
    /* draw_portal_title(portal); */ // TODO: Bring back.

    // Draw triggers within the title bar.
    draw_portal_triggers(portal);

    // Draw the border around the window.
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);
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
