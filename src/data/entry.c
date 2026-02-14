#include "entry.h"
#include "../utils/date_utils.h"
#include <string.h>
#include <stdio.h>

// Canned response strings
static const char* CANNED_RESPONSE_STRINGS[] = {
  "family",
  "friends",
  "health",
  "work",
  "nature",
  "food",
  "music",
  "rest",
  "learning",
  "pets"
};

// Mood strings
static const char* MOOD_STRINGS[] = {
  "Sad",
  "Anxious",
  "Stressed",
  "Tired",
  "Neutral",
  "Content",
  "Happy",
  "Excited",
  "Grateful"
};

void entry_init(Entry *entry, time_t date, Mood mood, uint16_t canned_flags) {
  if (!entry) return;

  memset(entry, 0, sizeof(Entry));
  entry->date = date_normalize_to_midnight(date);
  entry->mood = mood;
  entry->canned_flags = canned_flags;
  entry_generate_text(canned_flags, entry->text, sizeof(entry->text));
}

void entry_generate_text(uint16_t canned_flags, char *buffer, size_t size) {
  if (!buffer || size == 0) return;

  buffer[0] = '\0';
  int count = 0;
  int first_index = -1;
  int last_index = -1;

  // Count selected responses and find first/last indices
  for (int i = 0; i < NUM_CANNED_RESPONSES; i++) {
    if (canned_flags & (1 << i)) {
      if (first_index == -1) {
        first_index = i;
      }
      last_index = i;
      count++;
    }
  }

  if (count == 0) {
    snprintf(buffer, size, "grateful");
    return;
  }

  // Build text with proper grammar
  int current = 0;
  for (int i = 0; i < NUM_CANNED_RESPONSES; i++) {
    if (canned_flags & (1 << i)) {
      size_t remaining = size - strlen(buffer) - 1;
      if (remaining < 2) break;  // Not enough space

      if (current > 0) {
        if (i == last_index && count > 1) {
          strncat(buffer, " and ", remaining);
        } else {
          strncat(buffer, ", ", remaining);
        }
      }

      remaining = size - strlen(buffer) - 1;
      strncat(buffer, CANNED_RESPONSE_STRINGS[i], remaining);
      current++;
    }
  }

  // Enforce 140 char limit
  if (strlen(buffer) > MAX_ENTRY_TEXT_LENGTH) {
    buffer[MAX_ENTRY_TEXT_LENGTH] = '\0';
  }
}

bool entry_validate(const Entry *entry) {
  if (!entry) return false;

  // Check date is normalized
  struct tm *time_info = localtime(&entry->date);
  if (time_info->tm_hour != 0 || time_info->tm_min != 0 || time_info->tm_sec != 0) {
    return false;
  }

  // Check mood is valid
  if (entry->mood > MOOD_GRATEFUL) {
    return false;
  }

  // Check text length
  if (strlen(entry->text) > MAX_ENTRY_TEXT_LENGTH) {
    return false;
  }

  // Check at least one canned response is selected
  if (entry->canned_flags == 0) {
    return false;
  }

  return true;
}

const char* mood_to_string(Mood mood) {
  if (mood > MOOD_GRATEFUL) {
    return "Unknown";
  }
  return MOOD_STRINGS[mood];
}

const char* canned_response_to_string(CannedResponse response) {
  for (int i = 0; i < NUM_CANNED_RESPONSES; i++) {
    if (response == (1 << i)) {
      return CANNED_RESPONSE_STRINGS[i];
    }
  }
  return "unknown";
}
