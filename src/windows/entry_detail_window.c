#include "entry_detail_window.h"
#include "edit_entry_window.h"
#include "../data/storage.h"
#include "../logic/stats.h"
#include "../utils/date_utils.h"
#include "../utils/constants.h"
#include <string.h>
#include <stdlib.h>

#define MAX_DAY_ENTRIES 10

static Window *s_window;
static MenuLayer *s_menu_layer;
static TextLayer *s_empty_layer;
static time_t s_current_date;

// Entries for the selected day (static so menu draw callbacks can access them)
static Entry s_day_entries[MAX_DAY_ENTRIES];
static uint16_t s_day_entry_count;

// Global entry indices for each day entry (for edit/delete)
static uint16_t s_global_indices[MAX_DAY_ENTRIES];

// Currently selected entry index (global) for action menu
static uint16_t s_selected_global_index;

// Action menu window
static Window *s_action_menu_window;
static SimpleMenuLayer *s_action_menu_layer;

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

// Forward declarations
static void load_entries_for_date(void);
static void cleanup_action_menu(void);

// Clean up the action menu window and its layers
static void cleanup_action_menu(void) {
  if (s_action_menu_layer) {
    simple_menu_layer_destroy(s_action_menu_layer);
    s_action_menu_layer = NULL;
  }
  if (s_action_menu_window) {
    window_destroy(s_action_menu_window);
    s_action_menu_window = NULL;
  }
}

// Action menu callbacks
static void edit_entry_callback(int index, void *context) {
  // Pop and clean up action menu before pushing edit window
  window_stack_pop(true);
  cleanup_action_menu();
  edit_entry_window_push(s_selected_global_index);
}

static void delete_entry_callback(int index, void *context) {
  if (storage_delete_entry(s_selected_global_index)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "entry_detail: entry deleted successfully");

    // Recalculate streak after deletion
    Stats stats;
    storage_load_stats(&stats);
    stats.current_streak = stats_calculate_streak();
    storage_save_stats(&stats);

    // Pop and clean up action menu
    window_stack_pop(true);
    cleanup_action_menu();

    // Reload entries for this day and refresh menu
    load_entries_for_date();
    if (s_day_entry_count == 0) {
      // No more entries - pop detail window too
      window_stack_pop(true);
    } else {
      menu_layer_reload_data(s_menu_layer);
    }
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

// Load entries for the current date and resolve raw storage indices
static void load_entries_for_date(void) {
  time_t normalized_date = date_normalize_to_midnight(s_current_date);
  uint16_t count = storage_get_entry_count();
  s_day_entry_count = 0;

  for (uint16_t i = 0; i < count && s_day_entry_count < MAX_DAY_ENTRIES; i++) {
    Entry entry;
    if (storage_get_entry_by_index(i, &entry)) {
      if (entry.date == normalized_date) {
        s_day_entries[s_day_entry_count] = entry;
        s_global_indices[s_day_entry_count] = i;  // Raw storage index
        s_day_entry_count++;
      }
    }
  }
}

// Menu layer callbacks
static uint16_t menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return s_day_entry_count > 0 ? s_day_entry_count : 1;
}

static void menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  if (s_day_entry_count == 0) {
    menu_cell_basic_draw(ctx, cell_layer, "No entries", "for this date", NULL);
    return;
  }

  uint16_t row = cell_index->row;
  if (row >= s_day_entry_count) return;

  Entry *entry = &s_day_entries[row];
  const char *mood = MOOD_LABELS[entry->mood];

  // Show entry number, mood as title; text preview as subtitle
  static char title_buf[32];
  snprintf(title_buf, sizeof(title_buf), "#%d - %s", row + 1, mood);

  // Truncate text for subtitle preview
  static char subtitle_buf[32];
  strncpy(subtitle_buf, entry->text, sizeof(subtitle_buf) - 1);
  subtitle_buf[sizeof(subtitle_buf) - 1] = '\0';

  menu_cell_basic_draw(ctx, cell_layer, title_buf, subtitle_buf, NULL);
}

static void menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  if (s_day_entry_count == 0) {
    window_stack_pop(true);
    return;
  }

  uint16_t row = cell_index->row;
  if (row >= s_day_entry_count) return;

  s_selected_global_index = s_global_indices[row];
  show_action_menu();
}

// Refresh data when window appears (handles return from edit)
static void window_appear(Window *window) {
  load_entries_for_date();
  if (s_menu_layer) {
    menu_layer_reload_data(s_menu_layer);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Load entries for this date
  load_entries_for_date();

  // Create menu layer to list all entries
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = menu_get_num_rows,
    .draw_row = menu_draw_row,
    .select_click = menu_select
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  if (s_menu_layer) {
    menu_layer_destroy(s_menu_layer);
    s_menu_layer = NULL;
  }
  if (s_empty_layer) {
    text_layer_destroy(s_empty_layer);
    s_empty_layer = NULL;
  }
}

void entry_detail_window_push(time_t date) {
  s_current_date = date;

  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      .appear = window_appear
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
