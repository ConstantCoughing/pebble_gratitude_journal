#include <pebble.h>
#include "data/storage.h"
#include "windows/home_window.h"

static void init(void) {
  // Initialize storage
  storage_init();

  // Show home window
  home_window_push();
}

static void deinit(void) {
  // Cleanup windows
  home_window_destroy();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
