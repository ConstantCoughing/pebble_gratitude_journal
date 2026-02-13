#include "reminder_settings_window.h"
#include "../logic/reminders.h"

static Window *s_window;
static SimpleMenuLayer *s_menu_layer;
static SimpleMenuSection s_menu_section;
static SimpleMenuItem s_menu_items[4];

static char s_enabled_text[16];
static char s_time_text[16];
static char s_snooze_15_text[32];
static char s_snooze_30_text[32];

static void toggle_enabled_callback(int index, void *context) {
  bool enabled = reminders_is_enabled();

  if (enabled) {
    // Disable reminders
    reminders_cancel();
  } else {
    // Enable reminders with default or saved time
    int hour, minute;
    reminders_get_time(&hour, &minute);
    reminders_schedule(hour, minute);
  }

  // Update display
  snprintf(s_enabled_text, sizeof(s_enabled_text), reminders_is_enabled() ? "On" : "Off");
  menu_layer_reload_data(simple_menu_layer_get_menu_layer(s_menu_layer));
}

static void change_time_callback(int index, void *context) {
  // For simplicity, cycle through common times
  // In a full implementation, this would use a time picker
  static const int times[][2] = {
    {8, 0},   // 8:00 AM
    {12, 0},  // 12:00 PM
    {18, 0},  // 6:00 PM
    {20, 0},  // 8:00 PM (default)
    {21, 0},  // 9:00 PM
    {22, 0}   // 10:00 PM
  };
  static int current_time_index = 3;  // Default to 8:00 PM

  // Get current time
  int hour, minute;
  reminders_get_time(&hour, &minute);

  // Find current time in array
  for (int i = 0; i < 6; i++) {
    if (times[i][0] == hour && times[i][1] == minute) {
      current_time_index = i;
      break;
    }
  }

  // Cycle to next time
  current_time_index = (current_time_index + 1) % 6;
  hour = times[current_time_index][0];
  minute = times[current_time_index][1];

  // Schedule reminder with new time
  if (reminders_is_enabled()) {
    reminders_schedule(hour, minute);
  } else {
    // Just save the time for when they enable it
    reminders_schedule(hour, minute);
    reminders_cancel();  // Cancel immediately if not enabled
  }

  // Update display
  const char *period = (hour < 12) ? "AM" : "PM";
  int display_hour = (hour == 0) ? 12 : (hour > 12) ? hour - 12 : hour;
  snprintf(s_time_text, sizeof(s_time_text), "%d:%02d %s", display_hour, minute, period);
  menu_layer_reload_data(simple_menu_layer_get_menu_layer(s_menu_layer));
}

static void snooze_15_callback(int index, void *context) {
  if (reminders_snooze(15)) {
    snprintf(s_snooze_15_text, sizeof(s_snooze_15_text), "Snoozed for 15 min");
  } else {
    snprintf(s_snooze_15_text, sizeof(s_snooze_15_text), "Failed to snooze");
  }
  menu_layer_reload_data(simple_menu_layer_get_menu_layer(s_menu_layer));

  // Close settings window after snooze
  window_stack_pop(true);
}

static void snooze_30_callback(int index, void *context) {
  if (reminders_snooze(30)) {
    snprintf(s_snooze_30_text, sizeof(s_snooze_30_text), "Snoozed for 30 min");
  } else {
    snprintf(s_snooze_30_text, sizeof(s_snooze_30_text), "Failed to snooze");
  }
  menu_layer_reload_data(simple_menu_layer_get_menu_layer(s_menu_layer));

  // Close settings window after snooze
  window_stack_pop(true);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Update text fields
  snprintf(s_enabled_text, sizeof(s_enabled_text), reminders_is_enabled() ? "On" : "Off");

  int hour, minute;
  reminders_get_time(&hour, &minute);
  const char *period = (hour < 12) ? "AM" : "PM";
  int display_hour = (hour == 0) ? 12 : (hour > 12) ? hour - 12 : hour;
  snprintf(s_time_text, sizeof(s_time_text), "%d:%02d %s", display_hour, minute, period);

  snprintf(s_snooze_15_text, sizeof(s_snooze_15_text), "15 minutes");
  snprintf(s_snooze_30_text, sizeof(s_snooze_30_text), "30 minutes");

  // Setup menu items
  s_menu_items[0] = (SimpleMenuItem) {
    .title = "Reminders",
    .subtitle = s_enabled_text,
    .callback = toggle_enabled_callback
  };

  s_menu_items[1] = (SimpleMenuItem) {
    .title = "Reminder Time",
    .subtitle = s_time_text,
    .callback = change_time_callback
  };

  s_menu_items[2] = (SimpleMenuItem) {
    .title = "Snooze 15 min",
    .subtitle = s_snooze_15_text,
    .callback = snooze_15_callback
  };

  s_menu_items[3] = (SimpleMenuItem) {
    .title = "Snooze 30 min",
    .subtitle = s_snooze_30_text,
    .callback = snooze_30_callback
  };

  s_menu_section = (SimpleMenuSection) {
    .title = "Daily Reminders",
    .items = s_menu_items,
    .num_items = 4
  };

  // Create menu layer
  s_menu_layer = simple_menu_layer_create(bounds, window, &s_menu_section, 1, NULL);
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  simple_menu_layer_destroy(s_menu_layer);
}

void reminder_settings_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void reminder_settings_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
