#pragma once

#include <pebble.h>

// Initialize reminder system (call on app init)
void reminders_init(void);

// Schedule daily reminder at specified time
bool reminders_schedule(int hour, int minute);

// Cancel all scheduled reminders
void reminders_cancel(void);

// Snooze reminder for specified minutes
bool reminders_snooze(int minutes);

// Check if reminders are enabled
bool reminders_is_enabled(void);

// Get configured reminder time
void reminders_get_time(int *hour, int *minute);

// Handle wakeup event (called from app init)
void reminders_handle_wakeup(WakeupId wakeup_id, int32_t reason);
