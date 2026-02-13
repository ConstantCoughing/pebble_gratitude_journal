#include "search.h"
#include "../data/storage.h"
#include "../utils/constants.h"
#include <string.h>
#include <ctype.h>

bool str_contains_case_insensitive(const char *haystack, const char *needle) {
  if (!haystack || !needle || needle[0] == '\0') {
    return true;  // Empty needle matches everything
  }

  size_t haystack_len = strlen(haystack);
  size_t needle_len = strlen(needle);

  if (needle_len > haystack_len) {
    return false;
  }

  // Simple case-insensitive substring search
  for (size_t i = 0; i <= haystack_len - needle_len; i++) {
    bool match = true;
    for (size_t j = 0; j < needle_len; j++) {
      if (tolower((unsigned char)haystack[i + j]) != tolower((unsigned char)needle[j])) {
        match = false;
        break;
      }
    }
    if (match) {
      return true;
    }
  }

  return false;
}

bool entry_matches_criteria(const Entry *entry, const SearchCriteria *criteria) {
  if (!entry || !criteria) {
    return false;
  }

  // Text search (case-insensitive)
  if (criteria->query[0] != '\0') {
    if (!str_contains_case_insensitive(entry->text, criteria->query)) {
      return false;
    }
  }

  // Mood filter
  if (criteria->mood_filter_enabled) {
    if (!(criteria->mood_flags & (1 << entry->mood))) {
      return false;
    }
  }

  // Date range filter
  if (criteria->date_filter_enabled) {
    if (entry->date < criteria->start_date || entry->date > criteria->end_date) {
      return false;
    }
  }

  // Canned response filter (must match at least one flag)
  if (criteria->canned_filter_enabled) {
    if (!(entry->canned_flags & criteria->canned_flags)) {
      return false;
    }
  }

  return true;
}

uint16_t search_entries(const SearchCriteria *criteria, Entry *results, uint16_t max_results) {
  if (!criteria || !results || max_results == 0) {
    return 0;
  }

  // Load all entries
  Entry all_entries[MAX_ENTRIES];
  uint16_t total_count = storage_get_all_entries(all_entries, MAX_ENTRIES);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "search_entries: Searching %d entries", total_count);

  // Filter entries
  uint16_t found = 0;
  for (uint16_t i = 0; i < total_count && found < max_results; i++) {
    if (entry_matches_criteria(&all_entries[i], criteria)) {
      memcpy(&results[found], &all_entries[i], sizeof(Entry));
      found++;
    }
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "search_entries: Found %d matches", found);
  return found;
}
