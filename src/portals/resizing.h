#pragma once
#include "../all.h"

/**
 * The thickness of the resize border in pixels.
 *
 * This defines the area near portal edges where resizing can be initiated.
 */
#define PORTAL_RESIZE_BORDER_SIZE 5

/**
 * A type representing the edge or corner of a portal where resizing can occur.
 */
typedef enum {
    PORTAL_RESIZE_EDGE_NONE,
    PORTAL_RESIZE_EDGE_TOP,
    PORTAL_RESIZE_EDGE_BOTTOM,
    PORTAL_RESIZE_EDGE_LEFT,
    PORTAL_RESIZE_EDGE_RIGHT,
    PORTAL_RESIZE_EDGE_TOP_LEFT,
    PORTAL_RESIZE_EDGE_TOP_RIGHT,
    PORTAL_RESIZE_EDGE_BOTTOM_LEFT,
    PORTAL_RESIZE_EDGE_BOTTOM_RIGHT
} PortalResizeEdge;

/**
 * Determines which resize edge or corner the pointer is hovering over.
 *
 * @param portal The portal to check.
 * @param x_portal The X coordinate relative to the portal.
 * @param y_portal The Y coordinate relative to the portal.
 *
 * @return The edge or corner the pointer is over, or PORTAL_RESIZE_EDGE_NONE.
 */
PortalResizeEdge get_portal_resize_edge(Portal *portal, int x_portal, int y_portal);

/**
 * Checks whether a portal resize operation is currently in progress.
 *
 * @return - `True (1)` A resize operation is in progress.
 * @return - `False (0)` No resize operation is in progress.
 */
bool is_portal_resizing();
