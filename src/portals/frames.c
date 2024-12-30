#include "../all.h"

static const long frame_event_mask =
    ExposureMask |
    SubstructureNotifyMask;

void create_portal_frame(Portal *portal, Window *out_window, cairo_t **out_cr)
{
    Display *display = portal->display;
    int x = portal->x;
    int y = portal->y;
    unsigned int width = portal->width;
    unsigned int height = portal->height;

    // Create the frame window.
    Window frame_window = XCreateSimpleWindow(
        display,
        DefaultRootWindow(display),
        x, y,
        width, height,
        2, 0x000000, 0xFFFFFF
    );

    // Choose which frame window events we should listen for.
    XSelectInput(display, frame_window, frame_event_mask);

    // Create the Cairo context for the frame window.
    cairo_surface_t *surface = cairo_xlib_surface_create(
        display,
        frame_window,
        DefaultVisual(display, 0),
        width, height
    );
    cairo_t *cr = cairo_create(surface);

    // Output the frame window and Cairo context.
    *out_window = frame_window;
    *out_cr = cr;
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
    draw_portal_title(portal);

    // Draw buttons within the title bar.
    draw_portal_buttons(portal);

    // Draw the border around the window.
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);
}

bool is_portal_frame_area(Portal *portal, int rel_x, int rel_y)
{
    (void)portal, (void)rel_x;
    return rel_y <= PORTAL_TITLE_BAR_HEIGHT;
}

bool is_portal_frame_valid(Portal *portal)
{
    return (
        portal != NULL &&
        portal->frame_window != 0 &&
        x_window_exists(portal->display, portal->frame_window)
    );
}

int destroy_portal_frame(Portal *portal)
{
    if(!is_portal_frame_valid(portal))
    {
        return -1;
    }

    // Destroy the Cairo context and surface.
    cairo_surface_t *surface = cairo_get_target(portal->frame_cr);
    cairo_destroy(portal->frame_cr);
    cairo_surface_destroy(surface);
    portal->frame_cr = NULL;

    // Destroy the frame window.
    int status = XDestroyWindow(portal->display, portal->frame_window);
    if (status == 0)
    {
        return -2;
    }

    // Clear the frame window reference from the portal.
    portal->frame_window = 0;

    return 0;
}

HANDLE(Expose)
{
    XExposeEvent *_event = &event->xexpose;

    if (_event->count > 0) return;

    Portal *portal = find_portal(_event->window);
    if (portal == NULL) return;

    if (_event->window != portal->frame_window) return;

    draw_portal_frame(portal);
}
