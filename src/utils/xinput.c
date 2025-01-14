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
