/**
 * This code is responsible for managing fullscreen state transitions for
 * portals, including EWMH compliance and compositor integration.
 */

#include "../all.h"

static Atom _NET_WM_STATE = None;
static Atom _NET_WM_STATE_FULLSCREEN = None;
static Atom _NET_FRAME_EXTENTS = None;

static void set_fullscreen_state(Portal *portal, bool fullscreen)
{
    Display *display = DefaultDisplay;
    Window window = portal->client_window;

    // Read current _NET_WM_STATE property.
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;
    Atom *states = NULL;
    unsigned long state_count = 0;
    if (XGetWindowProperty(
        display, window, _NET_WM_STATE, 0, 1024, False,
        XA_ATOM, &actual_type, &actual_format, &nitems,
        &bytes_after, &data) == Success && data != NULL
    ) {
        states = (Atom *)data;
        state_count = nitems;
    }

    // Check if _NET_WM_STATE_FULLSCREEN is already in the list.
    bool already_set = false;
    unsigned long existing_index = 0;
    for (unsigned long i = 0; i < state_count; i++)
    {
        if (states[i] == _NET_WM_STATE_FULLSCREEN)
        {
            already_set = true;
            existing_index = i;
            break;
        }
    }

    // Update the property based on desired fullscreen state.
    if (fullscreen && !already_set)
    {
        // Add _NET_WM_STATE_FULLSCREEN to the property.
        Atom *new_states = malloc((state_count + 1) * sizeof(Atom));
        if (new_states == NULL)
        {
            LOG_ERROR("Could not update _NET_WM_STATE, memory allocation failed.");
            if (data != NULL) XFree(data);
            return;
        }

        // Copy existing states.
        for (unsigned long i = 0; i < state_count; i++)
        {
            new_states[i] = states[i];
        }
        new_states[state_count] = _NET_WM_STATE_FULLSCREEN;

        // Update the property.
        XChangeProperty(
            display, window, _NET_WM_STATE, XA_ATOM, 32,
            PropModeReplace, (unsigned char *)new_states,
            state_count + 1
        );
        free(new_states);
    }
    else if (!fullscreen && already_set)
    {
        // Remove _NET_WM_STATE_FULLSCREEN from the property.
        Atom *new_states = malloc(state_count * sizeof(Atom));
        if (new_states == NULL)
        {
            LOG_ERROR("Could not update _NET_WM_STATE, memory allocation failed.");
            if (data != NULL) XFree(data);
            return;
        }

        // Copy all states except the existing fullscreen state.
        unsigned long new_count = 0;
        for (unsigned long i = 0; i < state_count; i++)
        {
            if (i != existing_index)
            {
                new_states[new_count++] = states[i];
            }
        }

        // Update the property.
        XChangeProperty(
            display, window, _NET_WM_STATE, XA_ATOM, 32,
            PropModeReplace, (unsigned char *)new_states,
            new_count
        );
        free(new_states);
    }

    // Cleanup.
    if (data != NULL) XFree(data);
}

void enter_portal_fullscreen(Portal *portal)
{
    if (portal == NULL || portal->fullscreen) return;

    Display *display = DefaultDisplay;
    int screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);
    bool has_frame = is_portal_frame_valid(portal);

    // Grab server to ensure atomic state change.
    XGrabServer(display);

    // Back up current portal geometry for restore.
    portal->geometry_backup = portal->geometry;

    // Redirect the client window for direct compositing.
    // When reparented to a frame, the client is no longer a direct child of 
    // root, so XCompositeRedirectSubwindows on root doesn't affect it.
    XCompositeRedirectWindow(display, portal->client_window, CompositeRedirectManual);

    if (has_frame)
    {
        // Move frame to cover the entire screen.
        XMoveResizeWindow(
            display, portal->frame_window, 0, 0,
            screen_width, screen_height
        );

        // Move client to (0, 0) within the frame (remove title bar offset).
        XMoveResizeWindow(
            display, portal->client_window, 0, 0,
            screen_width, screen_height
        );
    }
    else
    {
        // No frame, move and resize the client window to cover the screen.
        XMoveResizeWindow(
            display, portal->client_window, 0, 0,
            screen_width, screen_height
        );
    }

    // Mark portal as fullscreen and update portal geometry to match screen.
    portal->fullscreen = true;
    portal->geometry.x_root = 0;
    portal->geometry.y_root = 0;
    portal->geometry.width = screen_width;
    portal->geometry.height = screen_height;

    // Update _NET_FRAME_EXTENTS to indicate no decorations in fullscreen.
    if (has_frame)
    {
        unsigned long extents[4] = {0, 0, 0, 0};
        XChangeProperty(
            display, portal->client_window, _NET_FRAME_EXTENTS,
            XA_CARDINAL, 32, PropModeReplace,
            (unsigned char *)extents, 4
        );
    }

    // Update _NET_WM_STATE property to inform the client.
    set_fullscreen_state(portal, true);

    // Raise portal to top of stacking order.
    raise_portal(portal);

    XUngrabServer(display);

    // Send synthetic ConfigureNotify to inform client of new geometry.
    // Per ICCCM, WM must send this when resizing a reparented client window.
    XSendEvent(
        display, portal->client_window, False, StructureNotifyMask,
        (XEvent *)&(XConfigureEvent){
            .type = ConfigureNotify,
            .display = display,
            .event = portal->client_window,
            .window = portal->client_window,
            .x = 0,
            .y = 0,
            .width = screen_width,
            .height = screen_height,
            .border_width = 0,
            .above = None,
            .override_redirect = False
        }
    );

    XSync(display, False);
}

void exit_portal_fullscreen(Portal *portal)
{
    if (portal == NULL || !portal->fullscreen) return;

    Display *display = DefaultDisplay;
    bool has_frame = is_portal_frame_valid(portal);

    // Grab server to ensure atomic state change.
    XGrabServer(display);

    if (has_frame)
    {
        // Calculate client dimensions from backed up portal geometry.
        unsigned int client_width = portal->geometry_backup.width;
        unsigned int client_height = portal->geometry_backup.height - PORTAL_TITLE_BAR_HEIGHT;

        // Restore frame position and size.
        XMoveResizeWindow(
            display, portal->frame_window,
            portal->geometry_backup.x_root, portal->geometry_backup.y_root,
            portal->geometry_backup.width, portal->geometry_backup.height
        );

        // Restore client position within frame (below title bar) and size.
        XMoveResizeWindow(
            display, portal->client_window,
            0, PORTAL_TITLE_BAR_HEIGHT,
            client_width, client_height
        );

        // Update portal geometry.
        portal->geometry = portal->geometry_backup;
    }
    else
    {
        // Restore client window to backed up position and dimensions.
        XMoveResizeWindow(
            display, portal->client_window,
            portal->geometry_backup.x_root, portal->geometry_backup.y_root,
            portal->geometry_backup.width, portal->geometry_backup.height
        );

        // Update portal geometry.
        portal->geometry = portal->geometry_backup;
    }

    // Unredirect the client window (restore normal frame-based compositing).
    XCompositeUnredirectWindow(display, portal->client_window, CompositeRedirectManual);

    // Restore _NET_FRAME_EXTENTS to indicate decorations are back.
    if (has_frame)
    {
        unsigned long extents[4] = {0, 0, PORTAL_TITLE_BAR_HEIGHT, 0};
        XChangeProperty(
            display, portal->client_window, _NET_FRAME_EXTENTS,
            XA_CARDINAL, 32, PropModeReplace,
            (unsigned char *)extents, 4
        );
    }

    // Calculate client geometry for ConfigureNotify.
    int restored_x = portal->geometry_backup.x_root;
    int restored_y = portal->geometry_backup.y_root + (has_frame ? PORTAL_TITLE_BAR_HEIGHT : 0);
    unsigned int restored_width = portal->geometry_backup.width;
    unsigned int restored_height = has_frame
        ? portal->geometry_backup.height - PORTAL_TITLE_BAR_HEIGHT
        : portal->geometry_backup.height;

    // Clear fullscreen state and geometry backup.
    portal->fullscreen = false;
    portal->geometry_backup = (PortalGeometry){0, 0, 0, 0};

    // Update _NET_WM_STATE property to inform the client.
    set_fullscreen_state(portal, false);

    XUngrabServer(display);

    // Send synthetic ConfigureNotify to inform client of restored geometry.
    XSendEvent(
        display, portal->client_window, False, StructureNotifyMask,
        (XEvent *)&(XConfigureEvent){
            .type = ConfigureNotify,
            .display = display,
            .event = portal->client_window,
            .window = portal->client_window,
            .x = restored_x,
            .y = restored_y,
            .width = restored_width,
            .height = restored_height,
            .border_width = 0,
            .above = None,
            .override_redirect = False
        }
    );

    XSync(display, False);
}

HANDLE(Prepare)
{
    Display *display = DefaultDisplay;
    _NET_WM_STATE = XInternAtom(display, "_NET_WM_STATE", False);
    _NET_WM_STATE_FULLSCREEN = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    _NET_FRAME_EXTENTS = XInternAtom(display, "_NET_FRAME_EXTENTS", False);
}

HANDLE(ClientMessage)
{
    XClientMessageEvent *xclient = &event->xclient;

    if (xclient->message_type != _NET_WM_STATE) return;

    // Parse the _NET_WM_STATE request.
    // data.l[0] = action: 0 = remove, 1 = add, 2 = toggle.
    // data.l[1] = first property to alter.
    // data.l[2] = second property to alter (or 0).
    long action = xclient->data.l[0];
    Atom prop1 = xclient->data.l[1];
    Atom prop2 = xclient->data.l[2];

    // Ensure fullscreen state is being requested.
    bool is_fullscreen_request = (
        prop1 == _NET_WM_STATE_FULLSCREEN ||
        prop2 == _NET_WM_STATE_FULLSCREEN
    );
    if (!is_fullscreen_request) return;

    // Find the portal for this window.
    Portal *portal = find_portal_by_window(xclient->window);
    if (portal == NULL) return;

    // Determine new fullscreen state.
    bool currently_fullscreen = portal->fullscreen;
    bool should_be_fullscreen;
    switch (action)
    {
        case 0: // _NET_WM_STATE_REMOVE
            should_be_fullscreen = false;
            break;
        case 1: // _NET_WM_STATE_ADD
            should_be_fullscreen = true;
            break;
        case 2: // _NET_WM_STATE_TOGGLE
            should_be_fullscreen = !currently_fullscreen;
            break;
        default:
            return;
    }

    // Enter or exit fullscreen.
    if (should_be_fullscreen && !currently_fullscreen)
    {
        enter_portal_fullscreen(portal);
    }
    else if (!should_be_fullscreen && currently_fullscreen)
    {
        exit_portal_fullscreen(portal);
    }
}

HANDLE(PortalInitialized)
{
    PortalInitializedEvent *_event = &event->portal_initialized;
    Portal *portal = _event->portal;

    // Check if client requested fullscreen before mapping via _NET_WM_STATE.
    Display *display = DefaultDisplay;
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;

    if (XGetWindowProperty(
        display, portal->client_window, _NET_WM_STATE, 0, 1024, False,
        XA_ATOM, &actual_type, &actual_format, &nitems,
        &bytes_after, &data) == Success && data != NULL
    ) {
        Atom *states = (Atom *)data;
        for (unsigned long i = 0; i < nitems; i++)
        {
            if (states[i] == _NET_WM_STATE_FULLSCREEN)
            {
                enter_portal_fullscreen(portal);
                break;
            }
        }
        XFree(data);
    }
}
