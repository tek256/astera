#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <astera/asset.h>
#include <astera/render.h>
#include <astera/input.h>
#include <astera/ui.h>

#define SPRITE_COUNT      16
#define BAKED_SHEET_SIZE  16 * 16 * 16
#define BAKED_SHEET_WIDTH 16

// Well I guess all that's left in rendering is the particle system now
// and then post processing, but yeah.
// Nice

r_shader      shader, baked, particle, fbo_shader;
r_sprite      sprite;
r_sheet       sheet;
r_ctx*        render_ctx;
i_ctx*        input_ctx;
r_baked_sheet baked_sheet;
r_particles   particles;

ui_ctx* ui_context;

r_framebuffer fbo, ui_fbo;
r_anim        anim;

r_sprite* sprites;

r_shader load_shader(const char* vs, const char* fs) {
  asset_t* vs_data = asset_get(vs);
  asset_t* fs_data = asset_get(fs);

  r_shader shader = r_shader_create(vs_data->data, vs_data->data_length,
                                    fs_data->data, fs_data->data_length);

  asset_free(vs_data);
  asset_free(fs_data);

  free(vs_data);
  free(fs_data);

  return shader;
}

enum { LEFT = 1 << 1, RIGHT = 1 << 2, UP = 1 << 3, DOWN = 1 << 4 } dir;

static int rnd(int min, int max) {
  int range = max - min;
  int value = rand() % range;
  return min + value;
}

void particle_spawn(r_particles* system, r_particle* particle) {
  float x = fmod(rand(), system->size[0]);
  float y = fmod(rand(), system->size[1]);

  particle->position[0] = x;
  particle->position[1] = y;
  particle->layer       = 10;

  int dir_x = 0, dir_y = 0;

  dir_x = rnd(-2, 2);
  dir_y = rnd(-2, 2);

  particle->direction[0] = dir_x;
  particle->direction[1] = dir_y;
}

void particle_animate(r_particles* system, r_particle* particle) {
  float life_span = system->particle_life - particle->life;
  float progress  = life_span / system->particle_life;

  particle->frame = life_span / (MS_TO_SEC / system->render.anim.rate);

  particle->velocity[0] =
      (sin(progress * 3.1459) * particle->direction[0]) * 0.0075f;
  particle->velocity[1] =
      (sin(progress * 3.1459) * particle->direction[1]) * 0.0075f;
}

void init_render(r_ctx* ctx) {
  shader =
      load_shader("resources/shaders/main.vert", "resources/shaders/main.frag");
  r_shader_cache(ctx, shader, "main");

  baked = load_shader("resources/shaders/basic.vert",
                      "resources/shaders/basic.frag");
  r_shader_cache(ctx, baked, "baked");

  particle = load_shader("resources/shaders/particles.vert",
                         "resources/shaders/particles.frag");
  r_shader_cache(ctx, particle, "particle");

  fbo_shader = load_shader("resources/shaders/screen.vert",
                           "resources/shaders/screen.frag");

  fbo    = r_framebuffer_create(1280, 720, fbo_shader);
  ui_fbo = r_framebuffer_create(1280, 720, fbo_shader);

  asset_t* sheet_data = asset_get("resources/textures/Dungeon_Tileset.png");
  sheet = r_sheet_create_tiled(sheet_data->data, sheet_data->data_length, 16,
                               16, 0, 0);

  asset_free(sheet_data);
  free(sheet_data);

  uint32_t anim_frames[6] = {0, 1, 2, 3, 4, 5};
  anim                    = r_anim_create(&sheet, anim_frames, 6, 6);
  anim.loop               = 1;

  sprites                = (r_sprite*)malloc(sizeof(r_sprite) * SPRITE_COUNT);
  static int SHEET_WIDTH = 128;
  vec2       sprite_size = {16.f, 16.f};

  for (int i = 0; i < SPRITE_COUNT; ++i) {
    int  x = i % SHEET_WIDTH, y = i / SHEET_WIDTH;
    vec2 sprite_pos = {16.f * x, 16.f * y};

    sprites[i]       = r_sprite_create(shader, sprite_pos, sprite_size);
    sprites[i].layer = 5;
    int test         = 40 + (rand() % 20);
    // r_sprite_set_tex(&sprites[i], &sheet, test);
    r_sprite_set_anim(&sprites[i], anim);
    r_sprite_anim_play(&sprites[i]);
  }

  r_baked_quad* quads =
      (r_baked_quad*)malloc(sizeof(r_baked_quad) * BAKED_SHEET_SIZE);
  for (int i = 0; i < BAKED_SHEET_SIZE; ++i) {
    int texture = rand() % 32;
    int flip_x  = rand() % 2;
    int flip_y  = rand() % 2;
    int x       = i % BAKED_SHEET_WIDTH;
    int y       = i / BAKED_SHEET_WIDTH;

    quads[i] = (r_baked_quad){.x      = (float)x * 16.f,
                              .y      = (float)y * 16.f,
                              .width  = 16.f,
                              .height = 16.f,
                              .subtex = texture,
                              .layer  = 0,
                              .flip_x = flip_x,
                              .flip_y = flip_y};
  }

  vec2 baked_sheet_pos = {0.f, 0.f};
  baked_sheet =
      r_baked_sheet_create(&sheet, quads, BAKED_SHEET_SIZE, baked_sheet_pos);

  free(quads);

  particles =
      r_particles_create(100, 10000.f, 500, 0, PARTICLE_ANIMATED, 1, 512);

  r_particles_set_spawner(&particles, particle_spawn);
  r_particles_set_animator(&particles, particle_animate);
  srand(time(NULL));

  vec4 particle_color;
  vec2 particle_size     = {16.f, 16.f};
  vec2 particle_velocity = {0.2f, 0.2f};
  vec2 system_size       = {128.f, 128.f};
  vec2 system_pos        = {64.f, 64.f};

  particles.particle_layer = 5;
  vec2_dup(particles.position, system_pos);
  vec2_dup(particles.size, system_size);

  r_get_color4f(particle_color, "1FDEA4");

  r_particles_set_particle(&particles, particle_color, 0.f, particle_size,
                           particle_velocity);
  r_particles_set_anim(&particles, anim);
  // r_particles_set_subtex(&particles, &sheet, 1);

  // 16x9 * 20
  vec2 camera_size = {320, 180};
  r_camera_set_size(r_ctx_get_camera(ctx), camera_size);
}

void init_ui() {
  vec2 size  = {1280, 720};
  ui_context = ui_ctx_create(size, 1.f, 1, 1);
}

void input(float delta) {
  if (i_key_clicked(input_ctx, KEY_ESCAPE)) {
    r_window_request_close(render_ctx);
  }

  if (i_key_clicked(input_ctx, 'P')) {
    printf("particles: %i\n", particles.count);
  }

  vec3 camera_move = {0.f, 0.f, 0.f};

  if (i_key_down(input_ctx, 'A')) {
    camera_move[0] = 1.f;
  }
  if (i_key_down(input_ctx, 'D')) {
    camera_move[0] = -1.f;
  }

  if (i_key_down(input_ctx, 'W')) {
    camera_move[1] = 1.f;
  }
  if (i_key_down(input_ctx, 'S')) {
    camera_move[1] = -1.f;
  }

  // vec3_scale(camera_move, camera_move, delta * 0.1f);
  r_camera_move(r_ctx_get_camera(render_ctx), camera_move);
}

void update(float delta) { r_particles_update(&particles, delta); }

int main(void) {
  r_window_params params =
      r_window_params_create(1280, 720, 0, 0, 1, 0, 0, "Basic Game");

  render_ctx = r_ctx_create(params, 0, 3, 128, 128, 4);
  r_window_clear_color("#0A0A0A");

  if (!render_ctx) {
    printf("Render context failed.\n");
    return 1;
  }

  input_ctx = i_ctx_create(16, 16, 0, 8, 16, 32);

  if (!input_ctx) {
    printf("Input context failed.\n");
    return 1;
  }

  printf("Loading icon\n");

  asset_t* icon = asset_get("resources/textures/icon.png");

  init_render(render_ctx);

  r_window_set_icon(render_ctx, icon->data, icon->data_length);
  printf("Set icon.\n");

  r_ctx_make_current(render_ctx);
  r_ctx_set_i_ctx(render_ctx, input_ctx);

  printf("Everything happened fine.\n");

  while (!r_window_should_close(render_ctx)) {
    i_ctx_update(input_ctx);
    i_poll_events();

    input(16.f);

    update(16.f);

    if (r_can_render(render_ctx)) {
      r_framebuffer_bind(fbo);
      r_window_clear();

      r_particles_draw(render_ctx, &particles, particle);

      r_ctx_update(render_ctx);

      r_baked_sheet_draw(render_ctx, baked, &baked_sheet);

      for (int i = 0; i < SPRITE_COUNT; ++i) {
        r_sprite_update(&sprites[i], 16.f);
        r_sprite_draw(render_ctx, &sprites[i]);
      }

      r_ctx_draw(render_ctx);

      glViewport(0, 0, 1280, 720);
      r_framebuffer_draw(render_ctx, fbo);
      r_window_swap_buffers(render_ctx);
    }
  }

  r_ctx_destroy(render_ctx);
  printf("Exited correctly.\n");
  return 0;
}
