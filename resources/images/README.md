# App Resources

This directory contains icon resources for the Pebble Gratitude Journal app.

## Current Implementation: Emoji Mood Icons

**Design Decision (v0.1.1)**: The app uses **emoji-style mood icons** (14×14px and 28×28px) to provide clear visual mood indicators across all platforms.

### Mood Icon Approach
- **Calendar View**: Small emoji icons (14×14px) in bottom-right of calendar cells
- **Mood Selection**: Large emoji icons (28×28px) next to mood labels
- **Entry Detail**: Mood name displayed as text

This approach:
- ✅ Works on both color and monochrome displays
- ✅ Clear visual distinction between different moods
- ✅ Recognizable emoji-style faces
- ✅ Small file size (~1-2KB per icon)
- ✅ Universal understanding of emotion icons

### Icon Files Included (19 total)
- **Small icons (14×14px)**: 9 mood icons for calendar display
- **Large icons (28×28px)**: 9 mood icons for mood selection
- **App icon (25×25px)**: Main launcher icon

## Icon Specifications

### Small Icons (14×14px) - Calendar View
Used in calendar grid to show mood for days with entries:
- `mood_sad_14.png` - 😢 Sad face
- `mood_anxious_14.png` - 😰 Anxious face
- `mood_stressed_14.png` - 😫 Stressed face
- `mood_tired_14.png` - 😴 Tired face
- `mood_neutral_14.png` - 😐 Neutral face
- `mood_content_14.png` - 😌 Content face
- `mood_happy_14.png` - 😊 Happy face
- `mood_excited_14.png` - 😃 Excited face
- `mood_grateful_14.png` - 🙏 Grateful hands

### Large Icons (28×28px) - Mood Selection
Used in mood selection menu for better visibility:
- `mood_sad_28.png` - Larger sad face
- `mood_anxious_28.png` - Larger anxious face
- `mood_stressed_28.png` - Larger stressed face
- `mood_tired_28.png` - Larger tired face
- `mood_neutral_28.png` - Larger neutral face
- `mood_content_28.png` - Larger content face
- `mood_happy_28.png` - Larger happy face
- `mood_excited_28.png` - Larger excited face
- `mood_grateful_28.png` - Larger grateful hands

### App Icon
- `app_icon_25.png` (25×25 pixels) - Main launcher icon

## Code Implementation

### Calendar View (calendar_window.c)
```c
// Draw mood indicator using emoji icons
if (s_cache.has_entry[grid_index]) {
  GBitmap *icon = gbitmap_create_with_resource(get_mood_icon_resource(mood));
  if (icon) {
    GRect icon_rect = GRect(x + CELL_SIZE - 8, y + CELL_SIZE - 8, 6, 6);
    graphics_draw_bitmap_in_rect(ctx, icon, icon_rect);
    gbitmap_destroy(icon);
  }
}
```

### Mood Selection (entry_window.c, edit_entry_window.c)
```c
// Draw mood menu item with emoji icon
GBitmap *icon = gbitmap_create_with_resource(get_mood_icon_resource(mood));
menu_cell_basic_draw(ctx, cell_layer, mood_label, NULL, icon);
gbitmap_destroy(icon);
```

## Benefits of This Approach

1. **Visual Clarity**: Emoji icons are universally understood
2. **Mood Recognition**: Easy to identify mood at a glance in calendar
3. **Cross-Platform**: Works on all Pebble models (Aplite, Basalt, Chalk, Diorite)
4. **Small Footprint**: Each icon is only 1-2KB
5. **Professional Look**: Polished emoji-style design

## Icon Design Guidelines

The icons follow these principles:
- **Simple geometry**: Clear at small sizes (14px)
- **Black & white**: High contrast for monochrome displays
- **Emoji-inspired**: Recognizable emotion expressions
- **Consistent style**: All icons match visual language

---

**Version**: v0.1.1-dev
**Last Updated**: February 2026
**Status**: ✅ Complete - 19 icons included
