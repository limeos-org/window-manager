/**
 * This code is responsible for theme management.
 *
 * It resolves titlebar colors per-portal: in adaptive mode the variant
 * is determined by sampling client content luminance; in light or dark
 * mode a single variant is applied globally.
 */

#include "../all.h"

static const Theme light_theme = {
    .variant = THEME_VARIANT_LIGHT,
    .titlebar_bg = { 0.95, 0.95, 0.95, 1.0 },
    .titlebar_text = { 0.05, 0.05, 0.05, 1.0 },
    .titlebar_border = { 0.0, 0.0, 0.0, 0.15 },
    .titlebar_separator = { 0.0, 0.0, 0.0, 0.15 }
};

static const Theme dark_theme = {
    .variant = THEME_VARIANT_DARK,
    .titlebar_bg = { 0.141, 0.141, 0.141, 1.0 },
    .titlebar_text = { 1.0, 1.0, 1.0, 1.0 },
    .titlebar_border = { 1.0, 1.0, 1.0, 0.15 },
    .titlebar_separator = { 1.0, 1.0, 1.0, 0.15 }
};

static ThemeMode theme_mode = THEME_MODE_ADAPTIVE;

const Theme* get_portal_theme(Portal *portal)
{
    if (portal != NULL && portal->theme == THEME_VARIANT_DARK)
    {
        return &dark_theme;
    }
    return &light_theme;
}

ThemeMode get_theme_mode(void)
{
    return theme_mode;
}

HANDLE(PortalCreated)
{
    PortalCreatedEvent *_event = &event->portal_created;

    // Set the initial theme variant based on the active mode. In adaptive mode 
    // neither condition matches, so the portal stays `THEME_VARIANT_UNRESOLVED`
    // until the compositor samples luminance.
    if (theme_mode == THEME_MODE_DARK)
    {
        _event->portal->theme = THEME_VARIANT_DARK;
    }
    else if (theme_mode == THEME_MODE_LIGHT)
    {
        _event->portal->theme = THEME_VARIANT_LIGHT;
    }
}

HANDLE(Initialize)
{
    // Read the theme setting from the configuration.
    char theme_config[CONFIG_MAX_VALUE_LENGTH];
    common.get_config_str(theme_config, sizeof(theme_config), CFG_KEY_THEME, CFG_DEFAULT_THEME);

    // Parse the theme mode.
    if (strcmp(theme_config, "light") == 0)
    {
        theme_mode = THEME_MODE_LIGHT;
    }
    else if (strcmp(theme_config, "dark") == 0)
    {
        theme_mode = THEME_MODE_DARK;
    }
    else
    {
        theme_mode = THEME_MODE_ADAPTIVE;
    }
}
