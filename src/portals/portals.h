#pragma once
#include "../all.h"

/** The height of the title bar in pixels. */
#define PORTAL_TITLE_BAR_HEIGHT 26

/** The minimum width of a portal in pixels. */
#define MINIMUM_PORTAL_WIDTH 128

/** The minimum height of a portal in pixels. */
#define MINIMUM_PORTAL_HEIGHT 64

/** The maximum number of portals that can be managed simultaneously. */
#define MAX_PORTALS 256

/** Decoration kinds for portals. */
typedef enum
{
    /** No decorations applied. */
    DECORATION_NONE,
    /** Shadow and border for portals without a titlebar. */
    DECORATION_FRAMELESS,
    /** Shadow and border for portals with a titlebar. */
    DECORATION_FRAMED
} DecorationKind;

/** A geometry rectangle with root-relative position and dimensions. */
typedef struct {
    int x_root, y_root;
    unsigned int width, height;
} PortalGeometry;

/**
 * A portal represents a window pair consisting of a decorative frame and the
 * client content area, along with its geometry and rendering state.
 */
typedef struct Portal {
    bool active;                     // Whether this registry slot is in use.
    char *title;
    bool initialized;                // Whether first-time setup has completed.
    bool top_level;                  // Whether this portal is a child of root.
    struct Portal *transient_for;    // Parent portal if transient, else NULL.
    bool mapped;
    bool override_redirect;          // Whether client manages its own geometry.
    bool fullscreen;
    int workspace;                   // -1 if unassigned.
    PortalGeometry geometry;
    PortalGeometry geometry_backup;  // Saved for fullscreen restore.
    Window frame_window;
    cairo_t *frame_cr;
    Visual *frame_visual;
    Window client_window;
    Atom client_window_type;         // The _NET_WM_WINDOW_TYPE of the client.
    Visual *client_visual;
    bool misaligned;                 // Whether client moved within frame.
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
 * Returns a fixed-size array with inactive slots scattered throughout
 * (tombstone pattern). Caller must iterate up to MAX_PORTALS and skip
 * entries where portal->active is false.
 *
 * @return The unsorted portal array (Portal[MAX_PORTALS]).
 */
Portal *get_unsorted_portals();

/**
 * Retrieves the sorted array of portal pointers from the registry.
 *
 * Unlike get_unsorted_portals(), this returns a packed array of pointers
 * to active portals only, sorted by stacking order (bottom to top).
 * The array is rebuilt on each stacking change, so no active checks needed.
 * Entries beyond out_count may be NULL.
 *
 * @param out_count Pointer to store the number of active portals.
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

/**
 * Walks up the transient chain to find the root ancestor.
 *
 * @param portal The portal to start from.
 *
 * @return The root ancestor, or the portal itself if it has no parent.
 */
Portal *find_portal_transient_root(Portal *portal);

/**
 * Determines the decoration kind for a portal.
 *
 * @param portal The portal to check.
 *
 * @return - `DECORATION_FRAMED` The portal has a WM frame.
 * @return - `DECORATION_FRAMELESS` The portal is a CSD or
 * override-redirect window.
 *
 * @return - `DECORATION_NONE` The portal should not be decorated.
 */
DecorationKind get_portal_decoration_kind(Portal *portal);
