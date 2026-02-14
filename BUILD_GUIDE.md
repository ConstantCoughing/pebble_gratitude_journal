# Build Guide - Pebble Gratitude Journal

## Pre-Build Checklist

Before building this project, ensure you have the following installed:

- **Pebble SDK**  
  Install the Pebble SDK and set up your environment.  
  [Pebble SDK Installation Guide](https://developer.rebble.io/)
- **Python 3.x**  
  Install via Homebrew:  
  ```bash
  brew install python
  ```
- **pip**  
  Comes with Python 3.x installed via Homebrew.
- **C Compiler (GCC/Clang)**  
  Install Xcode Command Line Tools:  
  ```bash
  xcode-select --install
  ```
- **Git**  
  Install via Homebrew:  
  ```bash
  brew install git
  ```
- **Node.js & npm**  
  Install via Homebrew:  
  ```bash
  brew install node
  ```
- **Make**  
  Install via Homebrew:  
  ```bash
  brew install make
  ```


---

**Optional (for development):**

- VS Code or other IDE
- Mobile device or emulator for testing the Pebble app

> Make sure all environment variables required by the Pebble SDK are set.

## Quick Build & Test


---

**Expected Output:**
```
[INFO    ] Building project...
[INFO    ] Compiling C files...
[INFO    ] Linking...
[SUCCESS] Build complete
```

### 2. Install to Emulator

---

**For color, rectangular display (recommended for first test):**
```bash
pebble install --emulator basalt
```

**For other platforms:**
```bash
pebble install --emulator aplite   # Monochrome, rectangular
pebble install --emulator chalk    # Color, round
pebble install --emulator diorite  # Monochrome, rectangular (newer)
```

### 3. View Logs

```bash
pebble logs --emulator basalt
```

Keep this terminal open to see debug output and catch any errors.

## First-Run Test Sequence

### Test 1: Home Screen
1. App launches successfully
2. Daily prompt displays
3. Menu shows: "Add Entry", "View Calendar", "Settings"
4. Bottom shows: "🔥 0 Day Streak"

### Test 2: Create First Entry
1. Select "Add Entry"
2. Multi-select screen appears with 10 canned responses
4. Select "Done"
5. Mood selection screen appears with 9 moods + icons
7. Returns to home screen automatically

### Test 3: Verify Entry Saved
1. From home screen, select "View Calendar"
3. Today's date is highlighted

### Test 4: Calendar Navigation
1. Press DOWN button → next month
2. Press UP button → previous month

### Test 5: Streak Counter
1. Return to home screen (BACK button)
2. Streak should show "🔥 1 Day Streak"
4. Return to home → still shows "🔥 1 Day Streak" (same day)

### Test 6: Settings
1. From home screen, select "Settings"
3. Prompt Mode shows: "Random" or "Sequential"
4. Tap Prompt Mode to toggle
5. About shows: "v0.1.0"

## Platform-Specific Testing

### Aplite (Monochrome, Rectangular)
```

**Check:**
- Mood icons display correctly (14×14 icons on calendar)
- Text is readable
- Layout fits screen

### Basalt (Color, Rectangular)
```bash
pebble install --emulator basalt
```

**Check:**
- Colored mood dots on calendar
- 28×28 mood icons in selection menu
- Colors are distinct and readable

### Chalk (Color, Round)
```bash
pebble install --emulator chalk
```

**Check:**
- Layout adjusts for round display
- Text doesn't clip at edges
- Calendar grid visible

## Common Build Issues

### Issue: `pebble: command not found`
**Solution:** Install Pebble SDK or add to PATH:
```bash
export PATH=$PATH:~/pebble-dev/pebble-sdk-4.5-mac/bin
```

### Issue: `waf: error: no such option`
**Solution:** Clean and rebuild:
```bash
pebble clean
pebble build
```

### Issue: `Resource not found: RESOURCE_ID_MOOD_*`
**Solution:** Resources not compiled. Check:
1. Icons exist in `resources/images/`
2. `appinfo.json` lists all resources
3. Run `pebble clean` then `pebble build`

### Issue: App crashes on launch
**Solution:** Check logs:
```bash
pebble logs --emulator basalt | grep -i error
```

Look for:
- Memory allocation failures
- Null pointer access
- Storage read/write errors

## Advanced Testing

### Test Multiple Entries
1. Create 5 entries on same day with different moods
2. Check calendar shows last entry's mood
3. Verify storage count: 5 / 180

### Test Date Boundaries
1. Change emulator date to end of month
2. Create entry
3. Navigate calendar → verify correct date

### Test Storage Capacity
This would require creating 162+ entries programmatically or manually, which is time-consuming. For now, verify:
- Storage count updates correctly
- Warning threshold check works (at 90% = 162 entries)

### Test Streak Logic
1. Create entry today
2. Change emulator date to tomorrow
3. Restart app → should show 1 day streak
4. Create entry tomorrow
5. Restart app → should show 2 day streak
6. Change date to 2 days later (skip a day)
7. Restart app → should show 0 day streak

## Performance Checks

### Memory Usage (Aplite - 24KB RAM)
```bash
pebble analyze-size build/
```

Check:
- Heap usage is reasonable
- Stack size is adequate
- No memory leaks

### Battery Impact
On physical device:
- Background: <1% drain per day
- Active use: Minimal drain

## Building for Release

### Generate .pbw File
```bash
pebble build
```

The `.pbw` file is in: `build/gratitude-journal.pbw`

### Test .pbw on Physical Device
```bash
pebble install --phone <YOUR_PHONE_IP>
```

Or:
1. Copy `build/gratitude-journal.pbw` to phone
2. Use Rebble app to install

## Troubleshooting

### App works in emulator but not on device
- Check SDK version compatibility
- Verify resources compile correctly
- Test on multiple emulators first

### Calendar rendering is slow
- Reduce entry count for testing
- Check for unnecessary redraws
- Profile with logs

### Mood icons don't appear
- Verify icon resources exist
- Check resource IDs in appinfo.json
- Ensure correct file names

## Next Steps After Successful Build

1. **Test thoroughly** on all emulators
2. **Create test entries** with various combinations
3. **Verify data persistence** across app restarts
4. **Test edge cases** (empty data, full storage, date boundaries)
5. **Fix any bugs** discovered during testing
6. **Optimize performance** if needed
7. **Submit to Rebble app store** (optional)

## Success Criteria

✅ App builds without errors
✅ Launches successfully on all emulators
✅ Can create entries with canned responses
✅ Mood selection works with icons
✅ Calendar displays correctly
✅ Streak counter increments/resets properly
✅ Data persists across restarts
✅ Settings display correctly
✅ No crashes or memory leaks

## Support

If you encounter issues:
1. Check logs: `pebble logs --emulator basalt`
2. Review DEVELOPMENT.md for debugging tips
3. Verify all files are present and correct
4. Try clean build: `pebble clean && pebble build`

---

**You're ready to build!** 🚀

Run: `pebble build && pebble install --emulator basalt`
