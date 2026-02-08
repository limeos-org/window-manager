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
    THEME_VARIANT_UNRESOLVED,
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

/** A type representing the active theme mode. */
typedef enum {
    THEME_MODE_LIGHT,
    THEME_MODE_DARK,
    THEME_MODE_ADAPTIVE
} ThemeMode;

struct Portal;

/** Returns the theme for a specific portal based on the active theme mode. */
const Theme* get_portal_theme(struct Portal *portal);

/** Returns the active theme mode. */
ThemeMode get_theme_mode(void);
