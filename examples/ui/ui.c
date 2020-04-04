/* Input Example - Astera
   Last Updated: Feb 5 2020
   Last Author: tek256

   Feb 5 2020:
        - Created example
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ASTERA_DEBUG_OUTPUT
#include <astera/debug.h>
#include <astera/input.h> // For the keyboard, mouse, and controller usage
#include <astera/sys.h>   // For the time definitions
#include <astera/ui.h>    // For the UI usage

int8_t init() {
  r_window_info window_info =
      r_window_info_create(1280, 720, "Astera 0.01 - UI Example", -1, 1, 0, 0);
  if (!r_init(window_info)) {
    _l("Unable to initialize render system.\n");
    return 0;
  }

  if (!i_init()) {
    _l("Unable to initialize input system.\n");
    return 0;
  }

  return 1;
}

int8_t create_ui() {
  return 1;
}

void update(time_s delta) {
}

void draw_ui() {
  //
}

void input_ui() {
}

void input() {
  if (i_key_clicked(KEY_ESCAPE)) {
    running = 0;
    return;
  }

  input_ui();
}

void render() {
  r_window_clear();
  draw_ui();
  r_window_swap_buffers();
}

int main(int argc, char** argv) {
  printf("Hello.\n");

  int8_t running = init();

  while (running) {
    r_poll_events();
    input();

    update(delta);
    render();

    i_update();
  }

  return EXIT_SUCCESS;
}
