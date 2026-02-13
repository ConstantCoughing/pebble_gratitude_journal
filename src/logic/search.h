#pragma once

#include <pebble.h>
#include "../data/entry.h"

typedef struct {
  // Text search
  char query[MAX_ENTRY_TEXT_LENGTH + 1];

  // Mood filter
  bool mood_filter_enabled;
  uint16_t mood_flags;  // Bitmask of selected moods (1 << mood)

  // Date range filter
  bool date_filter_enabled;
  time_t start_date;
  time_t end_date;

  // Canned response filter
  bool canned_filter_enabled;
  uint16_t canned_flags;  // Must match at least one flag
} SearchCriteria;

// Search entries based on criteria
uint16_t search_entries(const SearchCriteria *criteria, Entry *results, uint16_t max_results);

// Helper: Check if entry matches criteria
bool entry_matches_criteria(const Entry *entry, const SearchCriteria *criteria);

// Helper: Case-insensitive substring search
bool str_contains_case_insensitive(const char *haystack, const char *needle);
