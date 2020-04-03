/* Audio Example - Astera
   Last Updated: Feb 5 2020
   Last Author: tek256

   Feb 5 2020:
        - Created example
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <astera/debug.h>

#include <astera/asset.h>
#include <astera/audio.h>
#include <astera/input.h>
#include <astera/render.h> // For the window management & rendering
#include <astera/sys.h>    // For the time definitions
#include <astera/ui.h>

#include "ui_utils.h"

static float volume = 1.f;

static a_music* music;
static a_req    music_req;

int8_t setup_render() {
  return 1;
}

int8_t setup_audio() {
  if (!a_init()) {
    return 0;
  }
  return 1;
}

int8_t load_audio() {
  asset_t* music_data = asset_get("../resources/music/test_song.ogg");
  music =
      a_music_create(music_data->data, music_data->data_length, 0, &music_req);
  return 1;
}

void setup_ui(void) {
}

void update(time_s delta) {
  // Update astera's audio system
  a_update(delta);

  // Update the UI System for controlling everything

  // Use astera's input system to look for key pressed
  if (i_key_clicked(KEY_PLUS)) {
    volume -= 0.1f;
    a_set_vol(volume);
  } else if (i_key_clicked(KEY_MINUS)) {
    volume -= 0.1f;
    a_set_vol(volume);
  }

  // Play / Pause the music
  if (i_key_clicked(KEY_SPACE)) {
    if (!req->playing) {
    }
  }
}

void render() {
}

int main(int argc, char** argv) {
  if (!setup_render()) {
    printf("Error: Unable to setup the rendering system.\n");
    return EXIT_FAILURE;
  }

  if (!setup_audio()) {
    printf("Error: Unable to setup the audio system.\n");
    return EXIT_FAILURE;
  }

  if (!load_audio()) {
    printf("Error: Unable to load audio files.\n");
    return EXIT_FAILURE;
  }

  setup_ui();

  int8_t running = 1;

  while (running) {
    update(delta);
    render();
  }

  return 0;
}
