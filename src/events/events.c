#include "../all.h"

static bool is_event_loop_initialized = false;

static const long x_root_event_mask =
    ExposureMask |
    SubstructureRedirectMask |
    SubstructureNotifyMask;

static const long xi_root_event_mask =
    XI_RawButtonPressMask |
    XI_RawButtonReleaseMask |
    XI_RawMotionMask;

static void handle_x_event(Display *display, Window window, XEvent *event)
{
    invoke_event_handlers(display, window, event->type, event);
}

static void handle_xi_event(Display *display, Window window, XEvent *event)
{
    // XInput2 event data doesn't fit in standard XEvent struct, so we need to
    // explicitly request the additional data with XGetEventData. This data must
    // later be freed with XFreeEventData to avoid memory leaks.
    if(XGetEventData(display, &event->xcookie) == False) return;
    XIRawEvent *raw_event = (XIRawEvent *)event->xcookie.data;

    // Retrieve the device type in order to check whether the event is coming
    // from from a master device. We only want to handle master device events
    // and ignore slave devices.
    int device_type;
    if(xi_get_device_type(display, raw_event->deviceid, &device_type) == -1)
    {
        XFreeEventData(display, &event->xcookie);
        return;
    }
    bool is_master_device = device_type == XIMasterPointer || device_type == XIMasterKeyboard;
    if(!is_master_device)
    {
        XFreeEventData(display, &event->xcookie);
        return;
    }

    // Convert raw XInput2 event to a normal X event and invoke the appropriate
    // event handlers.
    if(event->xcookie.evtype == XI_RawButtonPress)
    {
        XEvent new_event = xi_convert_raw_button_press_event(display, window, raw_event);
        invoke_event_handlers(display, window, GlobalButtonPress, &new_event);
    }
    if(event->xcookie.evtype == XI_RawButtonRelease)
    {
        XEvent new_event = xi_convert_raw_button_release_event(display, window, raw_event);
        invoke_event_handlers(display, window, GlobalButtonRelease, &new_event);
    }
    if(event->xcookie.evtype == XI_RawMotion)
    {
        XEvent new_event = xi_convert_raw_motion_event(display, window, raw_event);
        invoke_event_handlers(display, window, GlobalMotionNotify, &new_event);
    }

    XFreeEventData(display, &event->xcookie);
}

void initialize_event_loop(Display *display, Window root_window)
{
    if (is_event_loop_initialized == true)
    {
        LOG_ERROR("Attempted to initialize duplicate event loop.");
        return;
    }

    int xi_opcode;
    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &(int){0}, &(int){0}))
    {
        LOG_ERROR("XInput2 extension not available.");
        return;
    }

    is_event_loop_initialized = true;

    // Choose which root window events we should listen for.
    XSelectInput(display, root_window, x_root_event_mask);
    xi_select_input(display, root_window, xi_root_event_mask);

    // Invoke all preparation and initialization handler functions.
    invoke_event_handlers(display, root_window, Prepare, NULL);
    invoke_event_handlers(display, root_window, Initialize, NULL);

    // Send an initial Expose event.
    XWindowAttributes root_window_attr;
    XGetWindowAttributes(display, root_window, &root_window_attr);
    XEvent custom_event;
    custom_event.type = Expose;
    custom_event.xexpose.window = root_window;
    custom_event.xexpose.display = display;
    custom_event.xexpose.x = 0;
    custom_event.xexpose.y = 0;
    custom_event.xexpose.width = root_window_attr.width;
    custom_event.xexpose.height = root_window_attr.height;
    custom_event.xexpose.count = 0;
    XSendEvent(display, root_window, False, ExposureMask, &custom_event);
    XFlush(display);

    // Start the event loop.
    XEvent event;
    bool is_xi_event;
    while (1)
    {
        XNextEvent(display, &event);
        is_xi_event = event.type == GenericEvent && event.xcookie.extension == xi_opcode;

        if(is_xi_event)
        {
            handle_xi_event(display, root_window, &event);
        }
        else
        {
            handle_x_event(display, root_window, &event);
        }
    }
}
