#pragma once
#include "../all.h"

/**
 * Alias for the `x_get_default_display()` function.
 */
#define DefaultDisplay x_get_default_display()

/**
 * Sets the default X11 display for global access across the codebase.
 * 
 * @param display The X11 display.
 * 
 * @note - The default display can be retrieved using the `DefaultDisplay` 
 * macro.
 */
void x_set_default_display(Display *display);

/**
 * Retrieves the default X11 display for global access across the codebase.
 * 
 * @return - `Display*` - The default X11 display.
 * @return - `NULL` - The default display has not been set.
 * 
 * @warning - Don't use directly! Use the `DefaultDisplay` macro instead.
 * 
 * @note - The default display can be set using the `x_set_default_display()` 
 * function.
 */
Display *x_get_default_display();

/**
 * Retrieves the current time.
 * 
 * @return - `Time` - The current time.
 */
Time x_get_current_time();

/**
 * Begins trapping X11 errors on the given display.
 *
 * While a trap is active, errors are silently recorded instead of being
 * passed to the normal error handler. Call `x_untrap_errors` to end the
 * trap, flush pending errors via `XSync`, and retrieve the error code.
 *
 * @param display The X11 display.
 *
 * @warning Not reentrant. Only one trap may be active at a time.
 */
void x_trap_errors(Display *display);

/**
 * Ends an active error trap, syncs with the server, and returns the
 * trapped error code (or 0 if no error occurred).
 *
 * @param display The X11 display.
 *
 * @return - `0` - No error occurred during the trap.
 * @return - `> 0` - The X11 error code of the first trapped error.
 */
int x_untrap_errors(Display *display);

/**
 * Retrieves the process ID of the X client that owns the window.
 * 
 * @param display The X11 display.
 * @param window The target window.
 * 
 * @return - `pid_t` - The process ID of the X client.
 * @return - `-1` - The process ID could not be retrieved.
 * 
 * @note - This function may fail as some X clients don't set the `_NET_WM_PID`
 * property.
 * @note - The `_NET_WM_PID` property is a part of the EWMH standard.
 */
pid_t x_get_window_pid(Display *display, Window window);

/**
 * Retrieves the parent window of a given window.
 * 
 * @param display The X11 display.
 * @param window The target window.
 * 
 * @return - `Window` - The parent window.
 * @return - `0` - The parent window could not be retrieved.
 */
Window x_get_window_parent(Display *display, Window window);

/**
 * Retrieves the name of an X11 window.
 * 
 * @param display The X11 display.
 * @param window The window to retrieve the name for.
 * @param out_name The buffer where the name will be stored.
 * @param name_size The size of the `out_name` buffer.
 * 
 * @return - `0` - The name was successfully retrieved.
 * @return - `-1` - The name could not be retrieved.
 */
int x_get_window_name(Display *display, Window window, char *out_name, size_t name_size);

/**
 * Checks if a window supports a given protocol.
 * 
 * @param display The X11 display.
 * @param window The window to check.
 * @param protocol The protocol to check for.
 * 
 * @return - `true` - The protocol is supported.
 * @return - `false` - The protocol is not supported.
 */
bool x_window_supports_protocol(Display *display, Window window, Atom protocol);

/**
 * Checks if a window exists.
 * 
 * @param display The X11 display.
 * @param window The window to check.
 * 
 * @return - `true` - The window exists.
 * @return - `false` - The window does not exist.
 */
bool x_window_exists(Display *display, Window window);

/**
 * Converts a keysym to its corresponding X11 modifier mask.
 *
 * @param keysym The keysym to convert.
 *
 * @return - The modifier mask (ShiftMask, ControlMask, Mod1Mask, Mod4Mask).
 * @return - `0` if the keysym is not a modifier key.
 */
unsigned int x_keysym_to_modifier(KeySym keysym);

/**
 * Converts a key name to a key symbol.
 * 
 * @param name The string containing the key name.
 * @param out_key The buffer where the key symbol will be stored.
 * 
 * @return - `0` - The conversion was succesful.
 * @return - `-1` - The conversion failed, key symbol output is `NoSymbol`.
 * 
 * @note - This function treats key names case-insensitively.
 * @note - If you want to convert multiple key names, use the 
 * `x_key_names_to_symbols()` function.
 */
int x_key_name_to_symbol(const char *name, int *out_key);

/**
 * Converts multiple key names to multiple key symbols.
 * 
 * @param names The string containing the key names.
 * @param delimiter The delimiter used to separate the key names.
 * @param out_keys The buffer where the key symbols will be stored.
 * @param keys_size The size of the `out_keys` buffer.
 * 
 * @return - `0` - The conversion was succesful.
 * @return - `-1` - The conversion was completed, but one or more key names were
 * invalid.
 * 
 * @note - This function treats key names case-insensitively.
 */
int x_key_names_to_symbols(char *names, const char delimiter, int *out_keys, int keys_size);

/**
 * Queries the children of a window recursively.
 * 
 * @param display The X11 display.
 * @param parent The parent window.
 * @param out_children The buffer where the children will be stored.
 * @param out_children_count The buffer where the children count will be stored.
 * 
 * @return - `0` - The query was completed succesfully.
 * @return - `-1` - The query failed, memory allocation error.
 * 
 * @warning - Recursive X server calls make this function slow and 
 * resource-intensive.
 */
int x_query_tree_recursively(Display *display, Window parent, Window **out_children, unsigned int *out_children_count);

/**
 * Checks if a window is a top-level window.
 * 
 * A top-level window, according to the ICCCM standard, is a window that has the
 * root window as its parent and is not an override-redirect window.
 * 
 * @param display The X11 display.
 * @param window The window to check.
 * 
 * @return - `true` - The window is a top-level window.
 * @return - `false` - The window is not a top-level window.
 */
bool x_window_is_top_level(Display *display, Window window);

/**
 * A wrapper of the `XCreateSimpleWindow()` Xlib function, with some minor
 * additional functionality.
 * 
 * The difference between this function and `XCreateSimpleWindow()` is that this
 * function also sets the `_NET_WM_PID` property to the window, containing the 
 * process ID of the X client that created the window.
 * 
 * @param display The X11 display.
 * @param parent The parent window.
 * @param x The X coordinate of the window.
 * @param y The Y coordinate of the window.
 * @param width The width of the window.
 * @param height The height of the window.
 * @param border_width The border width of the window.
 * @param border_pixel The border pixel of the window.
 * @param background The background color of the window.
 * 
 * @return The created window.
 */
Window x_create_simple_window(
    Display *display,
    Window parent,
    int x, int y,
    unsigned int width, unsigned int height,
    unsigned int border_width, unsigned long border_pixel,
    unsigned long background
);

/**
 * Sets the `WM_STATE` property on a window as required by ICCCM.
 *
 * @param display The X11 display.
 * @param window The target window.
 * @param state The WM state value (0 = WithdrawnState, 1 = NormalState,
 * 3 = IconicState).
 */
void x_set_wm_state(Display *display, Window window, unsigned long state);

/**
 * Retrieves the transient-for window hint for a given window.
 *
 * @param display The X11 display.
 * @param window The window to query.
 * @param out_transient_for Pointer to store the transient-for window.
 *
 * @return - `0` - Success, the window is transient for another window.
 * @return - `-1` - The window is not transient (no WM_TRANSIENT_FOR hint).
 *
 * @note - Per ICCCM, transient windows (dialogs, popups) should be stacked
 * above their parent window.
 */
int x_get_transient_for(Display *display, Window window, Window *out_transient_for);

/**
 * Checks if a window wants decorations based on _MOTIF_WM_HINTS.
 *
 * @param display The X11 display.
 * @param window The window to query.
 *
 * @return - `true` - The window wants decorations (or no hint set).
 * @return - `false` - The window explicitly requests no decorations.
 */
bool x_window_wants_decorations_motif(Display *display, Window window);

/**
 * Checks if a window wants decorations based on EWMH _NET_WM_WINDOW_TYPE.
 *
 * @param display The X11 display.
 * @param window_type The window type atom from _NET_WM_WINDOW_TYPE.
 *
 * @return - `true` - The window type expects decorations.
 * @return - `false` - The window type should not have decorations.
 */
bool x_window_wants_decorations_ewmh(Display *display, Atom window_type);

/**
 * Retrieves the window type from the _NET_WM_WINDOW_TYPE property.
 *
 * @param display The X11 display.
 * @param window The window to query.
 *
 * @return - The first atom from _NET_WM_WINDOW_TYPE property.
 * @return - _NET_WM_WINDOW_TYPE_NORMAL if the property is not set.
 */
Atom x_get_window_type(Display *display, Window window);

/**
 * Retrieves the workspace index from the _NET_WM_DESKTOP property.
 *
 * @param display The X11 display.
 * @param window The target window.
 *
 * @return - `>= 0` - The workspace index.
 * @return - `-1` - The property is not set or could not be read.
 */
int x_get_window_desktop(Display *display, Window window);

/**
 * Sets input focus to a window, but only if it is currently viewable.
 *
 * Avoids the `BadMatch` error that `XSetInputFocus` produces when the
 * target window is not viewable (e.g. due to map/unmap races).
 *
 * @param display The X11 display.
 * @param window The target window.
 *
 * @return - `true` Focus was set successfully.
 * @return - `false` The window was not viewable; focus was not changed.
 */
bool x_focus_window(Display *display, Window window);

/**
 * Reads luminance from an already-fetched XImage at (x, y).
 *
 * Uses BT.601 coefficients: 0.299*R + 0.587*G + 0.114*B.
 *
 * @param image The XImage to read from.
 * @param x The x coordinate within the image.
 * @param y The y coordinate within the image.
 *
 * @return Luminance value from 0.0 (dark) to 1.0 (light).
 */
float x_pixel_luminance(XImage *image, int x, int y);

/**
 * Samples the average luminance of a rectangular region of a pixmap.
 *
 * Fetches the region via XGetImage, computes per-pixel BT.601 luminance,
 * and returns the average.
 *
 * @param display The X11 display.
 * @param pixmap The pixmap to sample from.
 * @param x The x offset of the region.
 * @param y The y offset of the region.
 * @param width The width of the region in pixels.
 * @param height The height of the region in pixels.
 *
 * @return Average luminance from 0.0 (dark) to 1.0 (light),
 *         or -1.0 if the region could not be read.
 */
float x_average_luminance(Display *display, Pixmap pixmap, int x, int y, int width, int height);

/**
 * Reads the `WM_CLASS` `res_class` string into the provided buffer.
 *
 * @param display The X11 display.
 * @param window The window to query.
 * @param out_buffer The buffer to store the class string.
 * @param buffer_size The size of the buffer.
 *
 * @return - `0` The class was successfully read.
 * @return - `-1` The class could not be read.
 */
int x_get_window_class(Display *display, Window window, char *out_buffer, size_t buffer_size);
