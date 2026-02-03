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

/** The maximum number of markers that can be managed simultaneously. */
#define MAX_MARKERS 32

typedef struct {
    bool active;           // Whether this slot is in use.
    unsigned int id;
    unsigned int sequence; // Insertion order for finding the top marker.
    Cursor cursor;
    bool grab;             // Whether to prevent pointer events passing through.
} Marker;

typedef struct {
    Marker items[MAX_MARKERS];
    unsigned int active_count;
    unsigned int next_sequence; // Counter for assigning sequence to new markers.
} MarkerDeck;

static MarkerDeck marker_deck = {
    .items = {{.active = false}},
    .active_count = 0,
    .next_sequence = 0
};

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

static Marker* find_marker(unsigned int id)
{
    for (int i = 0; i < MAX_MARKERS; i++)
    {
        if (!marker_deck.items[i].active) continue;
        if (marker_deck.items[i].id == id)
        {
            return &marker_deck.items[i];
        }
    }
    return NULL;
}

static Marker* find_top_marker()
{
    Marker *top = NULL;
    for (int i = 0; i < MAX_MARKERS; i++)
    {
        if (!marker_deck.items[i].active) continue;
        if (top == NULL || marker_deck.items[i].sequence > top->sequence)
        {
            top = &marker_deck.items[i];
        }
    }
    return top;
}

void add_marker(unsigned int id, unsigned int shape, bool grab)
{
    Display *display = DefaultDisplay;

    // Ensure the marker isn't already in the deck.
    if (find_marker(id) != NULL) return;

    // Find the first inactive slot.
    int slot = -1;
    for (int i = 0; i < MAX_MARKERS; i++)
    {
        if (!marker_deck.items[i].active)
        {
            slot = i;
            break;
        }
    }
    if (slot == -1)
    {
        LOG_ERROR("Could not add marker, maximum marker count reached.");
        return;
    }

    // Add the marker to the deck.
    marker_deck.items[slot] = (Marker){
        .active = true,
        .id = id,
        .sequence = marker_deck.next_sequence++,
        .cursor = XCreateFontCursor(display, shape),
        .grab = grab
    };
    marker_deck.active_count++;

    // Show the marker.
    show_marker(&marker_deck.items[slot]);
}

void remove_marker(unsigned int id)
{
    Display *display = DefaultDisplay;

    // Ensure the marker is in the deck.
    Marker *marker = find_marker(id);
    if (marker == NULL) return;

    // Free the cursor.
    XFreeCursor(display, marker->cursor);

    // Mark the slot as inactive (tombstone).
    marker->active = false;
    marker_deck.active_count--;

    // Show the new top marker, if any remain.
    Marker *top = find_top_marker();
    if (top != NULL)
    {
        show_marker(top);
    }
}

HANDLE(Initialize)
{
    // Add the default marker.
    add_marker(common.string_to_id("default"), XC_left_ptr, false);
}
