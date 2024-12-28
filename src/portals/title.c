#include "../all.h"

static void set_portal_title(Portal *portal, const char *title)
{
    free(portal->title);
    portal->title = strdup(title);
}

void draw_portal_title(Portal *portal)
{
    cairo_t *cr = portal->frame_cr;
    unsigned int width = portal->width;

    // Set the font and color.
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 10.0);

    // Determine the position for the title text.
    cairo_text_extents_t title_extents;
    cairo_text_extents(cr, portal->title, &title_extents);
    double title_x = (width - title_extents.width) / 2;
    double title_y = (PORTAL_TITLE_BAR_HEIGHT + title_extents.height) / 2;
    cairo_move_to(cr, title_x, title_y);

    // Draw the title text.
    cairo_show_text(cr, portal->title);
}

HANDLE(PropertyNotify)
{
    XPropertyEvent *_event = &event->xproperty;

    // Ensure the property change is related to a portal.
    Portal *portal = find_portal(_event->window);
    if(portal == NULL) return;

    // Ensure the property change is related to the window title.
    Atom net_wm_name = XInternAtom(display, "_NET_WM_NAME", False);
    Atom wm_name = XInternAtom(display, "WM_NAME", False);
    if (_event->atom != wm_name && _event->atom != net_wm_name) return;

    // Retrieve the client window title, and update the portal title.
    char title[256];
    if(x_get_window_name(display, portal->client_window, title, sizeof(title)) == 0)
    {
        set_portal_title(portal, title);
        draw_portal_frame(portal);
    }
}
