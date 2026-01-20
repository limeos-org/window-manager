/**
 * This code is responsible for showing and hiding markers (i.e. cursors).
 * Markers are stored in a deck, where the marker on top of the deck (i.e. most
 * recently added) is the one currently being shown. Markers may be removed
 * from the deck, no matter their position.
 *
 * The reason the name "marker" was chosen instead of "cursor" is to create a
 * clear distinction between X11 cursor logic and our own cursor logic.
 */

#include "../all.h"

typedef struct {
    unsigned int id;
    Cursor cursor;
    bool grab;
} Marker;

typedef struct {
    Marker *items;
    int size;
    int capacity;
} MarkerDeck;

static MarkerDeck marker_deck;

static void show_marker(Marker *marker)
{
    Display *display = DefaultDisplay;
    Window root_window = DefaultRootWindow(display);

    // Ungrab the pointer, in case it was previously grabbed.
    XUngrabPointer(display, CurrentTime);

    // Show the specified marker.
    Cursor cursor = marker->cursor;
    if (marker->grab == true)
    {
        XGrabPointer(
            display,        // Display
            root_window,    // Window
            True,           // OwnerEvents
            NoEventMask,    // EventMask
            GrabModeAsync,  // PointerMode
            GrabModeAsync,  // KeyboardMode
            None,           // ConfineTo
            cursor,         // Cursor
            CurrentTime     // Time
        );
    }
    else
    {
        XDefineCursor(display, root_window, cursor);
    }
}

static Marker* find_marker(unsigned int id, int *out_index)
{
    for (int i = 0; i < marker_deck.size; i++)
    {
        if (marker_deck.items[i].id == id)
        {
            if (out_index != NULL) *out_index = i;
            return &marker_deck.items[i];
        }
    }
    if (out_index != NULL) *out_index = -1;
    return NULL;
}

void add_marker(unsigned int id, unsigned int shape, bool grab)
{
    Display *display = DefaultDisplay;

    // Ensure the marker isn't already in the deck.
    if (find_marker(id, NULL) != NULL) return;

    // Increase the capacity of the marker deck if necessary.
    if (marker_deck.size >= marker_deck.capacity)
    {
        int new_capacity = marker_deck.capacity * 2;
        Marker *new_items = realloc(marker_deck.items, new_capacity * sizeof(Marker));
        if (new_items == NULL)
        {
            LOG_ERROR("Could not add marker, memory allocation failed.");
            return;
        }
        marker_deck.capacity = new_capacity;
        marker_deck.items = new_items;
    }

    // Add the marker to the deck.
    marker_deck.items[marker_deck.size].id = id;
    marker_deck.items[marker_deck.size].cursor = XCreateFontCursor(display, shape);
    marker_deck.items[marker_deck.size].grab = grab;
    marker_deck.size++;

    // Show the marker.
    show_marker(&marker_deck.items[marker_deck.size - 1]);
}

void remove_marker(unsigned int id)
{
    Display *display = DefaultDisplay;

    // Ensure the marker is in in the deck.
    int index = -1;
    if (find_marker(id, &index) == NULL) return;

    // Free the cursor of the target marker.
    XFreeCursor(display, marker_deck.items[index].cursor);

    // Shift markers left to overwrite the target marker.
    for (int j = index; j < marker_deck.size - 1; j++)
    {
        marker_deck.items[j] = marker_deck.items[j + 1];
    }
    marker_deck.size--;

    // Show the last marker in the deck.
    if (marker_deck.size > 0)
    {
        show_marker(&marker_deck.items[marker_deck.size - 1]);
    }
}

HANDLE(Initialize)
{
    // Initialize the marker deck.
    marker_deck.size = 0;
    marker_deck.capacity = 10;
    marker_deck.items = malloc(marker_deck.capacity * sizeof(Marker));
    if (marker_deck.items == NULL)
    {
        LOG_ERROR("Could not initialize marker deck, memory allocation failed.");
        return;
    }

    // Add the default marker.
    add_marker(string_to_id("default"), XC_left_ptr, false);
}
