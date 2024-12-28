#include "../all.h"

/**
 * This code is responsible for managing the `_NET_CLIENT_LIST` property on
 * the root window, which is an array of client windows that are managed by
 * the window manager. EWMH specification requires every window manager to
 * advertise this list so that other applications can query it.
 * 
 * @note https://specifications.freedesktop.org/wm-spec/1.5/ar01s03.html#id-1.4.4
 */

static Window *client_list = NULL;
static size_t client_count = 0;
static size_t client_capacity = 0;

static void publish_ewmh_client_list(Display *display, Window root_window) {
    Atom net_client_list = XInternAtom(display, "_NET_CLIENT_LIST", False);
    XChangeProperty(
        display,
        root_window,
        net_client_list,
        XA_WINDOW,
        32,
        PropModeReplace,
        (unsigned char *)client_list,
        client_count
    );
}

static void add_to_ewmh_client_list(Window window) {
    if (client_count >= client_capacity) {
        client_capacity = (client_capacity == 0) ? 16 : client_capacity * 2;
        client_list = realloc(client_list, client_capacity * sizeof(Window));
    }
    client_list[client_count++] = window;
}

static void remove_from_ewmh_client_list(Window window) {
    for (size_t i = 0; i < client_count; i++) {
        if (client_list[i] == window) {
            memmove(&client_list[i], &client_list[i + 1], (client_count - i - 1) * sizeof(Window));
            client_count--;
            break;
        }
    }
}

static Bool is_in_ewmh_client_list(Window window) {
    for (size_t i = 0; i < client_count; i++) {
        if (client_list[i] == window) {
            return True;
        }
    }
    return False;
}

HANDLE(MapRequest)
{
    XMapRequestEvent *_event = &event->xmaprequest;

    // Considering a MapRequest event can only be sent by clients, we can
    // safely assume that the window is a client window.
    Window client_window = _event->window;

    // Prevent duplicate entries in the client list.
    if (is_in_ewmh_client_list(client_window)) return;

    add_to_ewmh_client_list(client_window);
    publish_ewmh_client_list(display, root_window);
}

HANDLE(DestroyNotify)
{
    XDestroyWindowEvent *_event = &event->xdestroywindow;

    // The source of a DestroyNotify event is not guaranteed to be a client
    // window, so we need to check if it's in the list before removing it.
    if (!is_in_ewmh_client_list(_event->window)) return;

    remove_from_ewmh_client_list(_event->window);
    publish_ewmh_client_list(display, root_window);
}
