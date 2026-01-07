#pragma once
#include "../all.h"

/**
 * A type representing background scaling modes.
 */
typedef enum {
    BACKGROUND_SCALING_ZOOM,    // Scale to fill, crop excess (default)
    BACKGROUND_SCALING_FIT,     // Scale to fit, letterbox
    BACKGROUND_SCALING_STRETCH, // Stretch to fill (distorts)
    BACKGROUND_SCALING_CENTER,  // Center at original size
    BACKGROUND_SCALING_TILE     // Repeat as tiles
} BackgroundScaling;

/**
 * Draws the desktop background on the root window.
 *
 * Uses configuration settings to determine mode (solid/image),
 * color, image path, and scaling options.
 */
void draw_background();

/**
 * Parses a scaling mode string into the enum value.
 *
 * @param scaling_str The scaling mode string ("zoom", "fit", etc.)
 *
 * @return The corresponding BackgroundScaling enum value.
 */
BackgroundScaling parse_background_scaling(const char *scaling_str);
