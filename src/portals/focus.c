#include "../all.h"

void focus_portal(Portal *portal)
{
    Display *display = portal->display;
    Window frame_window = portal->frame_window;

    // Raise the frame window and set input focus to it.
    XRaiseWindow(display, frame_window);
    XSetInputFocus(display, frame_window, RevertToParent, CurrentTime);

    // Invoke the PortalFocused event.
    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.display = display;
    event.xclient.window = DefaultRootWindow(display);
    event.xclient.message_type = XInternAtom(display, "_WM_PORTAL_FOCUSED", False);
    event.xclient.format = 32;
    event.xclient.data.l[0] = (long)portal;
    invoke_event_handlers(display, DefaultRootWindow(display), PortalFocused, &event);
}

HANDLE(GlobalButtonPress)
{
    XButtonEvent *_event = &event->xbutton;

    if (_event->button != Button1) return;

    Portal *portal = find_portal(_event->window);
    if(portal == NULL) return;
    
    focus_portal(portal);
}
