#include "stats.h"
#include "../data/storage.h"
#include "../utils/date_utils.h"
#include <string.h>

// Context for collecting unique dates
#define MAX_STREAK_DATES 62
typedef struct {
  time_t dates[MAX_STREAK_DATES];
  uint16_t count;
} StreakDateCtx;

static bool collect_dates_callback(const Entry *entry, uint16_t index, void *context) {
  StreakDateCtx *ctx = (StreakDateCtx *)context;
  time_t normalized = date_normalize_to_midnight(entry->date);

  // Check if date already collected
  for (uint16_t i = 0; i < ctx->count; i++) {
    if (ctx->dates[i] == normalized) {
      return true;  // already have this date, continue
    }
  }

  if (ctx->count < MAX_STREAK_DATES) {
    ctx->dates[ctx->count++] = normalized;
  }
  return true;  // continue iterating
}

uint16_t stats_calculate_streak(void) {
  uint16_t count = storage_get_entry_count();
  if (count == 0) return 0;

  // Collect unique dates using iterator (no bulk malloc)
  StreakDateCtx ctx;
  ctx.count = 0;
  storage_iterate_entries(collect_dates_callback, &ctx);

  if (ctx.count == 0) return 0;

  // Sort dates newest first (insertion sort - good for small arrays)
  for (uint16_t i = 1; i < ctx.count; i++) {
    time_t key = ctx.dates[i];
    int32_t j = i - 1;
    while (j >= 0 && ctx.dates[j] < key) {
      ctx.dates[j + 1] = ctx.dates[j];
      j--;
    }
    ctx.dates[j + 1] = key;
  }

  time_t today = date_get_today();

  // Check if there's an entry for today or yesterday
  if (!date_is_same_day(ctx.dates[0], today) &&
      !date_is_same_day(ctx.dates[0], date_add_days(today, -1))) {
    return 0;  // Streak is broken
  }

  // Count consecutive days backwards from today
  uint16_t streak = 0;
  for (int32_t day_offset = 0; day_offset <= (int32_t)ctx.count; day_offset++) {
    time_t check_date = date_add_days(today, -day_offset);
    bool found = false;

    for (uint16_t i = 0; i < ctx.count; i++) {
      if (date_is_same_day(ctx.dates[i], check_date)) {
        found = true;
        break;
      }
    }

    if (found) {
      streak++;
    } else {
      break;
    }
  }

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
  if (entry->mood <= MOOD_GRATEFUL) {
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
