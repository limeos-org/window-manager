#include "../all.h"

// TODO: Remove this, it's temporary.
#undef UINT_MAX
#define UINT_MAX (0x7fffffff * 2U + 1U)

typedef struct {
    Portal *unsorted;
    Portal **sorted;
    unsigned int count;
    unsigned int capacity;
} PortalRegistry;

static PortalRegistry registry = {
    .unsorted = NULL,
    .sorted = NULL,
    .count = 0,
    .capacity = 0
};

static Portal *register_portal(
    const char *title, 
    bool initialized,
    bool top_level,
    int x_root, int y_root, 
    unsigned int width, unsigned int height, 
    Window frame_window,
    cairo_t *frame_cr,
    Window client_window
)
{
    // Increase the portal count.
    registry.count++;

    // Allocate additional memory, if neccessary.
    if (registry.count > registry.capacity)
    {
        // Calculate the new capacity.
        registry.capacity = registry.capacity == 0 ? 2 : registry.capacity * 2;

        // Reallocate memory for the unsorted portals array.
        Portal *new_unsorted = realloc(registry.unsorted, registry.capacity * sizeof(Portal));
        if (new_unsorted == NULL) {
            LOG_ERROR("Could not register portal, memory allocation failed.");
            free(new_unsorted);
            registry.count--;
            return NULL;
        }
        registry.unsorted = new_unsorted;

        // Reallocate memory for the sorted portals array.
        Portal **new_sorted = realloc(registry.sorted, registry.capacity * sizeof(Portal *));
        if (new_unsorted == NULL) {
            LOG_ERROR("Could not register portal, memory allocation failed.");
            free(new_sorted);
            registry.count--;
            return NULL;
        }
        registry.sorted = new_sorted;
    }

    // Add the portal to the registry.
    registry.unsorted[registry.count - 1] = (Portal){
        .title = strdup(title),
        .initialized = initialized,
        .top_level = top_level,
        .x_root = x_root,
        .y_root = y_root,
        .width = width,
        .height = height,
        .frame_window = frame_window,
        .frame_cr = frame_cr,
        .client_window = client_window
    };

    return &registry.unsorted[registry.count - 1];
}

static void unregister_portal(Portal *portal)
{
    // Find the index of the portal in the registry.
    int index = -1;
    for (int i = 0; i < (int)registry.count; i++)
    {
        if (&registry.unsorted[i] == portal)
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        LOG_ERROR(
            "Could not unregister portal (%p), portal not found in registry.",
            (void*)portal
        );
        return;
    }

    // Free the allocated memory for the title.
    free(registry.unsorted[index].title);

    // Overwrite target portal by shifting all subsequent portals.
    for (int i = index; i < (int)registry.count - 1; i++)
    {
        registry.unsorted[i] = registry.unsorted[i + 1];
    }

    // Decrease the portal count.
    registry.count--;
}

static void sort_portals()
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
        LOG_ERROR("Could not sort portals (Status: %d).", status);
        free(windows);
        return;
    }

    // Build sorted portals array from windows array.
    int added = 0;
    for (int i = 0; i < (int)window_count; i++)
    {
        // Ensure the window belongs to a portal.
        Portal *portal = find_portal_by_window(windows[i]);
        if (portal == NULL) continue;

        // Ensure we only handle client windows, and not frame windows as well,
        // preventing duplicate entries in the sorted portals array.
        if (windows[i] != portal->client_window) continue;

        // Add portal to the sorted array.
        registry.sorted[added] = portal;
        added++;
    }

    // Clear remaining slots, if any.
    while (added < (int)registry.count) {
        registry.sorted[added] = NULL;
        added++;
    }

    // Cleanup.
    free(windows);
}

Portal *create_portal(Window client_window)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Remove the border from the client window, as the window manager will
    // be responsible for handling all window decorations.
    XSetWindowBorderWidth(display, client_window, 0);
    
    // Determine the portal geometry.
    int portal_x_root = 0, portal_y_root = 0;
    unsigned int portal_width = 0, portal_height = 0;
    {
        // Get the client window dimensions.
        unsigned int client_width = 0, client_height = 0;
        XGetGeometry(
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
        
        // Get the client window coordinates relative to root.
        int client_x_root = 0, client_y_root = 0;
        XTranslateCoordinates(
            display,            // Display
            client_window,      // Source window
            root_window,        // Reference window
            0, 0,               // Source window origin coordinates
            &client_x_root,     // Translated X coordinate
            &client_y_root,     // Translated Y coordinate
            &(Window){0}        // Child window (Unused)
        );

        // Determine the portal geometry.
        portal_x_root = client_x_root;
        portal_y_root = client_y_root;
        portal_width = client_width;
        portal_height = client_height;
    }

    // Determine the portal title.
    char portal_title[256] = "Untitled";
    x_get_window_name(display, client_window, portal_title, sizeof(portal_title));

    // Register the portal.
    Portal *portal = register_portal(
        portal_title,       // Title
        false,              // Initialized
        false,              // Top Level
        portal_x_root,      // X (Relative to root)
        portal_y_root,      // Y (Relative to root)
        portal_width,       // Width
        portal_height,      // Height
        None,               // Frame Window
        NULL,               // Frame Cairo Context
        client_window       // Client Window
    );

    // Re-sort the portals.
    sort_portals();

    // Call all event handlers of the PortalCreated event.
    call_event_handlers((Event*)&(PortalCreatedEvent){
        .type = PortalCreated,
        .portal = portal
    });

    return portal;
}

void destroy_portal(Portal *portal)
{
    Window client_window = portal->client_window;
    Window frame_window = portal->frame_window;

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

    // Unregister the portal.
    unregister_portal(portal);

    // Re-sort the portals.
    sort_portals();

    // Call all event handlers of the PortalDestroyed event.
    call_event_handlers((Event*)&(PortalDestroyedEvent){
        .type = PortalDestroyed,
        .client_window = client_window,
        .frame_window = frame_window
    });
}

void move_portal(Portal *portal, int x_root, int y_root, bool move_windows)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
    Window client_window = portal->client_window;
    Window frame_window = portal->frame_window;

    // Ensure the portal has been initialized.
    if (portal->initialized == false) return;

    // Move the portal windows.
    if (move_windows)
    {
        // Determine which window to move.
        Window target_window = (is_portal_frame_valid(portal)) ? frame_window : client_window;

        // The `XMoveWindow()` function expects coordinates relative to the
        // parent window. So we have to translate the provided root coordinates
        // to parent coordinates.

        // Retrieve the parent window of the target window.
        Window parent_window = x_get_window_parent(display, target_window);
        if (parent_window == None)
        {
            LOG_ERROR(
                "Could not move portal (%p), parent window not found.",
                (void*)portal
            );
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
            unsigned int client_width = UINT_MAX, client_height = UINT_MAX;
            XGetGeometry(
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
            if (client_width == UINT_MAX || client_height == UINT_MAX)
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

    // Move the portal itself.
    portal->x_root = x_root;
    portal->y_root = y_root;

    // Call all event handlers of the PortalTransformed event.
    call_event_handlers((Event*)&(PortalTransformedEvent) {
        .type = PortalTransformed,
        .portal = portal
    });
}

void resize_portal(Portal *portal, unsigned int width, unsigned int height, bool resize_windows)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);
    Window frame_window = portal->frame_window;
    Window client_window = portal->client_window;

    // Ensure the portal has been initialized.
    if (portal->initialized == false) return;

    // Resize the portal windows.
    if (resize_windows)
    {
        // Determine which window to resize.
        Window target_window = (is_portal_frame_valid(portal)) ? frame_window : client_window;

        // Resize the target window.
        XResizeWindow(display, target_window, width, height);

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
                LOG_ERROR(
                    "Could not resize portal (%p), "
                    "coordinate translation failed.",
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
                .x = client_x_root,
                .y = client_y_root,
                .width = max(1, width),
                .height = max(1, height - PORTAL_TITLE_BAR_HEIGHT),
                .border_width = 0,
                .above = None,
                .override_redirect = False
            });

            // Process all pending X events.
            XSync(display, False);
        }
    }

    // Resize the portal itself.
    portal->width = width;
    portal->height = height;

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
    unsigned int portal_width = max(1, client_width);
    unsigned int portal_height = max(1, client_height + (is_framed ? PORTAL_TITLE_BAR_HEIGHT : 0));

    // Move the portal, only if the position has changed.
    if (portal_x_root != portal->x_root || portal_y_root != portal->y_root)
    {
        move_portal(portal, portal_x_root, portal_y_root, true);
    }

    // Resize the portal, only if the dimensions have changed.
    if (portal_width != portal->width || portal_height != portal->height)
    {
        resize_portal(portal, portal_width, portal_height, true);
    }

    // Synchronize all child portals.
    Window *child_windows;
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
}

void raise_portal(Portal *portal)
{
    Display *display = DefaultDisplay;
    Window client_window = portal->client_window;
    Window frame_window = portal->frame_window;

    // Ensure the portal has been initialized.
    if (portal->initialized == false) return;

    // Determine which window to raise.
    Window target_window = (is_portal_frame_valid(portal)) ? frame_window : client_window;

    // Raise the portal windows.
    XRaiseWindow(display, target_window);

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

    // Handle the portals first time being mapped.
    if (portal->initialized == false)
    {
        // Determine whether the portals client window is a top-level window.
        portal->top_level = x_window_is_top_level(display, portal->client_window);

        // Create a frame for the portal, if necessary.
        if (portal->top_level == true)
        {
            create_portal_frame(portal);
        }

        // Set the portal as initialized.
        portal->initialized = true;
    }

    // Map the frame window, if it exists.
    if (is_portal_frame_valid(portal))
    {
        XMapWindow(display, portal->frame_window);
    }

    // Map the client window, if it exists.
    if (is_portal_client_valid(portal))
    {
        XMapWindow(display, portal->client_window);
    }

    // Synchronize the portal geometry.
    synchronize_portal(portal);

    // Re-sort the portals.
    sort_portals();

    // Call all event handlers of the PortalMapped event.
    call_event_handlers((Event*)&(PortalMappedEvent){
        .type = PortalMapped,
        .portal = portal
    });
}

void unmap_portal(Portal *portal)
{
    Display *display = DefaultDisplay;

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

    // Re-sort the portals.
    sort_portals();

    // Call all event handlers of the PortalUnmapped event.
    call_event_handlers((Event*)&(PortalUnmappedEvent){
        .type = PortalUnmapped,
        .portal = portal
    });
}

Portal *get_unsorted_portals(unsigned int *out_count)
{
    *out_count = registry.count;
    return registry.unsorted;
}

Portal **get_sorted_portals(unsigned int *out_count)
{
    *out_count = registry.count;
    return registry.sorted;
}

Portal *find_portal_by_window(Window window)
{
    // Iterate over the unsorted portals to find the portal associated with
    // the specified window.
    for (int i = 0; i < (int)registry.count; i++)
    {
        Portal *portal = &registry.unsorted[i];

        // Skip invalid portals.
        if (portal == NULL) continue;

        // Check if the portal is associated with the specified window.
        if (portal->frame_window == window ||
            portal->client_window == window)
        {
            return portal;
        }
    }
    return NULL;
}

Portal *find_portal_at_pos(int x_root, int y_root)
{
    // Iterate over the sorted portals in reverse order, to find the topmost
    // portal at the specified position.
    for (int i = registry.count - 1; i >= 0; i--)
    {
        Portal *portal = registry.sorted[i];

        // Skip invalid portals.
        if (portal == NULL) continue;
        if (portal->initialized == false) continue;

        // Check if the portal is located at the specified position.
        if (x_root >= portal->x_root &&
            y_root >= portal->y_root &&
            x_root < portal->x_root + (int)portal->width &&
            y_root < portal->y_root + (int)portal->height)
        {
            return portal;
        }
    }
    return NULL;
}
