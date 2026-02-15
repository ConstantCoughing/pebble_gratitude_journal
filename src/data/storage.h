#pragma once

#include <pebble.h>
#include "entry.h"

// Initialize storage (call on app init)
void storage_init(void);

// Save a new entry (handles circular buffer)
bool storage_save_entry(const Entry *entry);

// Get all entries for a specific date
uint16_t storage_get_entries_for_date(time_t date, Entry *entries, uint16_t max_entries);

// Get all entries sorted by date (newest first)
uint16_t storage_get_all_entries(Entry *entries, uint16_t max_entries);

// Get total entry count
uint16_t storage_get_entry_count(void);

// Load stats from storage
void storage_load_stats(Stats *stats);

// Save stats to storage
void storage_save_stats(const Stats *stats);

// Delete oldest entry (for manual cleanup)
bool storage_delete_oldest_entry(void);

// Check if storage is near capacity
bool storage_is_near_capacity(void);

// Get storage utilization percentage (0-100)
uint8_t storage_get_utilization(void);

// Update an existing entry by index
bool storage_update_entry(uint16_t index, const Entry *entry);

// Delete an entry by index
bool storage_delete_entry(uint16_t index);

// Get entry by index
bool storage_get_entry_by_index(uint16_t index, Entry *entry);

// Callback-based entry iterator (reads one entry at a time, no bulk malloc)
// Return true from callback to continue, false to stop early
typedef bool (*StorageEntryCallback)(const Entry *entry, uint16_t index, void *context);
void storage_iterate_entries(StorageEntryCallback callback, void *context);
