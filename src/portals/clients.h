#pragma once
#include "../all.h"

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
 * @return - `-1` Internal `XSendEvent()` call failed.
 * @return - `-2` Internal `XDestroyWindow()` call failed.
 * 
 * @warning - Ensure the portal client is valid before calling this function.
 * 
 * @note - Client may ignore or delay responding to the closing request.
 */
int destroy_portal_client(Portal *portal);
