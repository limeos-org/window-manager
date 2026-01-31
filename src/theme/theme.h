#pragma once

/** A type representing an RGBA color value. */
typedef struct {
    double r, g, b, a;
} ThemeColorRGBA;

/** A type representing an RGB color value. */
typedef struct {
    double r, g, b;
} ThemeColorRGB;

/** A type representing the theme variant (light or dark). */
typedef enum {
    THEME_VARIANT_LIGHT,
    THEME_VARIANT_DARK
} ThemeVariant;

/** A type representing the complete theme configuration. */
typedef struct {
    ThemeVariant variant;
    ThemeColorRGBA titlebar_bg;
    ThemeColorRGBA titlebar_text;
    ThemeColorRGBA titlebar_border;
    ThemeColorRGBA titlebar_separator;
} Theme;

/** Returns the current theme based on system color scheme preference. */
const Theme* get_current_theme(void);

/** Returns the D-Bus file descriptor for theme updates, or -1 if unavailable. */
int get_theme_dbus_fd(void);

/** Dispatches pending D-Bus messages for theme updates. */
void dispatch_theme_dbus(void);
