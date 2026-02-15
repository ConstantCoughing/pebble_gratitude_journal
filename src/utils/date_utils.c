#include "date_utils.h"

// Month names
static const char* MONTH_NAMES[] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
};

// Short day names
static const char* DAY_NAMES_SHORT[] = {
  "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"
};

// Days in each month (non-leap year)
static const uint8_t DAYS_IN_MONTH[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

time_t date_normalize_to_midnight(time_t timestamp) {
  struct tm time_info_copy = *localtime(&timestamp);
  time_info_copy.tm_hour = 0;
  time_info_copy.tm_min = 0;
  time_info_copy.tm_sec = 0;
  return mktime(&time_info_copy);
}

time_t date_get_today(void) {
  return date_normalize_to_midnight(time(NULL));
}

bool date_is_same_day(time_t date1, time_t date2) {
  return date_normalize_to_midnight(date1) == date_normalize_to_midnight(date2);
}

int32_t date_days_between(time_t date1, time_t date2) {
  time_t normalized1 = date_normalize_to_midnight(date1);
  time_t normalized2 = date_normalize_to_midnight(date2);
  return (int32_t)((normalized2 - normalized1) / (24 * 60 * 60));
}

time_t date_add_days(time_t date, int32_t days) {
  return date + (days * 24 * 60 * 60);
}

time_t date_get_first_of_month(time_t date) {
  struct tm time_info_copy = *localtime(&date);
  time_info_copy.tm_mday = 1;
  time_info_copy.tm_hour = 0;
  time_info_copy.tm_min = 0;
  time_info_copy.tm_sec = 0;
  return mktime(&time_info_copy);
}

time_t date_get_last_of_month(time_t date) {
  struct tm time_info_copy = *localtime(&date);
  uint8_t days = date_get_days_in_month(date);
  time_info_copy.tm_mday = days;
  time_info_copy.tm_hour = 0;
  time_info_copy.tm_min = 0;
  time_info_copy.tm_sec = 0;
  return mktime(&time_info_copy);
}

uint8_t date_get_days_in_month(time_t date) {
  struct tm *time_info = localtime(&date);
  uint8_t month = time_info->tm_mon;
  uint16_t year = time_info->tm_year + 1900;

  if (month == 1 && date_is_leap_year(year)) {
    return 29;
  }
  return DAYS_IN_MONTH[month];
}

uint8_t date_get_day_of_week(time_t date) {
  struct tm *time_info = localtime(&date);
  return time_info->tm_wday;
}

bool date_is_leap_year(uint16_t year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

time_t date_next_month(time_t date) {
  struct tm time_info_copy = *localtime(&date);

  if (time_info_copy.tm_mon == 11) {
    time_info_copy.tm_mon = 0;
    time_info_copy.tm_year++;
  } else {
    time_info_copy.tm_mon++;
  }

  // Adjust day if it exceeds days in new month
  uint8_t max_days = DAYS_IN_MONTH[time_info_copy.tm_mon];
  if (time_info_copy.tm_mon == 1 && date_is_leap_year(time_info_copy.tm_year + 1900)) {
    max_days = 29;
  }
  if (time_info_copy.tm_mday > max_days) {
    time_info_copy.tm_mday = max_days;
  }

  return mktime(&time_info_copy);
}

time_t date_prev_month(time_t date) {
  struct tm time_info_copy = *localtime(&date);

  if (time_info_copy.tm_mon == 0) {
    time_info_copy.tm_mon = 11;
    time_info_copy.tm_year--;
  } else {
    time_info_copy.tm_mon--;
  }

  // Adjust day if it exceeds days in new month
  uint8_t max_days = DAYS_IN_MONTH[time_info_copy.tm_mon];
  if (time_info_copy.tm_mon == 1 && date_is_leap_year(time_info_copy.tm_year + 1900)) {
    max_days = 29;
  }
  if (time_info_copy.tm_mday > max_days) {
    time_info_copy.tm_mday = max_days;
  }

  return mktime(&time_info_copy);
}

const char* date_get_month_name(uint8_t month) {
  if (month >= 12) return "Unknown";
  return MONTH_NAMES[month];
}

const char* date_get_day_name_short(uint8_t day_of_week) {
  if (day_of_week >= 7) return "??";
  return DAY_NAMES_SHORT[day_of_week];
}
