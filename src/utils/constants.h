#pragma once

// Storage constants
#define MAX_ENTRIES 180
#define STORAGE_WARNING_THRESHOLD 162  // 90% of MAX_ENTRIES
#define MAX_ENTRY_TEXT_LENGTH 140

// Storage keys
#define STORAGE_KEY_VERSION 1
#define STORAGE_KEY_ENTRY_COUNT 2
#define STORAGE_KEY_CURRENT_STREAK 3
#define STORAGE_KEY_LAST_ENTRY_DATE 4
#define STORAGE_KEY_PROMPT_INDEX 5
#define STORAGE_KEY_PROMPT_MODE 6
#define STORAGE_KEY_ENTRIES_START 100

// App version
#define APP_VERSION "0.1.1"

// Prompt constants
#define NUM_PROMPTS 50

// Canned response labels
#define NUM_CANNED_RESPONSES 10
