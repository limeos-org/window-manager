#include "../all.h"

Portal *portals = NULL;
int portals_count = 0;

static Portal *register_portal(
    Display *display,
    const char *title,
    Window frame_window,
    cairo_t *frame_cr,
    Window client_window,
    int x, int y,
    unsigned int width, unsigned int height
)
{
    // Allocate additional memory for the portals registry.
    Portal *buffer = realloc(portals, (portals_count + 1) * sizeof(Portal));
    portals = buffer;

    // Allocate memory for the title string.
    char *allocated_title = strdup(title);

    // Set the new portal properties.
    portals[portals_count].display = display;
    portals[portals_count].title = allocated_title;
    portals[portals_count].frame_window = frame_window;
    portals[portals_count].frame_cr = frame_cr;
    portals[portals_count].client_window = client_window;
    portals[portals_count].x = x;
    portals[portals_count].y = y;
    portals[portals_count].width = width;
    portals[portals_count].height = height;

    portals_count++;

    return &portals[portals_count - 1];
}

static void unregister_portal(Portal *portal)
{
    // Find the index of the portal in the array.
    int index = -1;
    for (int i = 0; i < portals_count; i++)
    {
        if (&portals[i] == portal)
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        LOG_ERROR("Attempted to unregister a portal that could not be found in the registry.");
        return;
    }

    // Free the allocated memory for the title.
    free(portals[index].title);

    // Shift all elements after the found index to the left.
    for (int i = index; i < portals_count - 1; i++)
    {
        portals[i] = portals[i + 1];
    }

    // Resize the array to the new size.
    portals_count--;
    Portal *buffer = realloc(portals, (portals_count + 1) * sizeof(Portal));
    portals = buffer;
}

Portal *create_portal(Display *display, Window client_window)
{
    XWindowAttributes client_attr;
    if(XGetWindowAttributes(display, client_window, &client_attr) == 0)
    {
        LOG_ERROR("Failed to get client window attributes while attempting to create a portal.");
        return NULL;
    }

    // Calculate the portal dimensions.
    int portal_x = client_attr.x;
    int portal_y = client_attr.y;
    unsigned int portal_width = client_attr.width;
    unsigned int portal_height = client_attr.height + PORTAL_TITLE_BAR_HEIGHT;

    // Ensure that the portal dimensions are within the limits.
    Screen *screen = DefaultScreenOfDisplay(display);
    int screen_width = screen->width;
    int screen_height = screen->height;
    if(portal_width < MINIMUM_PORTAL_WIDTH)
    {
        portal_width = MINIMUM_PORTAL_WIDTH;
    }
    if(portal_height < MINIMUM_PORTAL_HEIGHT)
    {
        portal_height = MINIMUM_PORTAL_HEIGHT;
    }
    if(portal_width > screen_width * MAXIMUM_INITIAL_PORTAL_WIDTH_PERCENT)
    {
        portal_width = screen_width * MAXIMUM_INITIAL_PORTAL_WIDTH_PERCENT;
    }
    if(portal_height > screen_height * MAXIMUM_INITIAL_PORTAL_HEIGHT_PERCENT)
    {
        portal_height = screen_height * MAXIMUM_INITIAL_PORTAL_HEIGHT_PERCENT;
    }
    XResizeWindow(display, client_window, portal_width, portal_height - PORTAL_TITLE_BAR_HEIGHT);

    // Get the client window name.
    char title[256];
    x_get_window_name(display, client_window, title, sizeof(title));

    // Register the portal and create the frame.
    Portal *portal = register_portal(
        display,
        title,
        0, // frame_window, will be set later.
        NULL, // frame_cr, will be set later.
        client_window,
        portal_x, portal_y,
        portal_width, portal_height
    );
    create_portal_frame(portal, &portal->frame_window, &portal->frame_cr);
    Window frame_window = portal->frame_window;

    // Reparent the client window to the frame window.
    XAddToSaveSet(display, client_window);
    XReparentWindow(display, client_window, frame_window, 0, PORTAL_TITLE_BAR_HEIGHT);

    // Map the frame and client windows.
    XMapWindow(display, frame_window);
    XMapWindow(display, client_window);

    return portal;
}

void destroy_portal(Portal *portal)
{
    destroy_portal_client(portal);
    destroy_portal_frame(portal);
    unregister_portal(portal);
}

Portal *find_portal(Window window)
{
    for (int i = 0; i < portals_count; i++)
    {
        if (portals[i].frame_window == window ||
            portals[i].client_window == window)
        {
            return &portals[i];
        }
    }
    return NULL;
}

bool is_portal_frame_area(Portal *portal, int rel_x, int rel_y)
{
    (void)portal, (void)rel_x;
    return rel_y <= PORTAL_TITLE_BAR_HEIGHT;
}

bool is_portal_client_area(Portal *portal, int rel_x, int rel_y)
{
    (void)portal, (void)rel_x;
    return rel_y > PORTAL_TITLE_BAR_HEIGHT;
}

HANDLE(MapRequest)
{
    XMapRequestEvent *_event = &event->xmaprequest;

    // Considering a MapRequest event can only be sent by clients, we can
    // safely assume that the window is a client window.
    Window client_window = _event->window;

    // Make sure the client window does not already belong to a portal.
    Portal *portal = find_portal(client_window);
    if (portal != NULL) return;

    create_portal(display, client_window);
}

HANDLE(DestroyNotify)
{
    XDestroyWindowEvent *_event = &event->xdestroywindow;

    // The source of a DestroyNotify event is not guaranteed to be a portal
    // related window, so we must check whether it is or not before proceeding.
    Portal *portal = find_portal(_event->window);
    if (portal == NULL) return;

    destroy_portal(portal);
}
