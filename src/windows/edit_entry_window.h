#pragma once

#include <pebble.h>
#include "../data/entry.h"

// Push edit entry window for a specific entry index
void edit_entry_window_push(uint16_t entry_index);

// Destroy edit entry window
void edit_entry_window_destroy(void);
