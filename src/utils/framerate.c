#include "../all.h"

int framerate_to_throttle_ms(int framerate)
{
    if (framerate <= 0) return 1;
    return 1000 / framerate;
}
