#include <misc/linmath.h>

#include "game.h"

#include "audio.h"
#include "conf.h"
#include "debug.h"
#include "input.h"
#include "render.h"
#include "sys.h"

#include "asset.h"

typedef struct {
} g_entity;

g_entity ent;

r_sprite sprite, sprite2;
u32 *frames;

static int tex_ids, models, flip_x, flip_y;

r_particles particles;
r_shader particle_shader;

int g_init(void) {
  r_init_anim_map(32);
  r_init_shader_map(2);
  r_init_batches(4);

  asset_t *window_icon = asset_get("sys", "res/tex/icon.png");
  r_window_set_icon(window_icon);
  asset_free(window_icon);
  window_icon = 0;

  // Initialize default shader
  asset_t *default_vert = asset_get("sys", "res/shd/main.vert");
  asset_t *default_frag = asset_get("sys", "res/shd/main.frag");
  r_shader default_shader = r_shader_create(default_vert, default_frag);
  asset_free(default_vert);
  asset_free(default_frag);

  // Initialize the particle shader
  asset_t *particle_vert = asset_get("sys", "res/shd/particle.vert");
  asset_t *particle_frag = asset_get("sys", "res/shd/particle.frag");
  particle_shader = r_shader_create(particle_vert, particle_frag);
  asset_free(particle_vert);
  asset_free(particle_frag);

  // Initialize default texture sheet
  asset_t *default_sheet_file = asset_get("sys", "res/tex/test_sheet.png");
  r_sheet default_sheet = r_sheet_create(default_sheet_file, 16, 16);
  asset_free(default_sheet_file);

  asset_t *particle_sheet_file = asset_get("sys", "res/tex/particle_sheet.png");
  r_sheet particle_sheet = r_sheet_create(particle_sheet_file, 16, 16);
  asset_free(particle_sheet_file);

  // r_anim r_anim_create(r_sheet sheet, u32 *frames, int frame_count,
  //                  int frame_rate);

  // TODO make this not terrible. (It's gotten better)
  // Let me look at the picture and see if I'm actually just stupid
  // Pointers are silly
  frames = malloc(sizeof(u32) * 4);
  frames[0] = 0;
  frames[1] = 1;
  frames[2] = 2;
  frames[3] = 3;

  // frames = (u32 *){0, 1, 2, 3, 4};

  r_anim anim = r_anim_create(default_sheet, frames, 4, 21);
  anim.loop = 1;
  r_anim_cache(anim, "default");

  // It just so happens that we made 4 frames for the particle animation too, so
  // we can just reuse the frame array, otherwise it's ill advised
  // We'll get texture rendering working first, then move to animations
  r_anim particle_anim = r_anim_create(particle_sheet, frames, 4, 21);
  particle_anim.loop = 1;

  // Initialize Uniform Arrays
  flip_x = r_shader_setup_array(default_shader, "flip_x", 512, r_int);
  flip_y = r_shader_setup_array(default_shader, "flip_y", 512, r_int);
  models = r_shader_setup_array(default_shader, "models", 512, r_mat);
  tex_ids = r_shader_setup_array(default_shader, "tex_ids", 512, r_int);

  vec2 size, position, position2;
  size[0] = 100.f;
  size[1] = 100.f;

  position2[0] = 100.f;

  r_subtex sub_tex = (r_subtex){default_sheet, 1};
  r_subtex sub_tex2 = (r_subtex){default_sheet, 6};

  r_subtex particle_tex = (r_subtex){particle_sheet, 0};

  sprite = r_sprite_create(default_shader, position, size);
  sprite2 = r_sprite_create(default_shader, position2, size);
  r_sprite_set_tex(&sprite, sub_tex);
  r_sprite_set_anim(&sprite2, anim);

  r_particles_init(&particles, 128);
  particles.max_emission = 0;
  particles.position[0] = 0.f;
  particles.position[1] = 0.f;

  particles.size[0] = 1280.0f;
  particles.size[1] = 720.0f;

  particles.velocity[0] = 0.f;
  particles.velocity[1] = 9.f;
  particles.render.anim = particle_anim;

  particles.particle_size[0] = 10.f;
  particles.particle_size[1] = 10.f;

  particles.particle_life = 4.0f;

  particles.spawn_type = CIRCLE;
  particles.spawn_rate = 20;
  particles.colored = 0;
  particles.animated = 1;
  particles.texture = 0;

  particles.color[0] = 1.f;
  particles.color[1] = 1.f;
  particles.color[2] = 1.f;
  particles.color[3] = 1.f;

  particles.size_frames = r_keyframes_create(3);
  r_keyframes_set(&particles.size_frames, 0, 0.2f, 10.f, LINEAR);
  r_keyframes_set(&particles.size_frames, 1, 1.f, 25.f, LINEAR);
  r_keyframes_set(&particles.size_frames, 2, 2.f, 10.f, LINEAR);

  particles.fade_frames = r_keyframes_create(4);
  r_keyframes_set(&particles.fade_frames, 0, 0.0f, 0.0f, LINEAR);
  r_keyframes_set(&particles.fade_frames, 1, 0.2f, 1.0f, LINEAR);
  r_keyframes_set(&particles.fade_frames, 2, 1.5f, 1.0f, LINEAR);
  r_keyframes_set(&particles.fade_frames, 3, 1.7f, 0.0f, LINEAR);

  return 1;
}

void g_exit(void) {}

void g_input(long delta) {
  if (i_key_clicked(GLFW_KEY_ESCAPE))
    r_window_request_close();

  int dir_x = 0;
  int dir_y = 0;
  if (i_key_down('A'))
    dir_x = -1;
  else if (i_key_down('D'))
    dir_x = 1;

  if (i_key_down('W'))
    dir_y = 1;
  else if (i_key_down('S'))
    dir_y = -1;

  if (i_key_clicked('P'))
    r_sprite_play(&sprite2);
  else if (i_key_clicked('O'))
    r_sprite_pause(&sprite2);

  r_cam_move(dir_x * 1.f * delta, dir_y * 1.f * delta);
}

void g_update(long delta) {
  r_sprite_update(&sprite, delta);
  r_sprite_update(&sprite2, delta);
  r_particles_update(&particles, delta);
}

void g_render(long delta) {
  r_sprite_draw(sprite);
  int tex_id = r_sprite_get_tex_id(sprite);

  r_shader_sprite_uniform(sprite, tex_ids, &tex_id);
  r_shader_sprite_uniform(sprite, models, &sprite.model);
  r_shader_sprite_uniform(sprite, flip_x, &sprite.flip_x);
  r_shader_sprite_uniform(sprite, flip_y, &sprite.flip_y);

  tex_id = r_sprite_get_tex_id(sprite2);

  r_sprite_draw(sprite2);
  r_shader_sprite_uniform(sprite2, tex_ids, &tex_id);
  r_shader_sprite_uniform(sprite2, models, &sprite2.model);
  r_shader_sprite_uniform(sprite2, flip_x, &sprite2.flip_x);
  r_shader_sprite_uniform(sprite2, flip_y, &sprite2.flip_y);

  r_particles_draw(&particles, particle_shader);
}
