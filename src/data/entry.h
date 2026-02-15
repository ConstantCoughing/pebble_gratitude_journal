#pragma once

#include <pebble.h>
#include "../utils/constants.h"

// Mood enum ordered by valence (0=worst, 8=best)
typedef enum {
  MOOD_SAD = 0,
  MOOD_ANXIOUS = 1,
  MOOD_STRESSED = 2,
  MOOD_TIRED = 3,
  MOOD_NEUTRAL = 4,
  MOOD_CONTENT = 5,
  MOOD_HAPPY = 6,
  MOOD_EXCITED = 7,
  MOOD_GRATEFUL = 8
} Mood;

// Canned response flags (bitmask for combination)
typedef enum {
  CANNED_FAMILY    = (1 << 0),
  CANNED_FRIENDS   = (1 << 1),
  CANNED_HEALTH    = (1 << 2),
  CANNED_WORK      = (1 << 3),
  CANNED_NATURE    = (1 << 4),
  CANNED_FOOD      = (1 << 5),
  CANNED_MUSIC     = (1 << 6),
  CANNED_REST      = (1 << 7),
  CANNED_LEARNING  = (1 << 8),
  CANNED_PETS      = (1 << 9)
} CannedResponse;

// Entry structure (~151 bytes)
typedef struct {
  time_t date;                    // Normalized to midnight (4 bytes)
  char text[141];                 // 140 chars + null terminator (141 bytes)
  Mood mood;                      // 1 byte (padded to 4)
  uint16_t canned_flags;          // Bitmask of selected responses (2 bytes)
} Entry;

// Stats structure
typedef struct {
  uint16_t current_streak;        // Consecutive days with entries
  time_t last_entry_date;         // For streak calculation
  uint16_t total_entries;         // All-time count
  uint16_t mood_counts[9];        // Count per mood type
} Stats;

// Centralized label arrays
extern const char* MOOD_LABELS[9];
extern const char* CANNED_LABELS[NUM_CANNED_RESPONSES];

// Function declarations
void entry_init(Entry *entry, time_t date, Mood mood, uint16_t canned_flags);
void entry_generate_text(uint16_t canned_flags, char *buffer, size_t size);
bool entry_validate(const Entry *entry);
const char* mood_to_string(Mood mood);
const char* canned_response_to_string(CannedResponse response);
