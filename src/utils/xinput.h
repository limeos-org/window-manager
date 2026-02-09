#pragma once
#include "../all.h"

/**
 * Configures which events the XInput2 extension should listen for on a given
 * window.
 * 
 * @param display The X11 display.
 * @param window The X11 window where the events should be listened for.
 * @param mask Bitmask of XInput2 events to select / listen to.
 * 
 * @return - `0` - Execution was successful.
 * @return - `-1` - Internal `XISelectEvents()` call failed.
 * 
 * @note - The function is designed to mimic the `XSelectInput()` function.
 */
int xi_select_input(Display *display, Window window, long mask);
