#pragma once
#include "../all.h"

/**
 * @brief Alias for the `x_get_default_display()` function.
 */
#define DefaultDisplay x_get_default_display()

/**
 * @brief Sets the default X11 display for global access across the codebase.
 * 
 * @param display The X11 display.
 * 
 * @note - The default display can be retrieved using the `DefaultDisplay` 
 * macro.
 */
void x_set_default_display(Display *display);

/**
 * @brief Retrieves the default X11 display for global access across the 
 * codebase.
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
 * @brief Retrieves the current time.
 * 
 * @return - `Time` - The current time.
 */
Time x_get_current_time();

/**
 * @brief Retrieves the process ID of the X client that owns the window.
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
 * @brief Retrieves the parent window of a given window.
 * 
 * @param display The X11 display.
 * @param window The target window.
 * 
 * @return - `Window` - The parent window.
 * @return - `0` - The parent window could not be retrieved.
 */
Window x_get_window_parent(Display *display, Window window);

/**
 * @brief Retrieves the name of an X11 window.
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
 * @brief Checks if a window supports a given protocol.
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
 * @brief Checks if a window exists.
 * 
 * @param display The X11 display.
 * @param window The window to check.
 * 
 * @return - `true` - The window exists.
 * @return - `false` - The window does not exist.
 */
bool x_window_exists(Display *display, Window window);

/**
 * @brief Converts a key name to a key symbol.
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
 * @brief Converts multiple key names to multiple key symbols.
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
 * @brief Queries the children of a window recursively.
 * 
 * @param display The X11 display.
 * @param parent The parent window.
 * @param out_children The buffer where the children will be stored.
 * @param out_children_count The buffer where the children count will be stored.
 * 
 * @return - `0` - The query was successful.
 * @return - `-1` - The query failed.
 * 
 * @warning - Recursive X server calls make this function slow and 
 * resource-intensive.
 */
int x_query_tree_recursively(Display *display, Window parent, Window **out_children, unsigned int *out_children_count);

/**
 * @brief A wrapper of the `XCreateSimpleWindow()` Xlib function, with some 
 * minor additional functionality.
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
 * @return - `Window` - The created window.
 */
Window x_create_simple_window(
    Display *display,
    Window parent,
    int x, int y,
    unsigned int width, unsigned int height,
    unsigned int border_width, unsigned long border_pixel,
    unsigned long background
);
