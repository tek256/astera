/* SPRITES EXAMPLE (Rendering & Input)

This example is meant to show how to use the various Rendering types in Astera.

CONTROLS:
W / Up Arrow - Previous Element
S / Down Arrow - Next Element
Space / Left Mouse Click - Select Element
Tab - Toggle text capture
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <astera/asset.h>
#include <astera/render.h>
#include <astera/sys.h>
#include <astera/input.h>
#include <astera/ui.h>

#define SPRITE_COUNT 16

#define BAKED_SHEET_SIZE  2048
#define BAKED_SHEET_WIDTH 64

#define BATCH_SIZE  32
#define USE_BATCHES 1

r_shader      shader, baked, particle, fbo_shader, ui_shader;
r_shader      single;
r_sprite      sprite;
r_sheet       sheet, sprite_sheet;
r_ctx*        render_ctx;
i_ctx*        input_ctx;
ui_ctx*       u_ctx;
r_baked_sheet baked_sheet;
r_particles   particles;
vec2          screen_size;

r_framebuffer fbo, ui_fbo;
r_anim        anim, anim2;

r_sprite* sprites;

s_timer timer;

r_shader load_shader(const char* vs, const char* fs) {
  asset_t* vs_data = asset_get(vs);
  asset_t* fs_data = asset_get(fs);

  r_shader shader = r_shader_create(vs_data->data, fs_data->data);

  asset_free(vs_data);
  asset_free(fs_data);

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

  particle->frame = r_particles_frame_at(system, life_span);

  particle->velocity[0] =
      (sin(progress * 3.1459) * particle->direction[0]) * 0.0075f;
  particle->velocity[1] =
      (sin(progress * 3.1459) * particle->direction[1]) * 0.0075f;
}

void init_render(r_ctx* ctx) {
  shader = load_shader("resources/shaders/instanced.vert",
                       "resources/shaders/instanced.frag");
  r_shader_cache(ctx, shader, "main");

  single = load_shader("resources/shaders/single.vert",
                       "resources/shaders/single.frag");

  r_shader_cache(ctx, single, "single");

  baked = load_shader("resources/shaders/simple.vert",
                      "resources/shaders/simple.frag");

  r_shader_cache(ctx, baked, "baked");

  particle = load_shader("resources/shaders/instanced.vert",
                         "resources/shaders/instanced.frag");
  r_shader_cache(ctx, particle, "particle");

  fbo_shader =
      load_shader("resources/shaders/fbo.vert", "resources/shaders/fbo.frag");

  ui_shader =
      load_shader("resources/shaders/fbo.vert", "resources/shaders/fbo.frag");

  screen_size[0] = 1280.f;
  screen_size[1] = 720.f;

  u_ctx = ui_ctx_create(screen_size, 1.f, 0, 1, 0);

  fbo    = r_framebuffer_create(1280, 720, fbo_shader, 0);
  ui_fbo = r_framebuffer_create(1280, 720, ui_shader, 0);

  asset_t* sheet_data = asset_get("resources/textures/tilemap.png");
  sheet = r_sheet_create_tiled(sheet_data->data, sheet_data->data_length, 16,
                               16, 0, 0);

  asset_free(sheet_data);

  asset_t* sprite_sheet_data = asset_get("resources/textures/spritesheet.png");
  sprite_sheet               = r_sheet_create_tiled(
                    sprite_sheet_data->data, sprite_sheet_data->data_length, 16, 16, 0, 0);
  asset_free(sprite_sheet_data);

  // variable time animations
  uint32_t anim_frames[4] = {0, 1, 2, 3};
  time_s   anim_times[4]  = {85.0f, 80.0f, 45.0f, 50.5f};
  anim      = r_anim_create(&sprite_sheet, anim_frames, anim_times, 4);
  anim.loop = 1;
  r_anim_cache(render_ctx, anim, "Test");

  uint32_t anim2_frames[6] = {7, 8, 9, 10, 11, 12};
  anim2      = r_anim_create_fixed(&sprite_sheet, anim2_frames, 6, 18);
  anim2.loop = 1;

  sprites                = (r_sprite*)calloc(SPRITE_COUNT, sizeof(r_sprite));
  static int SHEET_WIDTH = 128;
  vec2       sprite_size = {16.f, 16.f};

  srand(time(NULL));

  for (int i = 0; i < SPRITE_COUNT; ++i) {
    int  x = i % SHEET_WIDTH, y = i / SHEET_WIDTH;
    vec2 sprite_pos = {16.f * x, 16.f * y};

#ifdef USE_BATCHES
    sprites[i] = r_sprite_create(shader, sprite_pos, sprite_size);
#else
    sprites[i] = r_sprite_create(single, sprite_pos, sprite_size);
#endif
    sprites[i].layer = 5;
    int test         = 40 + (rand() % 20);

    sprites[i].flip_x = rand() % 2;
    sprites[i].flip_y = rand() % 2;

    int use_anim2 = rand() % 2;
    r_sprite_set_anim(&sprites[i], (use_anim2) ? &anim2 : &anim);
    // r_sprite_set_tex(&sprites[i], anim.sheet, 1);
    r_sprite_anim_play(&sprites[i]);
  }

  r_baked_quad* quads =
      (r_baked_quad*)calloc(BAKED_SHEET_SIZE, sizeof(r_baked_quad));
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

  vec2 baked_sheet_pos = {-(BAKED_SHEET_WIDTH / 4) * 16.f,
                          -((BAKED_SHEET_SIZE / BAKED_SHEET_WIDTH) / 8) * 16.f};

  baked_sheet =
      r_baked_sheet_create(&sheet, quads, BAKED_SHEET_SIZE, baked_sheet_pos);

  free(quads);

  particles =
      r_particles_create(5, 10000.f, 500, 0, PARTICLE_ANIMATED, 1, BATCH_SIZE);

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
  r_particles_set_anim(&particles, &anim);

  r_particles_set_system(&particles, 0.f, 100000.f);

  // 16x9 * 20
  vec2 camera_size = {320, 180};
  r_camera_set_size(r_ctx_get_camera(ctx), camera_size);
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
    camera_move[0] = -1.f;
  }
  if (i_key_down(input_ctx, 'D')) {
    camera_move[0] = 1.f;
  }

  if (i_key_down(input_ctx, 'W')) {
    camera_move[1] = -1.f;
  }
  if (i_key_down(input_ctx, 'S')) {
    camera_move[1] = 1.f;
  }

  vec2_scale(camera_move, camera_move, delta * 0.1f);
  r_camera_move(r_ctx_get_camera(render_ctx), camera_move);
}

void update(float delta) {
  r_particles_update(&particles, delta);
}

void render(void) {
  r_framebuffer_bind(fbo);
  r_window_clear_color_empty();
  r_window_clear();

  r_particles_draw(render_ctx, &particles, particle);

  r_ctx_update(render_ctx);

  r_baked_sheet_draw(render_ctx, baked, &baked_sheet);

  for (int i = 0; i < SPRITE_COUNT; ++i) {
    r_sprite_update(&sprites[i], 16.f);
#ifndef USE_BATCHES
    r_sprite_draw(render_ctx, &sprites[i]);
#endif
  }

#ifdef USE_BATCHES
  r_sprites_draw(render_ctx, sprites, SPRITE_COUNT);
#endif

  r_ctx_draw(render_ctx);
}

int main(void) {
  r_window_params params =
      r_window_params_create(1280, 720, 0, 0, 1, 0, 60, "Sprites Example");

  render_ctx = r_ctx_create(params, 3, 32, 128, 4);
  r_window_clear_color("#0A0A0A");

  if (!render_ctx) {
    printf("Render context failed.\n");
    return 1;
  }

  input_ctx = i_ctx_create(16, 32, 4, 1, 32);

  if (!input_ctx) {
    printf("Input context failed.\n");
    return 1;
  }

  init_render(render_ctx);

  asset_t* icon = asset_get("resources/textures/icon.png");
  r_window_set_icon(render_ctx, icon->data, icon->data_length);
  asset_free(icon);

  r_ctx_make_current(render_ctx);
  r_ctx_set_i_ctx(render_ctx, input_ctx);

  timer = s_timer_create();

  while (!r_window_should_close(render_ctx)) {
    time_s delta = s_timer_update(&timer);

    i_ctx_update(input_ctx);

    input(delta);

    update(delta);

    if (r_can_render(render_ctx)) {
      render();

      r_framebuffer_unbind();
      r_window_clear_color("#0a0a0a");
      r_window_clear();

      r_shader_bind(fbo_shader);
      r_set_uniformf(fbo_shader, "use_vig", 1.f);
      r_set_v2(fbo_shader, "resolution", screen_size);
      r_set_uniformf(fbo_shader, "vig_intensity", 16.f);
      r_set_uniformf(fbo_shader, "vig_scale", 0.1f);

      r_framebuffer_draw(render_ctx, fbo);

      r_set_uniformf(fbo_shader, "use_vig", 0.f);
      r_framebuffer_draw(render_ctx, ui_fbo);
      r_window_swap_buffers(render_ctx);
    }
  }

  r_ctx_destroy(render_ctx);
  i_ctx_destroy(input_ctx);

  return 0;
}
