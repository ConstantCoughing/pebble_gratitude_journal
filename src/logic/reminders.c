#include "reminders.h"
#include "../utils/constants.h"

// Storage keys for reminders
#define STORAGE_KEY_REMINDER_ENABLED 10
#define STORAGE_KEY_REMINDER_HOUR 11
#define STORAGE_KEY_REMINDER_MINUTE 12
#define STORAGE_KEY_WAKEUP_ID 13

// Default reminder time (8:00 PM)
#define DEFAULT_REMINDER_HOUR 20
#define DEFAULT_REMINDER_MINUTE 0

// Wakeup reason codes
#define WAKEUP_REASON_DAILY_REMINDER 1
#define WAKEUP_REASON_SNOOZE 2

static WakeupId s_wakeup_id = -1;

// Get next occurrence of specified time
static time_t get_next_reminder_time(int hour, int minute) {
  time_t now = time(NULL);
  struct tm *time_info = localtime(&now);

  // Set to target time today
  time_info->tm_hour = hour;
  time_info->tm_min = minute;
  time_info->tm_sec = 0;

  time_t target = mktime(time_info);

  // If target time has passed today, schedule for tomorrow
  if (target <= now) {
    target += 86400;  // Add 24 hours
  }

  return target;
}

void reminders_init(void) {
  // Check if there's a pending wakeup
  if (persist_exists(STORAGE_KEY_WAKEUP_ID)) {
    s_wakeup_id = (WakeupId)persist_read_int(STORAGE_KEY_WAKEUP_ID);

    // Verify wakeup is still valid
    if (!wakeup_query(s_wakeup_id, NULL)) {
      s_wakeup_id = -1;
      persist_delete(STORAGE_KEY_WAKEUP_ID);
      APP_LOG(APP_LOG_LEVEL_INFO, "reminders_init: cleared invalid wakeup ID");
    }
  }

  // Check if we were launched by a wakeup event
  WakeupId wakeup_id = 0;
  int32_t reason = 0;

  if (wakeup_get_launch_event(&wakeup_id, &reason)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "reminders_init: launched by wakeup, reason=%ld", (long)reason);
    reminders_handle_wakeup(wakeup_id, reason);
  }

  // If reminders are enabled but no wakeup scheduled, reschedule
  if (reminders_is_enabled() && s_wakeup_id == -1) {
    int hour, minute;
    reminders_get_time(&hour, &minute);
    reminders_schedule(hour, minute);
    APP_LOG(APP_LOG_LEVEL_INFO, "reminders_init: rescheduled reminder for %d:%02d", hour, minute);
  }
}

bool reminders_schedule(int hour, int minute) {
  if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "reminders_schedule: invalid time %d:%02d", hour, minute);
    return false;
  }

  // Cancel existing reminder
  reminders_cancel();

  // Calculate next reminder time
  time_t reminder_time = get_next_reminder_time(hour, minute);

  // Schedule wakeup
  s_wakeup_id = wakeup_schedule(reminder_time, WAKEUP_REASON_DAILY_REMINDER, true);

  if (s_wakeup_id < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "reminders_schedule: wakeup_schedule failed with error %ld", (long)s_wakeup_id);
    return false;
  }

  // Save settings
  persist_write_bool(STORAGE_KEY_REMINDER_ENABLED, true);
  persist_write_int(STORAGE_KEY_REMINDER_HOUR, hour);
  persist_write_int(STORAGE_KEY_REMINDER_MINUTE, minute);
  persist_write_int(STORAGE_KEY_WAKEUP_ID, s_wakeup_id);

  APP_LOG(APP_LOG_LEVEL_INFO, "reminders_schedule: scheduled wakeup ID %ld for %d:%02d",
          (long)s_wakeup_id, hour, minute);
  return true;
}

void reminders_cancel(void) {
  if (s_wakeup_id >= 0) {
    wakeup_cancel(s_wakeup_id);
    APP_LOG(APP_LOG_LEVEL_INFO, "reminders_cancel: cancelled wakeup ID %ld", (long)s_wakeup_id);
    s_wakeup_id = -1;
  }

  persist_write_bool(STORAGE_KEY_REMINDER_ENABLED, false);
  persist_delete(STORAGE_KEY_WAKEUP_ID);
}

bool reminders_snooze(int minutes) {
  if (minutes <= 0 || minutes > 120) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "reminders_snooze: invalid minutes %d", minutes);
    return false;
  }

  // Cancel existing wakeup
  if (s_wakeup_id >= 0) {
    wakeup_cancel(s_wakeup_id);
  }

  // Schedule snooze
  time_t snooze_time = time(NULL) + (minutes * 60);
  s_wakeup_id = wakeup_schedule(snooze_time, WAKEUP_REASON_SNOOZE, false);

  if (s_wakeup_id < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "reminders_snooze: wakeup_schedule failed with error %ld", (long)s_wakeup_id);
    return false;
  }

  persist_write_int(STORAGE_KEY_WAKEUP_ID, s_wakeup_id);
  APP_LOG(APP_LOG_LEVEL_INFO, "reminders_snooze: snoozed for %d minutes, wakeup ID %ld",
          minutes, (long)s_wakeup_id);
  return true;
}

bool reminders_is_enabled(void) {
  if (!persist_exists(STORAGE_KEY_REMINDER_ENABLED)) {
    return false;
  }
  return persist_read_bool(STORAGE_KEY_REMINDER_ENABLED);
}

void reminders_get_time(int *hour, int *minute) {
  if (hour) {
    if (persist_exists(STORAGE_KEY_REMINDER_HOUR)) {
      *hour = persist_read_int(STORAGE_KEY_REMINDER_HOUR);
    } else {
      *hour = DEFAULT_REMINDER_HOUR;
    }
  }

  if (minute) {
    if (persist_exists(STORAGE_KEY_REMINDER_MINUTE)) {
      *minute = persist_read_int(STORAGE_KEY_REMINDER_MINUTE);
    } else {
      *minute = DEFAULT_REMINDER_MINUTE;
    }
  }
}

void reminders_handle_wakeup(WakeupId wakeup_id, int32_t reason) {
  APP_LOG(APP_LOG_LEVEL_INFO, "reminders_handle_wakeup: ID %ld, reason %ld",
          (long)wakeup_id, (long)reason);

  // Clear the wakeup ID
  s_wakeup_id = -1;
  persist_delete(STORAGE_KEY_WAKEUP_ID);

  if (reason == WAKEUP_REASON_DAILY_REMINDER) {
    // Daily reminder triggered - reschedule for tomorrow
    if (reminders_is_enabled()) {
      int hour, minute;
      reminders_get_time(&hour, &minute);
      reminders_schedule(hour, minute);
    }

    // Note: The notification is shown automatically by Pebble when app is launched by wakeup
    // The app will launch and show the home screen with the daily prompt
    APP_LOG(APP_LOG_LEVEL_INFO, "reminders_handle_wakeup: daily reminder triggered");
  } else if (reason == WAKEUP_REASON_SNOOZE) {
    // Snooze expired - reschedule regular reminder
    if (reminders_is_enabled()) {
      int hour, minute;
      reminders_get_time(&hour, &minute);
      // This will schedule for tomorrow since we've passed today's time
      reminders_schedule(hour, minute);
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "reminders_handle_wakeup: snooze expired");
  }
}
