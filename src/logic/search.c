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

// Context for iterator-based search
typedef struct {
  const SearchCriteria *criteria;
  Entry *results;
  uint16_t max_results;
  uint16_t found;
} SearchIterCtx;

static bool search_iterator_callback(const Entry *entry, uint16_t index, void *context) {
  SearchIterCtx *ctx = (SearchIterCtx *)context;
  if (ctx->found >= ctx->max_results) {
    return false;  // stop early, buffer full
  }
  if (entry_matches_criteria(entry, ctx->criteria)) {
    memcpy(&ctx->results[ctx->found], entry, sizeof(Entry));
    ctx->found++;
  }
  return true;  // continue
}

uint16_t search_entries(const SearchCriteria *criteria, Entry *results, uint16_t max_results) {
  if (!criteria || !results || max_results == 0) {
    return 0;
  }

  SearchIterCtx ctx = {
    .criteria = criteria,
    .results = results,
    .max_results = max_results,
    .found = 0
  };

  storage_iterate_entries(search_iterator_callback, &ctx);

  APP_LOG(APP_LOG_LEVEL_INFO, "search_entries: Found %d matches", ctx.found);
  return ctx.found;
}
