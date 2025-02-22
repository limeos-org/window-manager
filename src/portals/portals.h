#pragma once
#include "../all.h"

/**
 * The height of the title bar in pixels.
 */
#define PORTAL_TITLE_BAR_HEIGHT 15

/**
 * The minimum width of a portal in pixels.
 * TODO: This is currently unused, re-implement min/max portal width.
 */
#define MINIMUM_PORTAL_WIDTH 200

/**
 * The minimum height of a portal in pixels.
 * TODO: This is currently unused, re-implement min/max portal width.
 */
#define MINIMUM_PORTAL_HEIGHT 150

/**
 * The maximum initial width of a portal as a percentage of the screen width.
 * TODO: This is currently unused, re-implement min/max portal width.
 */
#define MAXIMUM_INITIAL_PORTAL_WIDTH_PERCENT 0.8

/**
 * The maximum initial height of a portal as a percentage of the screen width.
 * TODO: This is currently unused, re-implement min/max portal width.
 */
#define MAXIMUM_INITIAL_PORTAL_HEIGHT_PERCENT 0.8

/**
 * A portal represents a window pair consisting of a decorative frame and the
 * client content area, along with its geometry and rendering state.
 */
typedef struct {
    char *title;
    bool initialized;
    bool top_level;
    bool mapped;
    int x_root, y_root;
    unsigned int width, height;
    Window frame_window;
    cairo_t *frame_cr;
    Window client_window;
} Portal;

// TODO: Document & sort.
Portal *create_portal(Window client_window);
void unregister_portal(Portal *portal);
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
 * TODO: Document this function.
 */
void move_portal(Portal *portal, int x_root, int y_root);

/**
 * TODO: Document this function.
 */
void resize_portal(Portal *portal, unsigned int width, unsigned int height);

/**
 * TODO: Document this function.
 */
void synchronize_portal(Portal *portal);

/**
 * TODO: Document this function.
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
 * TODO: Document this function.
 */
int get_portal_index(Portal *portal);

/**
 * TODO: Document this function.
 */
Portal *get_unsorted_portals(unsigned int *count);

/**
 * TODO: Document this function.
 */
Portal **get_sorted_portals(unsigned int *count);

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
