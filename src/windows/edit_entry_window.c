#include "edit_entry_window.h"
#include "../data/storage.h"
#include "../logic/stats.h"
#include "../utils/date_utils.h"
#include "../utils/constants.h"
#include <string.h>

// Canned response labels
static const char* CANNED_LABELS[NUM_CANNED_RESPONSES] = {
  "Family", "Friends", "Health", "Work", "Nature",
  "Food", "Music", "Rest", "Learning", "Pets"
};

// Mood labels
static const char* MOOD_LABELS[9] = {
  "Sad", "Anxious", "Stressed", "Tired", "Neutral",
  "Content", "Happy", "Excited", "Grateful"
};

typedef enum {
  STAGE_CANNED_RESPONSE,
  STAGE_MOOD_SELECTION,
  STAGE_COMPLETE
} EditStage;

static Window *s_window;
static MenuLayer *s_menu_layer;
static EditStage *s_current_stage;
static uint16_t s_selected_canned_flags;
static Mood s_selected_mood;
static uint16_t s_entry_index;
static Entry s_original_entry;

// Forward declarations
static void show_mood_selection(void);
static void save_and_close(void);

// Canned response menu callbacks
static uint16_t canned_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return NUM_CANNED_RESPONSES + 1;  // +1 for "Done"
}

static void canned_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  if (cell_index->row == NUM_CANNED_RESPONSES) {
    menu_cell_basic_draw(ctx, cell_layer, "Done", NULL, NULL);
  } else {
    bool is_selected = (s_selected_canned_flags & (1 << cell_index->row)) != 0;
    menu_cell_basic_draw(ctx, cell_layer, CANNED_LABELS[cell_index->row],
                        is_selected ? "✓" : NULL, NULL);
  }
}

static void canned_menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  if (cell_index->row == NUM_CANNED_RESPONSES) {
    // Done button
    if (s_selected_canned_flags == 0) {
      s_selected_canned_flags = CANNED_FAMILY;  // Default
    }
    show_mood_selection();
  } else {
    // Toggle selection
    uint16_t flag = (1 << cell_index->row);
    s_selected_canned_flags ^= flag;
    menu_layer_reload_data(s_menu_layer);
  }
}

// Get mood icon resource ID (28×28 for mood selection)
static uint32_t get_mood_icon_resource(Mood mood) {
  switch (mood) {
    case MOOD_SAD: return RESOURCE_ID_MOOD_SAD_28;
    case MOOD_ANXIOUS: return RESOURCE_ID_MOOD_ANXIOUS_28;
    case MOOD_STRESSED: return RESOURCE_ID_MOOD_STRESSED_28;
    case MOOD_TIRED: return RESOURCE_ID_MOOD_TIRED_28;
    case MOOD_NEUTRAL: return RESOURCE_ID_MOOD_NEUTRAL_28;
    case MOOD_CONTENT: return RESOURCE_ID_MOOD_CONTENT_28;
    case MOOD_HAPPY: return RESOURCE_ID_MOOD_HAPPY_28;
    case MOOD_EXCITED: return RESOURCE_ID_MOOD_EXCITED_28;
    case MOOD_GRATEFUL: return RESOURCE_ID_MOOD_GRATEFUL_28;
    default: return RESOURCE_ID_MOOD_NEUTRAL_28;
  }
}

// Mood selection menu callbacks
static uint16_t mood_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return 9;
}

static void mood_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  // Draw mood with icon
  GBitmap *icon = gbitmap_create_with_resource(get_mood_icon_resource((Mood)cell_index->row));
  menu_cell_basic_draw(ctx, cell_layer, MOOD_LABELS[cell_index->row], NULL, icon);
  if (icon) {
    gbitmap_destroy(icon);
  }
}

static void mood_menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  s_selected_mood = (Mood)cell_index->row;
  save_and_close();
}

static void show_mood_selection(void) {
  s_current_stage = STAGE_MOOD_SELECTION;

  // Recreate menu layer for mood selection
  menu_layer_destroy(s_menu_layer);

  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = mood_menu_get_num_rows,
    .draw_row = mood_menu_draw_row,
    .select_click = mood_menu_select
  });

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void save_and_close(void) {
  s_current_stage = STAGE_COMPLETE;

  // Update entry with new values
  Entry updated_entry;
  entry_init(&updated_entry, s_original_entry.date, s_selected_mood, s_selected_canned_flags);

  // Save updated entry
  if (storage_update_entry(s_entry_index, &updated_entry)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "edit_entry_window: entry updated successfully");

    // Recalculate stats after edit
    Stats stats;
    storage_load_stats(&stats);
    storage_save_stats(&stats);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "edit_entry_window: failed to update entry");
  }

  // Close window
  window_stack_pop(true);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Load the entry to edit
  if (!storage_get_entry_by_index(s_entry_index, &s_original_entry)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "edit_entry_window: failed to load entry %d", s_entry_index);
    window_stack_pop(true);
    return;
  }

  // Initialize state with current values
  s_current_stage = STAGE_CANNED_RESPONSE;
  s_selected_canned_flags = s_original_entry.canned_flags;
  s_selected_mood = s_original_entry.mood;

  // Create menu layer for canned responses
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = canned_menu_get_num_rows,
    .draw_row = canned_menu_draw_row,
    .select_click = canned_menu_select
  });

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

void edit_entry_window_push(uint16_t entry_index) {
  s_entry_index = entry_index;

  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void edit_entry_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
