#include "../all.h"

static Display *default_display = NULL;

void x_set_default_display(Display *display)
{
    default_display = display;
}

Display *x_get_default_display()
{
    return default_display;
}

Time x_get_current_time()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec * 1000) + (now.tv_usec / 1000);
}

pid_t x_get_window_pid(Display *display, Window window)
{
    // Retrieve the `_NET_WM_PID` property from the window.
    unsigned char *data;
    unsigned long item_count;
    Atom _NET_WM_PID = XInternAtom(display, "_NET_WM_PID", False);
    int status = XGetWindowProperty(
        display,                // Display
        window,                 // Window
        _NET_WM_PID,            // Property
        0, 1,                   // Offset, length
        False,                  // Delete
        XA_CARDINAL,            // Type
        &(Atom){0},             // Response type (unused)
        &(int){0},              // Response format (unused)
        &item_count,            // Item count
        &(unsigned long){0},    // Bytes after (unused)
        &data                   // Data
    );
    if (status != Success || data == NULL || item_count != 1) return -1;

    // Store the PID, so we can free the property data.
    pid_t pid = *(uint32_t*)data;

    // Free the property data.
    XFree(data);

    return pid;
}

Window x_get_window_parent(Display *display, Window window)
{
    Window parent, *children;
    int status = XQueryTree(display, window, &(Window){0}, &parent, &children, &(unsigned int){0});
    if (status == 0) return None;
    XFree(children);
    return parent;
}

int x_get_window_name(Display *display, Window window, char *out_name, size_t name_size)
{
    // List of properties to check for the window name.
    const int property_count = 2;
    Atom properties[property_count];
    properties[0] = XInternAtom(display, "_NET_WM_NAME", False);
    properties[1] = XInternAtom(display, "WM_NAME", False);

    // Loop over the properties, and stores the first one that is available.
    unsigned char *name = NULL;
    for(int i = 0; i < property_count; i++)
    {
        int status = XGetWindowProperty(
            display,                // Display
            window,                 // Window
            properties[i],          // Property
            0, (~0L),               // Offset, length
            False,                  // Delete
            AnyPropertyType,        // Type
            &(Atom){0},             // Actual type (unused)
            &(int){0},              // Actual format (unused)
            &(unsigned long){0},    // N items (unused)
            &(unsigned long){0},    // Bytes after (unused)
            &name                   // Data
        );
        if (status == Success && name != NULL)
        {
            strncpy(out_name, (const char *)name, name_size);
            out_name[name_size - 1] = '\0';  // Ensure null termination.

            XFree(name);
            break;
        }
    }

    return (name != NULL) ? 0 : -1;
}

bool x_window_supports_protocol(Display *display, Window window, Atom protocol)
{
    Atom *protocols;
    int count;
    if (XGetWMProtocols(display, window, &protocols, &count))
    {
        for (int i = 0; i < count; i++)
        {
            if (protocols[i] == protocol)
            {
                XFree(protocols);
                return True;
            }
        }
        XFree(protocols);
    }
    return False;
}

bool x_window_exists(Display *display, Window window)
{
    return XGetWindowAttributes(display, window, &(XWindowAttributes){0}) != 0;
}

int x_key_name_to_symbol(const char *name, int *out_key)
{
    // Create a copy of the provided key name string.
    char *name_copy = strdup(name);

    // Make the key name string lowercase.
    for (int i = 0; i < (int)strlen(name_copy); i++)
    {
        name_copy[i] = tolower(name_copy[i]);
    }

    // Convert the key name to a key symbol.
    if (strcmp(name_copy, "super") == 0)
    {
        *out_key = XK_Super_L;
    }
    else if (strcmp(name_copy, "control") == 0)
    {
        *out_key = XK_Control_L;
    }
    else if (strcmp(name_copy, "ctrl") == 0)
    {
        *out_key = XK_Control_L;
    }
    else if (strcmp(name_copy, "shift") == 0)
    {
        *out_key = XK_Shift_L;
    }
    else if (strcmp(name_copy, "alt") == 0)
    {
        *out_key = XK_Alt_L;
    }
    else if (strcmp(name_copy, "enter") == 0)
    {
        *out_key = XK_Return;
    }
    else if (strcmp(name_copy, "esc") == 0)
    {
        *out_key = XK_Escape;
    }
    else
    {
        // The `XStringToKeysym()` function expects the first character to be
        // uppercase, with the exception of single letters (e.g. 'a', 'b', 'c').
        if (strlen(name_copy) > 1)
        {
            name_copy[0] = toupper(name_copy[0]);
        }
        *out_key = XStringToKeysym(name_copy);
    }

    // Free the duplicate string.
    free(name_copy);

    return (out_key != NoSymbol) ? 0 : -1;
}

int x_key_names_to_symbols(char *names, const char delimiter, int *out_keys, int keys_size)
{
    int status = 0;

    // Create a copy of the provided key names string.
    char *names_copy = strdup(names);

    // Make the key names string lowercase.
    for (int i = 0; i < (int)strlen(names_copy); i++)
    {
        names_copy[i] = tolower(names_copy[i]);
    }
    
    // Split the key names string by the delimiter and iterate over each token.
    char *token = strtok(names_copy, &delimiter);
    for (int i = 0; i < keys_size; i++)
    {
        // If we ran out of tokens, but the loop isn't completed yet, fill the 
        // remaining key symbol slots with NoSymbol.
        if (token == NULL)
        {
            out_keys[i] = NoSymbol;
            continue;
        }

        // Convert the token (key name) to a key symbol.
        int key = 0;
        x_key_name_to_symbol(token, &key);
        if (key == NoSymbol) status = -1;
        out_keys[i] = key;

        // Prepare for the next iteration.
        token = strtok(NULL, &delimiter);
    }

    // Free the duplicate string.
    free(names_copy);

    return status;
}

static int _x_query_tree_recursively(
    Display *display,
    Window parent,
    Window **out_children,
    unsigned int *out_children_count,
    int *out_current_position
)
{
    // Query the children of the parent.
    Window *children;
    unsigned int children_count;
    if (XQueryTree(display, parent, &(Window){0}, &(Window){0}, &children, &children_count) == 0)
    {
        return -1;
    }

    // Ensure we don't handle parents without children.
    if (children_count == 0) return 0;

    // Recalculate the output children count and reallocate memory for them.
    *out_children_count += children_count;
    *out_children = realloc(*out_children, (*out_children_count) * sizeof(Window));

    // Iterate over each child from the tree query.
    for (unsigned int i = 0; i < children_count; i++) {
        // Insert this child into the children output.
        (*out_children)[*out_current_position] = children[i];
        (*out_current_position)++;

        // Insert its children into the children output.
        int status = _x_query_tree_recursively(display, children[i], out_children, out_children_count, out_current_position);
        if (status != 0)
        {
            XFree(children);
            return status;
        }
    }

    XFree(children);
    return 0;
}

int x_query_tree_recursively(Display *display, Window parent, Window **out_children, unsigned int *out_children_count)
{
    // Initialize the required variables.
    *out_children = NULL;
    *out_children_count = 0;
    int current_position = 0;

    // Recursively query the tree.
    return _x_query_tree_recursively(display, parent, out_children, out_children_count, &current_position);
}

Window x_create_simple_window(
    Display *display,
    Window parent,
    int x, int y,
    unsigned int width, unsigned int height,
    unsigned int border_width, unsigned long border_pixel,
    unsigned long background
)
{
    // Grab the server so events are not processed while creating the window.
    XGrabServer(display);

    // Create the window.
    Window window = XCreateSimpleWindow(
        display,        // Display
        parent,         // Parent
        x, y,           // X, Y
        width, height,  // Width, Height
        border_width,   // Border width
        border_pixel,   // Border pixel
        background      // Background
    );

    // Assign the `_NET_WM_PID` property to the window.
    pid_t pid = getpid();
    Atom _NET_WM_PID = XInternAtom(display, "_NET_WM_PID", False);
    XChangeProperty(
        display,                // Display
        window,                 // Window
        _NET_WM_PID,            // Property
        XA_CARDINAL,            // Type
        32,                     // Format (32-bit)
        PropModeReplace,        // Mode
        (unsigned char *)&pid,  // Data
        1                       // Data item count
    );

    // Ungrab the server so events can be processed again.
    XUngrabServer(display);

    return window;
}
