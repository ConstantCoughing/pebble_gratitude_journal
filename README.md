# Gratitude Journal - Pebble Smartwatch App

A gratitude journal app for Pebble smartwatches that helps you track daily moments of gratitude, visualize mood patterns, and build a consistent journaling habit with streak tracking.

## Features

### v0.1.0 (MVP)

- **Quick Entry Creation**: Create journal entries in under 30 seconds using 10 predefined canned responses
- **Mood Tagging**: Tag each entry with one of 9 moods (Sad, Anxious, Stressed, Tired, Neutral, Content, Happy, Excited, Grateful)
- **Daily Prompts**: Get inspired by 50 built-in gratitude prompts that rotate daily
- **Streak Counter**: Track your journaling consistency with a daily streak counter
- **Calendar View**: Visualize your entries and moods in a monthly calendar with color-coded mood indicators
- **Persistent Storage**: Store up to 180 entries locally on your Pebble with automatic circular buffer management
- **Cross-Platform**: Works on all Pebble models (Aplite, Basalt, Chalk, Diorite)

## Installation

### Prerequisites

- Pebble SDK 4.x installed
- Python 2.7 or 3.x
- Pebble emulator or physical Pebble device

### Build Instructions

1. Clone this repository:
   ```bash
   cd pebble_gratitude_journal
   ```

2. Build the app:
   ```bash
   pebble build
   ```

3. Install to emulator:
   ```bash
   pebble install --emulator basalt
   ```

4. Or install to device:
   ```bash
   pebble install --phone <YOUR_PHONE_IP>
   ```

## Usage

### Creating an Entry

1. From the home screen, select **Add Entry**
2. Choose one or more canned responses (Family, Friends, Health, Work, Nature, Food, Music, Rest, Learning, Pets)
3. Select **Done** when finished
4. Choose your mood from 9 options
5. Entry is automatically saved

### Viewing the Calendar

1. From the home screen, select **View Calendar**
2. Use UP/DOWN buttons to navigate months
3. Days with entries show mood indicators (colored dots on color displays, small dots on monochrome)
4. Current day is highlighted

### Settings

1. From the home screen, select **Settings**
2. View storage utilization
3. Toggle prompt mode between Random and Sequential
4. View app version

## Project Structure

```
pebble_gratitude_journal/
├── src/
│   ├── main.c                      # App lifecycle
│   ├── data/
│   │   ├── entry.h/c               # Entry data structures and logic
│   │   └── storage.h/c             # Persistent storage with circular buffer
│   ├── windows/
│   │   ├── home_window.h/c         # Home screen with menu
│   │   ├── entry_window.h/c        # Entry creation UI
│   │   ├── calendar_window.h/c     # Calendar view
│   │   └── settings_window.h/c     # Settings screen
│   ├── logic/
│   │   ├── prompts.h/c             # Daily prompt rotation
│   │   └── stats.h/c               # Streak calculation
│   └── utils/
│       ├── date_utils.h/c          # Date manipulation utilities
│       └── constants.h             # App-wide constants
├── resources/
│   └── images/                     # Mood icons and app icon
├── package.json
├── appinfo.json
└── wscript
```

## Technical Details

### Data Storage

- **Circular Buffer**: Automatically manages 180 entries, deleting oldest when full
- **Entry Size**: ~151 bytes per entry
- **Warning Threshold**: Alert shown at 90% capacity (162 entries)
- **Storage Keys**: Uses persistent storage keys 1-6 for metadata, 100+ for entries

### Streak Calculation

- Counts consecutive days with at least one entry
- Allows multiple entries per day (counts as one day)
- Resets to 0 if no entry yesterday or today
- Persists across app restarts

### Date Handling

- All dates normalized to midnight for consistency
- Handles leap years correctly
- Supports month/year navigation
- Day-of-week calculation for calendar grid

### Platform Support

- **Aplite**: Monochrome, rectangular display
- **Basalt**: Color, rectangular display
- **Chalk**: Color, round display
- **Diorite**: Monochrome, rectangular display

Uses preprocessor directives for platform-specific features:
- `PBL_COLOR`: Color vs monochrome mood indicators
- `PBL_ROUND`: Adjusted layouts for round displays

## Known Limitations (v0.1.0)

- No text input - canned responses only
- No custom prompts - 50 built-in only
- No data export - local storage only
- No daily reminders - manual entry creation
- No entry editing/deletion - append-only
- No data sync between devices

## Future Features (v0.1.1+)

- Voice input support (where available)
- Daily reminder notifications
- Data export to phone app
- Entry search and filtering
- Mood charts and statistics
- Custom prompt creation
- Entry editing capability
- Cloud sync support

## Development

### Testing

Run on emulators for different platforms:
```bash
pebble install --emulator aplite   # Monochrome, rectangular
pebble install --emulator basalt   # Color, rectangular
pebble install --emulator chalk    # Color, round
```

### Debugging

View logs:
```bash
pebble logs --emulator basalt
```

### Code Style

- Follow Pebble SDK conventions
- Use descriptive variable names
- Comment complex logic
- Keep functions focused and small

## Contributing

This is an educational project for learning Pebble development. Contributions, suggestions, and bug reports are welcome!

## License

MIT License - Feel free to use and modify for your own purposes.

## Acknowledgments

- Built with Pebble SDK 4.x
- Inspired by the gratitude journaling practice
- Thanks to the Rebble community for keeping Pebble alive

## Support

For issues or questions:
- Check the code documentation
- Review the tech spec document
- Open an issue on GitHub

---

**Note**: Pebble smartwatches are no longer manufactured, but continue to work thanks to the Rebble community. Visit [rebble.io](https://rebble.io) for more information on keeping your Pebble running.
