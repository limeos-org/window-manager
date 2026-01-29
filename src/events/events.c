#include "../all.h"

static Time last_update_time = 0;
static Time last_select_time = 0;
static Time throttle_ms = 0;

static const long x_root_event_mask =
    StructureNotifyMask |
    SubstructureRedirectMask |
    SubstructureNotifyMask;

static const long xi_root_event_mask =
    XI_RawButtonPressMask |
    XI_RawButtonReleaseMask |
    XI_RawMotionMask |
    XI_RawKeyPressMask |
    XI_RawKeyReleaseMask;

void initialize_event_loop()
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Retrieve the XInput2 extension opcode.
    int xi_opcode;
    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &(int){0}, &(int){0}))
    {
        LOG_ERROR("Could not retrieve opcode of XInput2 extension.");
        exit(EXIT_FAILURE);
    }

    // Select which events we should listen for on the root window.
    XSelectInput(display, root_window, x_root_event_mask);
    xi_select_input(display, root_window, xi_root_event_mask);

    // Call all event handlers of the Prepare event.
    call_event_handlers((Event*)&(PrepareEvent){
        .type = Prepare
    });

    // Call all event handlers of the Initialize event.
    call_event_handlers((Event*)&(InitializeEvent){
        .type = Initialize
    });

    while (true)
    {
        // Calculate timeout to enforce minimum spacing between iterations.
        Time current_time = x_get_current_time();
        Time since_last_select = current_time - last_select_time;
        Time remaining_time = (since_last_select < throttle_ms)
            ? (throttle_ms - since_last_select)
            : 0;
        struct timeval timeout = {
            .tv_sec = 0,
            .tv_usec = remaining_time * 1000
        };

        // Block until an X event or D-Bus message is received, or timeout.
        fd_set read_fd_set;
        FD_ZERO(&read_fd_set);
        int display_fd = ConnectionNumber(display);
        int dbus_fd = get_theme_dbus_fd();
        int highest_fd = display_fd;
        FD_SET(display_fd, &read_fd_set);
        if (dbus_fd >= 0)
        {
            FD_SET(dbus_fd, &read_fd_set);
            if (dbus_fd > highest_fd) highest_fd = dbus_fd;
        }
        select(highest_fd + 1, &read_fd_set, NULL, NULL, &timeout);

        // Record when we woke up to enforce spacing on next iteration.
        last_select_time = x_get_current_time();

        // Dispatch D-Bus messages if available.
        if (dbus_fd >= 0 && FD_ISSET(dbus_fd, &read_fd_set))
        {
            dispatch_theme_dbus();
        }

        // Process pending X events in batches. Without a limit, a flood of
        // events (e.g., rapid mouse movement) could starve the Update event,
        // preventing compositor redraws and freezing the UI.
        int events_processed = 0;
        const int max_events_per_iteration = 50;
        while (XPending(display) > 0 && events_processed < max_events_per_iteration)
        {
            events_processed++;

            // Retrieve the next X event.
            XEvent x_event;
            XNextEvent(display, &x_event);

            // Downcast the X event to a standard event.
            Event *event = (Event*)&x_event;

            // Check if the X event originated from the XInput2 extension, if it
            // did, convert it to a more developer-friendly event type.
            if (event->type == GenericEvent && event->xcookie.extension == xi_opcode)
            {
                // Extract the XInput2 event data.
                XGenericEventCookie *cookie = &x_event.xcookie;
                XGetEventData(display, cookie);
                XIRawEvent *xi_raw_event = cookie->data;

                // Ignore the event if it originated from a slave device.
                int device_type = 0;
                xi_get_device_type(display, xi_raw_event->deviceid, &device_type);
                if (device_type != XIMasterPointer && device_type != XIMasterKeyboard)
                {
                    XFreeEventData(display, cookie);
                    continue;
                }

                // Construct a new event from the XInput2 event data.
                Event new_event = convert_raw_xinput_event(xi_raw_event);
                event = &new_event;

                // Cleanup the original cookie data.
                XFreeEventData(display, cookie);
            }

            // Call the appropriate event handlers.
            call_event_handlers(event);
        }

        // Get fresh time after processing events for accurate Update timing.
        Time update_check_time = x_get_current_time();

        // Check if sufficient time has passed since the last update.
        if (update_check_time - last_update_time >= throttle_ms)
        {
            // Call all event handlers of the Update event.
            call_event_handlers((Event*)&(UpdateEvent){
                .type = Update
            });

            // Update the last update time.
            last_update_time = update_check_time;
        }
    }
}

HANDLE(Initialize)
{
    // Get the framerate from the configuration.
    int framerate;
    get_config_int(&framerate, CFG_KEY_FRAMERATE, CFG_DEFAULT_FRAMERATE);

    // Convert the framerate to a throttle time and store it.
    throttle_ms = (Time)framerate_to_throttle_ms(framerate);
}
