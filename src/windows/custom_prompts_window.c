#include "custom_prompts_window.h"
#include "../logic/prompts.h"
#include <string.h>

static Window *s_window;
static MenuLayer *s_menu_layer;
static DictationSession *s_dictation_session;

// Dictation callback for adding custom prompts via voice
static void dictation_callback(DictationSession *session, DictationSessionStatus status,
                                char *transcription, void *context) {
  if (status == DictationSessionStatusSuccess && transcription) {
    // Truncate to max custom prompt length
    char prompt[CUSTOM_PROMPT_MAX_LENGTH + 1];
    strncpy(prompt, transcription, CUSTOM_PROMPT_MAX_LENGTH);
    prompt[CUSTOM_PROMPT_MAX_LENGTH] = '\0';

    if (prompts_add_custom(prompt)) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Custom prompt added via voice: %s", prompt);
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to add custom prompt");
    }

    // Reload menu to show new prompt
    if (s_menu_layer) {
      menu_layer_reload_data(s_menu_layer);
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Dictation failed for custom prompt, status: %d", (int)status);
  }

  // Cleanup
  dictation_session_destroy(s_dictation_session);
  s_dictation_session = NULL;
}

// Menu callbacks
static uint16_t menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return prompts_get_custom_count() + 1;  // +1 for "Add New" option
}

static void menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  uint8_t custom_count = prompts_get_custom_count();

  if (cell_index->row == custom_count) {
    // "Add New" option
    menu_cell_basic_draw(ctx, cell_layer, "+ Add Custom Prompt", NULL, NULL);
  } else {
    // Custom prompt
    char prompt_text[CUSTOM_PROMPT_MAX_LENGTH + 1];
    if (prompts_get_custom(cell_index->row, prompt_text, sizeof(prompt_text))) {
      menu_cell_basic_draw(ctx, cell_layer, prompt_text, NULL, NULL);
    } else {
      menu_cell_basic_draw(ctx, cell_layer, "(Error loading prompt)", NULL, NULL);
    }
  }
}

static void menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  uint8_t custom_count = prompts_get_custom_count();

  if (cell_index->row == custom_count) {
    // Add new prompt via voice dictation
    if (custom_count < MAX_CUSTOM_PROMPTS) {
      s_dictation_session = dictation_session_create(CUSTOM_PROMPT_MAX_LENGTH,
                                                      dictation_callback, NULL);
      if (s_dictation_session) {
        dictation_session_start(s_dictation_session);
        APP_LOG(APP_LOG_LEVEL_INFO, "Dictation started for custom prompt");
      } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create dictation session for custom prompt");
      }
    }
  }
}

static void menu_long_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  uint8_t custom_count = prompts_get_custom_count();

  if (cell_index->row < custom_count) {
    // Delete prompt
    prompts_delete_custom(cell_index->row);
    menu_layer_reload_data(s_menu_layer);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create menu layer
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = menu_get_num_rows,
    .draw_row = menu_draw_row,
    .select_click = menu_select,
    .select_long_click = menu_long_select
  });

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  if (s_dictation_session) {
    dictation_session_destroy(s_dictation_session);
    s_dictation_session = NULL;
  }
  menu_layer_destroy(s_menu_layer);
}

void custom_prompts_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void custom_prompts_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
