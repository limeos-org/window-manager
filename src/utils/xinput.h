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

/**
 * Retrieves the type of a given XInput2 device.
 * 
 * @param display The X11 display.
 * @param device_id The ID of the XInput2 device.
 * @param out_device_type The output parameter to store the device type.
 * 
 * @return - `0` - Execution was successful.
 * @return - `-1` - Internal `XIQueryDevice()` call failed.
 */
int xi_get_device_type(Display *display, int device_id, int *out_device_type);
