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
  if (!entry || !entry_validate(entry)) {
    return false;
  }

  uint16_t count = storage_get_entry_count();

  // If at max capacity, delete oldest entry
  if (count >= MAX_ENTRIES) {
    storage_delete_oldest_entry();
    count = storage_get_entry_count();
  }

  // Save entry at next available slot
  uint32_t key = get_entry_key(count);
  int result = persist_write_data(key, entry, sizeof(Entry));

  if (result == sizeof(Entry)) {
    // Update count
    persist_write_int(STORAGE_KEY_ENTRY_COUNT, count + 1);
    return true;
  }

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
  for (uint16_t i = 0; i < loaded - 1; i++) {
    for (uint16_t j = 0; j < loaded - i - 1; j++) {
      if (entries[j].date < entries[j + 1].date) {
        Entry temp = entries[j];
        entries[j] = entries[j + 1];
        entries[j + 1] = temp;
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
  if (!stats) return;

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
  if (entries) {
    uint16_t count = storage_get_all_entries(entries, MAX_ENTRIES);
    for (uint16_t i = 0; i < count; i++) {
      if (entries[i].mood >= MOOD_SAD && entries[i].mood <= MOOD_GRATEFUL) {
        stats->mood_counts[entries[i].mood]++;
      }
    }
    free(entries);
  }
}

void storage_save_stats(const Stats *stats) {
  if (!stats) return;

  persist_write_int(STORAGE_KEY_CURRENT_STREAK, stats->current_streak);
  persist_write_int(STORAGE_KEY_LAST_ENTRY_DATE, (int)stats->last_entry_date);
}

bool storage_delete_oldest_entry(void) {
  uint16_t count = storage_get_entry_count();
  if (count == 0) {
    return false;
  }

  // Load all entries to find oldest
  Entry *entries = malloc(sizeof(Entry) * MAX_ENTRIES);
  if (!entries) {
    return false;
  }

  uint16_t loaded = storage_get_all_entries(entries, MAX_ENTRIES);
  if (loaded == 0) {
    free(entries);
    return false;
  }

  // Find oldest (entries are sorted newest first, so oldest is at end)
  time_t oldest_date = entries[loaded - 1].date;

  // Shift all entries down by one slot
  for (uint16_t i = 0; i < loaded - 1; i++) {
    uint32_t key = get_entry_key(i);
    persist_write_data(key, &entries[i], sizeof(Entry));
  }

  // Delete last slot
  uint32_t last_key = get_entry_key(loaded - 1);
  persist_delete(last_key);

  // Update count
  persist_write_int(STORAGE_KEY_ENTRY_COUNT, loaded - 1);

  free(entries);
  return true;
}

bool storage_is_near_capacity(void) {
  return storage_get_entry_count() >= STORAGE_WARNING_THRESHOLD;
}

uint8_t storage_get_utilization(void) {
  uint16_t count = storage_get_entry_count();
  return (uint8_t)((count * 100) / MAX_ENTRIES);
}
