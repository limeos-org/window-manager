#pragma once
#include "../all.h"

/**
 * Focuses a portal by setting keyboard input focus and raising it.
 *
 * @param portal The portal to focus.
 */
void focus_portal(Portal *portal);

/**
 * Returns the portal that currently has input focus.
 *
 * @return The focused portal, or NULL if no portal is focused.
 */
Portal *get_focused_portal();
