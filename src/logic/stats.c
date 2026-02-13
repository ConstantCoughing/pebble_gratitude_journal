#include "stats.h"
#include "../data/storage.h"
#include "../utils/date_utils.h"
#include <string.h>

uint16_t stats_calculate_streak(void) {
  Entry *entries = malloc(sizeof(Entry) * MAX_ENTRIES);
  if (!entries) {
    return 0;
  }

  uint16_t count = storage_get_all_entries(entries, MAX_ENTRIES);
  if (count == 0) {
    free(entries);
    return 0;
  }

  // Entries are sorted newest first
  time_t today = date_get_today();
  time_t current_date = today;
  uint16_t streak = 0;

  // Check if there's an entry for today or yesterday
  bool found_recent = false;
  for (uint16_t i = 0; i < count; i++) {
    if (date_is_same_day(entries[i].date, today) ||
        date_is_same_day(entries[i].date, date_add_days(today, -1))) {
      found_recent = true;
      break;
    }
  }

  if (!found_recent) {
    free(entries);
    return 0;  // Streak is broken
  }

  // Count consecutive days backwards from today
  for (int32_t day_offset = 0; day_offset <= count; day_offset++) {
    time_t check_date = date_add_days(today, -day_offset);
    bool found = false;

    for (uint16_t i = 0; i < count; i++) {
      if (date_is_same_day(entries[i].date, check_date)) {
        found = true;
        break;
      }
    }

    if (found) {
      streak++;
    } else {
      // Streak broken
      break;
    }
  }

  free(entries);
  return streak;
}

void stats_update_after_entry(const Entry *entry) {
  if (!entry) return;

  Stats stats;
  storage_load_stats(&stats);

  // Update streak
  stats.current_streak = stats_calculate_streak();
  stats.last_entry_date = entry->date;

  // Update mood count
  if (entry->mood >= MOOD_SAD && entry->mood <= MOOD_GRATEFUL) {
    stats.mood_counts[entry->mood]++;
  }

  // Total entries will be updated by storage layer
  storage_save_stats(&stats);
}

void stats_get_mood_distribution(uint16_t mood_counts[9]) {
  if (!mood_counts) return;

  Stats stats;
  storage_load_stats(&stats);

  memcpy(mood_counts, stats.mood_counts, sizeof(stats.mood_counts));
}

uint16_t stats_get_total_entries(void) {
  return storage_get_entry_count();
}

Mood stats_get_most_common_mood(void) {
  Stats stats;
  storage_load_stats(&stats);

  Mood most_common = MOOD_NEUTRAL;
  uint16_t max_count = 0;

  for (uint8_t i = MOOD_SAD; i <= MOOD_GRATEFUL; i++) {
    if (stats.mood_counts[i] > max_count) {
      max_count = stats.mood_counts[i];
      most_common = (Mood)i;
    }
  }

  return most_common;
}
