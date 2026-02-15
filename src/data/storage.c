#include "storage.h"
#include "../utils/date_utils.h"
#include <string.h>

#define STORAGE_VERSION 1

// Internal helper to get storage key for entry index
static uint32_t get_entry_key(uint16_t index) {
  return STORAGE_KEY_ENTRIES_START + (index % MAX_ENTRIES);
}

void storage_init(void) {
  // Check if storage is initialized
  if (!persist_exists(STORAGE_KEY_VERSION)) {
    // First run - initialize storage
    persist_write_int(STORAGE_KEY_VERSION, STORAGE_VERSION);
    persist_write_int(STORAGE_KEY_ENTRY_COUNT, 0);
    persist_write_int(STORAGE_KEY_CURRENT_STREAK, 0);
    persist_write_int(STORAGE_KEY_LAST_ENTRY_DATE, 0);
    persist_write_int(STORAGE_KEY_PROMPT_INDEX, 0);
    persist_write_int(STORAGE_KEY_PROMPT_MODE, 0);  // 0 = random
  } else {
    // Check version for migration
    int version = persist_read_int(STORAGE_KEY_VERSION);
    if (version < STORAGE_VERSION) {
      // Perform migration if needed in future versions
    }
  }
}

bool storage_save_entry(const Entry *entry) {
  if (!entry) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_save_entry: entry is NULL");
    return false;
  }

  if (!entry_validate(entry)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_save_entry: entry validation failed");
    return false;
  }

  uint16_t count = storage_get_entry_count();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "storage_save_entry: current count = %d", count);

  // If at max capacity, delete oldest entry
  if (count >= MAX_ENTRIES) {
    APP_LOG(APP_LOG_LEVEL_INFO, "storage_save_entry: at capacity, deleting oldest");
    if (!storage_delete_oldest_entry()) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "storage_save_entry: failed to delete oldest entry");
      return false;
    }
    count = storage_get_entry_count();
  }

  // Save entry at next available slot
  uint32_t key = get_entry_key(count);
  int result = persist_write_data(key, entry, sizeof(Entry));

  if (result == sizeof(Entry)) {
    // Update count
    int count_result = persist_write_int(STORAGE_KEY_ENTRY_COUNT, count + 1);
    if (count_result != sizeof(int)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "storage_save_entry: failed to update entry count");
      // Entry saved but count not updated - data inconsistency possible
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "storage_save_entry: successfully saved entry %d", count);
    return true;
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "storage_save_entry: persist_write_data failed, result = %d", result);
  return false;
}

uint16_t storage_get_entries_for_date(time_t date, Entry *entries, uint16_t max_entries) {
  if (!entries || max_entries == 0) {
    return 0;
  }

  time_t normalized_date = date_normalize_to_midnight(date);
  uint16_t count = storage_get_entry_count();
  uint16_t found = 0;

  for (uint16_t i = 0; i < count && found < max_entries; i++) {
    Entry entry;
    uint32_t key = get_entry_key(i);

    if (persist_exists(key)) {
      int result = persist_read_data(key, &entry, sizeof(Entry));
      if (result == sizeof(Entry) && entry.date == normalized_date) {
        memcpy(&entries[found], &entry, sizeof(Entry));
        found++;
      }
    }
  }

  return found;
}

uint16_t storage_get_all_entries(Entry *entries, uint16_t max_entries) {
  if (!entries || max_entries == 0) {
    return 0;
  }

  uint16_t count = storage_get_entry_count();
  uint16_t loaded = 0;

  // Load all entries
  for (uint16_t i = 0; i < count && loaded < max_entries; i++) {
    uint32_t key = get_entry_key(i);

    if (persist_exists(key)) {
      int result = persist_read_data(key, &entries[loaded], sizeof(Entry));
      if (result == sizeof(Entry)) {
        loaded++;
      }
    }
  }

  // Sort by date (newest first) using bubble sort
  // Simple sort is fine for 180 entries max
  if (loaded > 1) {
    for (uint16_t i = 0; i < loaded - 1; i++) {
      for (uint16_t j = 0; j < loaded - i - 1; j++) {
        if (entries[j].date < entries[j + 1].date) {
          Entry temp = entries[j];
          entries[j] = entries[j + 1];
          entries[j + 1] = temp;
        }
      }
    }
  }

  return loaded;
}

uint16_t storage_get_entry_count(void) {
  if (!persist_exists(STORAGE_KEY_ENTRY_COUNT)) {
    return 0;
  }
  return (uint16_t)persist_read_int(STORAGE_KEY_ENTRY_COUNT);
}

void storage_load_stats(Stats *stats) {
  if (!stats) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_load_stats: stats is NULL");
    return;
  }

  memset(stats, 0, sizeof(Stats));

  if (persist_exists(STORAGE_KEY_CURRENT_STREAK)) {
    stats->current_streak = (uint16_t)persist_read_int(STORAGE_KEY_CURRENT_STREAK);
  }

  if (persist_exists(STORAGE_KEY_LAST_ENTRY_DATE)) {
    stats->last_entry_date = (time_t)persist_read_int(STORAGE_KEY_LAST_ENTRY_DATE);
  }

  stats->total_entries = storage_get_entry_count();

  // Calculate mood counts from all entries
  Entry *entries = malloc(sizeof(Entry) * MAX_ENTRIES);
  if (!entries) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_load_stats: malloc failed for entries");
    return;
  }

  uint16_t count = storage_get_all_entries(entries, MAX_ENTRIES);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "storage_load_stats: loaded %d entries for stats calculation", count);

  for (uint16_t i = 0; i < count; i++) {
    if (entries[i].mood <= MOOD_GRATEFUL) {
      stats->mood_counts[entries[i].mood]++;
    } else {
      APP_LOG(APP_LOG_LEVEL_WARNING, "storage_load_stats: invalid mood value %d for entry %d", entries[i].mood, i);
    }
  }

  free(entries);
  APP_LOG(APP_LOG_LEVEL_INFO, "storage_load_stats: streak=%d, total=%d", stats->current_streak, stats->total_entries);
}

void storage_save_stats(const Stats *stats) {
  if (!stats) return;

  persist_write_int(STORAGE_KEY_CURRENT_STREAK, stats->current_streak);
  persist_write_int(STORAGE_KEY_LAST_ENTRY_DATE, (int)stats->last_entry_date);
}

bool storage_delete_oldest_entry(void) {
  uint16_t count = storage_get_entry_count();
  if (count == 0) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "storage_delete_oldest_entry: no entries to delete");
    return false;
  }

  // Load all entries to find oldest
  Entry *entries = malloc(sizeof(Entry) * MAX_ENTRIES);
  if (!entries) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_delete_oldest_entry: malloc failed");
    return false;
  }

  uint16_t loaded = storage_get_all_entries(entries, MAX_ENTRIES);
  if (loaded == 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_delete_oldest_entry: no entries loaded");
    free(entries);
    return false;
  }

  // Find oldest (entries are sorted newest first, so oldest is at end)
  time_t oldest_date = entries[loaded - 1].date;
  APP_LOG(APP_LOG_LEVEL_INFO, "storage_delete_oldest_entry: deleting entry from %ld", (long)oldest_date);

  // Shift all entries down by one slot
  for (uint16_t i = 0; i < loaded - 1; i++) {
    uint32_t key = get_entry_key(i);
    int result = persist_write_data(key, &entries[i], sizeof(Entry));
    if (result != sizeof(Entry)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "storage_delete_oldest_entry: failed to write entry %d during shift", i);
      // Continue anyway to try to maintain data integrity
    }
  }

  // Delete last slot
  uint32_t last_key = get_entry_key(loaded - 1);
  persist_delete(last_key);

  // Update count
  int count_result = persist_write_int(STORAGE_KEY_ENTRY_COUNT, loaded - 1);
  if (count_result != sizeof(int)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_delete_oldest_entry: failed to update count");
  }

  free(entries);
  APP_LOG(APP_LOG_LEVEL_INFO, "storage_delete_oldest_entry: successfully deleted oldest entry");
  return true;
}

bool storage_is_near_capacity(void) {
  return storage_get_entry_count() >= STORAGE_WARNING_THRESHOLD;
}

uint8_t storage_get_utilization(void) {
  uint16_t count = storage_get_entry_count();
  return (uint8_t)((count * 100) / MAX_ENTRIES);
}

bool storage_update_entry(uint16_t index, const Entry *entry) {
  if (!entry || !entry_validate(entry)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_update_entry: invalid entry");
    return false;
  }

  uint16_t count = storage_get_entry_count();
  if (index >= count) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_update_entry: index %d out of range (count=%d)", index, count);
    return false;
  }

  uint32_t key = get_entry_key(index);
  int result = persist_write_data(key, entry, sizeof(Entry));

  if (result == sizeof(Entry)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "storage_update_entry: successfully updated entry %d", index);
    return true;
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "storage_update_entry: persist_write_data failed, result=%d", result);
  return false;
}

bool storage_delete_entry(uint16_t index) {
  uint16_t count = storage_get_entry_count();
  if (index >= count) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_delete_entry: index %d out of range (count=%d)", index, count);
    return false;
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "storage_delete_entry: deleting entry %d of %d", index, count);

  // Shift entries after deleted index down by one using raw persist keys
  for (uint16_t i = index; i < count - 1; i++) {
    Entry entry;
    uint32_t next_key = get_entry_key(i + 1);
    uint32_t cur_key = get_entry_key(i);

    if (persist_exists(next_key)) {
      int read_result = persist_read_data(next_key, &entry, sizeof(Entry));
      if (read_result == sizeof(Entry)) {
        int write_result = persist_write_data(cur_key, &entry, sizeof(Entry));
        if (write_result != sizeof(Entry)) {
          APP_LOG(APP_LOG_LEVEL_ERROR, "storage_delete_entry: failed to write entry %d during shift", i);
        }
      } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "storage_delete_entry: failed to read entry %d during shift", i + 1);
      }
    }
  }

  // Delete last entry
  uint32_t last_key = get_entry_key(count - 1);
  persist_delete(last_key);

  // Update count
  persist_write_int(STORAGE_KEY_ENTRY_COUNT, count - 1);

  APP_LOG(APP_LOG_LEVEL_INFO, "storage_delete_entry: successfully deleted entry");
  return true;
}

bool storage_get_entry_by_index(uint16_t index, Entry *entry) {
  if (!entry) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_get_entry_by_index: entry is NULL");
    return false;
  }

  uint16_t count = storage_get_entry_count();
  if (index >= count) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_get_entry_by_index: index %d out of range (count=%d)", index, count);
    return false;
  }

  uint32_t key = get_entry_key(index);
  if (!persist_exists(key)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "storage_get_entry_by_index: key %lu does not exist", (unsigned long)key);
    return false;
  }

  int result = persist_read_data(key, entry, sizeof(Entry));
  if (result == sizeof(Entry)) {
    return true;
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "storage_get_entry_by_index: persist_read_data failed, result=%d", result);
  return false;
}
