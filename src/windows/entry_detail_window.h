#pragma once

#include <pebble.h>
#include "../data/entry.h"

// Push entry detail window for a specific date
void entry_detail_window_push(time_t date);

// Destroy entry detail window
void entry_detail_window_destroy(void);
