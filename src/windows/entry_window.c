#include "entry_window.h"
#include "../data/entry.h"
#include "../data/storage.h"
#include "../logic/stats.h"
#include "../utils/date_utils.h"
#include <string.h>

// Canned response labels
static const char* CANNED_LABELS[NUM_CANNED_RESPONSES] = {
  "Family",
  "Friends",
  "Health",
  "Work",
  "Nature",
  "Food",
  "Music",
  "Rest",
  "Learning",
  "Pets"
};

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

typedef enum {
  STAGE_INPUT_METHOD,
  STAGE_VOICE_INPUT,
  STAGE_CANNED_RESPONSE,
  STAGE_MOOD_SELECTION,
  STAGE_COMPLETE
} EntryStage;

static Window *s_window;
static MenuLayer *s_menu_layer;
static TextLayer *s_voice_preview_layer;
static DictationSession *s_dictation_session;
static EntryStage s_current_stage;
static uint16_t s_selected_canned_flags;
static Mood s_selected_mood;
static char s_voice_text[MAX_ENTRY_TEXT_LENGTH + 1];

// Forward declarations
static void show_input_method_menu(void);
static void show_voice_input(void);
static void show_canned_response_menu(void);
static void show_mood_selection(void);
static void save_and_close(void);

// Menu callback forward declarations
static uint16_t input_method_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context);
static void input_method_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context);
static void input_method_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context);
static uint16_t canned_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context);
static void canned_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context);
static void canned_menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context);
static uint16_t mood_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context);
static void mood_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context);
static void mood_menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context);

// Voice input callbacks
static void dictation_session_callback(DictationSession *session, DictationSessionStatus status,
                                       char *transcription, void *context) {
  if (status == DictationSessionStatusSuccess && transcription) {
    // Copy transcription (enforce 140 char limit)
    strncpy(s_voice_text, transcription, MAX_ENTRY_TEXT_LENGTH);
    s_voice_text[MAX_ENTRY_TEXT_LENGTH] = '\0';

    APP_LOG(APP_LOG_LEVEL_INFO, "Voice input successful: %s", s_voice_text);

    // Proceed to mood selection
    show_mood_selection();
  } else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Voice input failed, status: %d", (int)status);

    // Fall back to canned responses
    show_canned_response_menu();
  }

  // Clean up session
  dictation_session_destroy(s_dictation_session);
  s_dictation_session = NULL;
}

// Input method menu callbacks
static uint16_t input_method_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return 2;  // Voice Input, Canned Responses
}

static void input_method_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  if (cell_index->row == 0) {
    menu_cell_basic_draw(ctx, cell_layer, "Voice Input", "Speak your entry", NULL);
  } else {
    menu_cell_basic_draw(ctx, cell_layer, "Canned Responses", "Select from list", NULL);
  }
}

static void input_method_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  if (cell_index->row == 0) {
    // Voice input
    show_voice_input();
  } else {
    // Canned responses
    show_canned_response_menu();
  }
}

static void show_input_method_menu(void) {
  s_current_stage = STAGE_INPUT_METHOD;

  if (s_menu_layer) {
    menu_layer_destroy(s_menu_layer);
  }

  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = input_method_get_num_rows,
    .draw_row = input_method_draw_row,
    .select_click = input_method_select
  });

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void show_voice_input(void) {
  s_current_stage = STAGE_VOICE_INPUT;

  // Create dictation session
  s_dictation_session = dictation_session_create(MAX_ENTRY_TEXT_LENGTH,
                                                  dictation_session_callback,
                                                  NULL);

  if (s_dictation_session) {
    // Start voice input
    dictation_session_start(s_dictation_session);
    APP_LOG(APP_LOG_LEVEL_INFO, "Voice input started");
  } else {
    // Voice not available, fall back
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create dictation session");
    show_canned_response_menu();
  }
}

static void show_canned_response_menu(void) {
  s_current_stage = STAGE_CANNED_RESPONSE;
  s_voice_text[0] = '\0';  // Clear voice text

  if (s_menu_layer) {
    menu_layer_destroy(s_menu_layer);
  }

  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = canned_menu_get_num_rows,
    .draw_row = canned_menu_draw_row,
    .select_click = canned_menu_select
  });

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

// Canned response menu callbacks
static uint16_t canned_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  (void)menu_layer;
  (void)section_index;
  (void)context;
  return NUM_CANNED_RESPONSES + 1;  // +1 for "Done" option
}

static void canned_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  (void)context;
  if (cell_index->row == NUM_CANNED_RESPONSES) {
    menu_cell_basic_draw(ctx, cell_layer, "Done", NULL, NULL);
  } else {
    bool is_selected = (s_selected_canned_flags & (1 << cell_index->row)) != 0;
    menu_cell_basic_draw(ctx, cell_layer, CANNED_LABELS[cell_index->row],
                        is_selected ? "✓" : NULL, NULL);
  }
}

static void canned_menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  (void)menu_layer;
  (void)context;
  if (cell_index->row == NUM_CANNED_RESPONSES) {
    // Done button - proceed to mood selection
    if (s_selected_canned_flags == 0) {
      // Nothing selected - default to "grateful"
      s_selected_canned_flags = CANNED_FAMILY;
    }
    show_mood_selection();
  } else {
    // Toggle selection
    uint16_t flag = (1 << cell_index->row);
    s_selected_canned_flags ^= flag;
    menu_layer_reload_data(s_menu_layer);
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

// Storage warning dialog
static Window *s_warning_dialog_window = NULL;
static TextLayer *s_warning_text_layer = NULL;

static void storage_warning_dialog_dismissed(ClickRecognizerRef recognizer, void *context) {
  // Pop the warning dialog
  window_stack_pop(true);
}

static void warning_dialog_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, storage_warning_dialog_dismissed);
  window_single_click_subscribe(BUTTON_ID_SELECT, storage_warning_dialog_dismissed);
  window_single_click_subscribe(BUTTON_ID_DOWN, storage_warning_dialog_dismissed);
  window_single_click_subscribe(BUTTON_ID_BACK, storage_warning_dialog_dismissed);
}

static void show_storage_warning(void) {
  if (!s_warning_dialog_window) {
    s_warning_dialog_window = window_create();
    Layer *window_layer = window_get_root_layer(s_warning_dialog_window);
    GRect bounds = layer_get_bounds(window_layer);

    // Create text layer for warning message
    s_warning_text_layer = text_layer_create(GRect(10, 30, bounds.size.w - 20, bounds.size.h - 60));
    text_layer_set_text(s_warning_text_layer,
      "Storage is 90% full!\n\n"
      "Oldest entries will be deleted automatically.\n\n"
      "Press any button to continue.");
    text_layer_set_text_alignment(s_warning_text_layer, GTextAlignmentCenter);
    text_layer_set_font(s_warning_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(s_warning_text_layer));

    // Set up click handlers to dismiss on any button
    window_set_click_config_provider(s_warning_dialog_window, warning_dialog_click_config_provider);
  }

  window_stack_push(s_warning_dialog_window, true);
}

static void save_and_close(void) {
  s_current_stage = STAGE_COMPLETE;

  // Create entry
  Entry entry;

  // Check if we have voice text or canned responses
  if (s_voice_text[0] != '\0') {
    // Voice input - use voice text directly
    entry.date = date_normalize_to_midnight(date_get_today());
    entry.mood = s_selected_mood;
    entry.canned_flags = 0;  // No canned flags for voice
    strncpy(entry.text, s_voice_text, MAX_ENTRY_TEXT_LENGTH);
    entry.text[MAX_ENTRY_TEXT_LENGTH] = '\0';
  } else {
    // Canned responses - use traditional method
    entry_init(&entry, date_get_today(), s_selected_mood, s_selected_canned_flags);
  }

  // Check storage capacity before saving
  bool was_near_capacity = storage_is_near_capacity();

  // Save entry
  if (storage_save_entry(&entry)) {
    // Update stats
    stats_update_after_entry(&entry);

    // Show warning if storage is getting full
    if (was_near_capacity || storage_is_near_capacity()) {
      show_storage_warning();
    }
  }

  // Close window
  window_stack_pop(true);
}

static void window_load(Window *window) {
  // Initialize state
  s_current_stage = STAGE_INPUT_METHOD;
  s_selected_canned_flags = 0;
  s_selected_mood = MOOD_NEUTRAL;
  s_voice_text[0] = '\0';
  s_dictation_session = NULL;

  // Show input method selection
  show_input_method_menu();
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

void entry_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void entry_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
