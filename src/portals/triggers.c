/**
 * This code is responsible for portal trigger buttons.
 *
 * It handles rendering and interaction with title bar buttons such as
 * close and arrange triggers.
 */

#include "../all.h"

/** The size of a trigger button in pixels. */
#define PORTAL_TRIGGER_SIZE 16

/** The padding between trigger buttons in pixels. */
#define PORTAL_TRIGGER_PADDING 6

/** The size of the icon within a trigger button in pixels. */
#define PORTAL_TRIGGER_ICON_SIZE 6

typedef enum {
    TRIGGER_CLOSE,
    TRIGGER_ARRANGE
} PortalTriggerType;

static void calc_portal_trigger_pos(Portal *portal, PortalTriggerType type, int *out_x, int *out_y)
{
    // Calculate starting position.
    int x = portal->geometry.width - PORTAL_TRIGGER_PADDING - PORTAL_TRIGGER_SIZE;
    int y = (PORTAL_TITLE_BAR_HEIGHT - PORTAL_TRIGGER_SIZE) / 2;

    // Adjust based on trigger type.
    if (type == TRIGGER_CLOSE)
    {
        *out_x = x;
        *out_y = y;
    }
    if (type == TRIGGER_ARRANGE)
    {
        *out_x = x - PORTAL_TRIGGER_SIZE - PORTAL_TRIGGER_PADDING;
        *out_y = y;
    }
}

static bool is_portal_trigger_area(Portal *portal, PortalTriggerType type, int rel_x, int rel_y)
{
    // Calculate the trigger position.
    int trigger_x, trigger_y;
    calc_portal_trigger_pos(portal, type, &trigger_x, &trigger_y);

    // Determine whether the mouse is within the trigger area.
    return (rel_x >= trigger_x && 
            rel_x <= trigger_x + PORTAL_TRIGGER_SIZE &&
            rel_y >= trigger_y && 
            rel_y <= trigger_y + PORTAL_TRIGGER_SIZE);
}

static void draw_portal_trigger(Portal *portal, PortalTriggerType type)
{
    cairo_t *cr = portal->frame_cr;
    const Theme *theme = get_portal_theme(portal);

    // Calculate trigger position.
    int trigger_x, trigger_y;
    calc_portal_trigger_pos(portal, type, &trigger_x, &trigger_y);

    // Define the drawing stroke style.
    cairo_set_source_rgb(cr,
        theme->titlebar_text.r,
        theme->titlebar_text.g,
        theme->titlebar_text.b
    );

    // Define the drawing path based on the trigger type.
    if (type == TRIGGER_CLOSE)
    {
        // Center the X within the trigger area.
        int offset = (PORTAL_TRIGGER_SIZE - PORTAL_TRIGGER_ICON_SIZE) / 2;
        cairo_set_line_width(cr, 1);
        cairo_move_to(cr,
            trigger_x + offset,
            trigger_y + offset);
        cairo_line_to(cr,
            trigger_x + offset + PORTAL_TRIGGER_ICON_SIZE,
            trigger_y + offset + PORTAL_TRIGGER_ICON_SIZE);
        cairo_move_to(cr,
            trigger_x + offset + PORTAL_TRIGGER_ICON_SIZE,
            trigger_y + offset);
        cairo_line_to(cr,
            trigger_x + offset,
            trigger_y + offset + PORTAL_TRIGGER_ICON_SIZE);
    }
    if (type == TRIGGER_ARRANGE)
    {
        // TODO: Implement arrange trigger drawing.
    }

    // Draw the defined path.
    cairo_stroke(cr);
}

void draw_portal_triggers(Portal *portal)
{
    draw_portal_trigger(portal, TRIGGER_CLOSE);
    // draw_portal_trigger(portal, TRIGGER_ARRANGE);
}

bool is_portal_triggers_area(Portal *portal, int rel_x, int rel_y)
{
    if (is_portal_trigger_area(portal, TRIGGER_CLOSE, rel_x, rel_y))
    {
        return true;
    }
    // if (is_portal_trigger_area(portal, TRIGGER_ARRANGE, rel_x, rel_y))
    // {
    //     return true;
    // }
    return false;
}

void handle_trigger_click(Portal *portal, int rel_x, int rel_y)
{
    // Handle the event based on the trigger type.
    if (is_portal_trigger_area(portal, TRIGGER_CLOSE, rel_x, rel_y))
    {
        destroy_portal(portal);
        return;
    }
    if (is_portal_trigger_area(portal, TRIGGER_ARRANGE, rel_x, rel_y))
    {
        // arrange_portals();
        // return;
    }
}
