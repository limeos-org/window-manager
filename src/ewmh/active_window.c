/**
 * This code is responsible for setting the `_NET_ACTIVE_WINDOW` property on the
 * root window. EWMH specification requires window managers to advertise this
 * property so other applications can determine the currently active window.
 *
 * @note https://specifications.freedesktop.org/wm-spec/1.5/ar01s03.html#id-1.4.10
 */

#include "../all.h"

static Window current_active_window = 0;

static void set_ewmh_active_window(Window client_window)
{
    Display *display = DefaultDisplay;

    // Update our internal state.
    current_active_window = client_window;

    // Set the `_NET_ACTIVE_WINDOW` property.
    Atom _NET_ACTIVE_WINDOW = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    XChangeProperty(
        display,                            // Display
        DefaultRootWindow(display),         // Window
        _NET_ACTIVE_WINDOW,                 // Atom / Property
        XA_WINDOW,                          // Type
        32,                                 // Format (32-bit)
        PropModeReplace,                    // Mode
        (unsigned char *)&client_window,    // Data
        1                                   // Element Count
    );
}

HANDLE(Initialize)
{
    set_ewmh_active_window(0);
}

HANDLE(PortalFocused)
{
    PortalFocusedEvent *_event = &event->portal_focused;
    Portal *portal = _event->portal;

    set_ewmh_active_window(portal->client_window);
}

HANDLE(PortalDestroyed)
{
    PortalDestroyedEvent *_event = &event->portal_destroyed;

    if (current_active_window == _event->portal->client_window)
    {
        set_ewmh_active_window(0);
    }
}
