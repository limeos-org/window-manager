#include "../all.h"

static void calc_portal_button_pos(Portal *portal, PortalButtonType type, int *out_x, int *out_y)
{
    // Calculate starting position.
    int x = portal->width - PORTAL_BUTTON_PADDING - PORTAL_BUTTON_SIZE;
    int y = (PORTAL_TITLE_BAR_HEIGHT - PORTAL_BUTTON_SIZE) / 2;
    
    // Calculate the position based on the button type.
    if(type == BUTTON_CLOSE)
    {
        *out_x = x;
        *out_y = y;
    }
    if(type == BUTTON_ARRANGE)
    {
        *out_x = x - PORTAL_BUTTON_SIZE - PORTAL_BUTTON_PADDING;
        *out_y = y;
    }
}

static bool is_portal_button_area(Portal *portal, PortalButtonType type, int mouse_rel_x, int mouse_rel_y)
{
    int button_x, button_y;
    calc_portal_button_pos(portal, type, &button_x, &button_y);

    return (mouse_rel_x >= button_x && 
            mouse_rel_x <= button_x + PORTAL_BUTTON_SIZE &&
            mouse_rel_y >= button_y && 
            mouse_rel_y <= button_y + PORTAL_BUTTON_SIZE);
}

static void draw_portal_button(Portal *portal, PortalButtonType type)
{
    cairo_t *cr = portal->frame_cr;

    int button_x, button_y;
    calc_portal_button_pos(portal, type, &button_x, &button_y);

    // Define the button stroke style.
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, 2);

    // Define the button path based on the button type.
    if (type == BUTTON_CLOSE)
    {
        cairo_move_to(cr,
            button_x + PORTAL_BUTTON_PADDING,
            button_y + PORTAL_BUTTON_PADDING);
        cairo_line_to(cr,
            button_x + PORTAL_BUTTON_SIZE - PORTAL_BUTTON_PADDING, 
            button_y + PORTAL_BUTTON_SIZE - PORTAL_BUTTON_PADDING);
        cairo_move_to(cr,
            button_x + PORTAL_BUTTON_SIZE - PORTAL_BUTTON_PADDING, 
            button_y + PORTAL_BUTTON_PADDING);
        cairo_line_to(cr,
            button_x + PORTAL_BUTTON_PADDING, 
            button_y + PORTAL_BUTTON_SIZE - PORTAL_BUTTON_PADDING);
    }
    if (type == BUTTON_ARRANGE)
    {
        cairo_rectangle(cr,
            button_x + PORTAL_BUTTON_PADDING, 
            button_y + PORTAL_BUTTON_PADDING,
            (PORTAL_BUTTON_SIZE - (2 * PORTAL_BUTTON_PADDING)) * 1.2,
            PORTAL_BUTTON_SIZE - (2 * PORTAL_BUTTON_PADDING));
    }

    // Render the defined path.
    cairo_stroke(cr);
}

void draw_portal_buttons(Portal *portal)
{
    draw_portal_button(portal, BUTTON_CLOSE);
    // draw_portal_button(portal, BUTTON_ARRANGE);
}

HANDLE(GlobalButtonPress)
{
    XButtonEvent *_event = &event->xbutton;

    if (_event->button != Button1) return;

    Portal *portal = find_portal(_event->window);
    if(portal == NULL) return;

    if(is_portal_frame_area(portal, _event->x, _event->y) == false) return;

    if(is_portal_button_area(portal, BUTTON_CLOSE, _event->x, _event->y))
    {
        destroy_portal(portal);
    }
    if(is_portal_button_area(portal, BUTTON_ARRANGE, _event->x, _event->y))
    {
        // TODO: Implement the arrange button.
    }
}
