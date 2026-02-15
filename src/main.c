#include <pebble.h>
#include "data/storage.h"
#include "logic/reminders.h"
#include "windows/home_window.h"
#include "windows/calendar_window.h"
#include "windows/entry_window.h"
#include "windows/entry_detail_window.h"
#include "windows/edit_entry_window.h"
#include "windows/search_window.h"
#include "windows/visualization_window.h"
#include "windows/settings_window.h"
#include "windows/reminder_settings_window.h"
#include "windows/custom_prompts_window.h"

static void init(void) {
  // Initialize storage
  storage_init();

  // Initialize reminder system
  reminders_init();

  // Show home window
  home_window_push();
}

static void deinit(void) {
  // Cleanup all windows
  home_window_destroy();
  calendar_window_destroy();
  entry_window_destroy();
  entry_detail_window_destroy();
  edit_entry_window_destroy();
  search_window_destroy();
  visualization_window_destroy();
  settings_window_destroy();
  reminder_settings_window_destroy();
  custom_prompts_window_destroy();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
