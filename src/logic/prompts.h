#pragma once

#include <pebble.h>

// Get today's prompt based on current mode
const char* prompts_get_daily(void);

// Set prompt mode (false = random, true = sequential)
void prompts_set_mode(bool is_sequential);

// Get current prompt mode
bool prompts_get_mode(void);

// Get total number of prompts (built-in + custom)
uint8_t prompts_get_count(void);

// Custom prompt management (max 20 custom prompts)
#define MAX_CUSTOM_PROMPTS 20
#define CUSTOM_PROMPT_MAX_LENGTH 100

// Add a custom prompt
bool prompts_add_custom(const char *text);

// Get custom prompt by index
bool prompts_get_custom(uint8_t index, char *buffer, size_t buffer_size);

// Delete custom prompt by index
bool prompts_delete_custom(uint8_t index);

// Get number of custom prompts
uint8_t prompts_get_custom_count(void);

// Get a prompt by absolute index (0-49 built-in, 50+ custom)
const char* prompts_get_by_index(uint8_t index);
