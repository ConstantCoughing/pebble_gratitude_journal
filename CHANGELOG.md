# Changelog

All notable changes to the Pebble Gratitude Journal project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.1] - 2026-02-13

### Added - Major Feature Release
- **Storage Warning Dialog**: Shows alert when storage reaches 90% capacity
- **Calendar Day Detail View**: Tap SELECT on calendar to view entry details for today
- **Entry Detail Window**: New window showing full entry text, mood, and date with scrolling
- **Enhanced Error Handling**: Added comprehensive APP_LOG statements and error checks throughout storage module
- **Daily Reminders System**: Complete implementation with Wakeup API
  - Configurable reminder time (cycles through 6 common times: 8 AM, 12 PM, 6 PM, 8 PM, 9 PM, 10 PM)
  - Enable/disable toggle in settings
  - Snooze functionality (15 min and 30 min options)
  - Auto-reschedule for next day after trigger
  - Reminder settings accessible from main settings menu
- **Entry Management System**: Full edit and delete functionality
  - Edit existing entries (modify canned responses and mood)
  - Delete entries with confirmation
  - SELECT button on entry detail shows action menu (Edit/Delete)
  - Streak recalculation after deletion
  - New `storage_update_entry()`, `storage_delete_entry()`, `storage_get_entry_by_index()` functions
- **Emoji Mood Icons**: Using emoji-style icons (14×14px and 28×28px) everywhere
  - Calendar shows mood-specific emoji in bottom-right of cells
  - Mood selection menu displays large emoji icons
  - 19 PNG icon files included (9 moods × 2 sizes + app icon)
  - Works beautifully on all platforms
- **Version Update**: App version now shows "0.1.1-dev"

#### Mood Visualizations
- **Visualization window** with 3 chart types accessible from home menu
- **Bar Chart**: Shows entries per week for last 4 weeks with counts
- **Mood Trend**: Line graph showing average mood progression over 4 weeks
- **Mood Distribution**: Horizontal bar chart of top 5 moods with percentages
- Navigate between charts via menu, press BACK to return

#### Custom Prompts & Export
- **Custom Prompt Management**: Add up to 20 custom gratitude prompts
- **Prompt Storage**: Persistent storage for custom prompts (keys 500-520)
- **Simple Interface**: Tap to add sample prompt, long-press to delete
- **Export Structure**: Basic PebbleKit JS framework for data export
- **Settings Integration**: Custom prompts and export accessible from settings menu

### Enhanced
- **App Version**: Updated to 0.1.1
- **Home Menu**: Now includes Visualizations option (4 menu items)
- **Settings Menu**: Expanded to 6 options (Storage, Prompt Mode, Reminders, Custom Prompts, Export, About)
- **Cross-Platform**: All features tested and working on Aplite, Basalt, Chalk, Diorite emulators

### Technical Details
- **New Files**: 10 additional source files (visualization, export, custom prompts, reminders, entry management)
- **Icon Resources**: 19 emoji PNG files (14×14px and 28×28px for 9 moods + app icon)
- **Storage Keys**: Extended storage schema (keys 1-6 metadata, 10-13 reminders, 100-279 entries, 500-520 custom prompts)
- **Code Quality**: Comprehensive error handling with APP_LOG throughout
- **Memory**: Optimized for Aplite's 24KB RAM limitation

### Known Issues
- **Export**: PebbleKit JS export requires full implementation for phone/computer data transfer
- **Custom Prompts**: Text input uses sample prompt; full text input dialog deferred
- **Visualizations**: Limited to 4-week window for performance

### Breaking Changes
None - fully backward compatible with v0.1.0 data

### Planned for v0.1.1 (Priority 1 & 2 Features)

#### High-Value Features
- **Daily Reminders**: Configurable notification time, snooze, skip options
- **Mood Visualizations**: Bar charts (entries/week), trend lines, distribution charts
- **Entry Management**: Edit existing entries, delete with confirmation, view details

#### Data Management
- **Data Export**: JSON and Markdown formats via PebbleKit JS
- **Custom Prompts**: Add, edit, delete, and toggle custom prompts (max 20)

#### Technical Debt & Improvements
- Storage warning dialog implementation
- Calendar day detail view
- Mood icon resources (18 PNG files)
- Memory optimization for Aplite
- Enhanced error handling
- Unit tests for critical modules

**Estimated Release**: 6 weeks from start

---

### Planned for v0.1.2 (Advanced Input & Search)

#### Advanced Input Features
- **Voice-to-Text**: Rebble voice API integration (if available) for natural entry creation
- **Entry Search**: Text search, filter by mood/date/tags for quick retrieval
- **T9 Input**: Alternative text input method (optional, based on v0.1.1 feedback)

**Estimated Release**: 4-5 weeks after v0.1.1

---

### Future Considerations (v0.2.0+)
- Cloud sync support
- Sharing entries via SMS/email
- Themes and customization
- Multi-language support
- Advanced streaks (weekly/monthly goals)
- Achievements and gamification
- Companion mobile app
- AI-powered insights

## [0.1.0] - 2026-02-13

### Added - Initial MVP Release
- Quick entry creation using 10 predefined canned responses
- Mood tagging with 9 mood options (Sad → Grateful)
- 50 built-in gratitude prompts with daily rotation (random/sequential modes)
- Streak tracking for daily consistency
- Calendar view with monthly navigation
- Mood indicators (color dots on color platforms, icons on monochrome)
- Persistent local storage with circular buffer (180 entries)
- Storage capacity warnings at 90% (162 entries)
- Settings window (storage info, prompt mode toggle, about)
- Cross-platform support (Aplite, Basalt, Chalk, Diorite)
- Platform-specific optimizations (PBL_COLOR, PBL_ROUND)

### Technical Details
- **Data structures**: Entry, Mood, CannedResponse, Stats
- **Storage**: Circular buffer with auto-deletion of oldest entries
- **Date utilities**: Normalization, calendar math, leap year handling
- **UI**: 4 windows (home, entry, calendar, settings)
- **Resources**: 18 mood icons (9 moods × 2 sizes) + app icon

### Known Limitations
- No free-text input (canned responses only)
- No custom prompts (50 built-in only)
- No data export (local storage only)
- No daily reminders (manual entry creation)
- No entry editing/deletion (append-only)
- No data sync between devices

---

## Version Numbering Scheme

**v0.1.x** - Initial development phase
- v0.1.0 - MVP with core features
- v0.1.1+ - Incremental feature additions

**v0.2.x** - Enhanced functionality
- Advanced features (voice, reminders, export)

**v1.0.0** - First stable release
- All planned features implemented
- Tested on physical devices
- Production-ready

---

[Unreleased]: https://github.com/yourusername/pebble_gratitude_journal/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/yourusername/pebble_gratitude_journal/releases/tag/v0.1.0
