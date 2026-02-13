#include "settings_window.h"
#include "reminder_settings_window.h"
#include "custom_prompts_window.h"
#include "../data/storage.h"
#include "../logic/prompts.h"
#include "../logic/reminders.h"
#include "../utils/constants.h"

static Window *s_window;
static SimpleMenuLayer *s_menu_layer;
static SimpleMenuSection s_menu_section;
static SimpleMenuItem s_menu_items[6];
static char s_storage_text[32];
static char s_prompt_mode_text[32];
static char s_reminders_text[32];
static char s_custom_prompts_text[32];
static char s_export_text[32];
static char s_about_text[32];

static void prompt_mode_callback(int index, void *context) {
  // Toggle prompt mode
  bool current_mode = prompts_get_mode();
  prompts_set_mode(!current_mode);

  // Update display
  snprintf(s_prompt_mode_text, sizeof(s_prompt_mode_text),
           prompts_get_mode() ? "Sequential" : "Random");
  menu_layer_reload_data(simple_menu_layer_get_menu_layer(s_menu_layer));
}

static void reminders_callback(int index, void *context) {
  // Open reminder settings window
  reminder_settings_window_push();
}

static void custom_prompts_callback(int index, void *context) {
  // Open custom prompts window
  custom_prompts_window_push();
}

static void export_callback(int index, void *context) {
  // Trigger data export via AppMessage
  // For simplicity, just show a message for now
  snprintf(s_export_text, sizeof(s_export_text), "Export initiated");
  menu_layer_reload_data(simple_menu_layer_get_menu_layer(s_menu_layer));

  // TODO: Implement actual export via PebbleKit JS
  APP_LOG(APP_LOG_LEVEL_INFO, "Export triggered - PebbleKit JS needed for implementation");
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Update storage text
  uint16_t count = storage_get_entry_count();
  uint8_t utilization = storage_get_utilization();
  snprintf(s_storage_text, sizeof(s_storage_text), "%d / %d (%d%%)", count, MAX_ENTRIES, utilization);

  // Update prompt mode text
  snprintf(s_prompt_mode_text, sizeof(s_prompt_mode_text),
           prompts_get_mode() ? "Sequential" : "Random");

  // Update reminders text
  snprintf(s_reminders_text, sizeof(s_reminders_text),
           reminders_is_enabled() ? "On" : "Off");

  // Update custom prompts text
  snprintf(s_custom_prompts_text, sizeof(s_custom_prompts_text),
           "%d custom", prompts_get_custom_count());

  // Export text
  snprintf(s_export_text, sizeof(s_export_text), "Export data");

  // About text
  snprintf(s_about_text, sizeof(s_about_text), "v%s", APP_VERSION);

  // Setup menu items
  s_menu_items[0] = (SimpleMenuItem) {
    .title = "Storage",
    .subtitle = s_storage_text
  };

  s_menu_items[1] = (SimpleMenuItem) {
    .title = "Prompt Mode",
    .subtitle = s_prompt_mode_text,
    .callback = prompt_mode_callback
  };

  s_menu_items[2] = (SimpleMenuItem) {
    .title = "Reminders",
    .subtitle = s_reminders_text,
    .callback = reminders_callback
  };

  s_menu_items[3] = (SimpleMenuItem) {
    .title = "Custom Prompts",
    .subtitle = s_custom_prompts_text,
    .callback = custom_prompts_callback
  };

  s_menu_items[4] = (SimpleMenuItem) {
    .title = "Export Data",
    .subtitle = s_export_text,
    .callback = export_callback
  };

  s_menu_items[5] = (SimpleMenuItem) {
    .title = "About",
    .subtitle = s_about_text
  };

  s_menu_section = (SimpleMenuSection) {
    .items = s_menu_items,
    .num_items = 6
  };

  // Create menu layer
  s_menu_layer = simple_menu_layer_create(bounds, window, &s_menu_section, 1, NULL);
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  simple_menu_layer_destroy(s_menu_layer);
}

void settings_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void settings_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
