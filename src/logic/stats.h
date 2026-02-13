#pragma once

#include <pebble.h>
#include "../data/entry.h"

// Calculate current streak from stored entries
uint16_t stats_calculate_streak(void);

// Update stats after adding a new entry
void stats_update_after_entry(const Entry *entry);

// Get mood distribution (fills array with counts for each mood)
void stats_get_mood_distribution(uint16_t mood_counts[9]);

// Get total entries count
uint16_t stats_get_total_entries(void);

// Get most common mood
Mood stats_get_most_common_mood(void);
