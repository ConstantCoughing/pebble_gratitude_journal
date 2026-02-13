# Development Guide

## Quick Start

```bash
# Build the project
pebble build

# Install to emulator
pebble install --emulator basalt

# View logs
pebble logs --emulator basalt
```

## Architecture Overview

### Data Flow

1. **Entry Creation**:
   - User selects canned responses → `entry_window.c`
   - User selects mood → `entry_window.c`
   - Entry created with `entry_init()` → `entry.c`
   - Entry saved with `storage_save_entry()` → `storage.c`
   - Stats updated with `stats_update_after_entry()` → `stats.c`

2. **Calendar Display**:
   - Month data loaded with `load_month_data()` → `calendar_window.c`
   - Entries retrieved with `storage_get_all_entries()` → `storage.c`
   - Calendar grid drawn with mood indicators
   - Navigation updates month and reloads data

3. **Streak Calculation**:
   - Triggered on home window load
   - Loads all entries sorted by date
   - Counts consecutive days backwards from today
   - Breaks on first missing day

### Key Constants

From `src/utils/constants.h`:

```c
#define MAX_ENTRIES 180                  // Circular buffer size
#define STORAGE_WARNING_THRESHOLD 162    // 90% capacity
#define MAX_ENTRY_TEXT_LENGTH 140        // Twitter-style limit
#define NUM_PROMPTS 50                   // Built-in prompts
#define NUM_CANNED_RESPONSES 10          // Response options
```

### Storage Keys

```
1:   STORAGE_KEY_VERSION           - Schema version
2:   STORAGE_KEY_ENTRY_COUNT       - Total entry count
3:   STORAGE_KEY_CURRENT_STREAK    - Current streak
4:   STORAGE_KEY_LAST_ENTRY_DATE   - Last entry date
5:   STORAGE_KEY_PROMPT_INDEX      - Current prompt index
6:   STORAGE_KEY_PROMPT_MODE       - Random (0) or Sequential (1)
100+: Entries (circular buffer)
```

## Testing Checklist

### Core Functionality

- [ ] Create entry with single canned response
- [ ] Create entry with multiple canned responses
- [ ] Create entry with all 10 canned responses
- [ ] Select each of the 9 moods
- [ ] Create multiple entries on same day
- [ ] View entries in calendar
- [ ] Navigate calendar months forward/backward
- [ ] Verify streak increments daily
- [ ] Verify streak resets when day is skipped

### Storage

- [ ] Create 180 entries successfully
- [ ] Storage warning appears at 162 entries
- [ ] 181st entry deletes oldest
- [ ] Data persists across app restarts
- [ ] Entry count is accurate

### Platform Compatibility

- [ ] Test on Aplite emulator (monochrome, rectangular)
- [ ] Test on Basalt emulator (color, rectangular)
- [ ] Test on Chalk emulator (color, round)
- [ ] Verify mood indicators display correctly
- [ ] Verify layouts work on round display
- [ ] Check memory usage on Aplite (lowest RAM)

### Edge Cases

- [ ] Calendar displays correctly for leap years
- [ ] Month boundaries handled correctly
- [ ] Entries dated in past work correctly
- [ ] Empty calendar displays correctly
- [ ] Settings update correctly
- [ ] Prompt mode toggle works
- [ ] Sequential prompt advances daily
- [ ] Random prompt is consistent per day

## Common Issues

### Build Errors

**Error**: `pebble.h not found`
- **Solution**: Ensure Pebble SDK is properly installed and in PATH

**Error**: `waf: error: no such option`
- **Solution**: Clean build directory: `pebble clean`

### Runtime Issues

**Issue**: Storage full despite showing low count
- **Cause**: Corrupted storage keys
- **Solution**: Clear app data from Pebble settings

**Issue**: Streak not calculating correctly
- **Cause**: Date normalization issue
- **Solution**: Verify `date_normalize_to_midnight()` is called

**Issue**: Calendar shows wrong days
- **Cause**: Day-of-week calculation error
- **Solution**: Verify `date_get_day_of_week()` implementation

## Code Conventions

### Naming

- **Functions**: `module_action_target()` (e.g., `storage_save_entry()`)
- **Static functions**: `function_name()` (e.g., `window_load()`)
- **Variables**: `snake_case` (e.g., `entry_count`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_ENTRIES`)
- **Types**: `PascalCase` (e.g., `Entry`, `Mood`)

### File Organization

- **Header files**: Public API declarations only
- **Source files**: Implementation details
- **Static variables**: Use `s_` prefix (e.g., `s_window`)
- **Global variables**: Avoid when possible

### Memory Management

- Always `destroy()` what you `create()`
- Use `window_unload()` for cleanup
- Minimize heap allocations
- Be mindful of Aplite's limited RAM (24KB)

## Performance Tips

1. **Calendar Loading**:
   - Cache month data to avoid redundant loads
   - Only reload on month change
   - Sort entries once, not per-day

2. **Storage**:
   - Read all entries once, filter in memory
   - Avoid multiple `persist_read_data()` calls
   - Use entry count before loading

3. **UI**:
   - Use `layer_mark_dirty()` only when needed
   - Minimize `layer_update_proc()` complexity
   - Cache formatted strings

## Extending the App

### Adding a New Canned Response

1. Update `NUM_CANNED_RESPONSES` in `constants.h`
2. Add enum value to `CannedResponse` in `entry.h`
3. Add string to `CANNED_RESPONSE_STRINGS[]` in `entry.c`
4. Add label to `CANNED_LABELS[]` in `entry_window.c`

### Adding a New Mood

1. Add enum value to `Mood` in `entry.h`
2. Add string to `MOOD_STRINGS[]` in `entry.c`
3. Add label to `MOOD_LABELS[]` in `entry_window.c`
4. Add color to `MOOD_COLORS[]` in `calendar_window.c`
5. Update `Stats.mood_counts[]` size if needed

### Adding a New Window

1. Create `window_name.h` with `push()` and `destroy()` functions
2. Create `window_name.c` with implementation
3. Add includes to calling window
4. Call `window_name_push()` from menu callback

## Debugging Tips

### Logging

Add debug prints:
```c
APP_LOG(APP_LOG_LEVEL_DEBUG, "Entry count: %d", count);
```

View logs:
```bash
pebble logs --emulator basalt
```

### Memory Leaks

Use Pebble tools to check:
```bash
pebble analyze-size build/
```

### Crash Investigation

Check stack traces in logs:
```bash
pebble logs --emulator basalt | grep -A 20 "Crashed"
```

## Resources

- [Pebble SDK Documentation](https://developer.rebble.io/developer.pebble.com/docs/)
- [Rebble Community](https://rebble.io/)
- [Pebble Developer Forum](https://forums.rebble.io/)
- [C SDK API Reference](https://developer.rebble.io/developer.pebble.com/docs/c/)

## Build Pipeline

The Pebble SDK uses `waf` for building:

1. **Configure**: Detects target platforms
2. **Compile**: Builds C source files
3. **Link**: Creates ELF binaries per platform
4. **Bundle**: Packages into `.pbw` file
5. **Resources**: Embeds images and resources

## Deployment

### Creating a Release

1. Update version in `appinfo.json` and `package.json`
2. Build release version: `pebble build`
3. Test on all emulators
4. Generate `.pbw`: Found in `build/` directory
5. Submit to Rebble app store (if desired)

### App Store Metadata

- **Name**: Gratitude Journal
- **Category**: Health & Fitness
- **Description**: Track daily gratitude, moods, and build journaling habits
- **Screenshots**: Capture from emulators
- **Supported Platforms**: All (Aplite, Basalt, Chalk, Diorite)

---

Happy coding! 🚀
