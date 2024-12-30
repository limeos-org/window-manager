#pragma once
#include "../all.h"

// The height of the title bar in pixels.
#define PORTAL_TITLE_BAR_HEIGHT 15

// The minimum width of a portal in pixels.
#define MINIMUM_PORTAL_WIDTH 200

// The minimum height of a portal in pixels.
#define MINIMUM_PORTAL_HEIGHT 150

// The maximum initial width of a portal as a percentage of the screen width.
#define MAXIMUM_INITIAL_PORTAL_WIDTH_PERCENT 0.8

// The maximum initial height of a portal as a percentage of the screen width.
#define MAXIMUM_INITIAL_PORTAL_HEIGHT_PERCENT 0.8

// A portal manages a window pair consisting of a decorative frame and the
// client content area, along with its geometry and rendering state.
typedef struct {
    Display *display;
    char *title;
    int x, y;
    unsigned int width, height;
    Window frame_window;
    cairo_t *frame_cr;
    Window client_window;
} Portal;

/**
 * Creates a portal and registers it to the portal registry.
 * 
 * @param display The display where the portal will be located.
 * @param client_window The client window to create the portal from.
 * 
 * @return The created portal, or `NULL` upon failure.
 */
Portal *create_portal(Display *display, Window client_window);

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
 * Finds a portal in the portal registry using the `window` provided.
 * 
 * @param window A client or frame window.
 * 
 * @return The found portal or `NULL` if not found.
 */
Portal *find_portal(Window window);
