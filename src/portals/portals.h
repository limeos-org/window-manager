#pragma once
#include "../all.h"

/** The height of the title bar in pixels. */
#define PORTAL_TITLE_BAR_HEIGHT 26

/** The width of the frame border in pixels. */
#define PORTAL_BORDER_WIDTH 1

/** The radius of the rounded corners in pixels. */
#define PORTAL_CORNER_RADIUS 6

/** The minimum width of a portal in pixels. */
#define MINIMUM_PORTAL_WIDTH 128

/** The minimum height of a portal in pixels. */
#define MINIMUM_PORTAL_HEIGHT 64

/** A geometry rectangle with root-relative position and dimensions. */
typedef struct {
    int x_root, y_root;
    unsigned int width, height;
} PortalGeometry;

/**
 * A portal represents a window pair consisting of a decorative frame and the
 * client content area, along with its geometry and rendering state.
 */
typedef struct {
    char *title;
    bool initialized;
    bool top_level;
    bool mapped;
    bool override_redirect;
    bool fullscreen;
    PortalGeometry geometry;
    PortalGeometry geometry_backup;
    Window frame_window;
    cairo_t *frame_cr;
    Visual *frame_visual;
    Window client_window;
    Atom client_window_type;
    Visual *client_visual;
} Portal;

/**
 * Sorts portals in the registry based on their stacking order.
 */
void sort_portals();

/**
 * Creates a portal and registers it to the portal registry.
 * 
 * @param client_window The client window to create the portal from.
 * 
 * @return - `Portal*` The portal was created successfully.
 * @return - `NULL` The portal could not be created.
 */
Portal *create_portal(Window client_window);

/**
 * Attempts to destroy a portal and unregisters it from the portal registry
 * if destruction was successful.
 * 
 * @param portal The portal to destroy.
 * 
 * @note Will not complete if the client or frame windows refuse to close.
 */
void destroy_portal(Portal *portal);

/**
 * Moves a portal to a new position.
 *
 * @param portal The portal to move.
 * @param x_root The new X coordinate relative to root.
 * @param y_root The new Y coordinate relative to root.
 */
void move_portal(Portal *portal, int x_root, int y_root);

/**
 * Resizes a portal to new dimensions.
 *
 * @param portal The portal to resize.
 * @param width The new width in pixels.
 * @param height The new height in pixels.
 */
void resize_portal(Portal *portal, unsigned int width, unsigned int height);

/**
 * Synchronizes the portal's stored geometry with its actual window geometry.
 *
 * @param portal The portal to synchronize.
 */
void synchronize_portal(Portal *portal);

/**
 * Returns the top portal in the stacking order.
 *
 * @return The top portal, or NULL if no portal has been raised.
 */
Portal *get_top_portal();

/**
 * Raises a portal to the top of the stacking order.
 * Skipped if portal is already on top (optimization).
 *
 * @param portal The portal to raise.
 */
void raise_portal(Portal *portal);

/**
 * Maps all portal windows to the screen.
 * 
 * @param portal The portal to map.
 */
void map_portal(Portal *portal);

/**
 * Unmaps all portal windows from the screen.
 * 
 * @param portal The portal to unmap.
 */
void unmap_portal(Portal *portal);

/**
 * Retrieves the index of a portal in the unsorted registry array.
 *
 * @param portal The portal to find.
 *
 * @return - `>= 0` The index of the portal in the registry.
 * @return - `-1` The portal was not found.
 */
int get_portal_index(Portal *portal);

/**
 * Retrieves the unsorted array of portals from the registry.
 *
 * @param out_count Pointer to store the number of portals.
 *
 * @return The unsorted portal array.
 */
Portal *get_unsorted_portals(unsigned int *out_count);

/**
 * Retrieves the sorted array of portal pointers from the registry.
 *
 * Portals are sorted by stacking order (bottom to top).
 *
 * @param out_count Pointer to store the number of portals.
 *
 * @return The sorted portal pointer array.
 */
Portal **get_sorted_portals(unsigned int *out_count);

/**
 * Finds a portal in the portal registry using the `window` provided.
 * 
 * @param window A client or frame window.
 * 
 * @return - `Portal*` The portal was found. 
 * @return - `NULL` The portal could not be found.
 */
Portal *find_portal_by_window(Window window);

/**
 * Finds a portal in the portal registry located at the specified position.
 * 
 * @param x_root The X coordinate relative to root.
 * @param y_root The Y coordinate relative to root.
 * 
 * @return - `Portal*` The portal was found.
 * @return - `NULL` The portal could not be found.
 */
Portal *find_portal_at_pos(int x_root, int y_root);
