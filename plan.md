# Implementation Plan: Pebble Gratitude Journal App (v1.0 MVP)

## Context

Building a gratitude journal app for Pebble smartwatches from scratch. This is a brand new project with only a tech specification document. The goal is to implement Phase 1 (v1.0 MVP) features that provide core functionality: quick entry creation with canned responses, mood tagging, calendar view with streak counter, and local persistent storage.

This addresses the need for a simple, fast daily gratitude tracking tool optimized for the constraints of a smartwatch (limited screen, no keyboard, limited storage). Users can quickly log gratitude entries using predefined responses, track their consistency via streaks, and visualize their mood patterns over time.

---

## Project Structure

```
pebble_gratitude_journal/
├── package.json                    # Pebble SDK 4.x project config
├── appinfo.json                    # App metadata, resources, capabilities
├── wscript                         # Build configuration
├── src/
│   ├── main.c                      # App lifecycle, window management
│   ├── data/
│   │   ├── entry.h                 # Entry, Mood enum, data structures
│   │   ├── entry.c                 # Entry creation, validation
│   │   ├── storage.h               # Storage interface
│   │   └── storage.c               # Persistent storage with circular buffer
│   ├── windows/
│   │   ├── home_window.c/h         # Home screen with prompt & streak
│   │   ├── entry_window.c/h        # Entry creation with canned responses
│   │   ├── calendar_window.c/h     # Calendar view with mood indicators
│   │   └── settings_window.c/h     # Settings (storage info)
│   ├── logic/
│   │   ├── prompts.c/h             # Prompt library & rotation
│   │   └── stats.c/h               # Streak calculation, mood stats
│   └── utils/
│       ├── date_utils.c/h          # Date manipulation utilities
│       └── constants.h             # App-wide constants
└── resources/
    ├── images/
    │   ├── mood_sad_14.png         # Mood icons (14×14 for calendar)
    │   ├── mood_sad_28.png         # Mood icons (28×28 for selector)
    │   └── ... (18 total: 9 moods × 2 sizes)
    └── app_icon_25.png             # App icon
```

---

## Data Architecture

### Core Data Structures (src/data/entry.h)

```c
// Mood enum ordered by valence (0=worst, 8=best)
typedef enum {
  MOOD_SAD = 0,
  MOOD_ANXIOUS = 1,
  MOOD_STRESSED = 2,
  MOOD_TIRED = 3,
  MOOD_NEUTRAL = 4,
  MOOD_CONTENT = 5,
  MOOD_HAPPY = 6,
  MOOD_EXCITED = 7,
  MOOD_GRATEFUL = 8
} Mood;

// Canned response flags (bitmask for combination)
typedef enum {
  CANNED_FAMILY    = (1 << 0),
  CANNED_FRIENDS   = (1 << 1),
  CANNED_HEALTH    = (1 << 2),
  CANNED_WORK      = (1 << 3),
  CANNED_NATURE    = (1 << 4),
  CANNED_FOOD      = (1 << 5),
  CANNED_MUSIC     = (1 << 6),
  CANNED_REST      = (1 << 7),
  CANNED_LEARNING  = (1 << 8),
  CANNED_PETS      = (1 << 9)
} CannedResponse;

// Entry structure (~151 bytes)
typedef struct {
  time_t date;                    // Normalized to midnight (4 bytes)
  char text[141];                 // 140 chars + null terminator (141 bytes)
  Mood mood;                      // 1 byte (padded to 4)
  uint16_t canned_flags;          // Bitmask of selected responses (2 bytes)
} Entry;

// Stats structure
typedef struct {
  uint16_t current_streak;        // Consecutive days with entries
  time_t last_entry_date;         // For streak calculation
  uint16_t total_entries;         // All-time count
  uint16_t mood_counts[9];        // Count per mood type
} Stats;
```

### Storage Strategy (src/data/storage.c)

**Circular Buffer Implementation:**
- 180 entries max (configurable)
- Each entry stored with key: `STORAGE_KEY_ENTRIES_START + (index % MAX_ENTRIES)`
- Oldest entries auto-deleted when limit reached
- Warning shown at 90% capacity (162 entries)

**Storage Keys:**
```c
#define STORAGE_KEY_VERSION           1
#define STORAGE_KEY_ENTRY_COUNT       2
#define STORAGE_KEY_CURRENT_STREAK    3
#define STORAGE_KEY_LAST_ENTRY_DATE   4
#define STORAGE_KEY_PROMPT_INDEX      5
#define STORAGE_KEY_PROMPT_MODE       6  // 0=random, 1=sequential
#define STORAGE_KEY_ENTRIES_START     100  // Entries stored from 100+
```

**Key Functions:**
- `storage_init()` - Initialize/migrate storage on first run
- `storage_save_entry(Entry*)` - Save entry, handle circular buffer
- `storage_get_entries_for_date(time_t, Entry[], uint16_t*)` - Load day's entries
- `storage_get_all_entries(Entry[], uint16_t*)` - Load all entries
- `storage_update_stats(Stats*)` - Update and persist stats
- `storage_delete_oldest_entry()` - Remove oldest for cleanup

---

## UI Architecture

### 1. Home Window (src/windows/home_window.c)

**Layout:**
```
┌──────────────────────┐
│ "What made you       │  <- Daily prompt (wrapping text)
│  smile today?"       │
│                      │
│ [Add Entry]          │  <- SimpleMenuLayer with 2 items
│ [View Calendar]      │
│                      │
│ 🔥 7 Day Streak      │  <- Streak counter (status bar)
└──────────────────────┘
```

**Implementation:**
- Use `TextLayer` for prompt (scrollable if long)
- Use `SimpleMenuLayer` for menu options
- Update prompt daily using `prompts_get_daily()`
- Load streak from storage on window_load

### 2. Entry Window (src/windows/entry_window.c)

**Flow:**
1. **Canned Response Selection** (multi-select menu)
2. **Mood Selection** (single-select menu with icons)
3. **Review & Save**

**Canned Response UI:**
- Use `MenuLayer` with checkmarks for selected items
- Allow multiple selections (tap to toggle)
- "Done" button proceeds to mood selection
- Generate combined text: "Family, Friends, Food" → "family and friends and food"

**Mood Selection UI:**
- 9-item menu with mood names + icons
- Single selection
- Immediately saves and returns to home

**Text Generation:**
```c
void entry_generate_text(uint16_t canned_flags, char *buffer, size_t size) {
  // Convert bitmask to text: "Family, Friends, and Food"
  // Enforce 140 char limit
}
```

### 3. Calendar Window (src/windows/calendar_window.c)

**Layout:**
```
┌──────────────────────┐
│   < January 2026 >   │  <- Month/year with nav arrows
│ Su Mo Tu We Th Fr Sa │  <- Day headers
│     1  2  3  4  5  6 │
│  7 😊 😐 😊 11 12 13 │  <- Grid with mood indicators
│ 14 15 🙁 17 18 19 20 │
│ 21 22 23 24 25 26 27 │
│ 28 29 30 31          │
└──────────────────────┘
```

**Implementation:**
- Use custom `Layer` with update_proc for calendar grid
- Load month data on window_load (cache for performance)
- Click handler: show entry details for selected day
- Up/Down buttons: navigate months
- Mood indicators: color dots (color) or icons (monochrome)

**Rendering Strategy:**
```c
// Calendar grid drawing
void calendar_layer_update_proc(Layer *layer, GContext *ctx) {
  // Draw 7×6 grid (42 cells max)
  // Highlight days with entries
  // Draw mood indicator (color or icon based on platform)
  // Highlight current day
}
```

**Performance Optimization:**
- Cache month data in `CalendarCache` struct
- Only reload when month changes
- Lazy-load entry details on day click

### 4. Settings Window (src/windows/settings_window.c)

**Menu Items:**
- Storage: "162 / 180 entries" (show utilization)
- Prompt Mode: "Sequential" or "Random" (toggle)
- About: App version

**Implementation:**
- Use `SimpleMenuLayer`
- Update storage count from `persist_read_int(STORAGE_KEY_ENTRY_COUNT)`
- Toggle prompt mode saves to storage

---

## Core Logic Modules

### 1. Prompts (src/logic/prompts.c)

**Responsibilities:**
- Store 50 built-in prompts
- Rotate daily (random or sequential)
- Persist current index/mode

**Key Functions:**
```c
const char* prompts_get_daily(void);           // Get today's prompt
void prompts_set_mode(bool is_sequential);     // Toggle mode
```

**Daily Rotation Logic:**
- Sequential: increment index daily, wrap at 50
- Random: use `rand()` seeded with day number for consistency

### 2. Stats (src/logic/stats.c)

**Responsibilities:**
- Calculate current streak
- Track mood distribution
- Update on each entry save

**Streak Calculation:**
```c
uint16_t stats_calculate_streak(void) {
  // Load all entries sorted by date
  // Count consecutive days from today backwards
  // Break on missing day
  // Return count
}
```

**Key Edge Cases:**
- Multiple entries per day: count as 1 day
- Dates normalized to midnight for comparison
- Streak resets to 0 if yesterday has no entry

---

## Platform-Specific Handling

### Display Differences

**Round vs Rectangular:**
```c
#ifdef PBL_ROUND
  // Add horizontal padding
  // Use centered text alignment
  int padding = 20;
#else
  // Standard layout
  int padding = 4;
#endif
```

**Color vs Monochrome:**
```c
#ifdef PBL_COLOR
  // Use mood colors (GColorBlue, GColorYellow, etc.)
  graphics_context_set_fill_color(ctx, get_mood_color(mood));
  graphics_fill_circle(ctx, center, 4);
#else
  // Use mood icon bitmaps
  GBitmap *icon = gbitmap_create_with_resource(get_mood_resource_id(mood));
  graphics_draw_bitmap_in_rect(ctx, icon, rect);
  gbitmap_destroy(icon);
#endif
```

---

## Implementation Phases

### Phase A: Project Setup & Foundation (Day 1-2)
1. Initialize Pebble SDK 4.x project structure
2. Create `package.json`, `appinfo.json`, `wscript`
3. Implement data structures (entry.h)
4. Implement storage layer (storage.c) with circular buffer
5. Implement date utilities (date_utils.c)
6. Write unit tests for storage and date functions

**Verification:**
- `pebble build` succeeds
- Empty app runs on emulator
- Storage save/load works (test with 200+ entries)

### Phase B: Core UI Screens (Day 3-5)
1. Implement home window with prompt display and menu
2. Implement entry window with canned response selection
3. Implement mood selection flow
4. Connect entry creation to storage
5. Test on emulator (Aplite and Basalt)

**Verification:**
- Can create entries via canned responses
- Entries persist across app restarts
- Text generation respects 140-char limit
- Mood saves correctly

### Phase C: Calendar & Stats (Day 6-8)
1. Implement calendar window with month grid
2. Implement calendar data loading and caching
3. Implement streak calculation logic
4. Add mood indicators to calendar (color + monochrome)
5. Test date edge cases (leap years, month boundaries)

**Verification:**
- Calendar displays correct days for all months
- Mood indicators show correctly on all platforms
- Streak counts consecutive days accurately
- Click on calendar day shows entry details

### Phase D: Polish & Testing (Day 9-10)
1. Add storage warning dialog (90% capacity)
2. Implement settings window
3. Create mood icon resources (18 PNG files)
4. Test on all platforms (Aplite, Basalt, Chalk emulators)
5. Fix bugs and optimize performance
6. Add error handling for storage failures

**Verification:**
- Storage warning appears at 162 entries
- No memory leaks (use Pebble tools)
- Smooth performance on Aplite (lowest RAM)
- All windows work on round displays

### Phase E: Deployment Preparation (Day 11)
1. Generate UUID and update metadata
2. Create app icon (25×25, 32×32, 48×48)
3. Build release version
4. Test .pbw on real device (if available)
5. Write README and user guide
6. Package for distribution

**Verification:**
- App installs successfully
- All features work on physical device
- Battery usage is minimal
- Ready for Rebble app store submission

---

## Critical Files (Priority Order)

1. **src/data/storage.c** - Circular buffer persistence (foundation)
2. **src/windows/entry_window.c** - Entry creation UI (primary interaction)
3. **src/windows/calendar_window.c** - Calendar rendering (most complex)
4. **src/data/entry.h** - Data model definitions (core structures)
5. **src/utils/date_utils.c** - Date manipulation (correctness critical)

---

## Testing Strategy

### Automated Testing
- Date utilities: leap years, month boundaries, day-of-week
- Streak calculation: consecutive days, gaps, multiple entries per day
- Circular buffer: 180 entries, 181st triggers deletion
- Text generation: canned response combinations, 140-char limit

### Manual Testing Checklist
- [ ] Create 180 entries successfully
- [ ] Storage warning at 162 entries
- [ ] 181st entry deletes oldest
- [ ] Streak increments daily, resets on skip
- [ ] Calendar shows correct months (test Feb in leap year)
- [ ] Mood indicators display on color and monochrome
- [ ] Round display layout works (Chalk)
- [ ] App restarts preserve data
- [ ] Multiple entries per day allowed
- [ ] Canned response combination generates correct text

### Platform Testing
- [ ] Aplite (monochrome, rectangular, limited RAM)
- [ ] Basalt (color, rectangular)
- [ ] Chalk (color, round)

---

## Configuration Files

### package.json
```json
{
  "name": "gratitude-journal",
  "author": "Your Name",
  "version": "1.0.0",
  "keywords": ["gratitude", "journal", "mood", "wellness"],
  "private": true,
  "dependencies": {},
  "pebble": {
    "sdkVersion": "3",
    "enableMultiJS": false,
    "targetPlatforms": ["aplite", "basalt", "chalk", "diorite"],
    "watchapp": {
      "watchface": false
    },
    "messageKeys": [],
    "resources": {
      "media": [
        {
          "type": "png",
          "name": "APP_ICON",
          "file": "images/app_icon_25.png"
        }
      ]
    }
  }
}
```

### appinfo.json
```json
{
  "uuid": "GENERATE-UUID-HERE",
  "shortName": "Gratitude",
  "longName": "Gratitude Journal",
  "companyName": "Your Name",
  "versionLabel": "1.0.0",
  "sdkVersion": "3",
  "targetPlatforms": ["aplite", "basalt", "chalk", "diorite"],
  "capabilities": [""],
  "watchapp": {
    "watchface": false
  },
  "appKeys": {},
  "resources": {
    "media": []
  }
}
```

---

## Known Limitations (v1.0)

- No text input (T9/voice) - canned responses only
- No custom prompts - 50 built-in prompts only
- No data export - local storage only
- No daily reminders - user-initiated entries only
- No entry editing/deletion - entries are append-only
- No visualizations beyond calendar - no charts/graphs

These features are planned for v1.1+ per tech spec.

---

## Success Criteria

**v1.0 is successful if:**
1. Users can create entries in <30 seconds using canned responses
2. Calendar accurately shows entries and moods for any month
3. Streak counter motivates daily use (increments/resets correctly)
4. Storage handles 180+ entries without issues
5. App works on all Pebble models (color and monochrome)
6. No crashes or data loss
7. Minimal battery impact (<1% per day)

---

## Next Steps After Plan Approval

1. Generate UUID for app
2. Initialize Pebble SDK project structure
3. Create directory structure and placeholder files
4. Implement Phase A (foundation: storage + data utils)
5. Implement Phase B (UI: home, entry, mood windows)
6. Implement Phase C (calendar + streak logic)
7. Implement Phase D (polish + cross-platform testing)
8. Implement Phase E (deployment preparation)
