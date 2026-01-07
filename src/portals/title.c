#include "../all.h"

/**
 * This code is responsible for rendering portal titles within the title bar
 * and handling dynamic title updates when window names change.
 */

void draw_portal_title(Portal *portal)
{
    // Ensure the portal has a valid frame.
    if (!is_portal_frame_valid(portal)) return;
    if (portal->title == NULL) return;

    cairo_t *cr = portal->frame_cr;
    unsigned int width = portal->width;

    // Calculate available width for title text.
    int available_width = (int)width - PORTAL_TITLE_PADDING_LEFT - PORTAL_TITLE_PADDING_RIGHT;
    if (available_width <= 0) return;

    // Save the Cairo state.
    cairo_save(cr);

    // Set up clipping region to prevent text overflow.
    cairo_rectangle(cr, 
        PORTAL_TITLE_PADDING_LEFT, 
        0, 
        available_width, 
        PORTAL_TITLE_BAR_HEIGHT
    );
    cairo_clip(cr);

    // Set up the font.
    cairo_select_font_face(cr, "Sans", 
        CAIRO_FONT_SLANT_NORMAL, 
        CAIRO_FONT_WEIGHT_NORMAL
    );
    cairo_set_font_size(cr, 11);

    // Set text color.
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);

    // Calculate vertical position to center the text.
    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);
    double text_y = (PORTAL_TITLE_BAR_HEIGHT + font_extents.ascent - font_extents.descent) / 2;

    // Draw the title text.
    cairo_move_to(cr, PORTAL_TITLE_PADDING_LEFT, text_y);
    cairo_show_text(cr, portal->title);

    // Restore the Cairo state.
    cairo_restore(cr);
}

void update_portal_title(Portal *portal)
{
    Display *display = DefaultDisplay;

    // Ensure the portal is valid.
    if (portal == NULL) return;
    if (portal->client_window == 0) return;

    // Get the new title from the client window.
    char new_title[256] = "Untitled";
    x_get_window_name(display, portal->client_window, new_title, sizeof(new_title));

    // Update the portal title if it changed.
    if (portal->title == NULL || strcmp(portal->title, new_title) != 0)
    {
        free(portal->title);
        portal->title = strdup(new_title);

        // Redraw the frame to show the new title.
        if (is_portal_frame_valid(portal))
        {
            draw_portal_frame(portal);
        }
    }
}

HANDLE(PropertyNotify)
{
    XPropertyEvent *_event = &event->xproperty;
    Display *display = DefaultDisplay;

    // Find the portal associated with the event window.
    Portal *portal = find_portal_by_window(_event->window);
    if (portal == NULL) return;

    // Check if this is a title-related property change.
    Atom wm_name = XInternAtom(display, "WM_NAME", False);
    Atom net_wm_name = XInternAtom(display, "_NET_WM_NAME", False);

    if (_event->atom == wm_name || _event->atom == net_wm_name)
    {
        update_portal_title(portal);
    }
}
