#include "calendar_window.h"
#include "entry_detail_window.h"
#include "../data/entry.h"
#include "../data/storage.h"
#include "../utils/date_utils.h"
#include <string.h>
#include <stdlib.h>

typedef enum {
  VIEW_MONTH_LIST,
  VIEW_DAY_LIST,
} CalendarView;

static Window *s_window;
static MenuLayer *s_menu_layer;
static CalendarView s_current_view;

// Month list state — first slot is "Today", rest are months with entries
#define MAX_MONTHS 13
static time_t s_month_list[MAX_MONTHS];
static uint16_t s_month_entry_counts[MAX_MONTHS];
static uint16_t s_month_count;

// Day list state — only days with entries (+ today if in this month)
#define MAX_DAY_ROWS 32
static time_t s_day_dates[MAX_DAY_ROWS];
static uint16_t s_day_entry_counts[MAX_DAY_ROWS];
static uint16_t s_day_row_count;
static time_t s_selected_month;

// Forward declarations
static void show_month_list(void);
static void show_day_list(time_t month);
static void calendar_click_config_provider(void *context);

// Helper functions
static void format_month_year(time_t date, char *buffer, size_t size) {
  struct tm *time_info = localtime(&date);
  snprintf(buffer, size, "%s %d",
           date_get_month_name(time_info->tm_mon),
           time_info->tm_year + 1900);
}

// Month list callbacks
static uint16_t month_list_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return s_month_count;
}

static void month_list_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  uint16_t row = cell_index->row;
  if (row == 0) {
    menu_cell_basic_draw(ctx, cell_layer, "Today", "View today's entries", NULL);
  } else {
    char month_text[32];
    format_month_year(s_month_list[row], month_text, sizeof(month_text));

    char subtitle[16];
    snprintf(subtitle, sizeof(subtitle), "%d entries", s_month_entry_counts[row]);
    menu_cell_basic_draw(ctx, cell_layer, month_text, subtitle, NULL);
  }
}

static void month_list_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  if (cell_index->row == 0) {
    entry_detail_window_push(date_get_today());
  } else {
    s_selected_month = s_month_list[cell_index->row];
    show_day_list(s_selected_month);
  }
}

// Day list callbacks
static uint16_t day_list_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return s_day_row_count > 0 ? s_day_row_count : 1;
}

static void day_list_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  if (s_day_row_count == 0) {
    menu_cell_basic_draw(ctx, cell_layer, "No entries", "for this month", NULL);
    return;
  }

  uint16_t row = cell_index->row;
  if (row >= s_day_row_count) return;

  time_t day_date = s_day_dates[row];
  struct tm *ti = localtime(&day_date);
  bool is_today = date_is_same_day(day_date, date_get_today());

  char day_text[32];
  snprintf(day_text, sizeof(day_text), "%s %d", date_get_month_name(ti->tm_mon), ti->tm_mday);

  char subtitle[32];
  if (is_today && s_day_entry_counts[row] > 0) {
    snprintf(subtitle, sizeof(subtitle), "Today - %d entries", s_day_entry_counts[row]);
  } else if (is_today) {
    snprintf(subtitle, sizeof(subtitle), "Today");
  } else {
    snprintf(subtitle, sizeof(subtitle), "%d entries", s_day_entry_counts[row]);
  }

  menu_cell_basic_draw(ctx, cell_layer, day_text, subtitle, NULL);
}

static void day_list_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  if (s_day_row_count == 0) return;
  uint16_t row = cell_index->row;
  if (row >= s_day_row_count) return;
  entry_detail_window_push(s_day_dates[row]);
}

// Build month list: "Today" + only months that have entries
static void show_month_list(void) {
  s_current_view = VIEW_MONTH_LIST;
  s_month_count = 0;

  time_t today = date_get_today();

  // First row is always "Today"
  s_month_list[s_month_count] = today;
  s_month_entry_counts[s_month_count] = 0;
  s_month_count++;

  // Scan all entries to find unique months with entries
  uint16_t total_count = storage_get_entry_count();

  // Temporary: collect unique month starts and counts
  time_t unique_months[MAX_MONTHS];
  uint16_t unique_counts[MAX_MONTHS];
  uint16_t num_unique = 0;

  for (uint16_t i = 0; i < total_count; i++) {
    Entry entry;
    if (!storage_get_entry_by_index(i, &entry)) continue;

    time_t month_start = date_get_first_of_month(entry.date);

    // Check if already in list
    bool found = false;
    for (uint16_t m = 0; m < num_unique; m++) {
      if (unique_months[m] == month_start) {
        unique_counts[m]++;
        found = true;
        break;
      }
    }
    if (!found && num_unique < MAX_MONTHS - 1) {
      unique_months[num_unique] = month_start;
      unique_counts[num_unique] = 1;
      num_unique++;
    }
  }

  // Sort months newest first (simple bubble sort)
  for (uint16_t i = 0; i < num_unique && i + 1 < num_unique; i++) {
    for (uint16_t j = 0; j < num_unique - i - 1; j++) {
      if (unique_months[j] < unique_months[j + 1]) {
        time_t tmp_m = unique_months[j];
        unique_months[j] = unique_months[j + 1];
        unique_months[j + 1] = tmp_m;
        uint16_t tmp_c = unique_counts[j];
        unique_counts[j] = unique_counts[j + 1];
        unique_counts[j + 1] = tmp_c;
      }
    }
  }

  // Add to month list
  for (uint16_t i = 0; i < num_unique && s_month_count < MAX_MONTHS; i++) {
    s_month_list[s_month_count] = unique_months[i];
    s_month_entry_counts[s_month_count] = unique_counts[i];
    s_month_count++;
  }

  // Recreate menu
  if (s_menu_layer) {
    layer_remove_from_parent(menu_layer_get_layer(s_menu_layer));
    menu_layer_destroy(s_menu_layer);
  }

  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = month_list_get_num_rows,
    .draw_row = month_list_draw_row,
    .select_click = month_list_select
  });

  window_set_click_config_provider(s_window, calendar_click_config_provider);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

// Build day list: only days with entries in the month (+ today if in this month)
static void show_day_list(time_t month) {
  s_current_view = VIEW_DAY_LIST;
  s_selected_month = date_get_first_of_month(month);
  s_day_row_count = 0;

  time_t today = date_get_today();
  time_t month_end = date_add_days(s_selected_month, date_get_days_in_month(s_selected_month));
  bool today_in_month = (today >= s_selected_month && today < month_end);
  bool today_added = false;

  // Scan entries for this month, collecting unique days
  uint16_t total_count = storage_get_entry_count();

  for (uint16_t i = 0; i < total_count; i++) {
    Entry entry;
    if (!storage_get_entry_by_index(i, &entry)) continue;
    if (entry.date < s_selected_month || entry.date >= month_end) continue;

    // Check if day already in list
    bool found = false;
    for (uint16_t d = 0; d < s_day_row_count; d++) {
      if (date_is_same_day(s_day_dates[d], entry.date)) {
        s_day_entry_counts[d]++;
        found = true;
        break;
      }
    }
    if (!found && s_day_row_count < MAX_DAY_ROWS) {
      s_day_dates[s_day_row_count] = date_normalize_to_midnight(entry.date);
      s_day_entry_counts[s_day_row_count] = 1;
      if (date_is_same_day(entry.date, today)) today_added = true;
      s_day_row_count++;
    }
  }

  // Add today if it's in this month and not already listed
  if (today_in_month && !today_added && s_day_row_count < MAX_DAY_ROWS) {
    s_day_dates[s_day_row_count] = today;
    s_day_entry_counts[s_day_row_count] = 0;
    s_day_row_count++;
  }

  // Sort days newest first
  for (uint16_t i = 0; i + 1 < s_day_row_count; i++) {
    for (uint16_t j = 0; j < s_day_row_count - i - 1; j++) {
      if (s_day_dates[j] < s_day_dates[j + 1]) {
        time_t tmp_d = s_day_dates[j];
        s_day_dates[j] = s_day_dates[j + 1];
        s_day_dates[j + 1] = tmp_d;
        uint16_t tmp_c = s_day_entry_counts[j];
        s_day_entry_counts[j] = s_day_entry_counts[j + 1];
        s_day_entry_counts[j + 1] = tmp_c;
      }
    }
  }

  // Recreate menu
  if (s_menu_layer) {
    layer_remove_from_parent(menu_layer_get_layer(s_menu_layer));
    menu_layer_destroy(s_menu_layer);
  }

  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = day_list_get_num_rows,
    .draw_row = day_list_draw_row,
    .select_click = day_list_select
  });

  window_set_click_config_provider(s_window, calendar_click_config_provider);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

// Unified click handlers for both month list and day list views
static void calendar_back_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_current_view == VIEW_DAY_LIST) {
    show_month_list();
  } else {
    window_stack_pop(true);
  }
}

static void calendar_up_handler(ClickRecognizerRef recognizer, void *context) {
  menu_layer_set_selected_next(s_menu_layer, true, MenuRowAlignCenter, true);
}

static void calendar_down_handler(ClickRecognizerRef recognizer, void *context) {
  menu_layer_set_selected_next(s_menu_layer, false, MenuRowAlignCenter, true);
}

static void calendar_select_handler(ClickRecognizerRef recognizer, void *context) {
  MenuIndex idx = menu_layer_get_selected_index(s_menu_layer);
  if (s_current_view == VIEW_MONTH_LIST) {
    month_list_select(s_menu_layer, &idx, NULL);
  } else {
    day_list_select(s_menu_layer, &idx, NULL);
  }
}

static void calendar_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, calendar_back_handler);
  window_single_click_subscribe(BUTTON_ID_UP, calendar_up_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, calendar_down_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, calendar_select_handler);
}

static void window_appear(Window *window) {
  // Refresh current view on re-appearance (after edit/delete)
  if (s_current_view == VIEW_DAY_LIST) {
    show_day_list(s_selected_month);
  } else {
    show_month_list();
  }
}

static void window_load(Window *window) {
  show_month_list();
}

static void window_unload(Window *window) {
  if (s_menu_layer) {
    menu_layer_destroy(s_menu_layer);
    s_menu_layer = NULL;
  }
}

void calendar_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .appear = window_appear,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void calendar_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
