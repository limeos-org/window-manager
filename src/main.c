#include "all.h"

static const char *libraries[] = {
    "libX11.so.6",
    "libXi.so.6",
    "libcairo.so.2"
};

int main()
{
    // Ensure that the required libraries are available.
    const int library_count = sizeof(libraries) / sizeof(libraries[0]);
    for(int i = 0; i < library_count; i++)
    {
        if(!is_library_available(libraries[i]))
        {
            LOG_ERROR("Missing library \"%s\".", libraries[i]);
            exit(EXIT_FAILURE);
        }
    }

    // Open the X11 display.
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        LOG_ERROR("Failed to open X11 display.");
        exit(EXIT_FAILURE);
    }

    // Assign a custom error handler to Xlib.
    XSetErrorHandler(x_error_handler);

    // Initialize the event loop.
    initialize_event_loop(display, DefaultRootWindow(display));
    
    return 0;
}
