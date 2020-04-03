/* Render Stress Example - Astera
   Last Updated: Feb 5 2020
   Last Author: tek256

   Feb 5 2020:
        - Created example
   April 2, 2020:
        - Actually create the example
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <astera/audio.h>  // For the uhh, audio stuff.
#include <astera/input.h>  // For the input / controller support
#include <astera/render.h> // For the window management & rendering
#include <astera/sys.h>    // For the time definitions

#define AUDIO_FPS 30

int game_tick = 0;

static time_s audio_fps_length;

time_s audio_timer;

int8_t setup_audio() {
  // Note: this allows for us to use less cpu
  // since the update duration for audio positions
  // past 30 fps isn't too terribly noticable
  // Tho, with the macro in place, it's easy enough to adjust
  audio_fps_length = MS_PER_SEC / AUDIO_FPS;
  return 1;
}

int8_t setup_render() {
  r_window_info window_info;

  if (!asset_exists("config.ini")) {
  }

  r_init();

  r_window_clear_color("0A0A0A");
  return 1;
}

void audio_update(time_s delta) {
  audio_timer += delta;
  if (audio_timer >= audio_fps_length) {
    audio_timer -= audio_fps_length;
  } else {
    return;
  }

  a_update(audio_fps_length);
}

void render_update(time_s delta) {
}

void game_update(time_s delta) {
  r_poll_events();
}

void update(time_s delta) {
  ++game_tick;
}

void render() {
  r_window_clear();

  r_poll_events();
}

int main(int argc, char** argv) {
  if (!setup_render()) {
    printf("Error: Unable to setup the rendering system.\n");
    return EXIT_FAILURE;
  }

  int8_t running = 1;

  while (running) {
    update(delta);
    render();
  }

  return 0;
}
