/**
 * This code is responsible for setting up the EWMH identification chain and
 * the list of supported EWMH features on the root window. This is required by
 * the EWMH specification to identify the window manager and its supported
 * features.
 * 
 * https://specifications.freedesktop.org/wm-spec/1.5/ar01s03.html#id-1.4.12
 * https://specifications.freedesktop.org/wm-spec/1.5/ar01s03.html#id-1.4.3
 */

#include "../all.h"

static void setup_ewmh_identification_chain()
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    Atom _NET_SUPPORTING_WM_CHECK = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
    Atom _NET_WM_NAME = XInternAtom(display, "_NET_WM_NAME", False);
    Atom UTF8_STRING = XInternAtom(display, "UTF8_STRING", False);

    // Create a hidden check window.
    Window check_window = x_create_simple_window(display, root_window, -1, -1, 1, 1, 0, 0, 0);

    // Set the `_NET_SUPPORTING_WM_CHECK` property on the root window pointing 
    // to our check window. Starting the identification chain.
    XChangeProperty(
        display,                        // Display
        root_window,                    // Window
        _NET_SUPPORTING_WM_CHECK,       // Property
        XA_WINDOW,                      // Type
        32,                             // Format (32-bit)
        PropModeReplace,                // Mode
        (unsigned char *)&check_window, // Property Data
        1                               // Element Count
    );

    // Set the `_NET_SUPPORTING_WM_CHECK` property on the check window, pointing 
    // to itself. Completing the identification chain.
    XChangeProperty(
        display,                        // Display
        check_window,                   // Window
        _NET_SUPPORTING_WM_CHECK,       // Property
        XA_WINDOW,                      // Type
        32,                             // Format (32-bit)
        PropModeReplace,                // Mode
        (unsigned char *)&check_window, // Property Data
        1                               // Element Count
    );

    // Set the `_NET_WM_NAME` property on the check window to identify the
    // window manager. This is the name that applications will see when querying
    // the window manager name.
    const char *wm_name = "LimeOS Window Manager";
    XChangeProperty(
        display,                        // Display
        check_window,                   // Window
        _NET_WM_NAME,                   // Property
        UTF8_STRING,                    // Type
        8,                              // Format (8-bit)
        PropModeReplace,                // Mode
        (unsigned char *)wm_name,       // Property Data
        strlen(wm_name)                 // Element Count
    );
}

static void setup_ewmh_supported_list()
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Define a list of supported EWMH features.
    Atom features[] = {
        XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False),
        XInternAtom(display, "_NET_WM_NAME", False),
        XInternAtom(display, "_NET_CLIENT_LIST", False),
        XInternAtom(display, "_NET_WM_ACTION_MOVE", False),
        XInternAtom(display, "_NET_WM_ACTION_RESIZE", False),
        XInternAtom(display, "_NET_WM_MOVERESIZE", False),
        XInternAtom(display, "_NET_MOVERESIZE_WINDOW", False),
        XInternAtom(display, "_NET_WM_WINDOW_TYPE", False),
        XInternAtom(display, "_NET_WM_STATE", False),
        XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False),
        XInternAtom(display, "_NET_NUMBER_OF_DESKTOPS", False),
        XInternAtom(display, "_NET_CURRENT_DESKTOP", False),
        XInternAtom(display, "_NET_WM_DESKTOP", False),
        XInternAtom(display, "_NET_DESKTOP_NAMES", False),
        XInternAtom(display, "_NET_CLOSE_WINDOW", False)
    };

    // Set the `_NET_SUPPORTED` property on the root window, listing all the
    // EWMH features that our window manager supports.
    Atom _NET_SUPPORTED = XInternAtom(display, "_NET_SUPPORTED", False);
    XChangeProperty(
        display,                            // Display
        root_window,                        // Window
        _NET_SUPPORTED,                     // Property
        XA_ATOM,                            // Type
        32,                                 // Format (32-bit)
        PropModeReplace,                    // Mode
        (unsigned char *)features,          // Property Data
        sizeof(features) / sizeof(Atom)     // Element Count
    );
}

HANDLE(Initialize)
{
    setup_ewmh_identification_chain();
    setup_ewmh_supported_list();
}
