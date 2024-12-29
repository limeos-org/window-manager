#pragma once
#include "../all.h"

/**
 * @brief Retrieves the name of an X11 window.
 * 
 * Attempts to retrieve the name of the window using the `_NET_WM_NAME` property
 * first. If unsuccesful, it will try to retrieve the name using the `WM_NAME`
 * property, which is an older standard, but ensures compatibility with legacy
 * applications.
 * 
 * @param display The display where the window is located.
 * @param window The window to retrieve the name for.
 * @param out_name The buffer where the name will be stored.
 * @param name_size The size of the `out_name` buffer.
 * 
 * @return 0 on success, non-zero integer otherwise.
 */
int x_get_window_name(Display *display, Window window, char *out_name, size_t name_size);

/**
 * @brief X11 error handler for non-fatal X protocol errors.
 * 
 * Called by X server when protocol errors occur. Logs the error message
 * but allows program to continue running, unlike the default Xlib error
 * handler which would terminate the program.
 * 
 * @param display The display where the error occured.
 * @param error The error event.
 * @return Always returns 0 to indicate that the error has been handled. This
 * is necessary to prevent the default Xlib error handler from being invoked.
 * 
 * @note Intended to be set as the Xlib error handler using `XSetErrorHandler()`.
 */
int x_error_handler(Display *display, XErrorEvent *error);

/**
 * @brief Checks if a window supports a given protocol.
 * 
 * @param display The X11 display.
 * @param window The window to check.
 * @param protocol The protocol to check for.
 * 
 * @return True (1) if the protocol is supported, False (0) otherwise.
 */
bool x_window_supports_protocol(Display *display, Window window, Atom protocol);

/**
 * @brief Checks if a window exists.
 * 
 * @param display The X11 display.
 * @param window The window to check.
 * 
 * @return True (1) if the window exists, False (0) otherwise.
 */
bool x_window_exists(Display *display, Window window);
