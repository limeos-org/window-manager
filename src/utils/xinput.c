#include "../all.h"

int xi_select_input(Display *display, Window window, long mask)
{
    XIEventMask event_mask;
    unsigned char mask_bytes[XIMaskLen(XI_LASTEVENT)] = {0};
    event_mask.deviceid = XIAllDevices;
    event_mask.mask_len = sizeof(mask_bytes);
    event_mask.mask = mask_bytes;

    for (int i = 0; i < XI_LASTEVENT; i++) {
        if (mask & (1UL << i)) {
            XISetMask(event_mask.mask, i);
        }
    }

    int status = XISelectEvents(display, window, &event_mask, 1);

    // Convert XInput2 success value (1) to standard convention (0).
    return status == 1 ? 0 : -1;
}

int xi_get_device_type(Display *display, int device_id, int *out_device_type)
{
    int num_devices;
    XIDeviceInfo *devices = XIQueryDevice(display, device_id, &num_devices);
    if (devices == NULL)
    {
        XIFreeDeviceInfo(devices);
        return -1;
    }
    else
    {
        *out_device_type = devices->use;
        XIFreeDeviceInfo(devices);
        return 0;
    }
}

static XEvent xi_convert_raw_mouse_event(Display *display, Window window, int event_type, XIRawEvent *raw_event)
{
    Window root_window, target_window;
    int mouse_root_x, mouse_root_y;
    bool same_screen;
    same_screen = XQueryPointer(display, window,
        &root_window,
        &target_window,
        &mouse_root_x,
        &mouse_root_y,
        &(int){0},
        &(int){0},
        &(unsigned int){0}
    );

    // Calculate the mouse position relative to the target window.
    // Even though the XQueryPointer function can provide this information,
    // it is not reliable enough, so this approach is preferred.
    int mouse_rel_x, mouse_rel_y = 0;
    if (target_window != 0)
    {
        XWindowAttributes target_window_attr;
        XGetWindowAttributes(display, target_window, &target_window_attr);
        mouse_rel_x = mouse_root_x - target_window_attr.x;
        mouse_rel_y = mouse_root_y - target_window_attr.y;   
    }
    else
    {
        mouse_rel_x = mouse_root_x;
        mouse_rel_y = mouse_root_y;
    }

    XEvent new_event;
    if(event_type == ButtonPress || event_type == ButtonRelease)
    {
        new_event.xbutton.type = event_type;
        new_event.xbutton.button = raw_event->detail;
        new_event.xbutton.serial = raw_event->serial;
        new_event.xbutton.send_event = raw_event->send_event;
        new_event.xbutton.display = raw_event->display;
        new_event.xbutton.window = target_window;
        new_event.xbutton.root = root_window;
        new_event.xbutton.subwindow = 0;
        new_event.xbutton.time = raw_event->time;
        new_event.xbutton.x = mouse_rel_x;
        new_event.xbutton.y = mouse_rel_y;
        new_event.xbutton.x_root = mouse_root_x;
        new_event.xbutton.y_root = mouse_root_y;
        new_event.xbutton.same_screen = same_screen;
    }
    if(event_type == MotionNotify)
    {
        new_event.xmotion.type = MotionNotify;
        new_event.xmotion.serial = raw_event->serial;
        new_event.xmotion.send_event = raw_event->send_event;
        new_event.xmotion.display = raw_event->display;
        new_event.xmotion.window = target_window;
        new_event.xmotion.root = root_window;
        new_event.xmotion.subwindow = 0;
        new_event.xmotion.time = raw_event->time;
        new_event.xmotion.x = mouse_rel_x;
        new_event.xmotion.y = mouse_rel_y;
        new_event.xmotion.x_root = mouse_root_x;
        new_event.xmotion.y_root = mouse_root_y;
        new_event.xmotion.same_screen = same_screen;
    }
    return new_event;
}

XEvent xi_convert_raw_button_press_event(Display *display, Window window, XIRawEvent *raw_event)
{
    return xi_convert_raw_mouse_event(display, window, ButtonPress, raw_event);
}

XEvent xi_convert_raw_button_release_event(Display *display, Window window, XIRawEvent *raw_event)
{
    return xi_convert_raw_mouse_event(display, window, ButtonRelease, raw_event);
}

XEvent xi_convert_raw_motion_event(Display *display, Window window, XIRawEvent *raw_event)
{
    return xi_convert_raw_mouse_event(display, window, MotionNotify, raw_event);
}

static XEvent xi_convert_raw_key_event(Display *display, Window window, int event_type, XIRawEvent *raw_event)
{
    Window root_window = DefaultRootWindow(display);

    XEvent new_event;
    new_event.xkey.type = event_type;
    new_event.xkey.keycode = raw_event->detail;
    new_event.xkey.serial = raw_event->serial;
    new_event.xkey.send_event = raw_event->send_event;
    new_event.xkey.display = raw_event->display;
    new_event.xkey.window = window;
    new_event.xkey.root = root_window;
    new_event.xkey.subwindow = 0;
    new_event.xkey.time = raw_event->time;
    new_event.xkey.x = 0;
    new_event.xkey.y = 0;
    new_event.xkey.x_root = 0;
    new_event.xkey.y_root = 0;
    new_event.xkey.same_screen = True;

    return new_event;
}

XEvent xi_convert_raw_key_press_event(Display *display, Window window, XIRawEvent *raw_event)
{
    return xi_convert_raw_key_event(display, window, KeyPress, raw_event);
}

XEvent xi_convert_raw_key_release_event(Display *display, Window window, XIRawEvent *raw_event)
{
    return xi_convert_raw_key_event(display, window, KeyRelease, raw_event);
}
