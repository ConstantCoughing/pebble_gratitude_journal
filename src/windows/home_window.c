#include "home_window.h"
#include "entry_window.h"
#include "calendar_window.h"
#include "visualization_window.h"
#include "settings_window.h"
#include "../logic/prompts.h"
#include "../logic/stats.h"
#include <string.h>

// Debug flag (set to 0 for release)
#define DEBUG_LOGGING 0

static Window *s_window;
static TextLayer *s_prompt_layer;
static SimpleMenuLayer *s_menu_layer;
static SimpleMenuSection s_menu_section;
static SimpleMenuItem s_menu_items[4];
static char s_streak_text[32];
static char s_prompt_buffer[CUSTOM_PROMPT_MAX_LENGTH + 1];

static void menu_select_callback(int index, void *context) {
  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
  APP_LOG(APP_LOG_LEVEL_INFO, "!!! MENU CALLBACK FIRED !!! index=%d", index);
  #endif  // DEBUG

  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE THIS BLOCK
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: menu item %d selected", index);
  #endif  // DEBUG

  switch (index) {
    case 0:
      // Add Entry
      #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
      APP_LOG(APP_LOG_LEVEL_INFO, "home_window: launching entry_window");
      #endif  // DEBUG
      entry_window_push();
      break;
    case 1:
      // View Calendar
      #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
      APP_LOG(APP_LOG_LEVEL_INFO, "home_window: launching calendar_window");
      #endif  // DEBUG
      calendar_window_push();
      break;
    case 2:
      // Visualizations
      #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
      APP_LOG(APP_LOG_LEVEL_INFO, "home_window: launching visualization_window");
      #endif  // DEBUG
      visualization_window_push();
      break;
    case 3:
      // Settings
      #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
      APP_LOG(APP_LOG_LEVEL_INFO, "home_window: launching settings_window");
      #endif  // DEBUG
      settings_window_push();
      break;
  }
}

static void window_load(Window *window) {
  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE THIS BLOCK
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: window_load called");
  #endif  // DEBUG

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: bounds w=%d h=%d", bounds.size.w, bounds.size.h);
  #endif  // DEBUG

#ifdef PBL_ROUND
  int padding = 20;
  int prompt_height = 70;
#else
  int padding = 4;
  int prompt_height = 60;
#endif

  // Create prompt text layer
  s_prompt_layer = text_layer_create(GRect(padding, 5, bounds.size.w - (padding * 2), prompt_height));
  strncpy(s_prompt_buffer, prompts_get_daily(), sizeof(s_prompt_buffer) - 1);
  s_prompt_buffer[sizeof(s_prompt_buffer) - 1] = '\0';
  text_layer_set_text(s_prompt_layer, s_prompt_buffer);
  text_layer_set_font(s_prompt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_prompt_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_prompt_layer, GTextOverflowModeTrailingEllipsis);
  layer_add_child(window_layer, text_layer_get_layer(s_prompt_layer));

  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: prompt layer created");
  #endif  // DEBUG

  // Setup menu items
  s_menu_items[0] = (SimpleMenuItem) {
    .title = "Add Entry",
    .callback = menu_select_callback
  };

  s_menu_items[1] = (SimpleMenuItem) {
    .title = "View Calendar",
    .callback = menu_select_callback
  };

  s_menu_items[2] = (SimpleMenuItem) {
    .title = "Visualizations",
    .callback = menu_select_callback
  };

  s_menu_items[3] = (SimpleMenuItem) {
    .title = "Settings",
    .callback = menu_select_callback
  };

  s_menu_section = (SimpleMenuSection) {
    .items = s_menu_items,
    .num_items = 4
  };

  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: menu items configured (4 items)");
  #endif  // DEBUG

  // Create menu layer
  int menu_y = prompt_height + 10;
  s_menu_layer = simple_menu_layer_create(
    GRect(0, menu_y, bounds.size.w, bounds.size.h - menu_y - 20),
    s_window,  // Use s_window instead of window parameter
    &s_menu_section,
    1,
    NULL
  );

  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: simple_menu_layer_create returned: %p", s_menu_layer);
  if (s_menu_layer == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "home_window: FAILED TO CREATE MENU LAYER!");
  }
  #endif  // DEBUG

  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE THIS BLOCK
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: menu layer created at y=%d, height=%d",
          menu_y, bounds.size.h - menu_y - 20);
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: menu_layer pointer = %p", s_menu_layer);
  #endif  // DEBUG

  // Create streak status bar BEFORE adding menu (so menu is on top)
  uint16_t streak = stats_calculate_streak();
  snprintf(s_streak_text, sizeof(s_streak_text), "\u2022 %d Day Streak", streak);

  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: streak = %d days", streak);
  #endif  // DEBUG

  // Status bar layer (at bottom)
  TextLayer *status_layer = text_layer_create(
    GRect(0, bounds.size.h - 20, bounds.size.w, 20)
  );
  text_layer_set_text(status_layer, s_streak_text);
  text_layer_set_font(status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(status_layer, GTextAlignmentCenter);
  text_layer_set_background_color(status_layer, GColorBlack);
  text_layer_set_text_color(status_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(status_layer));

  // Add menu layer LAST so it's on top and receives clicks
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));

  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: menu layer added to window (on top)");
  #endif  // DEBUG
}

static void window_appear(Window *window) {
  // Refresh prompt in case settings changed (mode toggle, custom prompts added)
  strncpy(s_prompt_buffer, prompts_get_daily(), sizeof(s_prompt_buffer) - 1);
  s_prompt_buffer[sizeof(s_prompt_buffer) - 1] = '\0';
  text_layer_set_text(s_prompt_layer, s_prompt_buffer);
}

static void window_unload(Window *window) {
  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: window_unload called");
  #endif  // DEBUG

  text_layer_destroy(s_prompt_layer);
  simple_menu_layer_destroy(s_menu_layer);
}

void home_window_push(void) {
  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: home_window_push called");
  #endif  // DEBUG

  if (!s_window) {
    #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
    APP_LOG(APP_LOG_LEVEL_INFO, "home_window: creating new window");
    #endif  // DEBUG

    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .appear = window_appear,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);

  #ifdef DEBUG_LOGGING  // DEBUG: REMOVE
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: window pushed to stack");
  #endif  // DEBUG
}

void home_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
