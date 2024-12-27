#pragma once
#include "../all.h"

#define PORTAL_BUTTON_SIZE 15
#define PORTAL_BUTTON_PADDING 5

typedef enum {
    BUTTON_CLOSE,
    BUTTON_ARRANGE
} PortalButtonType;

#ifdef STATIC

/**
 * Calculates where the position of a portal button should be.
 * 
 * @param portal The portal to calculate the button position for.
 * @param type Type of portal button (E.g `BUTTON_CLOSE`, `BUTTON_ARRANGE`).
 * @param out_x Pointer to store the calculated x coordinate.
 * @param out_y Pointer to store the calculated y coordinate.
 */
static void calc_portal_button_pos(Portal *portal, PortalButtonType type, int *out_x, int *out_y);

/**
 * Checks if the mouse is within a specific buttons area.
 * 
 * @param portal The portal to check the button area for.
 * @param type Type of portal button (E.g. `BUTTON_CLOSE`, `BUTTON_ARRANGE`).
 * @param mouse_rel_x Mouse X position relative to the portal.
 * @param mouse_rel_y Mouse Y position relative to the portal.
 * 
 * @return True (1) if mouse position is within button area, False (0) otherwise.
 */
static bool is_portal_button_area(Portal *portal, PortalButtonType type, int mouse_rel_x, int mouse_rel_y);

/**
 * Draws a singular portal button.
 * 
 * @param portal The portal to draw the button for.
 * @param type Type of button to draw (E.g `BUTTON_CLOSE`, `BUTTON_ARRANGE`).
 */
static void draw_portal_button(Portal *portal, PortalButtonType type);

#endif

/**
 * Draws all portal buttons (E.g. close, arrange).
 * 
 * @param portal The portal to draw the buttons for.
 * 
 * @note Intended to be used by `draw_portal_frame()`.
 */
void draw_portal_buttons(Portal *portal);
