#pragma once

#include <pebble.h>

// Get today's prompt based on current mode
const char* prompts_get_daily(void);

// Set prompt mode (false = random, true = sequential)
void prompts_set_mode(bool is_sequential);

// Get current prompt mode
bool prompts_get_mode(void);

// Get total number of prompts
uint8_t prompts_get_count(void);
