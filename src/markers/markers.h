#pragma once
#include "../all.h"

/**
 * Adds a marker to the marker deck.
 *
 * @param id The ID for the marker.
 * @param shape The shape of the marker (E.g. `XC_hand1`).
 * @param grab Whether to grab the pointer, prevents click-through.
 *
 * @note - Use `string_to_id()` to generate a unique ID from a string.
 * @note - Safe to call even if the marker is already in the deck, the function
 * will return early and do nothing.
 */
void add_marker(unsigned int id, unsigned int shape, bool grab);

/**
 * Removes a marker from the marker deck.
 *
 * @param id The ID of the marker.
 *
 * @note - Use `string_to_id()` to generate a unique ID from a string.
 * @note - Safe to call even if the marker is not in the deck, the function
 * will return early and do nothing.
 */
void remove_marker(unsigned int id);
