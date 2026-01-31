/**
 * This code is responsible for EWMH desktop/workspace compliance, allowing
 * external tools (pagers, taskbars, wmctrl, xdotool) to query and interact
 * with the workspace system.
 *
 * Implements:
 * - _NET_NUMBER_OF_DESKTOPS: Total workspace count.
 * - _NET_CURRENT_DESKTOP: Active workspace index.
 * - _NET_WM_DESKTOP: Per-window workspace assignment.
 * - _NET_DESKTOP_NAMES: Human-readable workspace names.
 *
 * https://specifications.freedesktop.org/wm-spec/1.5/ar01s03.html#id-1.4.2
 */

#include "../all.h"

static void set_number_of_desktops()
{
    Display *display = DefaultDisplay;
    Window root = DefaultRootWindow(display);

    // Set the `_NET_NUMBER_OF_DESKTOPS` property on the root window.
    Atom _NET_NUMBER_OF_DESKTOPS = XInternAtom(display, "_NET_NUMBER_OF_DESKTOPS", False);
    unsigned long count = MAX_WORKSPACES;
    XChangeProperty(
        display,
        root,
        _NET_NUMBER_OF_DESKTOPS,
        XA_CARDINAL,
        32,
        PropModeReplace,
        (unsigned char *)&count,
        1
    );
}

static void set_current_desktop(int workspace)
{
    Display *display = DefaultDisplay;
    Window root = DefaultRootWindow(display);

    // Set the `_NET_CURRENT_DESKTOP` property on the root window.
    Atom _NET_CURRENT_DESKTOP = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);
    unsigned long desktop = workspace;
    XChangeProperty(
        display,
        root,
        _NET_CURRENT_DESKTOP,
        XA_CARDINAL,
        32,
        PropModeReplace,
        (unsigned char *)&desktop,
        1
    );
}

static void set_desktop_names()
{
    Display *display = DefaultDisplay;
    Window root = DefaultRootWindow(display);

    Atom _NET_DESKTOP_NAMES = XInternAtom(display, "_NET_DESKTOP_NAMES", False);
    Atom UTF8_STRING = XInternAtom(display, "UTF8_STRING", False);

    // Build null-separated string of workspace names.
    // Format: "1\02\03\04\05\06\0"
    char names[MAX_WORKSPACES * 2];
    int offset = 0;
    for (int i = 0; i < MAX_WORKSPACES; i++)
    {
        names[offset++] = '1' + i;
        names[offset++] = '\0';
    }

    // Set the `_NET_DESKTOP_NAMES` property on the root window.
    XChangeProperty(
        display,
        root,
        _NET_DESKTOP_NAMES,
        UTF8_STRING,
        8,
        PropModeReplace,
        (unsigned char *)names,
        offset
    );
}

static void set_window_desktop(Window window, int workspace)
{
    Display *display = DefaultDisplay;

    // Set the `_NET_WM_DESKTOP` property on the specified window.
    Atom _NET_WM_DESKTOP = XInternAtom(display, "_NET_WM_DESKTOP", False);
    unsigned long desktop = workspace;
    XChangeProperty(
        display,
        window,
        _NET_WM_DESKTOP,
        XA_CARDINAL,
        32,
        PropModeReplace,
        (unsigned char *)&desktop,
        1
    );
}

HANDLE(Initialize)
{
    // Initialize all EWMH desktop properties on the root window.
    set_number_of_desktops();
    set_current_desktop(0);
    set_desktop_names();
}

HANDLE(WorkspaceSwitched)
{
    WorkspaceSwitchedEvent *_event = &event->workspace_switched;

    // Update the `_NET_CURRENT_DESKTOP` property to reflect the new workspace.
    set_current_desktop(_event->new_workspace);
}

HANDLE(PortalInitialized)
{
    Portal *portal = event->portal_initialized.portal;

    // Set `_NET_WM_DESKTOP` for top-level windows.
    if (portal->top_level)
    {
        set_window_desktop(portal->client_window, portal->workspace);
    }
}

HANDLE(PortalWorkspaceChanged)
{
    PortalWorkspaceChangedEvent *_event = &event->portal_workspace_changed;
    Portal *portal = _event->portal;

    // Update `_NET_WM_DESKTOP` for top-level windows.
    if (portal->top_level)
    {
        set_window_desktop(portal->client_window, _event->new_workspace);
    }
}
