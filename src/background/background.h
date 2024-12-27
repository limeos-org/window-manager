#pragma once
#include "../all.h"

#ifdef STATIC

/**
 * Loads and scales a PNG image to fit the screen dimensions.
 *
 * @param display X11 display connection.
 * @param filename Path to the PNG image file to load.
 *
 * @return Cairo surface containing the background image, or NULL if the image
 * could not be loaded.
 *
 * @note The returned surface needs to be freed with `cairo_surface_destroy()`
 * when no longer needed.
 */
static cairo_surface_t *load_background_image(Display *display, const char *filename);

/**
 * Draws the specified Cairo surface onto the specified Cairo context.
 *
 * @param cr Cairo context to draw on.
 * @param png_surface Cairo surface containing the image to draw.
 * 
 * @note Use `load_background_image()` to load the image.
 */
static void draw_background_image(cairo_t *cr, cairo_surface_t *png_surface);

/**
 * Draws a solid color onto the specified Cairo context.
 *
 * @param cr Cairo context to draw on.
 * @param color Color to draw in hex format (e.g. 0xRRGGBB).
 */
static void draw_background_solid(cairo_t *cr, unsigned long color);

#endif
