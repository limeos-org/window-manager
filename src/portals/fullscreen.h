#pragma once
#include "../all.h"

/**
 * Enters fullscreen mode for a portal.
 * Saves the current client dimensions and resizes to cover the screen.
 *
 * @param portal The portal to make fullscreen.
 */
void enter_portal_fullscreen(Portal *portal);

/**
 * Exits fullscreen mode for a portal.
 * Restores the client to its saved dimensions.
 *
 * @param portal The portal to exit fullscreen.
 */
void exit_portal_fullscreen(Portal *portal);
