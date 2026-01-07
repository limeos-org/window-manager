#include "../all.h"

/**
 * This code is responsible for handling portal resizing operations, including
 * detecting resize edges, tracking resize state, and updating portal geometry
 * in response to mouse movements.
 */

// State variables for tracking the current resize operation.
static Portal *resizing_portal = NULL;
static PortalResizeEdge resize_edge = PORTAL_RESIZE_EDGE_NONE;
static int resize_start_x = 0;
static int resize_start_y = 0;
static int original_x = 0;
static int original_y = 0;
static unsigned int original_width = 0;
static unsigned int original_height = 0;

PortalResizeEdge get_portal_resize_edge(Portal *portal, int x_portal, int y_portal)
{
    // Ensure the portal is valid.
    if (portal == NULL) return PORTAL_RESIZE_EDGE_NONE;

    int width = (int)portal->width;
    int height = (int)portal->height;
    int border = PORTAL_RESIZE_BORDER_SIZE;

    // Determine which edges the pointer is near.
    bool near_left = (x_portal >= 0 && x_portal < border);
    bool near_right = (x_portal > width - border && x_portal <= width);
    bool near_top = (y_portal >= 0 && y_portal < border);
    bool near_bottom = (y_portal > height - border && y_portal <= height);

    // Return the appropriate edge or corner.
    if (near_top && near_left) return PORTAL_RESIZE_EDGE_TOP_LEFT;
    if (near_top && near_right) return PORTAL_RESIZE_EDGE_TOP_RIGHT;
    if (near_bottom && near_left) return PORTAL_RESIZE_EDGE_BOTTOM_LEFT;
    if (near_bottom && near_right) return PORTAL_RESIZE_EDGE_BOTTOM_RIGHT;
    if (near_top) return PORTAL_RESIZE_EDGE_TOP;
    if (near_bottom) return PORTAL_RESIZE_EDGE_BOTTOM;
    if (near_left) return PORTAL_RESIZE_EDGE_LEFT;
    if (near_right) return PORTAL_RESIZE_EDGE_RIGHT;

    return PORTAL_RESIZE_EDGE_NONE;
}

bool is_portal_resizing()
{
    return resizing_portal != NULL;
}

static void start_resize(Portal *portal, PortalResizeEdge edge, int x_root, int y_root)
{
    // Store the resize state.
    resizing_portal = portal;
    resize_edge = edge;
    resize_start_x = x_root;
    resize_start_y = y_root;
    original_x = portal->x_root;
    original_y = portal->y_root;
    original_width = portal->width;
    original_height = portal->height;
}

static void end_resize()
{
    // Clear the resize state.
    resizing_portal = NULL;
    resize_edge = PORTAL_RESIZE_EDGE_NONE;
    resize_start_x = 0;
    resize_start_y = 0;
    original_x = 0;
    original_y = 0;
    original_width = 0;
    original_height = 0;
}

static void update_resize(int x_root, int y_root)
{
    // Ensure a resize operation is in progress.
    if (resizing_portal == NULL) return;

    // Calculate the mouse movement delta.
    int delta_x = x_root - resize_start_x;
    int delta_y = y_root - resize_start_y;

    // Calculate new geometry based on which edge is being dragged.
    int new_x = original_x;
    int new_y = original_y;
    int new_width = (int)original_width;
    int new_height = (int)original_height;

    // Handle horizontal resizing.
    if (resize_edge == PORTAL_RESIZE_EDGE_LEFT ||
        resize_edge == PORTAL_RESIZE_EDGE_TOP_LEFT ||
        resize_edge == PORTAL_RESIZE_EDGE_BOTTOM_LEFT)
    {
        new_x = original_x + delta_x;
        new_width = (int)original_width - delta_x;
    }
    else if (resize_edge == PORTAL_RESIZE_EDGE_RIGHT ||
             resize_edge == PORTAL_RESIZE_EDGE_TOP_RIGHT ||
             resize_edge == PORTAL_RESIZE_EDGE_BOTTOM_RIGHT)
    {
        new_width = (int)original_width + delta_x;
    }

    // Handle vertical resizing.
    if (resize_edge == PORTAL_RESIZE_EDGE_TOP ||
        resize_edge == PORTAL_RESIZE_EDGE_TOP_LEFT ||
        resize_edge == PORTAL_RESIZE_EDGE_TOP_RIGHT)
    {
        new_y = original_y + delta_y;
        new_height = (int)original_height - delta_y;
    }
    else if (resize_edge == PORTAL_RESIZE_EDGE_BOTTOM ||
             resize_edge == PORTAL_RESIZE_EDGE_BOTTOM_LEFT ||
             resize_edge == PORTAL_RESIZE_EDGE_BOTTOM_RIGHT)
    {
        new_height = (int)original_height + delta_y;
    }

    // Enforce minimum dimensions.
    if (new_width < (int)MINIMUM_PORTAL_WIDTH)
    {
        if (resize_edge == PORTAL_RESIZE_EDGE_LEFT ||
            resize_edge == PORTAL_RESIZE_EDGE_TOP_LEFT ||
            resize_edge == PORTAL_RESIZE_EDGE_BOTTOM_LEFT)
        {
            new_x = original_x + (int)original_width - (int)MINIMUM_PORTAL_WIDTH;
        }
        new_width = (int)MINIMUM_PORTAL_WIDTH;
    }
    if (new_height < (int)MINIMUM_PORTAL_HEIGHT)
    {
        if (resize_edge == PORTAL_RESIZE_EDGE_TOP ||
            resize_edge == PORTAL_RESIZE_EDGE_TOP_LEFT ||
            resize_edge == PORTAL_RESIZE_EDGE_TOP_RIGHT)
        {
            new_y = original_y + (int)original_height - (int)MINIMUM_PORTAL_HEIGHT;
        }
        new_height = (int)MINIMUM_PORTAL_HEIGHT;
    }

    // Apply the new geometry.
    if (new_x != resizing_portal->x_root || new_y != resizing_portal->y_root)
    {
        move_portal(resizing_portal, new_x, new_y);
    }
    if ((unsigned int)new_width != resizing_portal->width ||
        (unsigned int)new_height != resizing_portal->height)
    {
        resize_portal(resizing_portal, (unsigned int)new_width, (unsigned int)new_height);
    }
}

HANDLE(PortalButtonPress)
{
    PortalButtonPressEvent *_event = &event->portal_button_press;

    // Ensure the event is a left button press.
    if (_event->button != Button1) return;

    // Ensure no other operation is in progress.
    if (is_portal_resizing()) return;
    if (is_portal_dragging()) return;

    // Determine which resize edge was clicked.
    PortalResizeEdge edge = get_portal_resize_edge(
        _event->portal,
        _event->x_portal,
        _event->y_portal
    );

    // Start resizing if an edge was clicked.
    if (edge != PORTAL_RESIZE_EDGE_NONE)
    {
        start_resize(_event->portal, edge, _event->x_root, _event->y_root);
    }
}

HANDLE(RawMotionNotify)
{
    (void)event;

    // Ensure a resize operation is in progress.
    if (!is_portal_resizing()) return;

    // Get the current pointer position.
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
    int pointer_x_root = 0, pointer_y_root = 0;
    XQueryPointer(
        display,            // Display
        root_window,        // Window
        &(Window){0},       // Root (Unused)
        &(Window){0},       // Child (Unused)
        &pointer_x_root,    // Pointer X (Relative to root)
        &pointer_y_root,    // Pointer Y (Relative to root)
        &(int){0},          // Window X (Unused)
        &(int){0},          // Window Y (Unused)
        &(unsigned int){0}  // Mask (Unused)
    );

    // Update the resize operation.
    update_resize(pointer_x_root, pointer_y_root);
}

HANDLE(RawButtonRelease)
{
    RawButtonReleaseEvent *_event = &event->raw_button_release;

    // Ensure the event is a left button release.
    if (_event->button != Button1) return;

    // End the resize operation if one is in progress.
    if (is_portal_resizing())
    {
        end_resize();
    }
}
