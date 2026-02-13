#include "calendar_window.h"
#include "entry_detail_window.h"
#include "../data/entry.h"
#include "../data/storage.h"
#include "../utils/date_utils.h"
#include <string.h>

#define CELL_SIZE 20
#define GRID_START_Y 30
#define GRID_START_X 4

// Mood icon resource IDs (used on all platforms)
static uint32_t get_mood_icon_resource(Mood mood) {
  switch (mood) {
    case MOOD_SAD: return RESOURCE_ID_MOOD_SAD_14;
    case MOOD_ANXIOUS: return RESOURCE_ID_MOOD_ANXIOUS_14;
    case MOOD_STRESSED: return RESOURCE_ID_MOOD_STRESSED_14;
    case MOOD_TIRED: return RESOURCE_ID_MOOD_TIRED_14;
    case MOOD_NEUTRAL: return RESOURCE_ID_MOOD_NEUTRAL_14;
    case MOOD_CONTENT: return RESOURCE_ID_MOOD_CONTENT_14;
    case MOOD_HAPPY: return RESOURCE_ID_MOOD_HAPPY_14;
    case MOOD_EXCITED: return RESOURCE_ID_MOOD_EXCITED_14;
    case MOOD_GRATEFUL: return RESOURCE_ID_MOOD_GRATEFUL_14;
    default: return RESOURCE_ID_MOOD_NEUTRAL_14;
  }
}

typedef struct {
  time_t month_start;
  Mood day_moods[42];  // Max 6 weeks × 7 days
  bool has_entry[42];
} CalendarCache;

static Window *s_window;
static Layer *s_calendar_layer;
static TextLayer *s_header_layer;
static CalendarCache s_cache;
static char s_header_text[32];

static void load_month_data(time_t month_date) {
  memset(&s_cache, 0, sizeof(CalendarCache));
  s_cache.month_start = date_get_first_of_month(month_date);

  // Get first day of week for the month
  uint8_t first_dow = date_get_day_of_week(s_cache.month_start);
  uint8_t days_in_month = date_get_days_in_month(s_cache.month_start);

  // Load entries for the month
  Entry entries[MAX_ENTRIES];
  uint16_t entry_count = storage_get_all_entries(entries, MAX_ENTRIES);

  // Map entries to calendar grid
  for (uint8_t day = 1; day <= days_in_month; day++) {
    time_t day_date = date_add_days(s_cache.month_start, day - 1);
    uint8_t grid_index = first_dow + day - 1;

    // Find entry for this day (use latest entry if multiple)
    for (uint16_t i = 0; i < entry_count; i++) {
      if (date_is_same_day(entries[i].date, day_date)) {
        s_cache.has_entry[grid_index] = true;
        s_cache.day_moods[grid_index] = entries[i].mood;
        break;
      }
    }
  }

  // Update header text
  struct tm *time_info = localtime(&s_cache.month_start);
  snprintf(s_header_text, sizeof(s_header_text), "%s %d",
           date_get_month_name(time_info->tm_mon), time_info->tm_year + 1900);
}

static void calendar_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw day headers
  graphics_context_set_text_color(ctx, GColorBlack);
  for (uint8_t dow = 0; dow < 7; dow++) {
    int x = GRID_START_X + (dow * CELL_SIZE);
    graphics_draw_text(ctx, date_get_day_name_short(dow),
                      fonts_get_system_font(FONT_KEY_GOTHIC_14),
                      GRect(x, GRID_START_Y - 20, CELL_SIZE, 20),
                      GTextOverflowModeTrailingEllipsis,
                      GTextAlignmentCenter, NULL);
  }

  // Draw calendar grid
  uint8_t first_dow = date_get_day_of_week(s_cache.month_start);
  uint8_t days_in_month = date_get_days_in_month(s_cache.month_start);
  time_t today = date_get_today();

  for (uint8_t day = 1; day <= days_in_month; day++) {
    uint8_t grid_index = first_dow + day - 1;
    uint8_t row = grid_index / 7;
    uint8_t col = grid_index % 7;

    int x = GRID_START_X + (col * CELL_SIZE);
    int y = GRID_START_Y + (row * CELL_SIZE);

    time_t day_date = date_add_days(s_cache.month_start, day - 1);
    bool is_today = date_is_same_day(day_date, today);

    // Draw cell background
    if (is_today) {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_rect(ctx, GRect(x, y, CELL_SIZE, CELL_SIZE), 2, GCornersAll);
    }

    // Draw day number
    char day_str[3];
    snprintf(day_str, sizeof(day_str), "%d", day);
    graphics_context_set_text_color(ctx, is_today ? GColorWhite : GColorBlack);
    graphics_draw_text(ctx, day_str,
                      fonts_get_system_font(FONT_KEY_GOTHIC_14),
                      GRect(x, y, CELL_SIZE, CELL_SIZE),
                      GTextOverflowModeTrailingEllipsis,
                      GTextAlignmentCenter, NULL);

    // Draw mood indicator using emoji icons
    if (s_cache.has_entry[grid_index]) {
      GBitmap *icon = gbitmap_create_with_resource(get_mood_icon_resource(s_cache.day_moods[grid_index]));
      if (icon) {
        // Draw icon in bottom-right corner of cell
        GRect icon_rect = GRect(x + CELL_SIZE - 8, y + CELL_SIZE - 8, 6, 6);
        graphics_draw_bitmap_in_rect(ctx, icon, icon_rect);
        gbitmap_destroy(icon);
      }
    }
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Show entry details for today (current selected day)
  // In a more advanced version, this would track the selected cell
  // For now, show today's entries
  time_t today = date_get_today();
  entry_detail_window_push(today);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Previous month
  time_t prev = date_prev_month(s_cache.month_start);
  load_month_data(prev);
  text_layer_set_text(s_header_layer, s_header_text);
  layer_mark_dirty(s_calendar_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Next month
  time_t next = date_next_month(s_cache.month_start);
  load_month_data(next);
  text_layer_set_text(s_header_layer, s_header_text);
  layer_mark_dirty(s_calendar_layer);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Load current month data
  load_month_data(date_get_today());

  // Create header
  s_header_layer = text_layer_create(GRect(0, 0, bounds.size.w, 25));
  text_layer_set_text(s_header_layer, s_header_text);
  text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

  // Create calendar layer
  s_calendar_layer = layer_create(bounds);
  layer_set_update_proc(s_calendar_layer, calendar_layer_update_proc);
  layer_add_child(window_layer, s_calendar_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_header_layer);
  layer_destroy(s_calendar_layer);
}

void calendar_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
    window_set_click_config_provider(s_window, click_config_provider);
  }
  window_stack_push(s_window, true);
}

void calendar_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
