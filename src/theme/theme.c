/**
 * This code is responsible for theme management.
 *
 * It queries the system color scheme preference via XDG Desktop Portal
 * and updates title bar appearance accordingly.
 */

#include "../all.h"

static const Theme light_theme = {
    .variant = THEME_VARIANT_LIGHT,
    .titlebar_bg = { 0.95, 0.95, 0.95, 1.0 },
    .titlebar_text = { 0.05, 0.05, 0.05, 1.0 },
    .titlebar_border = { 0.0, 0.0, 0.0, 0.15 },
    .titlebar_separator = { 0.0, 0.0, 0.0 }
};

static const Theme dark_theme = {
    .variant = THEME_VARIANT_DARK,
    .titlebar_bg = { 0.141, 0.141, 0.141, 1.0 },
    .titlebar_text = { 1.0, 1.0, 1.0, 1.0 },
    .titlebar_border = { 1.0, 1.0, 1.0, 0.15 },
    .titlebar_separator = { 1.0, 1.0, 1.0 }
};

static const Theme *current = &light_theme;

static DBusConnection *dbus_connection = NULL;

static void apply_color_scheme(uint32_t scheme)
{
    // Determine theme based on scheme value (1 = prefer-dark).
    current = (scheme == 1) ? &dark_theme : &light_theme;
}

static uint32_t query_color_scheme(void)
{
    // Ensure D-Bus connection exists.
    if (!dbus_connection) return 0;

    // Build the method call message.
    DBusMessage *message = dbus_message_new_method_call(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Settings",
        "Read");
    if (!message) return 0;

    const char *namespace = "org.freedesktop.appearance";
    const char *key = "color-scheme";
    dbus_message_append_args(message,
        DBUS_TYPE_STRING, &namespace,
        DBUS_TYPE_STRING, &key,
        DBUS_TYPE_INVALID);

    // Send the message and wait for reply.
    DBusError error;
    dbus_error_init(&error);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(
        dbus_connection, message, 1000, &error
    );
    dbus_message_unref(message);
    if (!reply)
    {
        dbus_error_free(&error);
        return 0;
    }

    // Extract the color scheme value from nested variants.
    DBusMessageIter iter, variant, inner;
    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_recurse(&iter, &variant);
    dbus_message_iter_recurse(&variant, &inner);
    uint32_t scheme = 0;
    dbus_message_iter_get_basic(&inner, &scheme);
    dbus_message_unref(reply);

    return scheme;
}

static DBusHandlerResult handle_settings_signal(
    DBusConnection *connection, DBusMessage *message, void *data)
{
    (void)connection;
    (void)data;

    // Ignore signals that are not SettingChanged.
    if (!dbus_message_is_signal(message,
        "org.freedesktop.portal.Settings", "SettingChanged"))
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    // Extract namespace and key from the signal.
    DBusMessageIter iter;
    dbus_message_iter_init(message, &iter);
    const char *namespace, *key;
    dbus_message_iter_get_basic(&iter, &namespace);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, &key);

    // Ignore settings that are not color-scheme.
    if (strcmp(namespace, "org.freedesktop.appearance") != 0 ||
        strcmp(key, "color-scheme") != 0)
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    // Extract the new color scheme value.
    dbus_message_iter_next(&iter);
    DBusMessageIter variant;
    dbus_message_iter_recurse(&iter, &variant);
    uint32_t scheme;
    dbus_message_iter_get_basic(&variant, &scheme);

    // Apply the new theme and notify listeners.
    apply_color_scheme(scheme);
    call_event_handlers((Event*)&(ThemeChangedEvent){
        .type = ThemeChanged
    });

    return DBUS_HANDLER_RESULT_HANDLED;
}

const Theme* get_current_theme(void)
{
    return current;
}

int get_theme_dbus_fd(void)
{
    if (!dbus_connection) return -1;

    int fd;
    if (!dbus_connection_get_unix_fd(dbus_connection, &fd)) return -1;
    
    return fd;
}

void dispatch_theme_dbus(void)
{
    if (!dbus_connection) return;

    // Read incoming data.
    dbus_connection_read_write(dbus_connection, 0);

    // Dispatch all buffered messages.
    while (dbus_connection_dispatch(dbus_connection) == DBUS_DISPATCH_DATA_REMAINS);
}

HANDLE(Initialize)
{
    // Check if theme is set explicitly in config.
    char theme_config[CONFIG_MAX_VALUE_LENGTH];
    common.get_config_str(theme_config, sizeof(theme_config), CFG_KEY_THEME, CFG_DEFAULT_THEME);

    if (strcmp(theme_config, "light") == 0)
    {
        current = &light_theme;
        return;
    }
    if (strcmp(theme_config, "dark") == 0)
    {
        current = &dark_theme;
        return;
    }

    // Theme is "listen", use signal value.
    DBusError error;
    dbus_error_init(&error);
    dbus_connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (!dbus_connection)
    {
        dbus_error_free(&error);
        return;
    }

    // Query and apply the initial color scheme.
    apply_color_scheme(query_color_scheme());

    // Subscribe to color scheme changes.
    dbus_bus_add_match(dbus_connection,
        "type='signal',"
        "interface='org.freedesktop.portal.Settings',"
        "member='SettingChanged'",
        &error
    );
    dbus_connection_add_filter(
        dbus_connection, handle_settings_signal, NULL, NULL
    );
}

