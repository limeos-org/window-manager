#include "../all.h"

typedef struct {
    Portal unsorted[MAX_PORTALS];
    Portal *sorted[MAX_PORTALS];
    unsigned int active_count;
} PortalRegistry;

static PortalRegistry registry = {
    .unsorted = {{.active = false}},
    .sorted = {NULL},
    .active_count = 0
};

static Portal *top_portal = NULL;

static void raise_portal_window(Portal *portal)
{
    // Determine which window to raise.
    Window target_window = (is_portal_frame_valid(portal))
        ? portal->frame_window
        : portal->client_window;

    // Raise the window.
    XRaiseWindow(DefaultDisplay, target_window);
}

void initialize_portal(Portal *portal)
{
    Display *display = DefaultDisplay;
    Window client_window = portal->client_window;

    // Remove the border from the client window, as the window manager will
    // be responsible for handling all window decorations.
    XSetWindowBorderWidth(display, client_window, 0);

    // Determine the portal title, based on the client window name.
    char portal_title[256] = "Untitled";
    x_get_window_name(display, client_window, portal_title, sizeof(portal_title));

    // Set the portal title.
    char *new_title = strdup(portal_title);
    if (new_title != NULL)
    {
        free(portal->title);
        portal->title = new_title;
    }

    // Determine whether the portal is top-level.
    portal->top_level = x_window_is_top_level(display, portal->client_window);

    // Query the client window type (e.g. tooltip, notification, normal).
    portal->client_window_type = x_get_window_type(display, portal->client_window);

    // Get the client window's geometry and visual BEFORE creating the frame.
    // This must be done while the client is still a child of root,
    // otherwise the coordinates will be wrong after reparenting.
    Window root_window = DefaultRootWindow(display);
    int client_x_root = 0, client_y_root = 0;
    unsigned int client_width = 1, client_height = 1;
    XTranslateCoordinates(
        display,        // Display
        client_window,  // Source window
        root_window,    // Reference window
        0, 0,           // Source window origin coordinates
        &client_x_root, // Translated X coordinate
        &client_y_root, // Translated Y coordinate
        &(Window){0}    // Child window (Unused)
    );
    XWindowAttributes client_attrs;
    if (XGetWindowAttributes(display, client_window, &client_attrs))
    {
        client_width = client_attrs.width;
        client_height = client_attrs.height;
        portal->client_visual = client_attrs.visual;
        portal->override_redirect = client_attrs.override_redirect;
    }

    // Create a frame for the portal, if necessary.
    if (should_portal_be_framed(portal))
    {
        // Set the portal geometry before creating the frame, so the frame
        // is created with the correct position and dimensions.
        portal->geometry.x_root = client_x_root;
        portal->geometry.y_root = client_y_root;
        portal->geometry.width = client_width;
        portal->geometry.height = client_height + PORTAL_TITLE_BAR_HEIGHT;

        create_portal_frame(portal);
    }

    // Set the portal as initialized.
    portal->initialized = true;

    // Call all event handlers of the PortalInitialized event.
    call_event_handlers((Event*)&(PortalInitializedEvent){
        .type = PortalInitialized,
        .portal = portal
    });
}

Portal *create_portal(Window client_window)
{
    // Choose which client window events we should listen for.
    XSelectInput(DefaultDisplay, client_window, SubstructureNotifyMask);

    // Find the first inactive slot in the registry.
    int slot = -1;
    for (int i = 0; i < MAX_PORTALS; i++)
    {
        if (!registry.unsorted[i].active)
        {
            slot = i;
            break;
        }
    }
    if (slot == -1)
    {
        LOG_ERROR("Could not register portal, maximum portal count reached.");
        return NULL;
    }

    // Allocate memory for the portal title.
    char *title = strdup("Untitled");
    if (title == NULL)
    {
        LOG_ERROR("Could not register portal, memory allocation failed.");
        return NULL;
    }

    // Add the portal to the registry.
    registry.unsorted[slot] = (Portal){
        .active = true,
        .title = title,
        .client_window_type = None,
        .transient_for = NULL,
        .initialized = false,
        .visibility = PORTAL_HIDDEN,
        .top_level = false,
        .fullscreen = false,
        .workspace = -1,
        .geometry = {0, 0, 1, 1},
        .geometry_backup = {0, 0, 0, 0},
        .frame_window = None,
        .frame_cr = NULL,
        .client_window = client_window,
        .client_visual = NULL,
        .frame_visual = NULL
    };

    // Store the portal in a variable for easier access.
    Portal *portal = &registry.unsorted[slot];

    // Increment active count.
    registry.active_count++;

    // Call all event handlers of the PortalCreated event.
    call_event_handlers((Event*)&(PortalCreatedEvent){
        .type = PortalCreated,
        .portal = portal
    });

    return portal;
}

void destroy_portal(Portal *portal)
{
    // Destroy the client window.
    if (is_portal_client_valid(portal))
    {
        destroy_portal_client(portal);
        if (is_portal_client_valid(portal)) return;
    }

    // Destroy the frame window.
    if (is_portal_frame_valid(portal))
    {
        destroy_portal_frame(portal);
        if (is_portal_frame_valid(portal)) return;
    }

    // Clear top portal tracking if this was it.
    if (top_portal == portal) top_portal = NULL;

    // Call all event handlers of the PortalDestroyed event.
    // This is done before unregistering so handlers can access the portal.
    call_event_handlers((Event*)&(PortalDestroyedEvent){
        .type = PortalDestroyed,
        .portal = portal
    });

    // Clear transient references to this portal.
    for (int i = 0; i < MAX_PORTALS; i++)
    {
        if (!registry.unsorted[i].active) continue;
        if (registry.unsorted[i].transient_for == portal)
        {
            registry.unsorted[i].transient_for = NULL;
        }
    }

    // Free the allocated memory for the title.
    free(portal->title);
    portal->title = NULL;

    // Mark the portal slot as inactive (tombstone).
    portal->active = false;

    // Decrease the active count.
    registry.active_count--;

    // Re-sort the portals.
    sort_portals();
}

void move_portal(Portal *portal, int x_root, int y_root)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Ensure the portal has been initialized.
    if (portal->initialized == false) return;

    // Move the portal itself.
    portal->geometry.x_root = x_root;
    portal->geometry.y_root = y_root;

    // Move the portal windows.
    {
        Window client_window = portal->client_window;
        Window frame_window = portal->frame_window;

        // Determine which window to move.
        Window target_window = (is_portal_frame_valid(portal)) ? frame_window : client_window;

        // Check if the target window still exists (may have been destroyed).
        if (!x_window_exists(display, target_window)) return;

        // The `XMoveWindow()` function expects coordinates relative to the
        // parent window. So we have to translate the provided root coordinates
        // to parent coordinates.

        // Retrieve the parent window of the target window.
        Window parent_window = x_get_window_parent(display, target_window);
        if (parent_window == None)
        {
            // Window was likely destroyed between the exists check and now.
            return;
        }

        // Calculate the parent window coordinates relative to root.
        int parent_x_root = -1, parent_y_root = -1;
        XTranslateCoordinates(
            display,        // Display
            parent_window,  // Source window
            root_window,    // Reference window
            0, 0,           // Source window origin coordinates
            &parent_x_root, // Translated X coordinate
            &parent_y_root, // Translated Y coordinate
            &(Window){0}    // Child window (Unused)
        );
        if (parent_x_root == -1 || parent_y_root == -1)
        {
            LOG_ERROR(
                "Could not move portal (%p), coordinate translation failed.",
                (void*)portal
            );
            return;
        }

        // Calculate target window coordinates relative to parent.
        int x_parent = x_root - parent_x_root;
        int y_parent = y_root - parent_y_root;

        // Move the target window.
        XMoveWindow(display, target_window, x_parent, y_parent);

        // According to the ICCCM (Sections 4.1.5 and 4.2.3), when a window
        // manager moves a reparented client window, it is responsible for
        // sending a synthetic ConfigureNotify event to the client with the
        // windows new dimensions and position relative to root.

        if (is_portal_frame_valid(portal))
        {
            // Retrieve the client window dimensions.
            unsigned int client_width = 0, client_height = 0;
            Status geometry_status = XGetGeometry(
                display,            // Display
                client_window,      // Drawable
                &(Window){0},       // Root window (Unused)
                &(int){0},          // X (Relative to parent) (Unused)
                &(int){0},          // Y (Relative to parent) (Unused)
                &client_width,      // Width
                &client_height,     // Height
                &(unsigned int){0}, // Border width (Unused)
                &(unsigned int){0}  // Depth (Unused)
            );
            if (geometry_status == 0)
            {
                LOG_ERROR(
                    "Could not move portal (%p), "
                    "client window dimensions unavailable.",
                    (void*)portal
                );
                return;
            }

            // Notify the client window of its new geometry.
            XSendEvent(display, client_window, False, StructureNotifyMask, (XEvent*)&(XConfigureEvent) {
                .type = ConfigureNotify,
                .display = display,
                .event = client_window,
                .window = client_window,
                .x = x_root,
                .y = y_root + PORTAL_TITLE_BAR_HEIGHT,
                .width = client_width,
                .height = client_height,
                .border_width = 0,
                .above = None,
                .override_redirect = False
            });

            // Process all pending X events.
            XSync(display, False);
        }
    }

    // Call all event handlers of the PortalTransformed event.
    call_event_handlers((Event*)&(PortalTransformedEvent) {
        .type = PortalTransformed,
        .portal = portal
    });
}

void resize_portal(Portal *portal, unsigned int width, unsigned int height)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Ensure the portal has been initialized.
    if (portal->initialized == false) return;

    // Check if the client window still exists (may have been destroyed).
    if (!is_portal_client_valid(portal)) return;

    // Resize the portal itself.
    portal->geometry.width = width;
    portal->geometry.height = height;

    // Resize the portal windows.
    {
        Window frame_window = portal->frame_window;
        Window client_window = portal->client_window;

        if (is_portal_frame_valid(portal))
        {
            // Resize both the frame and client windows.
            XResizeWindow(display, frame_window, width, height);
            XResizeWindow(
                display,
                client_window,
                width,
                common.int_max(1, height - PORTAL_TITLE_BAR_HEIGHT)
            );
        }
        else
        {
            // No frame, just resize the client window.
            XResizeWindow(display, client_window, width, height);
        }

        // According to the ICCCM (Sections 4.1.5 and 4.2.3), when a window
        // manager resizes a reparented client window, it is responsible for
        // sending a synthetic ConfigureNotify event to the client with the
        // windows new dimensions and position relative to root.

        if (is_portal_frame_valid(portal))
        {
            // Retrieve the client window position relative to root.
            int client_x_root = -1, client_y_root = -1;
            XTranslateCoordinates(
                display,            // Display
                client_window,      // Source window
                root_window,        // Reference window
                0, 0,               // Source window origin coordinates
                &client_x_root,     // Translated X coordinate
                &client_y_root,     // Translated Y coordinate
                &(Window){0}        // Child window (Unused)
            );
            if (client_x_root == -1 || client_y_root == -1)
            {
                // Window was likely destroyed during the operation.
                return;
            }

            // Notify the client window of its new geometry.
            XSendEvent(display, client_window, False, StructureNotifyMask, (XEvent*)&(XConfigureEvent) {
                .type = ConfigureNotify,
                .display = display,
                .event = client_window,
                .window = client_window,
                .x = client_x_root,
                .y = client_y_root,
                .width = common.int_max(1, width),
                .height = common.int_max(1, height - PORTAL_TITLE_BAR_HEIGHT),
                .border_width = 0,
                .above = None,
                .override_redirect = False
            });

            // Process all pending X events.
            XSync(display, False);
        }
    }

    // Call all event handlers of the PortalTransformed event.
    call_event_handlers((Event*)&(PortalTransformedEvent){
        .type = PortalTransformed,
        .portal = portal
    });
}

void synchronize_portal(Portal *portal)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
    Window client_window = portal->client_window;

    // Ensure the portal has been initialized.
    if (portal->initialized == false) return;

    // Skip synchronization for fullscreen portals, their geometry is
    // managed by fullscreen.c and should not be modified.
    if (portal->fullscreen) return;

    // Check if the client window still exists (may have been destroyed).
    if (!is_portal_client_valid(portal)) return;

    // Retrieve the client window root coordinates.
    int client_x_root = 0, client_y_root = 0;
    XTranslateCoordinates(
        display,                // Display
        client_window,          // Source window
        root_window,            // Reference window
        0, 0,                   // Source window origin coordinates
        &client_x_root,         // Translated x coordinate
        &client_y_root,         // Translated y coordinate
        &(Window){0}            // Child Window (Unused)
    );

    // Retrieve the client window dimensions.
    unsigned int client_width = 0, client_height = 0;
    XGetGeometry(
        display,                // Display
        client_window,          // Drawable
        &(Window){0},           // Root window (Unused)
        &(int){0},              // X (Relative to parent) (Unused)
        &(int){0},              // Y (Relative to parent) (Unused)
        &client_width,          // Width
        &client_height,         // Height
        &(unsigned int){0},     // Border width (Unused)
        &(unsigned int){0}      // Depth (Unused)
    );

    // Calculate the new portal geometry.
    bool is_framed = is_portal_frame_valid(portal);
    int portal_x_root = client_x_root;
    int portal_y_root = client_y_root + (is_framed ? -PORTAL_TITLE_BAR_HEIGHT : 0);
    unsigned int portal_width = common.int_max(1, client_width);
    unsigned int portal_height = common.int_max(1, client_height + (is_framed ? PORTAL_TITLE_BAR_HEIGHT : 0));

    // Move the portal if the position has changed and the portal is not framed.
    // Framed portals have their position controlled by the WM, not the client.
    if (!is_framed && (portal_x_root != portal->geometry.x_root || portal_y_root != portal->geometry.y_root))
    {
        move_portal(portal, portal_x_root, portal_y_root);
    }

    // Resize the portal, only if the dimensions have changed.
    if (portal_width != portal->geometry.width || portal_height != portal->geometry.height)
    {
        resize_portal(portal, portal_width, portal_height);
    }

    // Synchronize all child portals as well.
    Window *child_windows = NULL;
    unsigned int child_window_count = 0;
    XQueryTree(
        display,            // Display
        client_window,      // Window
        &(Window){0},       // Root window (Unused)
        &(Window){0},       // Parent window (Unused)
        &child_windows,     // Children
        &child_window_count // Children count
    );
    for (int i = 0; i < (int)child_window_count; i++)
    {
        Portal *child_portal = find_portal_by_window(child_windows[i]);
        if (child_portal != NULL)
        {
            synchronize_portal(child_portal);
        }
    }
    XFree(child_windows);
}

Portal *get_top_portal()
{
    return top_portal;
}

void raise_portal(Portal *portal)
{
    // Ensure the portal has been initialized.
    if (portal->initialized == false) return;

    // Find the root of this portal's transient group.
    Portal *root = find_portal_transient_root(portal);

    // Raise the root portal first.
    raise_portal_window(root);

    // Raise all transient children whose root matches, so they stack
    // above the parent. This naturally handles chains since each child
    // is raised after its ancestor.
    for (int i = 0; i < MAX_PORTALS; i++)
    {
        Portal *candidate = &registry.unsorted[i];
        if (!candidate->active) continue;
        if (!candidate->initialized) continue;
        if (candidate->transient_for == NULL) continue;
        if (find_portal_transient_root(candidate) == root)
        {
            raise_portal_window(candidate);
        }
    }

    // Re-sort the portals.
    sort_portals();

    // Call all event handlers of the PortalRaised event.
    call_event_handlers((Event*)&(PortalRaisedEvent){
        .type = PortalRaised,
        .portal = portal
    });
}

void map_portal(Portal *portal)
{
    Display *display = DefaultDisplay;

    // Track if this is the first time the portal is being mapped.
    bool first_map = !portal->initialized;

    // Handle the portals first time being mapped.
    if (portal->initialized == false)
    {
        initialize_portal(portal);
    }

    // Map non-override-redirect portals. Override-redirect clients manage
    // themselves, but we still transition them to visible later.
    if (!portal->override_redirect)
    {
        // Map the frame window, if it exists.
        if (is_portal_frame_valid(portal))
        {
            XMapWindow(display, portal->frame_window);
        }

        // Map the client window, if it exists.
        if (is_portal_client_valid(portal))
        {
            XMapWindow(display, portal->client_window);

            // Set `WM_STATE` to NormalState as required by ICCCM.
            x_set_wm_state(display, portal->client_window, 1);
        }

        // Ensure map requests are processed before transitioning to
        // visible. Downstream handlers (e.g. focus) need the windows
        // to be viewable.
        XSync(display, False);
    }

    // Transition the portal to visible.
    portal->visibility = PORTAL_VISIBLE;

    // Populate the portal's `transient_for` so positioning can use it.
    populate_portal_transient_for(portal);

    // Apply `WM_NORMAL_HINTS` position if specified by the client, otherwise
    // center the portal relative to either the screen (normal) or parent
    // (transient). Override-redirect windows position themselves, so skip them.
    // Only do this on first map to preserve portal position across when
    // suspending and revealing (Fullscreen, Workspace switching).
    if (!portal->override_redirect && first_map)
    {
        bool should_center = true;
        XSizeHints hints;
        if (XGetWMNormalHints(display, portal->client_window, &hints, &(long){0}))
        {
            // Check if position hints represent an intentional placement.
            // Positions at or near origin (0,0 or 1,1) are often toolkit
            // defaults.
            bool is_default_position = (hints.x <= 1 && hints.y <= 1);
            bool has_real_position = (hints.flags & (USPosition | PPosition)) && !is_default_position;

            if (has_real_position)
            {
                int portal_x = hints.x;
                int portal_y = hints.y;
                if (is_portal_frame_valid(portal))
                {
                    portal_y -= PORTAL_TITLE_BAR_HEIGHT;
                }

                move_portal(portal, portal_x, portal_y);
                should_center = false;
            }
        }

        if (should_center)
        {
            Portal *parent = portal->transient_for;
            if (parent == NULL)
            {
                // Normal - Center on screen.
                int screen = DefaultScreen(display);
                int screen_width = DisplayWidth(display, screen);
                int screen_height = DisplayHeight(display, screen);
                int center_x = (screen_width - (int)portal->geometry.width) / 2;
                int center_y = (screen_height - (int)portal->geometry.height) / 2;
                move_portal(portal, center_x, center_y);
            }
            else
            {
                // Transient - Center on parent portal.
                int center_x = parent->geometry.x_root + ((int)parent->geometry.width - (int)portal->geometry.width) / 2;
                int center_y = parent->geometry.y_root + ((int)parent->geometry.height - (int)portal->geometry.height) / 2;
                move_portal(portal, center_x, center_y);
            }
        }
    }

    // Synchronize the portal geometry.
    synchronize_portal(portal);

    // Raise the portal above its parent if applicable, as per ICCCM.
    // `raise_portal()` calls `sort_portals()` internally.
    if (portal->transient_for != NULL)
    {
        raise_portal(portal);
    }
    else
    {
        sort_portals();
    }

    // Call all event handlers of the PortalMapped event.
    call_event_handlers((Event*)&(PortalMappedEvent){
        .type = PortalMapped,
        .portal = portal
    });
}

void unmap_portal(Portal *portal)
{
    Display *display = DefaultDisplay;

    // Unmap non-override-redirect portals. Override-redirect clients manage
    // themselves, but we still transition them to hidden later.
    if (!portal->override_redirect)
    {
        // Unmap the frame window, if it exists.
        if (is_portal_frame_valid(portal))
        {
            XUnmapWindow(display, portal->frame_window);
        }

        // Unmap the client window, if it exists.
        if (is_portal_client_valid(portal))
        {
            XUnmapWindow(display, portal->client_window);
        }
    }

    // Transition the portal to hidden (client withdrawal).
    portal->visibility = PORTAL_HIDDEN;

    // Call all event handlers of the PortalUnmapped event.
    call_event_handlers((Event*)&(PortalUnmappedEvent){
        .type = PortalUnmapped,
        .portal = portal
    });
}

void suspend_portal(Portal *portal)
{
    Display *display = DefaultDisplay;

    // Only visible and hidden portals can be suspended.
    if (portal->visibility == PORTAL_SUSPENDED) return;

    bool visible_before_suspend = (portal->visibility == PORTAL_VISIBLE);

    // Unmap non-override-redirect portals that are currently visible.
    if (visible_before_suspend && !portal->override_redirect)
    {
        if (is_portal_frame_valid(portal))
        {
            XUnmapWindow(display, portal->frame_window);
        }

        if (is_portal_client_valid(portal))
        {
            XUnmapWindow(display, portal->client_window);
        }
    }

    // Transition the portal to suspended.
    portal->visibility = PORTAL_SUSPENDED;

    // Only fire `PortalUnmapped` when transitioning from visible.
    // Hidden -> suspended is a pre-map deferral, not an unmap.
    if (visible_before_suspend)
    {
        call_event_handlers((Event*)&(PortalUnmappedEvent){
            .type = PortalUnmapped,
            .portal = portal
        });
    }
}

void reveal_portal(Portal *portal)
{
    // Only reveal portals that are suspended.
    if (portal->visibility != PORTAL_SUSPENDED) return;

    // Map the portal (transitions to PORTAL_VISIBLE).
    map_portal(portal);
}

int get_portal_index(Portal *portal)
{
    for (int i = 0; i < MAX_PORTALS; i++)
    {
        if (&registry.unsorted[i] == portal && registry.unsorted[i].active)
        {
            return i;
        }
    }
    return -1;
}

Portal *get_unsorted_portals()
{
    return registry.unsorted;
}

Portal **get_sorted_portals(unsigned int *out_count)
{
    *out_count = registry.active_count;
    return registry.sorted;
}

void sort_portals()
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Retrieve all windows in stacking order.
    Window *windows = NULL;
    unsigned int window_count = 0;
    int status = x_query_tree_recursively(
        display,        // Display
        root_window,    // Parent
        &windows,       // Children
        &window_count   // Children count
    );
    if (status != 0)
    {
        LOG_ERROR("Could not sort portals, tree query failed (%d).", status);
        free(windows);
        return;
    }

    // Build sorted portals array from windows array.
    int portals_added = 0;
    for (int i = 0; i < (int)window_count; i++)
    {
        // Ensure the window belongs to a portal.
        Portal *portal = find_portal_by_window(windows[i]);
        if (portal == NULL) continue;
        if (!portal->active) continue;

        // Ensure we only handle client windows, and not frame windows as well,
        // preventing duplicate entries in the sorted portals array.
        if (windows[i] != portal->client_window) continue;

        // Add portal to the sorted array.
        registry.sorted[portals_added] = portal;
        portals_added++;
    }

    // Synchronize top_portal to the topmost visible portal.
    top_portal = NULL;
    for (int i = portals_added - 1; i >= 0; i--)
    {
        if (registry.sorted[i]->visibility == PORTAL_VISIBLE)
        {
            top_portal = registry.sorted[i];
            break;
        }
    }

    // Clear remaining slots, if any.
    while (portals_added < MAX_PORTALS) {
        registry.sorted[portals_added] = NULL;
        portals_added++;
    }

    // Cleanup.
    free(windows);
}

Portal *find_portal_by_window(Window window)
{
    // Iterate over the unsorted portals to find the portal associated with
    // the specified window.
    for (int i = 0; i < MAX_PORTALS; i++)
    {
        Portal *portal = &registry.unsorted[i];
        if (!portal->active) continue;

        // Check if the portal is associated with the specified window.
        if (portal->frame_window == window ||
            portal->client_window == window)
        {
            return portal;
        }
    }
    return NULL;
}

Portal *find_or_create_portal(Window window)
{
    Portal *portal = find_portal_by_window(window);
    if (portal != NULL) return portal;

    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    pid_t pid = x_get_window_pid(display, window);
    if (pid == getpid()) return NULL;

    if (x_get_window_parent(display, window) != root_window) return NULL;

    return create_portal(window);
}

Portal *find_portal_at_pos(int x_root, int y_root)
{
    // Iterate over the sorted portals in reverse order, to find the topmost
    // portal at the specified position.
    for (int i = MAX_PORTALS - 1; i >= 0; i--)
    {
        Portal *portal = registry.sorted[i];

        // Skip invalid portals.
        if (portal == NULL) continue;
        if (!portal->active) continue;
        if (portal->initialized == false) continue;
        if (portal->visibility != PORTAL_VISIBLE) continue;

        // Check if the portal is located at the specified position.
        if (x_root >= portal->geometry.x_root &&
            y_root >= portal->geometry.y_root &&
            x_root < portal->geometry.x_root + (int)portal->geometry.width &&
            y_root < portal->geometry.y_root + (int)portal->geometry.height)
        {
            return portal;
        }
    }
    return NULL;
}

Portal *find_portal_transient_root(Portal *portal)
{
    // Walk up the transient chain to the root.
    int depth = 0;
    while (portal->transient_for != NULL && depth < MAX_PORTALS)
    {
        portal = portal->transient_for;
        depth++;
    }
    return portal;
}

void populate_portal_transient_for(Portal *portal)
{
    // Skip if the transient relationship is already resolved.
    if (portal->transient_for != NULL) return;

    // Query the `WM_TRANSIENT_FOR` property and look up the parent portal.
    Window transient_parent = None;
    if (x_get_transient_for(DefaultDisplay, portal->client_window, &transient_parent) == 0)
    {
        portal->transient_for = find_portal_by_window(transient_parent);
    }
}

void adopt_existing_portal_windows()
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Grab the server to prevent events while scanning.
    XGrabServer(display);

    // Query all children of the root window (bottom-to-top stacking order).
    Window *children = NULL;
    unsigned int child_count = 0;
    XQueryTree(display, root_window, &(Window){0}, &(Window){0}, &children, &child_count);

    for (unsigned int i = 0; i < child_count; i++)
    {
        // Skip windows that are override-redirect or not visible.
        XWindowAttributes attrs;
        if (!XGetWindowAttributes(display, children[i], &attrs)) continue;
        if (attrs.override_redirect) continue;
        if (attrs.map_state != IsViewable) continue;

        // Skip windows owned by this WM process.
        pid_t pid = x_get_window_pid(display, children[i]);
        if (pid == getpid()) continue;

        // Create a portal for this window.
        Portal *portal = create_portal(children[i]);
        if (portal == NULL) continue;

        // Read `_NET_WM_DESKTOP` to restore the workspace assignment.
        int desktop = x_get_window_desktop(display, children[i]);
        if (desktop >= 0 && desktop < MAX_WORKSPACES)
        {
            portal->workspace = desktop;
        }

        // Initialize the portal (captures geometry, creates frame, fires events).
        initialize_portal(portal);

        // Map portals on the current workspace; suspend others.
        // Adoption intentionally bypasses the workspace portal limit
        // to avoid discarding existing windows on WM restart.
        if (portal->workspace == get_current_workspace())
        {
            map_portal(portal);
        }
        else
        {
            suspend_portal(portal);
        }
    }

    XFree(children);

    // Ungrab the server so events can be processed again.
    XUngrabServer(display);

    // Synchronize stacking order once after all portals are adopted.
    sort_portals();
}

PortalDecoration get_portal_decoration_kind(Portal *portal)
{
    // Framed windows get full decorations.
    if (is_portal_frame_valid(portal))
    {
        return PORTAL_DECORATION_FRAMED;
    }

    // Skip tooltips and notifications (not managed as separate portals).
    Display *display = DefaultDisplay;
    static Atom tooltip = None;
    if (tooltip == None)
    {
        tooltip = XInternAtom(display, "_NET_WM_WINDOW_TYPE_TOOLTIP", False);
    }
    static Atom notification = None;
    if (notification == None)
    {
        notification = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
    }
    if (portal->client_window_type == tooltip ||
        portal->client_window_type == notification
    ) {
        return PORTAL_DECORATION_NONE;
    }

    // Return frameless decorations for CSD apps and override-redirect windows.
    if (portal->top_level || portal->override_redirect)
    {
        return PORTAL_DECORATION_FRAMELESS;
    }

    return PORTAL_DECORATION_NONE;
}

HANDLE(Initialize)
{
    adopt_existing_portal_windows();
}
