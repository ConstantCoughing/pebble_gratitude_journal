#pragma once

#include <pebble.h>

// Normalize time_t to midnight (00:00:00) of the same day
time_t date_normalize_to_midnight(time_t timestamp);

// Get the current date normalized to midnight
time_t date_get_today(void);

// Check if two timestamps are on the same day
bool date_is_same_day(time_t date1, time_t date2);

// Get number of days between two dates
int32_t date_days_between(time_t date1, time_t date2);

// Add/subtract days from a date
time_t date_add_days(time_t date, int32_t days);

// Get the first day of the month for a given date
time_t date_get_first_of_month(time_t date);

// Get the last day of the month for a given date
time_t date_get_last_of_month(time_t date);

// Get number of days in the month for a given date
uint8_t date_get_days_in_month(time_t date);

// Get day of week (0=Sunday, 6=Saturday)
uint8_t date_get_day_of_week(time_t date);

// Check if a year is a leap year
bool date_is_leap_year(uint16_t year);

// Navigate to next month
time_t date_next_month(time_t date);

// Navigate to previous month
time_t date_prev_month(time_t date);

// Get month name
const char* date_get_month_name(uint8_t month);

// Get short day name (Su, Mo, Tu, etc.)
const char* date_get_day_name_short(uint8_t day_of_week);
