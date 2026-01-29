#pragma once
#include "../all.h"

/**
 * Converts a framerate to a throttle time in milliseconds.
 *
 * @param framerate The framerate to convert.
 *
 * @return The throttle time in milliseconds.
 */
int framerate_to_throttle_ms(int framerate);
