#include "../all.h"

static RawButtonPressEvent construct_raw_button_press_event(XIRawEvent *raw_event)
{
    RawButtonPressEvent event = {
        .type = RawButtonPress,
        .button = raw_event->detail,
    };
    return event;
}

static RawButtonReleaseEvent construct_raw_button_release_event(XIRawEvent *raw_event)
{
    RawButtonReleaseEvent event = {
        .type = RawButtonRelease,
        .button = raw_event->detail,
    };
    return event;
}

static RawMotionNotifyEvent construct_raw_motion_notify_event(XIRawEvent *raw_event)
{
    (void)raw_event;
    RawMotionNotifyEvent event = {
        .type = RawMotionNotify,
    };
    return event;
}

static RawKeyPressEvent construct_raw_key_press_event(XIRawEvent *raw_event)
{
    RawKeyPressEvent event = {
        .type = RawKeyPress,
        .key_code = raw_event->detail,
    };
    return event;
}

static RawKeyReleaseEvent construct_raw_key_release_event(XIRawEvent *raw_event)
{
    RawKeyReleaseEvent event = {
        .type = RawKeyRelease,
        .key_code = raw_event->detail,
    };
    return event;
}

Event convert_raw_xinput_event(XIRawEvent *raw_event)
{
    if (raw_event->evtype == XI_RawButtonPress)
    {
        return (Event)construct_raw_button_press_event(raw_event);
    }
    else if (raw_event->evtype == XI_RawButtonRelease)
    {
        return (Event)construct_raw_button_release_event(raw_event);
    }
    else if (raw_event->evtype == XI_RawMotion)
    {
        return (Event)construct_raw_motion_notify_event(raw_event);
    }
    else if (raw_event->evtype == XI_RawKeyPress)
    {
        return (Event)construct_raw_key_press_event(raw_event);
    }
    else if (raw_event->evtype == XI_RawKeyRelease)
    {
        return (Event)construct_raw_key_release_event(raw_event);
    }
    else
    {
        LOG_WARNING("Attempted to convert unsupported XInput2 event type (%d).", raw_event->type);
        return (Event){0};
    }
}
