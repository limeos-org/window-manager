#include "all.h"

static const char *libraries[] = {
    "libX11.so.6",
    "libXi.so.6",
    "libXfixes.so.3",
    "libXrandr.so.2",
    "libXcomposite.so.1",
    "libcairo.so.2",
};

static int custom_x_error_handler(Display *display, XErrorEvent *error)
{
    // Ignore BadWindow errors.
    if (error->error_code == BadWindow) return 0;

    // Retrieve the error text.
    char error_text[1024];
    XGetErrorText(display, error->error_code, error_text, sizeof(error_text));
    
    // Log the error.
    LOG_ERROR(
        "%s (request: %d, resource: 0x%lx)", 
        error_text, error->request_code, error->resourceid
    );

    return 0;
}

int main()
{
    // Ensure the program isn't being run as root.
    if(geteuid() == 0)
    {
        LOG_ERROR("Running as root is not secure.");
        exit(EXIT_FAILURE);
    }

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

    // Set the default X11 display.
    x_set_default_display(display);

    // Set input focus on the root window.
    XSetInputFocus(display, DefaultRootWindow(display), RevertToParent, CurrentTime);

    // Set a custom X11 error handler.
    XSetErrorHandler(custom_x_error_handler);

    // TEMP - Remove me after marker logic reintroduced.
    XDefineCursor(display, DefaultRootWindow(display), XCreateFontCursor(display, XC_left_ptr));

    // Initialize the event loop.
    initialize_event_loop();
    
    return 0;
}
