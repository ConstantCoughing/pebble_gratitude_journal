#include "home_window.h"
#include "entry_window.h"
#include "calendar_window.h"
#include "visualization_window.h"
#include "settings_window.h"
#include "../logic/prompts.h"
#include "../logic/stats.h"

static Window *s_window;
static TextLayer *s_prompt_layer;
static SimpleMenuLayer *s_menu_layer;
static SimpleMenuSection s_menu_section;
static SimpleMenuItem s_menu_items[4];
static char s_streak_text[32];

static void menu_select_callback(int index, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: menu item %d selected", index);
  switch (index) {
    case 0:
      // Add Entry
      entry_window_push();
      break;
    case 1:
      // View Calendar
      calendar_window_push();
      break;
    case 2:
      // Visualizations
      visualization_window_push();
      break;
    case 3:
      // Settings
      settings_window_push();
      break;
  }
}

static void window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: window_load called");
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

#ifdef PBL_ROUND
  int padding = 20;
  int prompt_height = 70;
#else
  int padding = 4;
  int prompt_height = 60;
#endif

  // Create prompt text layer
  s_prompt_layer = text_layer_create(GRect(padding, 5, bounds.size.w - (padding * 2), prompt_height));
  text_layer_set_text(s_prompt_layer, prompts_get_daily());
  text_layer_set_font(s_prompt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_prompt_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_prompt_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_prompt_layer));

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

  // Create menu layer
  int menu_y = prompt_height + 10;
  s_menu_layer = simple_menu_layer_create(
    GRect(0, menu_y, bounds.size.w, bounds.size.h - menu_y - 20),
    window,
    &s_menu_section,
    1,
    NULL
  );

  APP_LOG(APP_LOG_LEVEL_INFO, "home_window: menu layer created at y=%d", menu_y);

  layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));

  // Create streak status bar
  uint16_t streak = stats_calculate_streak();
  snprintf(s_streak_text, sizeof(s_streak_text), "🔥 %d Day Streak", streak);

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
}

static void window_unload(Window *window) {
  text_layer_destroy(s_prompt_layer);
  simple_menu_layer_destroy(s_menu_layer);
}

void home_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void home_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
