#pragma once
#include "../all.h"

/**
 * Checks if the provided coordinates are within the client area of the portal.
 * 
 * @param portal The portal to check the client area for.
 * @param rel_x The x coordinate, relative to the portal.
 * @param rel_y The y coordinate, relative to the portal.
 * 
 * @return - `True (1)` The coordinates are within the client area.
 * @return - `False (0)` The coordinates are not within the client area.
 */
bool is_portal_client_area(Portal *portal, int rel_x, int rel_y);

/**
 * Checks if the portal client is valid.
 * 
 * @param portal The portal containing the client window.
 * 
 * @return - `True (1)` The portal client is valid.
 * @return - `False (0)` The portal client is invalid.
 */
bool is_portal_client_valid(Portal *portal);

/**
 * Closes portal client window gracefully if supported, closes forcefully 
 * otherwise.
 * 
 * @param portal The portal containing the client window.
 * 
 * @return - `0` The request was sent successfully or the client was
 * forcefully closed.
 * @return - `-1` The portal client is invalid.
 * @return - `-2` Internal `XSendEvent()` call failed.
 * @return - `-3` Internal `XDestroyWindow()` call failed.
 * 
 * @note - Client may ignore or delay responding to the closing request.
 * @note - Can safely be called without checking if the client is valid first.
 */
int destroy_portal_client(Portal *portal);
