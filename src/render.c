#include <glad/glad_gl.c>

#include "render.h"

#include <math.h>
#include <string.h>

#define STBI_NO_BMP
#define STBI_NO_TGA
#define STBI_NO_JPEG
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_HDR

#define STB_IMAGE_IMPLEMENTATION
#include <misc/stb_image.h>

#include "debug.h"
#include "sys.h"

static r_flags flags;

static GLFWmonitor *r_default_monitor;
static const GLFWvidmode *r_vidmodes;
static vec2 r_res;
static uint32_t default_quad_vao, default_quad_vbo, default_quad_vboi;

static r_anim_map g_anim_map;
static r_shader_map g_shader_map;

#ifndef CUSTOM_GLFW_CALLBACKS

#include "input.h"
static void glfw_err_cb(int error, const char *msg) {
  _e("ERROR: %i %s\n", error, msg);
}

static void glfw_window_pos_cb(GLFWwindow *window, int x, int y) {
  if (g_window.glfw == window) {
    g_window.x = x;
    g_window.y = y;
  }
}

static void glfw_window_size_cb(GLFWwindow *window, int w, int h) {
  if (g_window.glfw == window) {
    g_window.width = w;
    g_window.height = h;
    glViewport(0, 0, w, h);
    flags.scaled = 1;
    i_set_screensize(w, h);
  }
}

static void glfw_window_close_cb(GLFWwindow *window) {
  if (g_window.glfw == window)
    g_window.close_requested = 1;
}

static void glfw_key_cb(GLFWwindow *window, int key, int scancode, int action,
                        int mods) {
  if (g_window.glfw != window)
    return;

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    i_key_callback(key, scancode, 1);
    if (i_key_binding_track()) {
      i_binding_track_callback(key, BINDING_KEY);
    }
  } else if (action == GLFW_RELEASE) {
    i_key_callback(key, scancode, 0);
  }
}

static void glfw_char_cb(GLFWwindow *window, uint32_t c) {
  if (window == g_window.glfw)
    i_char_callback(c);
}

static void glfw_mouse_pos_cb(GLFWwindow *window, double x, double y) {
  if (window == g_window.glfw)
    i_mouse_pos_callback(x, y);
}

static void glfw_mouse_button_cb(GLFWwindow *window, int button, int action,
                                 int mods) {
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    i_mouse_button_callback(button);
    if (i_key_binding_track()) {
      i_binding_track_callback(button, BINDING_MB);
    }
  }
}

static void glfw_scroll_cb(GLFWwindow *window, double dx, double dy) {
  if (g_window.glfw == window)
    i_mouse_scroll_callback(dx, dy);
}

static void glfw_joy_cb(int joystick, int action) {
  if (action == GLFW_CONNECTED) {
    i_create_joy(joystick);
  } else if (action == GLFW_DISCONNECTED) {
    i_destroy_joy(joystick);
  }
}

#endif
void r_init_anim_map(int size) {
  r_anim_map anim_map;

  anim_map.anims = (r_anim *)malloc(sizeof(r_anim) * size);
  anim_map.names = (const char **)malloc(sizeof(char *) * size);
  anim_map.capacity = size;
  anim_map.count = 0;

  if (!anim_map.anims || !anim_map.names) {
    _e("Unable to initialize the anim map with a size of %i.\n", size);
  } else {
    g_anim_map = anim_map;
  }
}

void r_init_shader_map(int size) {
  r_shader_map shader_map;

  shader_map.shaders = (r_shader *)malloc(sizeof(r_shader) * size);
  shader_map.names = (const char **)malloc(sizeof(char *) * size);
  shader_map.uniform_maps =
      (r_uniform_map *)malloc(sizeof(r_uniform_map) * size);
  shader_map.capacity = size;

  if (!shader_map.shaders || !shader_map.names || !shader_map.uniform_maps) {
    _e("Unable to initialize shader map with size of %i.\n", size);
  } else {
    g_shader_map = shader_map;
  }
}

void r_init_batches(int size) {
  r_shader_batch *batches;

  batches = (r_shader_batch *)malloc(sizeof(r_shader_batch) * size);

  if (!batches) {
    _e("Unable to initialize batches with a size of %i.\n", size);
  } else {
    for (int i = 0; i < size; ++i) {
      batches[i].used = 0;
      batches[i].shader = 0;
      batches[i].sheet = (r_sheet){0};
    }

    g_shader_map.batches = batches;
    g_shader_map.batch_capacity = size;
  }
}

int r_init(r_window_info info) {
  if (!r_window_create(info)) {
    _e("Unable to create window.\n");
    return 0;
  }

  r_window_center();

  r_cam_create(&g_camera, (vec2){r_res[0], r_res[1]}, (vec2){0.f, 0.f});

  float verts[16] = {-0.5f, -0.5f, 0.f, 0.f, -0.5f, 0.5f,  0.f, 1.f,
                     0.5f,  0.5f,  1.f, 1.f, 0.5f,  -0.5f, 1.f, 0.f};

  uint16_t inds[6] = {0, 1, 2, 2, 3, 0};

  glGenVertexArrays(1, &default_quad_vao);
  glGenBuffers(1, &default_quad_vbo);
  glGenBuffers(1, &default_quad_vboi);

  glBindVertexArray(default_quad_vao);

  glBindBuffer(GL_ARRAY_BUFFER, default_quad_vbo);
  glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), &verts[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, default_quad_vboi);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint16_t), &inds[0],
               GL_STATIC_DRAW);

  glBindVertexArray(0);

  return 1;
}

void r_update(void) { r_cam_update(); }

void r_end(void) {
  for (int i = 0; i < g_shader_map.batch_capacity; ++i) {
    if (g_shader_map.batches[i].used) {
      r_batch_draw(&g_shader_map.batches[i]);
    }
  }
}

void r_exit(void) {
  for (int i = 0; i < g_anim_map.capacity; ++i) {
    if (g_anim_map.anims[i].frames)
      free(g_anim_map.anims[i].frames);
  }

  glDeleteVertexArrays(1, &default_quad_vao);

  r_window_destroy();
  glfwTerminate();
}

void r_cam_create(r_camera *cam, vec2 size, vec2 position) {
  cam->pos[0] = position[0];
  cam->pos[1] = position[1];
  cam->pos[2] = position[2];

  cam->size[0] = size[0];
  cam->size[1] = size[1];

  cam->near = -10.f;
  cam->far = 10.f;

  float x, y;
  x = floorf(cam->pos[0]);
  y = floorf(cam->pos[1]);

  mat4x4_ortho(cam->proj, 0, cam->size[0], cam->size[1], 0, cam->near,
               cam->far);

  mat4x4_translate(cam->view, x, y, 0.f);
  mat4x4_rotate_x(cam->view, cam->view, 0.0);
  mat4x4_rotate_y(cam->view, cam->view, 0.0);
  mat4x4_rotate_z(cam->view, cam->view, 0.0);
}

void r_cam_move(float x, float y) {
  g_camera.pos[0] -= x;
  g_camera.pos[1] += y;
}

void r_cam_get_size(float *width, float *height) {
  if (width)
    *width = g_camera.size[0];
  if (height)
    *height = g_camera.size[1];
}

void r_cam_set_size(float width, float height) {
  g_camera.size[0] = width;
  g_camera.size[1] = height;
  mat4x4_ortho(g_camera.proj, 0, g_camera.size[0], g_camera.size[1], 0,
               g_camera.near, g_camera.far);
}

void r_cam_update(void) {
  /*float x, y;
  x = floorf(g_camera.pos[0]);
  y = floorf(g_camera.pos[1]);*/
  mat4x4_identity(g_camera.view);
  mat4x4_translate(g_camera.view, g_camera.pos[0], g_camera.pos[1], 0.f);
}

r_framebuffer r_framebuffer_create(uint32_t width, uint32_t height,
                                   r_shader shader) {
  r_framebuffer fbo;
  fbo.width = width;
  fbo.height = height;
  fbo.shader = shader;

  glGenFramebuffers(1, &fbo.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);

  glGenTextures(1, &fbo.tex);
  glBindTexture(GL_TEXTURE_2D, fbo.tex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
               GL_FLOAT, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         fbo.tex, 0);

  glGenRenderbuffers(1, &fbo.rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, fbo.rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, fbo.rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    _e("Incomplete FBO: %i\n", fbo.fbo);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  float verts[] = {-1.0f, 1.0f,  0.0f,  1.0f,  1.0f,  1.0f,
                   1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  0.0f,

                   1.0f,  -1.0f, 1.0f,  0.0f,  -1.0f, -1.0f,
                   0.0f,  0.0f,  -1.0f, 1.0f,  0.0f,  1.0f};

  glGenVertexArrays(1, &fbo.vao);
  glGenBuffers(1, &fbo.vbo);

  glBindVertexArray(fbo.vao);

  glBindBuffer(GL_ARRAY_BUFFER, fbo.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0);

  return fbo;
}

void r_framebuffer_destroy(r_framebuffer fbo) {
  glDeleteFramebuffers(1, &fbo.fbo);
  glDeleteTextures(1, &fbo.tex);
  glDeleteBuffers(1, &fbo.vbo);
  glDeleteBuffers(1, &fbo.vto);
  glDeleteVertexArrays(1, &fbo.vao);
}

void r_framebuffer_bind(r_framebuffer fbo) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
}

void r_framebuffer_draw(r_framebuffer fbo) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // TODO test these states
  glDisable(GL_DEPTH_TEST);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindVertexArray(fbo.vao);
  glUseProgram(fbo.shader);

  r_set_uniformf(fbo.shader, "gamma", g_window.gamma);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, fbo.tex);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindVertexArray(0);
  glUseProgram(0);

  glEnable(GL_DEPTH_TEST);
}

void r_tex_bind(uint32_t tex) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
}

r_tex r_tex_create(asset_t *asset) {
  int w, h, ch;
  unsigned char *img =
      stbi_load_from_memory(asset->data, asset->data_length, &w, &h, &ch, 0);
  uint32_t id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               img);

  stbi_image_free(img);
  asset->req_free = 1;
  return (r_tex){id, (uint32_t)w, (uint32_t)h};
}

r_sheet r_sheet_create(asset_t *asset, uint32_t sub_width,
                       uint32_t sub_height) {
  int w, h, ch;
  unsigned char *img =
      stbi_load_from_memory(asset->data, asset->data_length, &w, &h, &ch, 0);
  uint32_t id;

  int format = (ch == 4) ? GL_RGBA : (ch == 3) ? GL_RGB : GL_RGB;

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  int count = (w / sub_width) * (h / sub_height);

  // glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, sub_width, sub_height,
  // count,
  //             0, GL_RGBA, GL_UNSIGNED_BYTE, img);

  /*glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, sub_width, sub_height,
                 count);

  int per_width = w / sub_width;
  for (int i = 0; i < count; ++i) {
    int row = i / per_width;
    int col = i % per_width;

    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, i * col * sub_width,
                    i * row * sub_width, 0, 0, sub_width, sub_height, i, format,
                    GL_UNSIGNED_BYTE, img);
  }*/
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               img);

  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(img);
  asset->req_free = 1;

  return (r_sheet){id, w, h, sub_width, sub_height};
}

void r_sheet_bind(uint32_t sheet) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, sheet);
}

float r_keyframe_get_value(r_keyframes frames, float point) {
  int start_index = -1;
  for (int i = 0; i < frames.count; ++i) {
    if (frames.list[i].point >= point) {
      start_index = i - 1;
      break;
    }
  }

  if (start_index < 0) {
    return frames.list[0].value;
  }

  if (frames.count - 1 == start_index) {
    return frames.list[start_index].value;
  }

  switch (frames.list[start_index].curve) {
  case CURVE_LINEAR: {
    float point_dist =
        frames.list[start_index + 1].point - frames.list[start_index].point;
    float value_dist =
        frames.list[start_index + 1].value - frames.list[start_index].value;
    float avg = (point - frames.list[start_index].point) / point_dist;
    return frames.list[start_index].value + (value_dist * avg);
  }
    /// TODO Easing
  case CURVE_EASE_IN:
    return frames.list[start_index].value;
  case CURVE_EASE_EASE:
    return frames.list[start_index].value;
  case CURVE_EASE_OUT:
    return frames.list[start_index].value;
  default:
    return frames.list[start_index].value;
  }
}

r_keyframes r_keyframes_create(int keyframe_count) {
  r_keyframes frames;
  frames.count = keyframe_count;
  frames.list = malloc(sizeof(r_keyframe) * keyframe_count);
  memset(frames.list, 0, sizeof(r_keyframe) * keyframe_count);
  return frames;
}

void r_keyframes_destroy(r_keyframes *frames) {
  if (frames->list) {
    free(frames->list);
    frames->list = 0;
  }
  frames->count = 0;
}

void r_keyframes_set(r_keyframes *frames, int frame, float point, float value,
                     r_keyframe_curve curve) {
  if (frame < 0 || frames->count < frame)
    _l("Invalid keyframe: %i in list.\n", frame);
  frames->list[frame].point = point;
  frames->list[frame].value = value;
  frames->list[frame].curve = curve;
}

void r_particles_init(r_particles *system, unsigned int particle_capacity) {
  if (system->list) {
    free(system->list);
  }

  if (system->max_emission > 1 && system->max_emission < particle_capacity) {
    particle_capacity = system->max_emission;
    system->capacity = particle_capacity;
  }

  r_particle *particles =
      (r_particle *)malloc(sizeof(r_particle) * particle_capacity);
  if (!particles) {
    _e("Unable to allocate sizeo of %i for a particle list.\n",
       (sizeof(r_particle) * particle_capacity));
    system->capacity = 0;
    system->list = 0;
    return;
  }

  for (int i = 0; i < particle_capacity; ++i) {
    particles[i].alive = 0;
    particles[i].reused = 0;
  }

  system->layer = 0;
  system->array_count = 0;
  system->count = 0;
  system->max_emission = 0;
  system->emission_count = 0;
  system->dir_type = DIR_NONE;

  system->animated = 0;
  system->texture = 0;
  system->colored = 0;

  system->valid_uniforms = 0;
  system->system_life = 0;

  system->position[0] = 0.f;
  system->position[1] = 0.f;

  system->velocity[0] = 0.f;
  system->velocity[1] = 0.f;

  system->particle_size[0] = 0.f;
  system->particle_size[1] = 0.f;

  system->list = particles;
  system->capacity = particle_capacity;
  system->time = 0;
}

void r_particles_update(r_particles *system, double delta) {
  system->time += delta;
  double rate = (MS_TO_SEC / system->spawn_rate);
  int to_spawn = system->time / rate;
  system->time -= rate * to_spawn;

  if (system->max_emission > 0) {
    if (to_spawn + system->emission_count > system->max_emission) {
      to_spawn = system->max_emission - system->emission_count;
    }
  }

  float anim_frame_length = 0.f;
  if (system->animated) {
    anim_frame_length = MS_TO_SEC / system->render.anim.frame_rate;
  }

  for (int i = 0; i < system->capacity; ++i) {
    r_particle *particle = &system->list[i];
    if (!particle->alive && to_spawn) {
      vec2 position;
      vec2_dup(position, system->position);
      switch (system->spawn_type) {
      case SPAWN_POINT:
        // Just do nothing
        break;
      case SPAWN_CIRCLE:
        position[0] += fmodf(rand(), system->size[0]);
        position[1] += fmodf(rand(), system->size[0]);
        break;
      case SPAWN_BOX:
        position[0] += fmodf(rand(), system->size[0]);
        position[1] += fmodf(rand(), system->size[1]);
        break;
      }

      vec2_dup(particle->position, position);
      vec2_dup(particle->size, system->particle_size);
      particle->life = 0;

      // allows for layer readjustment if we do it here and not just once
      particle->layer = system->layer;

      mat4x4_identity(particle->model);
      mat4x4_translate(particle->model, particle->position[0],
                       particle->position[1], particle->layer * 0.1f);
      mat4x4_scale_aniso(particle->model, particle->model, particle->size[0],
                         particle->size[1], 1.f);

      if (!particle->reused) {
        if (system->animated) {
          particle->render.anim = system->render.anim;
          particle->render.anim.time = 0;
          particle->render.anim.frame = 0;
          particle->render.anim.state = R_ANIM_PLAY;
        } else if (system->texture) {
          particle->render.tex = system->render.tex;
        }

        vec4_dup(particle->color, system->color);
        particle->reused = 1;
      } else {
        if (system->animated) {
          particle->render.anim.time = 0;
          particle->render.anim.frame = 0;
          particle->render.anim.state = R_ANIM_PLAY;
        }
      }

      system->count++;
      particle->alive = 1;
      --to_spawn;
    }

    if (particle->alive) {
      particle->life += delta;
      if (particle->life > (system->particle_life * MS_TO_SEC)) {
        particle->alive = 0;
        system->count--;
      } else {
        int force_change = 0;
        if (system->fade_frames.count > 0) {
          particle->color[3] = r_keyframe_get_value(system->fade_frames,
                                                    particle->life / MS_TO_SEC);
        }

        if (system->size_frames.count > 0) {
          float size = r_keyframe_get_value(system->size_frames,
                                            particle->life / MS_TO_SEC);
          // TODO factor out
          if (particle->size[0] != size || particle->size[1] != size)
            force_change = 1;
          particle->size[0] = size;
          particle->size[1] = size;
        }

        // Particle animations
        if (system->animated) {
          r_anim *anim = &particle->render.anim;
          if (anim->state == R_ANIM_PLAY) {
            if (anim->time + delta > anim_frame_length) {
              if (anim->frame == anim->frame_count - 1) {
                if (!anim->loop) {
                  anim->state = R_ANIM_STOP;
                  anim->pstate = R_ANIM_PLAY;
                } else {
                  anim->frame = 0;
                }
              } else {
                ++anim->frame;
              }

              anim->time -= anim_frame_length;
            } else {
              anim->time += delta;
            }
          }
        }

        if (system->velocity[0] || system->velocity[1] || force_change) {
          particle->position[0] += system->velocity[0] * (delta / MS_TO_SEC);
          particle->position[1] += system->velocity[1] * (delta / MS_TO_SEC);
          mat4x4_identity(particle->model);
          mat4x4_translate(particle->model, particle->position[0],
                           particle->position[1], 0.f);
          mat4x4_scale_aniso(particle->model, particle->model,
                             particle->size[0], particle->size[1], 1.f);
        }
      }
    }
  }
}

void r_particles_destroy(r_particles *particles) {
  free(particles->list);

  if (particles->array_count) {
    for (int i = 0; i < particles->array_count; ++i) {
      switch (particles->arrays[i].type) {
      case r_int:
        free(particles->arrays[i].data.int_ptr);
        break;
      case r_float:
        free(particles->arrays[i].data.float_ptr);
        break;
      case r_mat:
        free(particles->arrays[i].data.mat_ptr);
        break;
      case r_vec2:
        free(particles->arrays[i].data.vec2_ptr);
        break;
      case r_vec3:
        free(particles->arrays[i].data.vec3_ptr);
        break;
      case r_vec4:
        free(particles->arrays[i].data.vec4_ptr);
        break;
      }
    }
    free(particles->arrays);
  }
  particles->capacity = 0;
}

static r_uniform_array *r_append_uniform_array(r_uniform_array *arrays,
                                               int array_count,
                                               r_uniform_array array) {
  r_uniform_array *new_array;
  if (array_count == 0) {
    new_array = malloc(sizeof(r_uniform_array));
    new_array[0] = array;
  } else {
    new_array = malloc(sizeof(r_uniform_array) * (array_count + 1));
    memcpy(new_array, arrays, sizeof(r_uniform_array) * array_count);
    free(arrays);
    new_array[array_count] = array;
  }
  return new_array;
}

static r_uniform_array r_create_uniform_array(const char *name, int type,
                                              int capacity) {
  r_uniform_array array;

  array.capacity = capacity;
  array.type = type;
  array.name = name;
  array.count = 0;

  switch (type) {
  case r_float:
    array.data.float_ptr = malloc(sizeof(float) * capacity);
    break;
  case r_int:
    array.data.int_ptr = malloc(sizeof(int) * capacity);
    break;
  case r_vec2:
    array.data.vec2_ptr = malloc(sizeof(vec2) * capacity);
    break;
  case r_vec3:
    array.data.vec3_ptr = malloc(sizeof(vec3) * capacity);
    break;
  case r_vec4:
    array.data.vec4_ptr = malloc(sizeof(vec4) * capacity);
    break;
  case r_mat:
    array.data.mat_ptr = malloc(sizeof(mat4x4) * capacity);
    break;
  default:
    _e("Unsupported data type: %i\n", type);
    break;
  }

  return array;
}

void r_particles_draw(r_particles *particles, r_shader shader) {
  r_uniform_array *tex_id_array, *color_array, *model_array;

  if (!particles->valid_uniforms) {
    if (particles->array_count != 0) {
      for (int i = 0; i < particles->array_count; ++i) {
        switch (particles->arrays[i].type) {
        case r_int:
          free(particles->arrays[i].data.int_ptr);
          break;
        case r_float:
          free(particles->arrays[i].data.float_ptr);
          break;
        case r_mat:
          free(particles->arrays[i].data.mat_ptr);
          break;
        case r_vec2:
          free(particles->arrays[i].data.vec2_ptr);
          break;
        case r_vec3:
          free(particles->arrays[i].data.vec3_ptr);
          break;
        case r_vec4:
          free(particles->arrays[i].data.vec4_ptr);
          break;
        }
      }
      free(particles->arrays);
      particles->array_count = 0;
    }

    r_uniform_array tmp_model_array =
        r_create_uniform_array("models", r_mat, particles->capacity);
    tmp_model_array.uid = particles->array_count;

    particles->arrays = r_append_uniform_array(
        particles->arrays, particles->array_count, tmp_model_array);
    ++particles->array_count;

    r_uniform_array tmp_tex_id_array =
        r_create_uniform_array("tex_ids", r_int, particles->capacity);
    tmp_tex_id_array.uid = particles->array_count;
    particles->arrays = r_append_uniform_array(
        particles->arrays, particles->array_count, tmp_tex_id_array);
    ++particles->array_count;

    r_uniform_array tmp_color_array =
        r_create_uniform_array("colors", r_vec4, particles->capacity);
    tmp_color_array.uid = particles->array_count;
    particles->arrays = r_append_uniform_array(
        particles->arrays, particles->array_count, tmp_color_array);

    ++particles->array_count;

    particles->valid_uniforms = 1;
  }

  for (int i = 0; i < particles->array_count; ++i) {
    if (!strcmp(particles->arrays[i].name, "colors")) {
      color_array = &particles->arrays[i];
    }
    if (!strcmp(particles->arrays[i].name, "models")) {
      model_array = &particles->arrays[i];
    }

    if (!strcmp(particles->arrays[i].name, "tex_ids")) {
      tex_id_array = &particles->arrays[i];
    }
  }

  int alive_count = 0;
  // Prepare the uniform arrays
  for (int i = 0; i < particles->capacity; ++i) {
    if (particles->list[i].alive) {
      ++alive_count;
      r_particle *particle = &particles->list[i];

      mat4x4_dup(model_array->data.mat_ptr[model_array->count],
                 particle->model);
      ++model_array->count;

      vec4_dup(color_array->data.vec4_ptr[color_array->count], particle->color);
      ++color_array->count;

      if (particles->animated) {
        tex_id_array->data.int_ptr[tex_id_array->count] =
            particle->render.anim.frames[particle->render.anim.frame];
        ++tex_id_array->count;
      } else if (particles->texture) {
        tex_id_array->data.int_ptr[tex_id_array->count] =
            particles->render.tex.sub_id;
        ++tex_id_array->count;
      }
    }
  }

  if (!alive_count) {
    return;
  }

  r_shader_bind(shader);
  r_set_m4(shader, "proj", g_camera.proj);
  r_set_m4(shader, "view", g_camera.view);
  r_set_m4x(shader, model_array->count, model_array->name,
            model_array->data.mat_ptr);

  r_set_v4x(shader, color_array->count, color_array->name,
            color_array->data.vec4_ptr);

  if (particles->animated || particles->texture) {
    r_set_ix(shader, tex_id_array->count, tex_id_array->name,
             tex_id_array->data.int_ptr);
    r_set_uniformi(shader, "render_mode", 1);

    // Sub Texture uniforms
    vec2 tex_size, sub_size;

    r_sheet sheet;
    if (particles->animated) {
      sheet = particles->render.anim.sheet;
    } else if (particles->texture) {
      sheet = particles->render.tex.sheet;
    }

    tex_size[0] = sheet.width;
    tex_size[1] = sheet.height;

    sub_size[0] = sheet.subwidth;
    sub_size[1] = sheet.subheight;

    r_set_v2(shader, "sub_size", sub_size);
    r_set_v2(shader, "tex_size", tex_size);

    r_sheet_bind(sheet.id);
  } else if (particles->colored) {
    r_set_uniformi(shader, "render_mode", 0);
  }

  glBindVertexArray(default_quad_vao);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, default_quad_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, default_quad_vboi);

  // Actually draw the stuff
  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0,
                          model_array->count);

  glBindVertexArray(0);

  // Clear out the uniform arrays
  memset(model_array->data.mat_ptr, 0, sizeof(mat4x4) * model_array->capacity);
  model_array->count = 0;
  memset(color_array->data.vec4_ptr, 0, sizeof(vec4) * color_array->count);
  color_array->count = 0;

  if (particles->animated || particles->texture) {
    memset(tex_id_array->data.int_ptr, 0, sizeof(int) * tex_id_array->count);
    tex_id_array->count = 0;
  } else if (particles->colored) {
  }
}

int r_sprite_get_sheet_id(r_sprite sprite) {
  if (sprite.animated) {
    return sprite.render.anim.sheet.id;
  } else {
    return sprite.render.tex.sheet.id;
  }
}

r_sheet r_sprite_get_sheet(r_sprite sprite) {
  if (sprite.animated) {
    return sprite.render.anim.sheet;
  } else {
    return sprite.render.tex.sheet;
  }
}

void r_sprite_draw(r_sprite draw) {
  if (!draw.visible)
    return;

  r_shader_batch *batch =
      r_shader_get_batch(draw.shader, r_sprite_get_sheet(draw));

  if (!batch) {
    batch = r_batch_create(draw.shader, r_sprite_get_sheet(draw));
  }

  if (!batch) {
    _l("Cache miss for shader ID [%i] and sheet ID [%i].\n", draw.shader,
       r_sprite_get_sheet_id(draw));
    return;
  }

  if (batch->sprite_count == batch->sprite_capacity) {
    r_batch_draw(batch);
  }

  ++batch->sprite_count;
}

void r_batch_draw(r_shader_batch *batch) {
  if (!batch || !batch->sprite_count)
    return;

  r_sheet sheet = batch->sheet;
  r_shader shader = batch->shader;

  vec2 sub_size, tex_size;
  sub_size[0] = sheet.subwidth;
  sub_size[1] = sheet.subheight;

  tex_size[0] = sheet.width;
  tex_size[1] = sheet.height;

  r_shader_bind(shader);

  // All the instanced uniform variables
  r_batch_set_arrays(batch);

  // All the global state uniform variables
  r_set_v2(shader, "sub_size", sub_size);
  r_set_v2(shader, "tex_size", tex_size);
  r_set_m4(shader, "proj", g_camera.proj);
  r_set_m4(shader, "view", g_camera.view);

  // r_set_uniformi(shader, "tex", GL_TEXTURE0);

  r_sheet_bind(sheet.id);

  glBindVertexArray(default_quad_vao);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, default_quad_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, default_quad_vboi);

  // Actually draw the stuff
  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0,
                          batch->sprite_count);

  glBindVertexArray(0);

  r_sheet_bind(0);

  r_batch_clear(batch);
}

int r_batch_count(void) {
  int count = 0;
  for (uint32_t i = 0; i < g_shader_map.batch_capacity; ++i) {
    if (g_shader_map.batches[i].used) {
      ++count;
    }
  }
  return count;
}

void r_batch_info(void) {
  for (uint32_t i = 0; i < g_shader_map.batch_capacity; ++i) {
    _l("Shader: %i\n", g_shader_map.batches[i].shader);
  }
}

int r_sprite_draw_count(void) {
  int count = 0;
  for (uint32_t i = 0; i < g_shader_map.batch_capacity; ++i) {
    if (g_shader_map.batches[i].used) {
      count += g_shader_map.batches[i].sprite_count;
    }
  }
  return count;
}

void r_batch_clear(r_shader_batch *batch) {
  for (uint32_t i = 0; i < batch->uniform_array_count; ++i) {
    r_shader_clear_array(&batch->uniform_arrays[i]);
  }

  batch->sprite_count = 0;
}
void r_shader_clear_array(r_uniform_array *array) {
  switch (array->type) {
  case r_vec2: {
    memset(array->data.vec2_ptr, 0, sizeof(vec2) * array->capacity);
  } break;
  case r_vec3: {
    memset(array->data.vec3_ptr, 0, sizeof(vec3) * array->capacity);
  } break;
  case r_vec4: {
    memset(array->data.vec4_ptr, 0, sizeof(vec4) * array->capacity);
  } break;
  case r_mat: {
    memset(array->data.vec4_ptr, 0, sizeof(vec4) * array->capacity);
  } break;
  case r_float: {
    memset(array->data.float_ptr, 0, sizeof(float) * array->capacity);
  } break;
  case r_int: {
    memset(array->data.int_ptr, 0, sizeof(int) * array->capacity);
  } break;
  default:
    _l("Unsupported data type: array_clear\n");
    break;
  }
  array->count = 0;
}

static GLuint r_shader_create_sub(asset_t *asset, int type) {
  GLint success = 0;
  GLuint id = glCreateShader(type);

  const char *ptr = (const char *)asset->data;

  glShaderSource(id, 1, &ptr, NULL);
  glCompileShader(id);

  glGetShaderiv(id, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    int maxlen = 0;
    int len;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxlen);

    char *log = malloc(maxlen);

    glGetShaderInfoLog(id, maxlen, &len, log);
    const char *type_str = (type == GL_FRAGMENT_SHADER) ? "FRAGMENT" : "VERTEX";
    _l("%s: %s\n", type_str, log);
    free(log);
  }

  return id;
}

r_shader r_shader_get(const char *name) {
  for (unsigned int i = 0; i < g_shader_map.count; ++i) {
    if (strcmp(name, g_shader_map.names[i]) == 0) {
      return g_shader_map.shaders[i];
    }
  }
  return 0;
}

r_shader r_shader_create(asset_t *vert, asset_t *frag) {
  GLuint v = r_shader_create_sub(vert, GL_VERTEX_SHADER);
  GLuint f = r_shader_create_sub(frag, GL_FRAGMENT_SHADER);

  GLuint id = glCreateProgram();

  glAttachShader(id, v);
  glAttachShader(id, f);

  glLinkProgram(id);

  GLint success;
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (success != GL_TRUE) {
    int maxlen = 0;
    int len;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxlen);
    char *log = malloc(maxlen);
    glGetProgramInfoLog(id, maxlen, &len, log);
    _l("%s\n", log);
    free(log);
  }

  return (r_shader){id};
}

void r_shader_cache(r_shader shader, const char *name) {
  if (g_shader_map.capacity == 0) {
    _e("No shader cache available.\n");
    return;
  }

  if (g_shader_map.capacity == g_shader_map.count) {
    // TODO implement cache overflow
    _e("No shader cache open to use.\n");
    return;
  }

  for (unsigned int i = 0; i < g_shader_map.count; ++i) {
    if (g_shader_map.batches[i].shader == shader) {
      _l("Shader: %d already contained with alias of: %s\n", shader,
         g_shader_map.names[i]);
      return;
    }
  }

  uint32_t count = g_shader_map.count;

  g_shader_map.names[count] = name;
  g_shader_map.batches[count].shader = shader;
  g_shader_map.count++;
}

void r_shader_clear_arrays(r_shader shader) {
  for (uint32_t i = 0; i < g_shader_map.capacity * 2; ++i) {
    if (g_shader_map.batches[i].shader == shader) {
      for (int j = 0; j < g_shader_map.batches[i].uniform_array_count; ++j) {
        r_shader_clear_array(&g_shader_map.batches[i].uniform_arrays[j]);
      }
      g_shader_map.batches[i].sprite_count = 0;
    }
  }
}

r_shader_batch *r_shader_get_batch(r_shader shader, r_sheet sheet) {
  for (unsigned int i = 0; i < g_shader_map.batch_capacity; ++i) {
    r_shader_batch *batch = &g_shader_map.batches[i];
    if (!batch->used)
      continue;
    if (batch->shader == shader && batch->sheet.id == sheet.id) {
      return batch;
    }
  }
  return 0;
}

void r_batch_set_arrays(r_shader_batch *batch) {
  if (!batch)
    return;
  r_shader shader = batch->shader;

  for (unsigned int i = 0; i < batch->uniform_array_count; ++i) {
    r_uniform_array *array = &batch->uniform_arrays[i];
    // That at least proved that the array isn't NULL
    switch (array->type) {
    case r_vec2:
      r_set_v2x(shader, array->count, array->name, array->data.vec2_ptr);
      break;
    case r_vec3:
      r_set_v3x(shader, array->count, array->name, array->data.vec3_ptr);
      break;
    case r_vec4:
      r_set_v4x(shader, array->count, array->name, array->data.vec4_ptr);
      break;
    case r_float:
      r_set_fx(shader, array->count, array->name, array->data.float_ptr);
      break;
    case r_int:
      r_set_ix(shader, array->count, array->name, array->data.int_ptr);
      break;
    case r_mat:
      r_set_m4x(shader, array->count, array->name, array->data.mat_ptr);
      break;
    default:
      break;
    }

    r_shader_clear_array(array);
  }
}

void r_batch_destroy(r_shader shader, r_sheet sheet) {
  r_shader_batch *batch = r_shader_get_batch(shader, sheet);

  for (unsigned int i = 0; i < batch->uniform_array_count; ++i) {
    switch (batch->uniform_arrays[i].type) {
    case r_int:
      free(batch->uniform_arrays[i].data.int_ptr);
      break;
    case r_float:
      free(batch->uniform_arrays[i].data.float_ptr);
      break;
    case r_mat:
      free(batch->uniform_arrays[i].data.mat_ptr);
      break;
    case r_vec2:
      free(batch->uniform_arrays[i].data.vec2_ptr);
      break;
    case r_vec3:
      free(batch->uniform_arrays[i].data.vec3_ptr);
      break;
    case r_vec4:
      free(batch->uniform_arrays[i].data.vec4_ptr);
      break;
    }
  }

  free(batch->uniform_arrays);
  batch->uniform_array_count = 0;
  batch->shader = 0;
}

void r_batch_destroy_all(r_shader shader) {
  for (uint32_t i = 0; i < g_shader_map.batch_capacity; ++i) {
    r_shader_batch *batch = &g_shader_map.batches[i];
    if (batch->shader == shader) {
      for (uint32_t i = 0; i < batch->uniform_array_count; ++i) {
        switch (batch->uniform_arrays[i].type) {
        case r_int:
          free(batch->uniform_arrays[i].data.int_ptr);
          break;
        case r_float:
          free(batch->uniform_arrays[i].data.float_ptr);
          break;
        case r_mat:
          free(batch->uniform_arrays[i].data.mat_ptr);
          break;
        case r_vec2:
          free(batch->uniform_arrays[i].data.vec2_ptr);
          break;
        case r_vec3:
          free(batch->uniform_arrays[i].data.vec3_ptr);
          break;
        case r_vec4:
          free(batch->uniform_arrays[i].data.vec4_ptr);
          break;
        }
      }

      free(batch->uniform_arrays);
      batch->uniform_array_count = 0;
      batch->shader = 0;
    }
  }
}

r_shader_batch *r_batch_create(r_shader shader, r_sheet sheet) {
  r_shader_batch *batch = 0;
  for (uint32_t i = 0; i < g_shader_map.batch_capacity; ++i) {
    if (!g_shader_map.batches[i].used) {
      batch = &g_shader_map.batches[i];
      break;
    }
  }

  if (!batch) {
    // TODO expandable batch capacity ???
    _e("Unable to find open spot for batch.\n");
    return 0;
  }

  r_uniform_map *uniform_map = r_shader_get_uniform_map(shader);

  if (!uniform_map) {
    _e("Unable to find uniform map when creating batch for shader %i.\n",
       shader);
    return 0;
  }

  if (uniform_map->count == 0) {
    _e("No uniforms for this uniform map..\n");
  }

  batch->uniform_arrays =
      (r_uniform_array *)malloc(sizeof(r_uniform_array) * uniform_map->count);
  batch->uniform_array_count = uniform_map->count;

  batch->used = 1;

  int max_capacity = 0;
  for (int i = 0; i < uniform_map->count; ++i) {
    r_uniform_array array;

    array.type = uniform_map->types[i];
    array.shader = shader;
    array.uid = uniform_map->uids[i];
    array.count = 0;
    array.name = uniform_map->names[i];

    if (!uniform_map->names[i]) {
      _l("No name set at index: %i\n", i);
    }

    array.capacity = uniform_map->capacities[i];

    switch (array.type) {
    case r_int:
      array.data.int_ptr = malloc(array.capacity * sizeof(int));
      break;
    case r_float:
      array.data.float_ptr = malloc(array.capacity * sizeof(float));
      break;
    case r_mat:
      array.data.mat_ptr = malloc(array.capacity * sizeof(mat4x4));
      break;
    case r_vec2:
      array.data.vec2_ptr = malloc(array.capacity * sizeof(vec2));
      break;
    case r_vec3:
      array.data.vec3_ptr = malloc(array.capacity * sizeof(vec3));
      break;
    case r_vec4:
      array.data.vec4_ptr = malloc(array.capacity * sizeof(vec4));
      break;
    }

    if (array.capacity > max_capacity)
      max_capacity = array.capacity;

    batch->uniform_arrays[i] = array;
  }

  batch->shader = shader;
  batch->sheet = sheet;

  batch->sprite_count = 0;
  batch->sprite_capacity = max_capacity;

  return batch;
}

void r_shader_destroy(r_shader shader) {
  r_batch_destroy_all(shader);
  glDeleteProgram(shader);
}

void r_shader_bind(r_shader shader) { glUseProgram(shader); }

static int r_hex_number(const char v) {
  if (v >= '0' && v <= '9') {
    return v - 0x30;
  } else {
    switch (v) {
    case 'A':
    case 'a':
      return 10;
    case 'B':
    case 'b':
      return 11;
    case 'C':
    case 'c':
      return 12;
    case 'D':
    case 'd':
      return 13;
    case 'E':
    case 'e':
      return 14;
    case 'F':
    case 'f':
      return 15;
    default:
      return 0;
    }
  }
}

static int r_hex_multi(const char *v, int len) {
  if (len == 2) {
    return r_hex_number(v[0]) * 16 + r_hex_number(v[1]);
  } else if (len == 1) {
    return r_hex_number(v[0]) * 16 + r_hex_number(v[0]);
  }
  return -1;
}

void r_get_color(vec3 val, const char *v) {
  int len = strlen(v);
  int offset = 0;
  if (len == 4) {
    offset = 1;
    len = 3;
  } else if (len == 7) {
    offset = 1;
    len = 6;
  }

  if (len == 3) {
    val[0] = r_hex_multi(&v[offset], 1) / 255.f;
    val[1] = r_hex_multi(&v[offset + 1], 1) / 255.f;
    val[2] = r_hex_multi(&v[offset + 2], 1) / 255.f;
  } else if (len == 6) {
    val[0] = r_hex_multi(&v[offset], 2) / 255.f;
    val[1] = r_hex_multi(&v[offset + 2], 2) / 255.f;
    val[2] = r_hex_multi(&v[offset + 4], 2) / 255.f;
  }
}
// It'd probably help if I actually updated the creation method
// This one is ~3 months old
/*   uint32_t **frames;
  uint32_t frame;
  uint32_t frame_count;
  uint32_t frame_rate;
  uint32_t uid;
  r_sheet sheet;

  uint8_t pstate;
  uint8_t state;

  float time;

  int loop : 1;
  int use : 1;*/

r_anim r_anim_create(r_sheet sheet, uint32_t *frames, int frame_count,
                     int frame_rate) {
  return (r_anim){frames, 0, frame_count, frame_rate, 0, sheet, 0, 0, 0, 0, 1};
}

void r_anim_destroy(int uid) {
  free(g_anim_map.anims[uid].frames);
  g_anim_map.anims[uid].use = 0;
}

int r_anim_cache(r_anim anim, const char *name) {
  if (g_anim_map.count >= g_anim_map.capacity) {
    _e("Animation cache at capacity.\n");
    // TODO overflow caching
    return 0;
  }

  for (int i = 0; i < g_anim_map.capacity; ++i) {
    if (!g_anim_map.anims[i].use) {
      anim.use = 1;
      g_anim_map.anims[i].frames = anim.frames;
      g_anim_map.anims[i] = anim;
      g_anim_map.names[i] = name;
      g_anim_map.anims[i].use = 1;

      if (i > g_anim_map.high) {
        g_anim_map.high = i;
      }

      g_anim_map.count++;

      return i;
    }
  }

  return -1;
}

void r_anim_play(r_anim *anim) {
  anim->pstate = anim->state;
  anim->state = R_ANIM_PLAY;
}

void r_anim_stop(r_anim *anim) {
  anim->pstate = anim->state;
  anim->state = R_ANIM_STOP;
}

void r_anim_pause(r_anim *anim) {
  anim->pstate = anim->state;
  anim->state = R_ANIM_PAUSE;
}

int r_anim_get_index(const char *name) {
  if (g_anim_map.count == 0) {
    _l("No animations in cache to check for.\n");
    return 0;
  }

  for (int i = 0; i < g_anim_map.count; ++i) {
    if (strcmp(g_anim_map.names[i], name) == 0) {
      return i;
    }
  }

  _l("No animations matching: %s in cache.\n", name);
  return 0;
}

r_anim *r_anim_get(int uid) {
  for (int i = 0; i < g_anim_map.count; ++i) {
    if (g_anim_map.anims[i].uid == uid) {
      return &g_anim_map.anims[i];
    }
  }
  return NULL;
}

int r_sprite_get_tex_id(r_sprite sprite) {
  if (sprite.animated) {
    int current_frame = sprite.render.anim.frame;
    uint32_t *frames = sprite.render.anim.frames;
    return frames[current_frame];
  } else {
    return sprite.render.tex.sub_id;
  }
}

void r_sprite_play(r_sprite *sprite) {
  if (!sprite->animated)
    return;
  r_anim_play(&sprite->render.anim);
}

void r_sprite_pause(r_sprite *sprite) {
  if (!sprite->animated)
    return;
  r_anim_pause(&sprite->render.anim);
}

void r_sprite_stop(r_sprite *sprite) {
  if (!sprite->animated)
    return;
  r_anim_stop(&sprite->render.anim);
}

void r_sprite_set_anim(r_sprite *drawable, r_anim anim) {
  r_anim view = anim;
  view.frame = 0;
  view.time = 0;

  drawable->animated = 1;
  drawable->render.anim = view;
}

void r_sprite_set_tex(r_sprite *sprite, r_subtex tex) {
  sprite->animated = 0;
  sprite->render.tex = tex;
}

r_sprite r_sprite_create(r_shader shader, vec2 pos, vec2 size) {
  r_sprite sprite;
  mat4x4_identity(sprite.model);
  mat4x4_translate(sprite.model, pos[0], pos[1], 0.f);
  mat4x4_scale_aniso(sprite.model, sprite.model, size[0], size[1], 1.f);

  sprite.layer = 0;

  vec2_dup(sprite.size, size);
  vec2_dup(sprite.position, pos);

  sprite.flip_x = 0;
  sprite.flip_y = 0;

  sprite.visible = 1;
  sprite.shader = shader;

  return sprite;
}

void r_sprite_update(r_sprite *drawable, long delta) {
  if (drawable->change) {
    // integer snapping, we'll see how effective this actually is in a bit.
    float x, y;
    x = floorf(drawable->position[0]);
    y = floorf(drawable->position[1]);
    mat4x4_translate(drawable->model, x, y, drawable->layer * 0.1f);
    mat4x4_scale_aniso(drawable->model, drawable->model, drawable->size[0],
                       drawable->size[0], drawable->size[1]);

    drawable->change = 0;
  }

  if (drawable->animated) {
    r_anim *anim = &drawable->render.anim;
    if (anim->state == R_ANIM_PLAY) {
      float frame_time = MS_TO_SEC / anim->frame_rate;
      if (anim->time + delta >= frame_time) {
        if (anim->frame >= anim->frame_count - 1) {
          if (!anim->loop) {
            anim->state = R_ANIM_STOP;
            anim->pstate = R_ANIM_PLAY;
          }

          anim->frame = 0;
        } else {
          anim->frame++;
        }

        anim->time -= frame_time;
      } else {
        anim->time += delta;
      }
    }
  }
}

r_uniform_array *r_shader_get_array(r_shader shader, r_sheet sheet,
                                    const char *name) {
  r_shader_batch *batch = r_shader_get_batch(shader, sheet);
  if (!batch) {
    return 0;
  }

  for (unsigned int i = 0; i < batch->uniform_array_count; ++i) {
    if (strcmp(batch->uniform_arrays[i].name, name) == 0) {
      return &batch->uniform_arrays[i];
    }
  }

  return 0;
}

int r_shader_get_array_index(r_shader shader, const char *name) {
  for (unsigned int i = 0; i < g_shader_map.count; ++i) {
    if (g_shader_map.uniform_maps[i].shader == shader) {
      r_uniform_map *map = &g_shader_map.uniform_maps[i];
      for (int j = 0; j < map->count; ++j) {
        if (strcmp(map->names[i], name) == 0) {
          return i;
        }
      }
    }
  }

  return -1;
}

r_uniform_array *r_shader_get_arrayi(r_shader shader, r_sheet sheet, int uid) {
  r_shader_batch *batch = r_shader_get_batch(shader, sheet);
  if (!batch) {
    _e("Unable to get batch for shader %i and sheet %i.\n", shader, sheet.id);
    return 0;
  }

  return &batch->uniform_arrays[uid];
}

int r_shader_setup_array(r_shader shader, const char *name, int capacity,
                         int type) {
  r_uniform_map *map = r_shader_get_uniform_map(shader);
  if (!map) {
    for (int i = 0; i < g_shader_map.capacity; ++i) {
      if (!g_shader_map.uniform_maps[i].used) {
        map = &g_shader_map.uniform_maps[i];
        map->count = 0;
        map->shader = shader;
        map->used = 1;

        map->names = malloc(sizeof(const char *));
        map->uids = malloc(sizeof(uint32_t));
        map->types = malloc(sizeof(int));
        map->locations = malloc(sizeof(int));
        map->capacities = malloc(sizeof(int));

        break;
      }
    }
  }

  if (!map) {
    _e("Unable to find shader uniform map for shader %i.\n", shader);
    return -1;
  }

  if (map->count > 0) {
    int count = map->count + 1;
    char *names = malloc(sizeof(char *) * count);
    uint32_t *uids = malloc(sizeof(uint32_t) * count);
    int *types = malloc(sizeof(int) * count);
    int *locations = malloc(sizeof(int) * count);
    int *capacities = malloc(sizeof(int) * count);

    memset(names, 0, sizeof(char *) * count);
    memset(uids, 0, sizeof(uint32_t) * count);
    memset(types, 0, sizeof(int) * count);
    memset(locations, 0, sizeof(int) * count);
    memset(capacities, 0, sizeof(int) * count);

    memcpy(names, map->names, map->count * sizeof(char *));
    memcpy(uids, map->uids, map->count * sizeof(uint32_t));
    memcpy(types, map->types, map->count * sizeof(int));
    memcpy(locations, map->locations, map->count * sizeof(int));
    memcpy(capacities, map->capacities, map->count * sizeof(int));

    free(map->names);
    free(map->uids);
    free(map->types);
    free(map->locations);
    free(map->capacities);

    map->names = (const char **)names;
    map->uids = uids;
    map->types = types;
    map->locations = (unsigned int *)locations;
    map->capacities = (unsigned int *)capacities;
  }

  int uid = map->count;
  map->names[map->count] = name;
  map->uids[map->count] = uid;
  map->types[map->count] = type;
  map->locations[map->count] = glGetUniformLocation(shader, name);
  map->capacities[map->count] = capacity;

  map->count++;

  return uid;
}

r_uniform_map *r_shader_get_uniform_map(r_shader shader) {
  for (int i = 0; i < g_shader_map.capacity; ++i) {
    if (g_shader_map.uniform_maps[i].shader == shader) {
      return &g_shader_map.uniform_maps[i];
    }
  }
  return 0;
}

void r_shader_uniform(r_shader shader, r_sheet sheet, const char *name,
                      void *data, int count) {
  r_uniform_array *array = r_shader_get_array(shader, sheet, name);

  if (!array) {
    _l("Unable to find Shader Uniform array with UID: %i\n");
    return;
  }

  if (array->count + count > array->capacity) {
    count = array->capacity - array->count;
  }

  switch (array->type) {
  case r_vec2: {
    vec2 *data_ptr = (vec2 *)data;
    for (int i = 0; i < count; ++i) {
      vec2_dup(array->data.vec2_ptr[array->count], data_ptr[i]);
      ++array->count;
    }
  } break;
  case r_vec3: {
    vec3 *data_ptr = (vec3 *)data;
    for (int i = 0; i < count; ++i) {
      vec3_dup(array->data.vec3_ptr[array->count], data_ptr[i]);
      ++array->count;
    }
  } break;
  case r_vec4: {
    vec4 *data_ptr = (vec4 *)data;
    for (int i = 0; i < count; ++i) {
      vec4_dup(array->data.vec4_ptr[array->count], data_ptr[i]);
      ++array->count;
    }
  } break;
  case r_int: {
    int *data_ptr = (int *)data;
    for (int i = 0; i < count; ++i) {
      array->data.int_ptr[array->count] = data_ptr[i];
      ++array->count;
    }
  } break;
  case r_mat: {
    mat4x4 *data_ptr = (mat4x4 *)data;
    for (int i = 0; i < count; ++i) {
      mat4x4_dup(array->data.mat_ptr[array->count], data_ptr[i]);
      ++array->count;
    }
  } break;
  case r_float: {
    float *data_ptr = (float *)data;
    for (int i = 0; i < count; ++i) {
      array->data.float_ptr[array->count] = data_ptr[i];
      ++array->count;
    }
  } break;
  default:
    _l("Unsupported data type.\n");
    return;
  }
}

void r_shader_uniformi(r_shader shader, r_sheet sheet, int uid, void *data,
                       int count) {
  r_uniform_array *array = r_shader_get_arrayi(shader, sheet, uid);

  if (!array) {
    _l("Unable to find Shader Uniform array with UID: %i\n");
    return;
  }

  if (array->count + count > array->capacity) {
    count = array->capacity - array->count;
  }

  switch (array->type) {
  case r_vec2: {
    vec2 *data_ptr = (vec2 *)data;
    for (int i = 0; i < count; ++i) {
      vec2_dup(array->data.vec2_ptr[array->count], data_ptr[i]);
      ++array->count;
    }
  } break;
  case r_vec3: {
    vec3 *data_ptr = (vec3 *)data;
    for (int i = 0; i < count; ++i) {
      vec3_dup(array->data.vec3_ptr[array->count], data_ptr[i]);
      ++array->count;
    }
  } break;
  case r_vec4: {
    vec4 *data_ptr = (vec4 *)data;
    for (int i = 0; i < count; ++i) {
      vec4_dup(array->data.vec4_ptr[array->count], data_ptr[i]);
      ++array->count;
    }
  } break;
  case r_int: {
    int *data_ptr = (int *)data;
    for (int i = 0; i < count; ++i) {
      array->data.int_ptr[array->count] = data_ptr[i];
      ++array->count;
    }
  } break;
  case r_mat: {
    mat4x4 *data_ptr = (mat4x4 *)data;
    for (int i = 0; i < count; ++i) {
      mat4x4_dup(array->data.mat_ptr[array->count], data_ptr[i]);
      ++array->count;
    }
  } break;
  case r_float: {
    float *data_ptr = (float *)data;
    for (int i = 0; i < count; ++i) {
      array->data.float_ptr[array->count] = data_ptr[i];
      ++array->count;
    }
  } break;
  default:
    _l("Unsupported data type.\n");
    return;
  }
}

void r_shader_sprite_uniform(r_sprite sprite, int uid, void *data) {
  r_uniform_array *array =
      r_shader_get_arrayi(sprite.shader, r_sprite_get_sheet(sprite), uid);

  if (!array) {
    _e("Unable to find array for shader %i and sheet %i.\n", sprite.shader,
       r_sprite_get_sheet_id(sprite));
  }

  if (array->count == array->capacity) {
    return;
  }

  switch (array->type) {
  case r_vec2: {
    vec2_dup(array->data.vec2_ptr[array->count], *((vec2 *)data));
  } break;
  case r_vec3: {
    vec3_dup(array->data.vec3_ptr[array->count], *((vec3 *)data));
  } break;
  case r_vec4: {
    vec4_dup(array->data.vec4_ptr[array->count], *((vec4 *)data));
  } break;
  case r_int: {
    int *src = data;
    array->data.int_ptr[array->count] = *src;
  } break;
  case r_mat: {
    mat4x4_dup(array->data.mat_ptr[array->count], *(mat4x4 *)data);
  } break;
  case r_float: {
    float *src = data;
    array->data.float_ptr[array->count] = *src;
  } break;
  default:
    _l("Unsupported data type.\n");
    return;
  }
  ++array->count;
}

inline void r_set_uniformf(r_shader shader, const char *name, float value) {
  glUniform1f(glGetUniformLocation(shader, name), value);
}

inline void r_set_uniformfi(int loc, float value) { glUniform1f(loc, value); }

inline void r_set_uniformi(r_shader shader, const char *name, int value) {
  glUniform1i(glGetUniformLocation(shader, name), value);
}

inline void r_set_uniformii(int loc, int val) { glUniform1i(loc, val); }

inline void r_set_v4(r_shader shader, const char *name, vec4 value) {
  glUniform4f(glGetUniformLocation(shader, name), value[0], value[1], value[2],
              value[3]);
}

inline void r_set_v4i(int loc, vec4 value) {
  glUniform4f(loc, value[0], value[1], value[2], value[3]);
}

inline void r_set_v3(r_shader shader, const char *name, vec3 value) {
  glUniform3f(glGetUniformLocation(shader, name), value[0], value[1], value[2]);
}

inline void r_set_v3i(int loc, vec3 val) {
  glUniform3f(loc, val[0], val[1], val[2]);
}

inline void r_set_v2(r_shader shader, const char *name, vec2 value) {
  glUniform2f(glGetUniformLocation(shader, name), value[0], value[1]);
}

inline void r_set_v2i(int loc, vec2 val) { glUniform2f(loc, val[0], val[1]); }

inline void r_set_m4(r_shader shader, const char *name, mat4x4 value) {
  glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE,
                     (GLfloat *)value);
}

inline void r_set_m4i(int loc, mat4x4 val) {
  glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *)val);
}

void r_set_m4x(r_shader shader, uint32_t count, const char *name,
               mat4x4 *values) {
  if (!count)
    return;
  glUniformMatrix4fv(glGetUniformLocation(shader, name), count, GL_FALSE,
                     (const GLfloat *)values);
}

void r_set_ix(r_shader shader, uint32_t count, const char *name, int *values) {
  if (!count)
    return;
  glUniform1iv(glGetUniformLocation(shader, name), count,
               (const GLint *)values);
}

void r_set_fx(r_shader shader, uint32_t count, const char *name,
              float *values) {
  if (!count)
    return;
  glUniform1fv(glGetUniformLocation(shader, name), count,
               (const GLfloat *)values);
}

void r_set_v2x(r_shader shader, uint32_t count, const char *name,
               vec2 *values) {
  if (!count)
    return;

  glUniform2fv(glGetUniformLocation(shader, name), count,
               (const GLfloat *)values);
}

void r_set_v3x(r_shader shader, uint32_t count, const char *name,
               vec3 *values) {
  if (!count)
    return;

  glUniform3fv(glGetUniformLocation(shader, name), count,
               (const GLfloat *)values);
}

void r_set_v4x(r_shader shader, uint32_t count, const char *name,
               vec4 *values) {
  if (!count)
    return;

  glUniform4fv(glGetUniformLocation(shader, name), count,
               (const GLfloat *)values);
}

void r_window_get_size(int *w, int *h) {
  *w = g_window.width;
  *h = g_window.height;
}

int r_get_videomode_str(char *dst, int index) {
  if (index >= flags.video_mode_count) {
    index = 0;
  }
  index = (flags.video_mode_count - 1) - index;
  int str_len =
      sprintf(dst, "%ix%i@%i", r_vidmodes[index].width,
              r_vidmodes[index].height, r_vidmodes[index].refreshRate);
  return str_len;
}

void r_select_mode(int index, int fullscreen, int vsync, int borderless) {
  if (index > flags.video_mode_count) {
    _l("Invalid video mode index, not setting.\n");
    return;
  }

  index = (flags.video_mode_count - 1) - index;

  const GLFWvidmode *selected_mode = &r_vidmodes[index];

  if (!fullscreen && borderless != g_window.borderless)
    flags.allowed = 0;

  if (fullscreen) {
    glfwWindowHint(GLFW_RED_BITS, selected_mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, selected_mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, selected_mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, selected_mode->refreshRate);

    g_window.refreshRate = selected_mode->refreshRate;

    vec2_dup(r_res, (vec2){selected_mode->width, selected_mode->height});

    glfwSetWindowMonitor(g_window.glfw, r_default_monitor, 0, 0,
                         selected_mode->width, selected_mode->height,
                         selected_mode->refreshRate);
  } else {
    // TODO fix borderless switching back to decorated
    if (g_window.borderless != borderless) {
      g_window.borderless = borderless;
      glfwSetWindowAttrib(g_window.glfw, GLFW_DECORATED,
                          (borderless == 0) ? GLFW_TRUE : GLFW_FALSE);
      _l("Setting borderless to: %i\n", borderless);
    }

    if (selected_mode->width != g_window.width ||
        selected_mode->height != g_window.height) {
      glfwSetWindowSize(g_window.glfw, selected_mode->width,
                        selected_mode->height);
      vec2_dup(r_res, (vec2){selected_mode->width, selected_mode->height});

      r_window_center();
    }

    if (fullscreen != g_window.fullscreen) {
      int x, y;
      glfwGetWindowPos(g_window.glfw, &x, &y);
      glfwSetWindowMonitor(g_window.glfw, 0, x, y, selected_mode->width,
                           selected_mode->height, selected_mode->refreshRate);
    }
  }

  g_window.fullscreen = fullscreen;
  g_window.vsync = vsync;
  if (vsync) {
    glfwSwapInterval(1);
  }

  if (!fullscreen) {
    const GLFWvidmode *monitor_mode = glfwGetVideoMode(r_default_monitor);
    glfwSetWindowPos(g_window.glfw, (monitor_mode->width - g_window.width) / 2,
                     (monitor_mode->height - g_window.height) / 2);
  }

  flags.allowed = 1;
}

int r_get_vidmode_count(void) { return flags.video_mode_count; }

int r_allow_render(void) { return flags.allowed; }

int r_is_vsync(void) { return g_window.vsync; }

int r_is_fullscreen(void) { return g_window.fullscreen; }

int r_is_borderless(void) { return g_window.borderless; }

static void r_window_get_modes(void) {
  if (r_default_monitor == NULL) {
    r_default_monitor = glfwGetPrimaryMonitor();
  }
  int count;
  r_vidmodes = glfwGetVideoModes(r_default_monitor, &count);
  flags.video_mode_count = count;
}

static const GLFWvidmode *r_find_closest_mode(r_window_info info) {
  if (flags.video_mode_count == 0) {
    r_window_get_modes();
  } else if (flags.video_mode_count == 1) {
    return r_vidmodes;
  }

  const GLFWvidmode *closest = &r_vidmodes[0];
  int distance =
      (abs(info.width - r_vidmodes[0].width) +
       abs(info.height - r_vidmodes[0].height) - r_vidmodes[0].refreshRate);

  for (int i = 0; i < flags.video_mode_count; ++i) {
    int d2 =
        (abs(info.width - r_vidmodes[i].width) +
         abs(info.height - r_vidmodes[i].height) - r_vidmodes[i].refreshRate);
    if (d2 < distance) {
      closest = &r_vidmodes[i];
      distance = d2;
    }
  }

  return closest;
}

static const GLFWvidmode *r_find_best_mode(void) {
  if (flags.video_mode_count == 0) {
    r_window_get_modes();
  } else if (flags.video_mode_count == 1) {
    return r_vidmodes;
  }

  const GLFWvidmode *selected = &r_vidmodes[0];
  int value = selected->width + selected->height * (selected->refreshRate * 2);

  for (int i = 0; i < flags.video_mode_count; ++i) {
    int vec2 = r_vidmodes[i].width +
               r_vidmodes[i].height * (r_vidmodes[i].refreshRate * 2);
    if (vec2 > value) {
      selected = &r_vidmodes[i];
      value = vec2;
    }
  }

  return selected;
}

int r_window_create(r_window_info info) {
#if defined(INIT_DEBUG)
  _l("Creating window.\n");
#endif

  glfwSetErrorCallback(glfw_err_cb);

  if (!glfwInit()) {
    _e("Unable to initialize GLFW\n");
    return 0;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
#endif

  /* OpenGL ES 2.0 stuff
   * glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);*/
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

  GLFWwindow *window = NULL;
  g_window = (r_window){0};

  if (info.fullscreen) {
    const GLFWvidmode *selected_mode;

    if (info.width > 0 && info.height > 0 && info.refreshRate > 0) {
      selected_mode = r_find_closest_mode(info);
    } else {
      selected_mode = r_find_best_mode();
    }

    glfwWindowHint(GLFW_RED_BITS, selected_mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, selected_mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, selected_mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, selected_mode->refreshRate);

    r_default_monitor = glfwGetPrimaryMonitor();

    g_window.refreshRate = selected_mode->refreshRate;
    g_window.width = selected_mode->width;
    g_window.height = selected_mode->height;
    g_window.fullscreen = 1;

    vec2_dup(r_res, (vec2){selected_mode->width, selected_mode->height});

    window = glfwCreateWindow(selected_mode->width, selected_mode->height,
                              info.title, r_default_monitor, NULL);
  } else {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED,
                   (info.borderless == 0) ? GLFW_TRUE : GLFW_FALSE);
    g_window.borderless = info.borderless;

    if (info.refreshRate > 0) {
      glfwWindowHint(GLFW_REFRESH_RATE, info.refreshRate);
      g_window.refreshRate = info.refreshRate;
    } else {
      g_window.refreshRate =
          glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;
    }

    if (info.gamma < 0.1f) {
      g_window.gamma = 2.2f;
    } else {
      g_window.gamma = info.gamma;
    }

    g_window.width = info.width;
    g_window.height = info.height;

    vec2_dup(r_res, (vec2){info.width, info.height});

    g_window.fullscreen = 0;
    g_window.vsync = info.vsync;

    window = glfwCreateWindow(info.width, info.height, info.title, NULL, NULL);
  }

  if (!window) {
    _e("Error: Unable to create GLFW window.\n");
    glfwTerminate();
    return 0;
  }

  g_window.glfw = window;

  if (info.icon) {
    asset_t *icon = asset_get(0, info.icon);
    r_window_set_icon(icon);
    asset_free(icon);
  }

  glfwMakeContextCurrent(window);

  gladLoadGL(glfwGetProcAddress);

  if (g_window.vsync) {
    glfwSwapInterval(1);
  }

#if defined(INIT_DEBUG)
  _l("Window context created successfully.\n");
#endif

  flags.allowed = 1;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  // glEnable(GL_CULL_FACE);
  glDisable(GL_CULL_FACE);

  // glEnable(GL_SCISSOR_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glfwGetWindowPos(g_window.glfw, &g_window.x, &g_window.y);

#if !defined(CUSTOM_GLFW_CALLBACKS)
#if defined(INIT_DEBUG)
  _l("Setting Callbacks.\n");
#endif
  glfwSetWindowPosCallback(g_window.glfw, glfw_window_pos_cb);
  glfwSetWindowSizeCallback(g_window.glfw, glfw_window_size_cb);
  glfwSetWindowCloseCallback(g_window.glfw, glfw_window_close_cb);
  glfwSetKeyCallback(g_window.glfw, glfw_key_cb);
  glfwSetCharCallback(g_window.glfw, glfw_char_cb);
  glfwSetMouseButtonCallback(g_window.glfw, glfw_mouse_button_cb);
  glfwSetCursorPosCallback(g_window.glfw, glfw_mouse_pos_cb);
  glfwSetScrollCallback(g_window.glfw, glfw_scroll_cb);
  glfwSetJoystickCallback(glfw_joy_cb);
#endif

  r_window_get_modes();

#if defined(INIT_DEBUG)
  _l("Setting default bindings.\n");
#endif

  i_default_bindings();

  return 1;
}

void r_window_center(void) {
  GLFWmonitor *mon = NULL;
  int monitor_count;
  GLFWmonitor **monitors = glfwGetMonitors(&monitor_count);

  if (monitor_count == 0) {
    return;
  } else if (monitor_count == 1) {
    const GLFWvidmode *mode = glfwGetVideoMode(monitors[0]);
    r_window_set_pos((mode->width - g_window.width) / 2,
                     (mode->height - g_window.height) / 2);
    return;
  }

  int mon_x, mon_y;
  int mon_w, mon_h;

  for (int i = 0; i < monitor_count; ++i) {
    glfwGetMonitorPos(monitors[i], &mon_x, &mon_y);
    const GLFWvidmode *mode = glfwGetVideoMode(monitors[i]);
    if (g_window.x > mon_x && g_window.x < mon_x + mode->width) {
      if (g_window.y > mon_y && g_window.y < mon_y + mode->height) {
        mon_w = mode->width;
        mon_h = mode->height;
        mon = monitors[i];
        break;
      }
    }
  }

  if (mon != NULL) {
    r_window_set_pos((mon_w - g_window.width) / 2,
                     (mon_h - g_window.height) / 2);
  }
}

void r_window_set_pos(int x, int y) { glfwSetWindowPos(g_window.glfw, x, y); }

int r_window_set_icon(asset_t *asset) {
  if (asset->filled) {
    int w, h, ch;
    unsigned char *img =
        stbi_load_from_memory(asset->data, asset->data_length, &w, &h, &ch, 0);

    GLFWimage glfw_img = (GLFWimage){w, h, img};
    // It's probably g_window.glfw now that I think about it
    glfwSetWindowIcon(g_window.glfw, 1, &glfw_img);

    free(img);

    asset->req_free = 1;
  } else {
    _l("No window icon passed to set.\n");
    return 0;
  }

  return 1;
}

void r_window_destroy(void) {
  flags.allowed = 0;

  glfwDestroyWindow(g_window.glfw);

  g_window.glfw = NULL;
  g_window.width = -1;
  g_window.height = -1;
  g_window.refreshRate = -1;
  g_window.fullscreen = 0;
  g_window.vsync = 0;
}

void r_window_request_close(void) { g_window.close_requested = 1; }

int r_window_should_close(void) { return g_window.close_requested; }

void r_window_swap_buffers(void) { glfwSwapBuffers(g_window.glfw); }

void r_window_clear(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void r_window_clear_color(const char *str) {
  vec3 color;
  r_get_color(color, str);
  glClearColor(color[0], color[1], color[2], 1.0f);
}

int r_get_refresh_rate(void) { return g_window.refreshRate; }
