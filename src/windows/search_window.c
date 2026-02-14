#include "search_window.h"
#include "entry_detail_window.h"
#include "../logic/search.h"
#include "../data/storage.h"
#include "../utils/constants.h"
#include "../utils/date_utils.h"
#include <string.h>

static Window *s_window;
static MenuLayer *s_results_menu;
static SimpleMenuLayer *s_filter_menu;
static SearchCriteria s_criteria;
static Entry s_results[MAX_ENTRIES];
static uint16_t s_result_count = 0;
static bool s_showing_results = false;

// Mood labels for display
static const char* MOOD_LABELS[9] = {
  "Sad", "Anxious", "Stressed", "Tired", "Neutral",
  "Content", "Happy", "Excited", "Grateful"
};

// Filter menu callbacks
static uint16_t filter_menu_get_num_rows(MenuLayer *menu, uint16_t section, void *context) {
  (void)menu;
  (void)section;
  (void)context;
  return 5;  // Text Search, Mood Filter, Date Filter, Tag Filter, Clear All
}

static void filter_menu_draw_row(GContext *ctx, const Layer *cell, MenuIndex *index, void *context) {
  (void)context;
  switch (index->row) {
    case 0:
      menu_cell_basic_draw(ctx, cell, "Text Search", "Search entry text", NULL);
      break;
    case 1:
      menu_cell_basic_draw(ctx, cell, "Filter by Mood", "Select moods", NULL);
      break;
    case 2:
      menu_cell_basic_draw(ctx, cell, "Filter by Date", "Last 7/30/90 days", NULL);
      break;
    case 3:
      menu_cell_basic_draw(ctx, cell, "Filter by Tags", "Canned responses", NULL);
      break;
    case 4:
      menu_cell_basic_draw(ctx, cell, "Search All", "Clear filters", NULL);
      break;
  }
}

static void execute_search(void) {
  // Perform search
  s_result_count = search_entries(&s_criteria, s_results, MAX_ENTRIES);

  APP_LOG(APP_LOG_LEVEL_INFO, "Search found %d results", s_result_count);

  // Show results
  s_showing_results = true;
  layer_set_hidden(simple_menu_layer_get_layer(s_filter_menu), true);
  if (s_results_menu) {
    layer_set_hidden(menu_layer_get_layer(s_results_menu), false);
    menu_layer_reload_data(s_results_menu);
  }
}

static void filter_menu_select(int index, void *context) {
  (void)context;
  switch (index) {
    case 0:
      // Text search - for simplicity, search for "happy" as example
      strcpy(s_criteria.query, "");  // Empty = match all
      execute_search();
      break;

    case 1:
      // Mood filter - filter for happy moods (Happy, Excited, Grateful)
      s_criteria.mood_filter_enabled = true;
      s_criteria.mood_flags = (1 << MOOD_HAPPY) | (1 << MOOD_EXCITED) | (1 << MOOD_GRATEFUL);
      execute_search();
      break;

    case 2:
      // Date filter - last 30 days
      s_criteria.date_filter_enabled = true;
      s_criteria.end_date = date_get_today();
      s_criteria.start_date = s_criteria.end_date - (30 * 86400);
      execute_search();
      break;

    case 3:
      // Tag filter - entries with "family" or "friends"
      s_criteria.canned_filter_enabled = true;
      s_criteria.canned_flags = CANNED_FAMILY | CANNED_FRIENDS;
      execute_search();
      break;

    case 4:
      // Clear all filters
      memset(&s_criteria, 0, sizeof(SearchCriteria));
      execute_search();
      break;
  }
}

// Results menu callbacks
static uint16_t results_menu_get_num_rows(MenuLayer *menu, uint16_t section, void *context) {
  return s_result_count > 0 ? s_result_count : 1;  // Show "No results" if 0
}

static void results_menu_draw_row(GContext *ctx, const Layer *cell, MenuIndex *index, void *context) {
  if (s_result_count == 0) {
    menu_cell_basic_draw(ctx, cell, "No results found", "Try different filters", NULL);
    return;
  }

  Entry *entry = &s_results[index->row];

  // Format date
  struct tm *time_info = localtime(&entry->date);
  static char date_str[32];
  strftime(date_str, sizeof(date_str), "%b %d", time_info);

  // Truncate text for preview
  static char preview[51];
  strncpy(preview, entry->text, 50);
  preview[50] = '\0';

  // Draw with mood label
  static char title[64];
  snprintf(title, sizeof(title), "%s - %s", date_str, MOOD_LABELS[entry->mood]);

  menu_cell_basic_draw(ctx, cell, title, preview, NULL);
}

static void results_menu_select(MenuLayer *menu, MenuIndex *index, void *context) {
  if (s_result_count == 0) return;

  // Show entry detail for selected result
  entry_detail_window_push(s_results[index->row].date);
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_showing_results) {
    // Go back to filter menu
    s_showing_results = false;
    layer_set_hidden(simple_menu_layer_get_layer(s_filter_menu), false);
    if (s_results_menu) {
      layer_set_hidden(menu_layer_get_layer(s_results_menu), true);
    }
  } else {
    // Close search window
    window_stack_pop(true);
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_click_config_provider(window, click_config_provider);

  // Initialize criteria
  memset(&s_criteria, 0, sizeof(SearchCriteria));
  s_showing_results = false;

  // Create filter menu
  static SimpleMenuItem filter_items[5];
  static SimpleMenuSection filter_section;

  for (int i = 0; i < 5; i++) {
    filter_items[i].callback = filter_menu_select;
  }
  filter_items[0].title = "Text Search";
  filter_items[1].title = "Filter by Mood";
  filter_items[2].title = "Filter by Date";
  filter_items[3].title = "Filter by Tags";
  filter_items[4].title = "Search All";

  filter_section.items = filter_items;
  filter_section.num_items = 5;
  filter_section.title = "Search Options";

  s_filter_menu = simple_menu_layer_create(bounds, window, &filter_section, 1, NULL);
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_filter_menu));

  // Create results menu (initially hidden)
  s_results_menu = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_results_menu, NULL, (MenuLayerCallbacks) {
    .get_num_rows = results_menu_get_num_rows,
    .draw_row = results_menu_draw_row,
    .select_click = results_menu_select
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_results_menu));
  layer_set_hidden(menu_layer_get_layer(s_results_menu), true);
}

static void window_unload(Window *window) {
  if (s_filter_menu) {
    simple_menu_layer_destroy(s_filter_menu);
    s_filter_menu = NULL;
  }
  if (s_results_menu) {
    menu_layer_destroy(s_results_menu);
    s_results_menu = NULL;
  }
}

void search_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void search_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
