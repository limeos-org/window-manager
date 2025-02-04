#pragma once
#include "../all.h"

/**
 * @brief Converts raw XInput2 event data to a standard event structure.
 * 
 * @param raw_event The raw XInput2 event.
 * 
 * @return `Event` - The converted standard event.
 */
Event convert_raw_xinput_event(XIRawEvent *raw_event);
