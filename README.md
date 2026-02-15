# Gratitude Journal - Pebble Smartwatch App

A gratitude journal app for Pebble smartwatches that helps you track daily moments of gratitude, visualize mood patterns, and build a consistent journaling habit with streak tracking.

## Features

- **Quick Entry Creation**: Create journal entries in under 30 seconds using 10 predefined canned responses
- **Mood Tagging**: Tag each entry with one of 9 moods with emoji icon indicators
- **Daily Prompts**: Get inspired by 50 built-in gratitude prompts that rotate daily (random or sequential)
- **Streak Counter**: Track your journaling consistency with a daily streak counter
- **Calendar View**: Visualize your entries in a monthly calendar with emoji mood indicators
- **Entry Management**: Edit existing entries and delete with confirmation
- **Daily Reminders**: Configurable notification time with snooze options (15/30 min)
- **Mood Visualizations**: Bar charts, mood trends, and distribution analytics
- **Custom Prompts**: Add, manage, and delete up to 20 custom gratitude prompts
- **Data Export**: Export your journal data (basic export structure implemented)
- **Persistent Storage**: Store up to 180 entries locally on your Pebble with automatic circular buffer management
  
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
3. Use UP/DOWN buttons to navigate dates with entries
4. Current day is highlighted
5. Press SELECT to view today's entry details

### Visualizations

1. From the home screen, select **Visualizations**
2. Choose from three visualization types:
   - **Entries per Week**: Bar chart of last 4 weeks
   - **Mood Trend**: Line graph of average mood over time
   - **Mood Distribution**: Top 5 moods with percentages
3. Press BACK to return to menu

### Managing Entries

1. From calendar, press SELECT to view entry details
2. Press SELECT again for action menu
3. Choose **Edit Entry** to modify canned responses and mood
4. Choose **Delete Entry** to remove (with confirmation)
5. Streak automatically recalculates after deletion

### Settings

1. From the home screen, select **Settings**
2. **Storage**: View utilization (entries/180)
3. **Prompt Mode**: Toggle Random/Sequential
4. **Reminders**: Configure daily reminder time and snooze
5. **Custom Prompts**: Add/manage up to 20 custom prompts (tap to add, long-press to delete)
6. **Export Data**: Export journal data (requires PebbleKit JS)
7. **About**: View app version

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

## Future Features
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
