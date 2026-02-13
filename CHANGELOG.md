# Changelog

All notable changes to the Pebble Gratitude Journal project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Planned for v0.1.1
- Voice input support (where available)
- Daily reminder notifications
- Data export to phone app
- Entry search and filtering
- Mood charts and statistics
- Custom prompt creation
- Entry editing capability
- Cloud sync support

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
