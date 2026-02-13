#include "entry_detail_window.h"
#include "edit_entry_window.h"
#include "../data/storage.h"
#include "../logic/stats.h"
#include "../utils/date_utils.h"
#include "../utils/constants.h"
#include <string.h>

static Window *s_window;
static Window *s_action_menu_window;
static SimpleMenuLayer *s_action_menu_layer;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_date_layer;
static TextLayer *s_mood_layer;
static TextLayer *s_text_layer;
static time_t s_current_date;
static uint16_t s_current_entry_index;

// Mood labels
static const char* MOOD_LABELS[9] = {
  "Sad",
  "Anxious",
  "Stressed",
  "Tired",
  "Neutral",
  "Content",
  "Happy",
  "Excited",
  "Grateful"
};

// Action menu callbacks
static void edit_entry_callback(int index, void *context) {
  edit_entry_window_push(s_current_entry_index);
}

static void delete_entry_callback(int index, void *context) {
  // Confirm deletion
  if (storage_delete_entry(s_current_entry_index)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "entry_detail: entry deleted successfully");

    // Recalculate streak after deletion
    Stats stats;
    storage_load_stats(&stats);
    stats.current_streak = stats_calculate_streak();
    storage_save_stats(&stats);

    // Pop both action menu and detail window
    window_stack_pop(true);  // Action menu
    window_stack_pop(true);  // Detail window
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "entry_detail: failed to delete entry");
  }
}

static void show_action_menu(void) {
  static SimpleMenuItem action_items[2];
  static SimpleMenuSection action_section;

  action_items[0] = (SimpleMenuItem) {
    .title = "Edit Entry",
    .callback = edit_entry_callback
  };

  action_items[1] = (SimpleMenuItem) {
    .title = "Delete Entry",
    .callback = delete_entry_callback
  };

  action_section = (SimpleMenuSection) {
    .title = "Actions",
    .items = action_items,
    .num_items = 2
  };

  s_action_menu_window = window_create();
  Layer *menu_layer = window_get_root_layer(s_action_menu_window);
  GRect bounds = layer_get_bounds(menu_layer);

  s_action_menu_layer = simple_menu_layer_create(bounds, s_action_menu_window, &action_section, 1, NULL);
  layer_add_child(menu_layer, simple_menu_layer_get_layer(s_action_menu_layer));

  window_stack_push(s_action_menu_window, true);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  show_action_menu();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Set click config provider
  window_set_click_config_provider(window, click_config_provider);

  // Create scroll layer
  s_scroll_layer = scroll_layer_create(bounds);
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));

  // Load entries for the date
  Entry entries[10];  // Support multiple entries per day
  uint16_t entry_count = storage_get_entries_for_date(s_current_date, entries, 10);

  if (entry_count == 0) {
    // No entries for this day
    s_text_layer = text_layer_create(GRect(10, 10, bounds.size.w - 20, 2000));
    text_layer_set_text(s_text_layer, "No entries for this date.");
    text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_text_layer));

    GSize content_size = text_layer_get_content_size(s_text_layer);
    text_layer_set_size(s_text_layer, content_size);
    scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, content_size.h + 20));
    return;
  }

  // Display first entry (if multiple, show the latest)
  Entry entry = entries[0];

  // Find the entry index for editing/deleting
  // Need to search all entries to find index
  Entry all_entries[MAX_ENTRIES];
  uint16_t all_count = storage_get_all_entries(all_entries, MAX_ENTRIES);
  s_current_entry_index = 0;
  for (uint16_t i = 0; i < all_count; i++) {
    if (all_entries[i].date == entry.date &&
        all_entries[i].mood == entry.mood &&
        strcmp(all_entries[i].text, entry.text) == 0) {
      s_current_entry_index = i;
      break;
    }
  }

  // Format date
  struct tm *time_info = localtime(&entry.date);
  static char date_text[64];
  strftime(date_text, sizeof(date_text), "%B %d, %Y", time_info);

  // Create date layer
  s_date_layer = text_layer_create(GRect(10, 10, bounds.size.w - 20, 30));
  text_layer_set_text(s_date_layer, date_text);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_date_layer));

  // Create mood layer
  static char mood_text[32];
  snprintf(mood_text, sizeof(mood_text), "Mood: %s", MOOD_LABELS[entry.mood]);
  s_mood_layer = text_layer_create(GRect(10, 45, bounds.size.w - 20, 25));
  text_layer_set_text(s_mood_layer, mood_text);
  text_layer_set_font(s_mood_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_mood_layer, GTextAlignmentCenter);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_mood_layer));

  // Create text layer for entry content
  s_text_layer = text_layer_create(GRect(10, 80, bounds.size.w - 20, 2000));
  text_layer_set_text(s_text_layer, entry.text);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_text_layer));

  // Set proper size for text layer
  GSize content_size = text_layer_get_content_size(s_text_layer);
  text_layer_set_size(s_text_layer, content_size);

  // Set scroll layer content size
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, content_size.h + 100));
}

static void window_unload(Window *window) {
  if (s_date_layer) {
    text_layer_destroy(s_date_layer);
    s_date_layer = NULL;
  }
  if (s_mood_layer) {
    text_layer_destroy(s_mood_layer);
    s_mood_layer = NULL;
  }
  if (s_text_layer) {
    text_layer_destroy(s_text_layer);
    s_text_layer = NULL;
  }
  if (s_scroll_layer) {
    scroll_layer_destroy(s_scroll_layer);
    s_scroll_layer = NULL;
  }
}

void entry_detail_window_push(time_t date) {
  s_current_date = date;

  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void entry_detail_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
