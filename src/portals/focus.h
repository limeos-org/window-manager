#pragma once
#include "../all.h"

/**
 * Handles focus changes in response to a portal click.
 *
 * Sets keyboard focus to the portal's client window and raises it if not
 * already on top.
 *
 * @param portal The portal that was clicked.
 */
void handle_portal_focus_click(Portal *portal);
