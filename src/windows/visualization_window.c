#include "visualization_window.h"
#include "../data/storage.h"
#include "../utils/constants.h"
#include "../utils/date_utils.h"
#include <string.h>

typedef enum {
  VIZ_BAR_CHART,
  VIZ_MOOD_TREND,
  VIZ_MOOD_DISTRIBUTION
} VisualizationType;

static Window *s_window;
static Layer *s_viz_layer;
static MenuLayer *s_menu_layer;
static VisualizationType s_current_viz;
static bool s_showing_menu = true;

// Mood labels
static const char* MOOD_LABELS[9] = {
  "Sad", "Anxious", "Stressed", "Tired", "Neutral",
  "Content", "Happy", "Excited", "Grateful"
};

// Draw bar chart: entries per week (last 4 weeks)
static void draw_bar_chart(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Load all entries
  Entry entries[MAX_ENTRIES];
  uint16_t count = storage_get_all_entries(entries, MAX_ENTRIES);

  // Count entries per week for last 4 weeks
  time_t now = time(NULL);
  uint16_t week_counts[4] = {0, 0, 0, 0};

  for (uint16_t i = 0; i < count; i++) {
    int days_ago = (now - entries[i].date) / 86400;
    if (days_ago < 7) week_counts[0]++;
    else if (days_ago < 14) week_counts[1]++;
    else if (days_ago < 21) week_counts[2]++;
    else if (days_ago < 28) week_counts[3]++;
  }

  // Draw title
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, "Entries per Week",
                    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                    GRect(0, 5, bounds.size.w, 20),
                    GTextOverflowModeTrailingEllipsis,
                    GTextAlignmentCenter, NULL);

  // Find max for scaling
  uint16_t max_count = 0;
  for (int i = 0; i < 4; i++) {
    if (week_counts[i] > max_count) max_count = week_counts[i];
  }
  if (max_count == 0) max_count = 1; // Avoid divide by zero

  // Draw bars
  int bar_width = 30;
  int bar_spacing = 8;
  int start_x = (bounds.size.w - (4 * bar_width + 3 * bar_spacing)) / 2;
  int max_height = bounds.size.h - 60;
  int base_y = bounds.size.h - 15;

  for (int i = 0; i < 4; i++) {
    int x = start_x + i * (bar_width + bar_spacing);
    int height = (week_counts[3-i] * max_height) / max_count;

    // Draw bar
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(x, base_y - height, bar_width, height), 0, GCornerNone);

    // Draw count
    char count_str[8];
    snprintf(count_str, sizeof(count_str), "%d", week_counts[3-i]);
    graphics_draw_text(ctx, count_str,
                      fonts_get_system_font(FONT_KEY_GOTHIC_14),
                      GRect(x, base_y - height - 18, bar_width, 18),
                      GTextOverflowModeTrailingEllipsis,
                      GTextAlignmentCenter, NULL);

    // Draw label
    char label[8];
    snprintf(label, sizeof(label), "W%d", 4-i);
    graphics_draw_text(ctx, label,
                      fonts_get_system_font(FONT_KEY_GOTHIC_14),
                      GRect(x, base_y, bar_width, 15),
                      GTextOverflowModeTrailingEllipsis,
                      GTextAlignmentCenter, NULL);
  }
}

// Draw mood trend line (average mood per week)
static void draw_mood_trend(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Load all entries
  Entry entries[MAX_ENTRIES];
  uint16_t count = storage_get_all_entries(entries, MAX_ENTRIES);

  // Calculate average mood per week for last 4 weeks
  time_t now = time(NULL);
  uint16_t week_mood_sum[4] = {0, 0, 0, 0};
  uint16_t week_mood_count[4] = {0, 0, 0, 0};

  for (uint16_t i = 0; i < count; i++) {
    int days_ago = (now - entries[i].date) / 86400;
    int week = -1;
    if (days_ago < 7) week = 0;
    else if (days_ago < 14) week = 1;
    else if (days_ago < 21) week = 2;
    else if (days_ago < 28) week = 3;

    if (week >= 0) {
      week_mood_sum[week] += entries[i].mood;
      week_mood_count[week]++;
    }
  }

  // Draw title
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, "Mood Trend",
                    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                    GRect(0, 5, bounds.size.w, 20),
                    GTextOverflowModeTrailingEllipsis,
                    GTextAlignmentCenter, NULL);

  // Draw axes
  int margin = 20;
  int graph_height = bounds.size.h - 60;
  int graph_width = bounds.size.w - 2 * margin;
  int base_y = bounds.size.h - 20;

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, GPoint(margin, base_y), GPoint(margin + graph_width, base_y));
  graphics_draw_line(ctx, GPoint(margin, base_y), GPoint(margin, base_y - graph_height));

  // Draw trend line
  GPoint points[4];
  for (int i = 0; i < 4; i++) {
    int x = margin + (3-i) * graph_width / 3;
    int avg_mood = week_mood_count[3-i] > 0 ?
                   week_mood_sum[3-i] / week_mood_count[3-i] : 4;
    int y = base_y - (avg_mood * graph_height / 8);
    points[i] = GPoint(x, y);
  }

  // Draw lines connecting points
  for (int i = 0; i < 3; i++) {
    graphics_draw_line(ctx, points[i], points[i+1]);
  }

  // Draw points
  for (int i = 0; i < 4; i++) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, points[i], 3);
  }

  // Draw labels
  const char *labels[] = {"W4", "W3", "W2", "W1"};
  for (int i = 0; i < 4; i++) {
    graphics_draw_text(ctx, labels[i],
                      fonts_get_system_font(FONT_KEY_GOTHIC_14),
                      GRect(points[i].x - 10, base_y + 2, 20, 15),
                      GTextOverflowModeTrailingEllipsis,
                      GTextAlignmentCenter, NULL);
  }
}

// Draw mood distribution (pie/bar chart of mood counts)
static void draw_mood_distribution(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Load stats
  Stats stats;
  storage_load_stats(&stats);

  // Find top 5 moods
  typedef struct {
    Mood mood;
    uint16_t count;
  } MoodCount;

  MoodCount mood_data[9];
  for (int i = 0; i < 9; i++) {
    mood_data[i].mood = (Mood)i;
    mood_data[i].count = stats.mood_counts[i];
  }

  // Simple bubble sort
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8-i; j++) {
      if (mood_data[j].count < mood_data[j+1].count) {
        MoodCount temp = mood_data[j];
        mood_data[j] = mood_data[j+1];
        mood_data[j+1] = temp;
      }
    }
  }

  // Draw title
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, "Mood Distribution",
                    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                    GRect(0, 5, bounds.size.w, 20),
                    GTextOverflowModeTrailingEllipsis,
                    GTextAlignmentCenter, NULL);

  // Find total for percentage
  uint16_t total = 0;
  for (int i = 0; i < 9; i++) {
    total += mood_data[i].count;
  }
  if (total == 0) total = 1;

  // Draw top 5 moods as horizontal bars
  int bar_height = 15;
  int start_y = 30;
  int max_bar_width = bounds.size.w - 80;

  for (int i = 0; i < 5 && i < 9; i++) {
    if (mood_data[i].count == 0) break;

    int y = start_y + i * (bar_height + 8);
    int bar_width = (mood_data[i].count * max_bar_width) / total;

    // Draw mood label
    graphics_draw_text(ctx, MOOD_LABELS[mood_data[i].mood],
                      fonts_get_system_font(FONT_KEY_GOTHIC_14),
                      GRect(5, y, 60, bar_height),
                      GTextOverflowModeTrailingEllipsis,
                      GTextAlignmentLeft, NULL);

    // Draw bar
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(65, y + 2, bar_width, bar_height - 4), 0, GCornerNone);

    // Draw percentage
    char pct_str[8];
    snprintf(pct_str, sizeof(pct_str), "%d%%", (mood_data[i].count * 100) / total);
    graphics_draw_text(ctx, pct_str,
                      fonts_get_system_font(FONT_KEY_GOTHIC_14),
                      GRect(65 + bar_width + 5, y, 40, bar_height),
                      GTextOverflowModeTrailingEllipsis,
                      GTextAlignmentLeft, NULL);
  }
}

static void viz_layer_update_proc(Layer *layer, GContext *ctx) {
  switch (s_current_viz) {
    case VIZ_BAR_CHART:
      draw_bar_chart(layer, ctx);
      break;
    case VIZ_MOOD_TREND:
      draw_mood_trend(layer, ctx);
      break;
    case VIZ_MOOD_DISTRIBUTION:
      draw_mood_distribution(layer, ctx);
      break;
  }
}

// Menu callbacks
static uint16_t menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return 3;
}

static void menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  const char *titles[] = {
    "Entries per Week",
    "Mood Trend",
    "Mood Distribution"
  };
  menu_cell_basic_draw(ctx, cell_layer, titles[cell_index->row], NULL, NULL);
}

static void menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  s_current_viz = (VisualizationType)cell_index->row;
  s_showing_menu = false;

  // Hide menu, show viz
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);
  layer_set_hidden(s_viz_layer, false);
  layer_mark_dirty(s_viz_layer);
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_showing_menu) {
    window_stack_pop(true);
  } else {
    // Go back to menu
    s_showing_menu = true;
    layer_set_hidden(menu_layer_get_layer(s_menu_layer), false);
    layer_set_hidden(s_viz_layer, true);
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_click_config_provider(window, click_config_provider);

  // Create menu layer
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = menu_get_num_rows,
    .draw_row = menu_draw_row,
    .select_click = menu_select
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  // Create visualization layer (initially hidden)
  s_viz_layer = layer_create(bounds);
  layer_set_update_proc(s_viz_layer, viz_layer_update_proc);
  layer_set_hidden(s_viz_layer, true);
  layer_add_child(window_layer, s_viz_layer);

  s_showing_menu = true;
}

static void window_unload(Window *window) {
  if (s_viz_layer) {
    layer_destroy(s_viz_layer);
    s_viz_layer = NULL;
  }
  if (s_menu_layer) {
    menu_layer_destroy(s_menu_layer);
    s_menu_layer = NULL;
  }
}

void visualization_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void visualization_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
