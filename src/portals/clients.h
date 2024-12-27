#pragma once
#include "../all.h"

#ifdef STATIC

/**
 * Handles configuration requests from clients.
 * 
 * Configuration requests are only processed when no portal exists yet
 * (during client window creation). Once a portal is assigned to manage
 * the client window, all subsequent configuration requests are ignored
 * since the window manager takes full control of window positioning
 * and sizing.
 * 
 * @param portal The portal structure (can be `NULL`).
 * @param event The X11 configure request event structure.
 */
static void handle_portal_client_config(Portal *portal, XConfigureRequestEvent *event);

#endif

/**
 * Destroys a portals client window.
 * 
 * @param portal The portal containing the client window.
 * 
 * @return 0 if successful, non-zero integer otherwise.
 */
int destroy_portal_client(Portal *portal);
