#include "prompts.h"
#include "../utils/constants.h"
#include "../utils/date_utils.h"
#include <stdlib.h>

// 50 built-in gratitude prompts
static const char* PROMPTS[NUM_PROMPTS] = {
  "What made you smile today?",
  "Who are you grateful for?",
  "What brought you joy recently?",
  "What's something beautiful you saw?",
  "What made you laugh today?",
  "What comfort did you enjoy?",
  "What helped you today?",
  "What are you looking forward to?",
  "What challenge did you overcome?",
  "What kindness did you receive?",
  "What music lifted your spirits?",
  "What food did you savor?",
  "What nature did you appreciate?",
  "What learning excited you?",
  "What rest did you enjoy?",
  "What creativity inspired you?",
  "What friendship warmed your heart?",
  "What family moment was special?",
  "What small thing made a difference?",
  "What gift did you receive?",
  "What opportunity came your way?",
  "What freedom do you cherish?",
  "What health are you thankful for?",
  "What shelter protects you?",
  "What technology helps you?",
  "What book enriched you?",
  "What conversation mattered?",
  "What memory makes you happy?",
  "What pet brought you joy?",
  "What hobby fulfills you?",
  "What weather did you enjoy?",
  "What drink refreshed you?",
  "What exercise energized you?",
  "What sleep restored you?",
  "What safety do you have?",
  "What choice could you make?",
  "What lesson did you learn?",
  "What problem got solved?",
  "What peace did you find?",
  "What surprise delighted you?",
  "What tradition do you value?",
  "What community supports you?",
  "What progress did you make?",
  "What beauty did you create?",
  "What wisdom did you gain?",
  "What courage did you show?",
  "What patience paid off?",
  "What hope inspires you?",
  "What love surrounds you?",
  "What moment was perfect?"
};

const char* prompts_get_daily(void) {
  bool is_sequential = prompts_get_mode();
  time_t today = date_get_today();

  if (is_sequential) {
    // Sequential mode: increment daily
    uint8_t index = 0;
    if (persist_exists(STORAGE_KEY_PROMPT_INDEX)) {
      index = (uint8_t)persist_read_int(STORAGE_KEY_PROMPT_INDEX);
    }

    // Check if we need to advance (new day)
    static time_t last_prompt_date = 0;
    if (last_prompt_date == 0 || !date_is_same_day(last_prompt_date, today)) {
      index = (index + 1) % NUM_PROMPTS;
      persist_write_int(STORAGE_KEY_PROMPT_INDEX, index);
      last_prompt_date = today;
    }

    return PROMPTS[index];
  } else {
    // Random mode: seed with day number for consistency
    struct tm *time_info = localtime(&today);
    uint32_t day_number = time_info->tm_yday + (time_info->tm_year * 365);

    // Simple pseudo-random based on day number
    uint8_t index = day_number % NUM_PROMPTS;
    return PROMPTS[index];
  }
}

void prompts_set_mode(bool is_sequential) {
  persist_write_int(STORAGE_KEY_PROMPT_MODE, is_sequential ? 1 : 0);
}

bool prompts_get_mode(void) {
  if (!persist_exists(STORAGE_KEY_PROMPT_MODE)) {
    return false;  // Default to random
  }
  return persist_read_int(STORAGE_KEY_PROMPT_MODE) != 0;
}

uint8_t prompts_get_count(void) {
  return NUM_PROMPTS;
}
