// For actual OpenGL Bindings
#include <glad/glad_gl.c>
#include <astera/render.h>

// For ASTERA_DBG/ASTERA_FUNC_DBG macro
#include <astera/debug.h>

#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// For callbacks only
static r_ctx* _r_ctx;

static void glfw_err_cb(int error, const char* msg) {
  ASTERA_DBG("GLFW ERROR: %i %s\n", error, msg);
  printf("GLFW error: %i %s\n", error, msg);
}

static void glfw_window_pos_cb(GLFWwindow* window, int x, int y) {
  if (_r_ctx->window.glfw == window) {
    _r_ctx->window.params.x = x;
    _r_ctx->window.params.y = y;
  }
}

static void glfw_window_size_cb(GLFWwindow* window, int w, int h) {
  if (_r_ctx->window.glfw == window) {
    _r_ctx->window.params.width  = w;
    _r_ctx->window.params.height = h;
    glViewport(0, 0, w, h);
    _r_ctx->scaled = 1;
  }
}

static void glfw_window_close_cb(GLFWwindow* window) {
  if (_r_ctx->window.glfw == window)
    _r_ctx->window.close_requested = 1;
}

static void glfw_key_cb(GLFWwindow* window, int key, int scancode, int action,
                        int mods) {
  if (_r_ctx->window.glfw != window)
    return;

  i_key_callback(_r_ctx->input_ctx, key, scancode,
                 (action == GLFW_PRESS || action == GLFW_REPEAT));
}

static void glfw_char_cb(GLFWwindow* window, uint32_t c) {
  if (window == _r_ctx->window.glfw)
    i_char_callback(_r_ctx->input_ctx, c);
}

static void glfw_mouse_pos_cb(GLFWwindow* window, double x, double y) {
  if (window == _r_ctx->window.glfw)
    i_mouse_pos_callback(_r_ctx->input_ctx, x, y);
}

static void glfw_mouse_button_cb(GLFWwindow* window, int button, int action,
                                 int mods) {
  if (window != _r_ctx->window.glfw)
    return;

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    i_mouse_button_callback(_r_ctx->input_ctx, button, 1);
    if (i_key_binding_track(_r_ctx->input_ctx)) {
      i_binding_track_callback(_r_ctx->input_ctx, 0, (uint16_t)button,
                               ASTERA_BINDING_MB);
    }
  } else {
    i_mouse_button_callback(_r_ctx->input_ctx, (uint16_t)button, 0);
  }
}

static void glfw_scroll_cb(GLFWwindow* window, double dx, double dy) {
  if (_r_ctx->window.glfw == window)
    i_mouse_scroll_callback(_r_ctx->input_ctx, dx, dy);
}

static void glfw_joy_cb(int joystick, int action) {
  if (action == GLFW_CONNECTED) {
    i_joy_create(_r_ctx->input_ctx, (uint8_t)joystick);
  } else if (action == GLFW_DISCONNECTED) {
    i_joy_destroy(_r_ctx->input_ctx, (uint8_t)joystick);
  }
}

static void r_batch_clear(r_batch* batch) {
  memset(batch->mats, 0, sizeof(mat4x4) * batch->count);
  memset(batch->coords, 0, sizeof(vec4) * batch->count);
  memset(batch->colors, 0, sizeof(vec4) * batch->count);
  memset(batch->flip_x, 0, sizeof(int) * batch->count);
  memset(batch->flip_y, 0, sizeof(int) * batch->count);
  batch->count = 0;
}

static void r_batch_check(r_batch* batch) {
  if (!batch) {
    return;
  }

  if (!batch->mats) {
    batch->mats = (mat4x4*)calloc(batch->capacity, sizeof(mat4x4));
  }

  if (!batch->coords) {
    batch->coords = (vec4*)calloc(batch->capacity, sizeof(vec4));
  }

  if (!batch->colors) {
    batch->colors = (vec4*)calloc(batch->capacity, sizeof(vec4));
  }

  if (!batch->flip_x) {
    batch->flip_x = (int*)calloc(batch->capacity, sizeof(int));
  }

  if (!batch->flip_y) {
    batch->flip_y = (int*)calloc(batch->capacity, sizeof(int));
  }
}

static void r_batch_add(r_batch* batch, r_sprite* sprite) {
  batch->flip_x[batch->count] = sprite->flip_x;
  batch->flip_y[batch->count] = sprite->flip_y;

  mat4x4_dup(batch->mats[batch->count], sprite->model);
  vec4_dup(batch->colors[batch->count], sprite->color);

  if (sprite->animated) {
    vec4_dup(batch->coords[batch->count],
             batch->sheet
                 ->subtexs[sprite->render.anim.anim
                               ->frames[sprite->render.anim.curr]]
                 .coords);
  } else {
    vec4_dup(batch->coords[batch->count],
             batch->sheet->subtexs[sprite->render.tex].coords);
  }

  ++batch->count;
}

static uint32_t r_batch_add_multi(r_batch* batch, r_sprite* sprites,
                                  uint32_t count) {
  for (uint32_t i = 0; i < count; ++i) {
    if (batch->count == batch->capacity)
      return i;
    batch->flip_x[batch->count] = sprites[i].flip_x;
    batch->flip_y[batch->count] = sprites[i].flip_y;

    mat4x4_dup(batch->mats[batch->count], sprites[i].model);
    vec4_dup(batch->colors[batch->count], sprites[i].color);

    if (sprites[i].animated) {
      vec4_dup(batch->coords[batch->count],
               batch->sheet
                   ->subtexs[sprites[i].render.anim.anim->frames
                                 [sprites[i].render.anim.curr]]
                   .coords);
    } else {
      vec4_dup(batch->coords[batch->count],
               batch->sheet->subtexs[sprites[i].render.tex].coords);
    }

    ++batch->count;
  }

  return count;
}

static r_batch* r_batch_get(r_ctx* ctx, r_sheet* sheet, r_shader shader) {
  for (uint32_t i = 0; i < ctx->batch_capacity; ++i) {
    r_batch* batch = &ctx->batches[i];

    if (batch->sheet && batch->shader) {
      if (sheet->id == batch->sheet->id && shader == batch->shader) {
        return batch;
      }
    }
  }

  for (uint32_t i = 0; i < ctx->batch_capacity; ++i) {
    r_batch* batch = &ctx->batches[i];

    if (batch->count == 0) {
      r_batch_check(batch);
      batch->sheet  = sheet;
      batch->shader = shader;

      return batch;
    }
  }

  return 0;
}

static void r_batch_draw(r_ctx* ctx, r_batch* batch) {
  if (!batch->count) {
    ASTERA_FUNC_DBG("nothing in batch to draw.\n");
    return;
  }

  if (!batch || !ctx) {
    ASTERA_FUNC_DBG("incomplete arguments passed.\n");
    return;
  }

  if (!batch->sheet) {
    ASTERA_FUNC_DBG("batch sheet is not set.\n");
    return;
  }

  r_shader_bind(batch->shader);
  r_tex_bind(batch->sheet->id);

  vec2 sheet_size = {(float)batch->sheet->width, (float)batch->sheet->height};
  r_set_v2(batch->shader, "sheet_size", sheet_size);

  r_set_m4(batch->shader, "view", ctx->camera.view);
  r_set_m4(batch->shader, "projection", ctx->camera.projection);

  r_set_ix(batch->shader, batch->count, "flip_x", (int*)batch->flip_x);
  r_set_ix(batch->shader, batch->count, "flip_y", (int*)batch->flip_y);
  r_set_v4x(batch->shader, batch->count, "coords", batch->coords);
  r_set_v4x(batch->shader, batch->count, "colors", batch->colors);
  r_set_m4x(batch->shader, batch->count, "mats", batch->mats);

  glBindVertexArray(ctx->default_quad.vao);
  glBindBuffer(GL_ARRAY_BUFFER, ctx->default_quad.vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->default_quad.vboi);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (const void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (const void*)12);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, batch->count);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  r_batch_clear(batch);

  glBindVertexArray(0);
  r_tex_bind(0);
  r_shader_bind(0);
}

uint32_t r_check_error(void) {
  return glGetError();
}

uint32_t r_check_error_loc(const char* loc) {
  uint32_t error = glGetError();

  if (error != GL_NO_ERROR) {
    ASTERA_DBG("GL Error: %i at location: %s\n", error, loc);
  }

  return error;
}

r_quad r_quad_create(float width, float height, uint8_t use_vto) {
  uint32_t vao = 0, vbo = 0, vboi = 0, vto = 0;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glGenBuffers(1, &vboi);

  uint16_t inds[6] = {0, 1, 2, 2, 3, 0};

  if (use_vto) {
    glGenBuffers(1, &vto);

    float verts[12] = {-0.5f, -0.5f, 0.f, -0.5f, 0.5f,  0.f,
                       0.5f,  0.5f,  0.f, 0.5f,  -0.5f, 0.f};
    float texcs[8]  = {0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f};

    for (uint8_t i = 0; i < 12; i += 3) {
      verts[i] *= width;
      verts[i + 1] *= height;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, vto);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), texcs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);
  } else {
    float verts[20] = {-0.5f, -0.5f, 0.f,   0.f,  0.f,  -0.5f, 0.5f,
                       0.f,   0.f,   1.f,   0.5f, 0.5f, 0.f,   1.f,
                       1.f,   0.5f,  -0.5f, 0.f,  1.f,  0.f};

    for (uint8_t i = 0; i < 20; i += 5) {
      verts[i] *= width;
      verts[i + 1] *= height;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (const void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (const void*)12);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint16_t), inds,
               GL_STATIC_DRAW);

  glBindVertexArray(0);

  return (r_quad){.vao     = vao,
                  .vbo     = vbo,
                  .vto     = vto,
                  .vboi    = vboi,
                  .width   = width,
                  .height  = height,
                  .use_vto = use_vto};
}

void r_quad_draw(r_quad quad) {
  glBindVertexArray(quad.vao);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

  glBindVertexArray(0);
  glUseProgram(0);
}

void r_quad_draw_instanced(r_quad quad, uint32_t count) {
  glBindVertexArray(quad.vao);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, count);

  glBindVertexArray(0);
  glUseProgram(0);
}

void r_quad_destroy(r_quad* quad) {
  glDeleteVertexArrays(1, &quad->vao);
  glDeleteBuffers(1, &quad->vbo);
  glDeleteBuffers(1, &quad->vboi);

  if (quad->use_vto) {
    glDeleteBuffers(1, &quad->vto);
  }
}

r_window_params r_window_params_create(uint32_t width, uint32_t height,
                                       uint8_t resizable, uint8_t fullscreen,
                                       uint8_t vsync, uint8_t borderless,
                                       uint16_t    refresh_rate,
                                       const char* title) {
  return (r_window_params){.width        = width,
                           .height       = height,
                           .resizable    = resizable,
                           .fullscreen   = fullscreen,
                           .vsync        = vsync,
                           .borderless   = borderless,
                           .refresh_rate = refresh_rate,
                           .title        = title,
                           .min_width    = 0,
                           .min_height   = 0,
                           .max_width    = 0,
                           .max_height   = 0,
                           .x            = 0,
                           .y            = 0};
}

r_ctx* r_ctx_create(r_window_params params, uint8_t batch_count,
                    uint32_t batch_size, uint16_t anim_map_size,
                    uint8_t shader_map_size) {
  r_ctx* ctx = (r_ctx*)calloc(1, sizeof(r_ctx));

  if (!r_window_create(ctx, params)) {
    ASTERA_FUNC_DBG("unable to create window.\n");
    free(ctx);
    return 0;
  }

  if (batch_count > 0) {
    ctx->batches = (r_batch*)calloc(batch_count, sizeof(r_batch));
  } else {
    ctx->batches = 0;
  }

  ctx->batch_capacity = batch_count;
  ctx->batch_count    = 0;
  ctx->batch_size     = batch_size;

  for (uint32_t i = 0; i < batch_count; ++i) {
    ctx->batches[i].capacity = batch_size;
  }

  if (anim_map_size > 0) {
    ctx->anim_names = (char**)calloc(anim_map_size, sizeof(char*));
    ctx->anims      = (r_anim*)calloc(anim_map_size, sizeof(r_anim));
  } else {
    ctx->anim_names = 0;
    ctx->anims      = 0;
  }

  ctx->anim_count    = 0;
  ctx->anim_capacity = anim_map_size;

  if (shader_map_size > 0) {
    ctx->shaders      = (r_shader*)calloc(shader_map_size, sizeof(r_shader));
    ctx->shader_names = (char**)calloc(shader_map_size, sizeof(char*));
  } else {
    ctx->shaders      = 0;
    ctx->shader_names = 0;
  }

  ctx->shader_count    = 0;
  ctx->shader_capacity = shader_map_size;

  ctx->default_quad = r_quad_create(1.f, 1.f, 0);

  vec3 camera_position = {0.f, 0.f, 0.f};
  vec2 camera_size     = {(float)params.width, (float)params.height};
  ctx->camera = r_camera_create(camera_position, camera_size, -100.f, 100.f);

  return ctx;
}

r_camera* r_ctx_get_camera(r_ctx* ctx) {
  return &ctx->camera;
}

void r_ctx_make_current(r_ctx* ctx) {
  _r_ctx = ctx;
}

void r_ctx_set_i_ctx(r_ctx* ctx, i_ctx* input) {
  ctx->input_ctx = input;
}

void r_ctx_destroy(r_ctx* ctx) {
  if (ctx->anims) {
    for (int i = 0; i < ctx->anim_count; ++i) {
      r_anim* anim = &ctx->anims[i];
      if (anim->count != 0 && anim->frames)
        free(anim->frames);
    }
    free(ctx->anims);
  }

  if (ctx->anim_names) {
    for (int i = 0; i < ctx->anim_capacity; ++i) {
      if (ctx->anim_names[i])
        free(ctx->anim_names[i]);
    }
    free(ctx->anim_names);
  }

  if (ctx->shaders) {
    for (uint16_t i = 0; i < ctx->shader_capacity; ++i) {
      if (ctx->shaders[i])
        glDeleteProgram(ctx->shaders[i]);
    }

    free(ctx->shaders);
    free(ctx->shader_names);
  }

  if (ctx->batches) {
    for (uint16_t i = 0; i < ctx->batch_capacity; ++i) {
      if (ctx->batches[i].mats)
        free(ctx->batches[i].mats);

      if (ctx->batches[i].coords)
        free(ctx->batches[i].coords);

      if (ctx->batches[i].colors)
        free(ctx->batches[i].colors);

      if (ctx->batches[i].flip_x)
        free(ctx->batches[i].flip_x);

      if (ctx->batches[i].flip_y)
        free(ctx->batches[i].flip_y);
    }

    free(ctx->batches);
  }

  r_quad_destroy(&ctx->default_quad);

  r_window_destroy(ctx);
  glfwTerminate();

  free(ctx);
}

void r_ctx_update(r_ctx* ctx) {
  r_camera_update(&ctx->camera);
}

void r_ctx_draw(r_ctx* ctx) {
  for (uint32_t i = 0; i < ctx->batch_capacity; ++i) {
    r_batch* batch = &ctx->batches[i];

    if (batch->count != 0) {
      r_batch_draw(ctx, batch);
    }
  }
}

r_camera r_camera_create(vec3 position, vec2 size, float near, float far) {
  r_camera cam = (r_camera){.near = near, .far = far, .rotation = 0.f};

  mat4x4_identity(cam.projection);
  mat4x4_ortho(cam.projection, 0, size[0], size[1], 0, near, far);
  vec2_dup(cam.size, size);
  vec2_dup(cam.position, position);

  mat4x4_identity(cam.view);
  mat4x4_translate(cam.view, position[0], position[1], 0.f);

  return cam;
}

void r_camera_move(r_camera* camera, vec2 dist) {
  camera->position[0] += dist[0];
  camera->position[1] += dist[1];
}

void r_camera_get_size(vec2 dst, r_camera* camera) {
  vec2_dup(dst, camera->size);
}

void r_camera_screen_to_world(vec2 dst, r_camera* camera, vec2 point) {
  dst[0] = (camera->position[0]) + (camera->size[0] * point[0]);
  dst[1] = (camera->position[1]) + (camera->size[1] * point[1]);
}

void r_camera_world_to_screen(vec2 dst, r_camera* camera, vec2 point) {
  dst[0] = (point[0] - camera->position[0]) / camera->size[0];
  dst[1] = (point[1] - camera->position[1]) / camera->size[1];
}

void r_camera_size_to_screen(vec2 dst, r_camera* camera, vec2 size) {
  vec2_div(dst, size, camera->size);
}

void r_camera_set_size(r_camera* camera, vec2 size) {
  vec2_dup(camera->size, size);
  mat4x4_ortho(camera->projection, 0, camera->size[0], camera->size[1], 0,
               camera->near, camera->far);
}

void r_camera_set_position(r_camera* camera, vec2 position) {
  vec2_dup(camera->position, position);
  r_camera_update(camera);
}

void r_camera_get_position(r_camera* camera, vec2 dst) {
  vec2_dup(dst, camera->position);
}

void r_camera_center_to(r_camera* camera, vec2 point) {
  vec2_dup(camera->position, point);
  vec2 halfsize;
  vec2_scale(halfsize, camera->size, 0.5f);
  vec2_sub(camera->position, camera->position, halfsize);

  r_camera_update(camera);
}

void r_camera_update(r_camera* camera) {
  mat4x4_identity(camera->view);
  mat4x4_translate(camera->view, -camera->position[0], -camera->position[1],
                   camera->position[2]);
}

void r_cam_screen_to_world(vec2 dst, r_camera* camera, vec2 point) {
  if (!dst || !camera || !point) {
    return;
  }

  dst[0] = point[0] * camera->size[0] + camera->position[0];
  dst[1] = point[1] * camera->size[1] + camera->position[1];
}

void r_cam_world_to_screen(vec2 dst, r_camera* camera, vec2 point) {
  if (!dst || !camera || !point) {
    return;
  }

  dst[0] =
      (point[0] / camera->size[0]) - (camera->position[0] / camera->size[0]);
  dst[1] =
      (point[1] / camera->size[1]) - (camera->position[1] / camera->size[1]);
}

r_framebuffer r_framebuffer_create(uint32_t width, uint32_t height,
                                   r_shader shader, uint8_t color_only) {
  r_framebuffer fbo = (r_framebuffer){.width      = width,
                                      .height     = height,
                                      .shader     = shader,
                                      .color_only = color_only};

  glGenFramebuffers(1, &fbo.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);

  glGenTextures(1, &fbo.tex);
  glBindTexture(GL_TEXTURE_2D, fbo.tex);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT,
               NULL);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         fbo.tex, 0);

  if (!color_only) {
    glGenRenderbuffers(1, &fbo.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, fbo.rbo);
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    ASTERA_FUNC_DBG("incomplete FBO: %i\n", fbo.fbo);
    if (!color_only)
      glDeleteRenderbuffers(1, &fbo.rbo);
    glDeleteFramebuffers(1, &fbo.fbo);
    glDeleteTextures(1, &fbo.tex);
    return (r_framebuffer){0};
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  float verts[20] = {-0.5f, -0.5f, 0.f, 0.f, 0.f, -0.5f, 0.5f,  0.f, 0.f, 1.f,
                     0.5f,  0.5f,  0.f, 1.f, 1.f, 0.5f,  -0.5f, 0.f, 1.f, 0.f};

  for (uint8_t i = 0; i < 20; i += 5) {
    verts[i] *= 2;
    verts[i + 1] *= 2;
  }

  uint16_t indices[6] = {0, 1, 2, 2, 3, 0};

  glGenVertexArrays(1, &fbo.vao);
  glGenBuffers(1, &fbo.vbo);
  glGenBuffers(1, &fbo.vboi);

  glBindVertexArray(fbo.vao);

  glBindBuffer(GL_ARRAY_BUFFER, fbo.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20, verts, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (const void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (const void*)12);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo.vboi);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices,
               GL_STREAM_DRAW);

  glBindVertexArray(0);

  mat4x4_identity(fbo.model);

  return fbo;
}

void r_framebuffer_unbind(void) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void r_framebuffer_destroy(r_framebuffer fbo) {
  glDeleteFramebuffers(1, &fbo.fbo);
  glDeleteTextures(1, &fbo.tex);
  glDeleteBuffers(1, &fbo.vbo);
  glDeleteVertexArrays(1, &fbo.vao);
}

void r_framebuffer_bind(r_framebuffer fbo) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
  if (!fbo.color_only) {
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
  }
}

void r_framebuffer_draw(r_ctx* ctx, r_framebuffer fbo) {
  glBindVertexArray(fbo.vao);
  glUseProgram(fbo.shader);

  r_set_uniformf(fbo.shader, "gamma", ctx->window.params.gamma);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, fbo.tex);

  glBindBuffer(GL_ARRAY_BUFFER, fbo.vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo.vboi);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (const void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (const void*)12);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glBindVertexArray(0);
  glUseProgram(0);
}

void r_tex_bind(uint32_t tex) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
}

r_tex r_tex_create(unsigned char* data, uint32_t length) {
  int            w, h, ch;
  unsigned char* img = stbi_load_from_memory(data, length, &w, &h, &ch, 0);
  uint32_t       id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               img);

  stbi_image_free(img);

  return (r_tex){id, (uint32_t)w, (uint32_t)h};
}

void r_tex_destroy(r_tex* tex) {
  glDeleteTextures(1, &tex->id);
}

r_sheet r_sheet_create(unsigned char* data, uint32_t length, vec4* sub_sprites,
                       vec2* origins, uint32_t subsprite_count) {
  if (!data || !length || !sub_sprites || !origins || !subsprite_count) {
    ASTERA_FUNC_DBG("invalid texture data passed.\n");
    return (r_sheet){0};
  }
  // Load the texture data
  int32_t  w, h, ch;
  uint32_t id;

  unsigned char* img = stbi_load_from_memory(data, length, &w, &h, &ch, 0);

  int format = (ch == 4) ? GL_RGBA : (ch == 3) ? GL_RGB : GL_RGB;

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE,
               img);

  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(img);

  return (r_sheet){0};
}

r_sheet r_sheet_create_tiled(unsigned char* data, uint32_t length,
                             uint32_t sub_width, uint32_t sub_height,
                             uint32_t width_pad, uint32_t height_pad) {
  if (!data || !length || !sub_width || !sub_height) {
    ASTERA_FUNC_DBG("invalid texture data passed.\n");
    return (r_sheet){0};
  }

  // Load the texture data
  int32_t  w, h, ch;
  uint32_t id;

  unsigned char* img = stbi_load_from_memory(data, length, &w, &h, &ch, 0);

  int format = (ch == 4) ? GL_RGBA : (ch == 3) ? GL_RGB : GL_RGB;

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE,
               img);

  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(img);

  uint32_t per_width = w / sub_width;
  uint32_t rows      = h / sub_height;
  uint32_t sub_count = rows * per_width;

  r_subtex* subtexs = (r_subtex*)calloc(sub_count, sizeof(r_subtex));

  for (uint32_t i = 0; i < sub_count; ++i) {
    uint32_t x = i % per_width;
    uint32_t y = i / per_width;

    // px values
    float x_offset = (float)((x * sub_width) + width_pad);
    float y_offset = (float)((y * sub_height) + height_pad);
    float width    = (float)(sub_width - (width_pad * 2));
    float height   = (float)(sub_height - (height_pad * 2));

    vec4 coords = {x_offset / w, y_offset / h, (x_offset + width) / w,
                   (y_offset + height) / h};

    uint32_t ox = (uint32_t)(width * 0.5f), oy = (uint32_t)(height * 0.5f);
    vec2     o_offset = {ox / width, oy / height};

    subtexs[i] = (r_subtex){.x      = (uint32_t)x_offset,
                            .y      = (uint32_t)y_offset,
                            .ox     = ox,
                            .oy     = oy,
                            .width  = (uint32_t)width,
                            .height = (uint32_t)height};
    vec2_dup(subtexs[i].o_offset, o_offset);
    vec4_dup(subtexs[i].coords, coords);
  }

  return (r_sheet){.id       = id,
                   .width    = (uint32_t)w,
                   .height   = (uint32_t)h,
                   .subtexs  = subtexs,
                   .count    = sub_count,
                   .capacity = sub_count};
}

void r_sheet_destroy(r_sheet* sheet) {
  glDeleteTextures(1, &sheet->id);
  free(sheet->subtexs);
}

r_baked_sheet r_baked_sheet_create(r_sheet* sheet, r_baked_quad* quads,
                                   uint32_t quad_count, vec2 position) {
  if (!quads || !quad_count) {
    ASTERA_FUNC_DBG("invalid quad parameters.\n");
    return (r_baked_sheet){0};
  }

  // vert: (x, y, z, s, t)
  uint32_t vert_cap = quad_count * 4 * 5, vert_count = 0;
  // ind: (0, 1, 2, 2, 3, 0)
  uint32_t ind_cap = quad_count * 6, ind_count = 0;
  // unique verts added (for ind offset calculation)
  uint32_t uvert_count = 0;

  float*    verts = (float*)calloc(vert_cap, sizeof(float));
  uint32_t* inds  = (uint32_t*)calloc(ind_cap, sizeof(uint32_t));

  uint32_t _inds[6]  = {0, 1, 2, 2, 3, 0};
  float    _verts[8] = {-0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f};
  float    _texcs[8] = {0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f};

  vec4 bounds = {0.f, 0.f, 0.f, 0.f};

  for (uint32_t i = 0; i < quad_count; ++i) {
    r_baked_quad* quad = &quads[i];

    if (quad->subtex >= sheet->count) {
      continue;
    }

    r_subtex* subtex = &sheet->subtexs[quad->subtex];

    vec2 _offset = {quad->x, quad->y};
    vec2 _size   = {quad->width, quad->height};

    vec2 _tex_offset = {subtex->coords[0], subtex->coords[1]};
    vec2 _tex_size   = {subtex->coords[2], subtex->coords[3]};
    vec2_sub(_tex_size, _tex_size, _tex_offset);

    for (uint8_t j = 0; j < 4; ++j) {
      verts[vert_count]     = (_verts[j * 2] * _size[0]) + _offset[0];
      verts[vert_count + 1] = (_verts[(j * 2) + 1] * _size[1]) + _offset[1];
      verts[vert_count + 2] = (float)(quad->layer * ASTERA_RENDER_LAYER_MOD);

      float sample_x = _texcs[j * 2];
      float sample_y = _texcs[(j * 2) + 1];

      if (quad->flip_x) {
        sample_x = 1.f - sample_x;
      }

      if (quad->flip_y) {
        sample_y = 1.f - sample_y;
      }

      verts[vert_count + 3] = (sample_x * _tex_size[0]) + _tex_offset[0];
      verts[vert_count + 4] = (sample_y * _tex_size[1]) + _tex_offset[1];

      vert_count += 5;
    }

    for (uint8_t j = 0; j < 6; ++j) {
      inds[ind_count] = _inds[j] + uvert_count;
      ++ind_count;
    }
    uvert_count += 4;
  }

  uint32_t vao, vbo, vboi;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glGenBuffers(1, &vboi);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vert_count, verts,
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (const void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (const void*)12);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * ind_count, inds,
               GL_STREAM_DRAW);

  glBindVertexArray(0);

  free(verts);
  free(inds);

  r_baked_sheet baked_sheet = (r_baked_sheet){
      .vao        = vao,
      .vbo        = vbo,
      .vboi       = vboi,
      .quad_count = quad_count,
      .sheet      = sheet,
  };

  vec2 sheet_size = {bounds[2] - bounds[0], bounds[3] - bounds[1]};
  vec2_dup(baked_sheet.size, sheet_size);
  vec2_dup(baked_sheet.position, position);

  mat4x4_identity(baked_sheet.model);
  mat4x4_translate(baked_sheet.model, position[0], position[1], 0.f);
  mat4x4_scale_aniso(baked_sheet.model, baked_sheet.model, 1.f, 1.f, 1.f);

  return baked_sheet;
}

void r_baked_sheet_draw(r_ctx* ctx, r_shader shader, r_baked_sheet* sheet) {
  if (shader == 0) {
    ASTERA_FUNC_DBG("invalid shader.\n");
    return;
  }

  r_shader_bind(shader);

  r_set_m4(shader, "projection", ctx->camera.projection);
  r_set_m4(shader, "view", ctx->camera.view);
  r_set_m4(shader, "model", sheet->model);

  r_tex_bind(sheet->sheet->id);

  glBindVertexArray(sheet->vao);

  glBindBuffer(GL_ARRAY_BUFFER, sheet->vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sheet->vboi);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (const void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (const void*)12);

  glDrawElements(GL_TRIANGLES, sheet->quad_count * 8, GL_UNSIGNED_INT, 0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  glBindVertexArray(0);
  r_tex_bind(0);
  r_shader_bind(0);
}

void r_baked_sheet_destroy(r_baked_sheet* sheet) {
  glDeleteBuffers(1, &sheet->vbo);
  glDeleteBuffers(1, &sheet->vto);
  glDeleteBuffers(1, &sheet->vboi);
  glDeleteVertexArrays(1, &sheet->vao);
}

r_particles r_particles_create(uint32_t emit_rate, float particle_life,
                               uint32_t particle_capacity, uint32_t emit_count,
                               int8_t particle_type, int8_t calculate,
                               uint16_t uniform_cap) {
  r_particles particles = (r_particles){0};

  if (!particle_capacity) {
    return particles;
  }

  particles.time       = 0;
  particles.spawn_time = 0;
  particles.alive      = 0;
  particles.spawn_rate = MS_TO_SEC / emit_rate;

  particles.calculate      = calculate;
  particles.max_emission   = emit_count;
  particles.emission_count = 0;

  if (calculate) {
    particles.mats   = (mat4x4*)malloc(sizeof(mat4x4) * uniform_cap);
    particles.colors = (vec4*)malloc(sizeof(vec4) * uniform_cap);
    particles.coords = (vec4*)malloc(sizeof(vec4) * uniform_cap);
  }

  particles.uniform_cap = uniform_cap;

  particles.particle_life = particle_life;

  particles.type = particle_type;

  particles.particle_size[0] = 1.f;
  particles.particle_size[1] = 1.f;

  particles.position[0] = 0.f;
  particles.position[1] = 0.f;

  particles.size[0] = 0.f;
  particles.size[1] = 0.f;

  particles.list = (r_particle*)calloc(particle_capacity, sizeof(r_particle));

  particles.capacity = particle_capacity;
  particles.count    = 0;

  return particles;
}

void r_particles_start(r_particles* particles) {
  r_particles_reset(particles);
  particles->alive = 1;
}

void r_particles_stop(r_particles* particles) {
  particles->alive = 0;
}

void r_particles_resume(r_particles* particles) {
  particles->alive = 1;
}

void r_particles_reset(r_particles* particles) {
  particles->time           = 0.f;
  particles->spawn_time     = 0.f;
  particles->count          = 0;
  particles->emission_count = 0;
  particles->uniform_count  = 0;
  particles->alive          = 0;
  memset(particles->mats, 0, sizeof(mat4x4) * particles->uniform_cap);
  memset(particles->colors, 0, sizeof(vec4) * particles->uniform_cap);
  memset(particles->coords, 0, sizeof(vec4) * particles->uniform_cap);
}

uint8_t r_particles_finished(r_particles* particles) {
  return !particles->alive;
}

uint32_t r_particles_frame_at(r_particles* system, time_s time) {
  if (system->type == PARTICLE_ANIMATED) {
    return r_anim_frame_at(system->render.anim.anim, time);
  } else {
    return system->render.subtex;
  }
}

void r_particles_set_system(r_particles* system, float lifetime,
                            float prespawn) {
  system->system_life = lifetime;

  float change = prespawn;
  if (system->prespawn != 0.f) {
    change -= system->prespawn;
  }

  system->prespawn = prespawn;

  if (change > 0.f) {
    uint32_t rounds = (uint32_t)(change / 100.f);
    for (uint32_t i = 0; i < rounds; ++i) {
      r_particles_update(system, (change > 100.f) ? 100.f : change);
      change -= 100.f;
    }
  }
}

void r_particles_set_position(r_particles* system, vec2 position) {
  vec2_dup(system->position, position);
}

void r_particles_set_size(r_particles* system, vec2 size) {
  vec2_dup(system->size, size);
}

void r_particles_set_particle(r_particles* system, vec4 color,
                              float particle_life, vec2 particle_size,
                              vec2 particle_velocity) {
  if (!system)
    return;

  if (color) {
    vec4_dup(system->color, color);
  }

  if (particle_life != 0.f) {
    system->particle_life = particle_life;
  }

  if (particle_size) {
    vec2_dup(system->particle_size, particle_size);
  }

  if (particle_velocity) {
    vec2_dup(system->particle_velocity, particle_velocity);
  }
}

void r_particles_update(r_particles* system, time_s delta) {
  if (system->alive) {
    system->time += (float)delta;
    system->spawn_time += (float)delta;
    int32_t to_spawn = (int32_t)system->spawn_time / system->spawn_rate;

    // cap to capacity of the particle system's bufferse
    if (system->count + to_spawn >= system->capacity) {
      to_spawn = system->capacity - system->count;
    }

    // cap to max emission
    /*if (system->max_emission) {
      if (to_spawn + system->emission_count > system->max_emission) {
        to_spawn = system->max_emission - system->emission_count;
      }
    }*/

    system->spawn_time -= system->spawn_rate * to_spawn;
    if (((system->time > system->system_life && system->system_life > 0.f) ||
         (system->emission_count > system->max_emission &&
          system->max_emission > 0))) {
      if (system->count == 0) {
        system->alive = 0;
        return;
      } else {
        to_spawn = 0;
      }
    }

    for (int i = 0; i < to_spawn; ++i) {
      r_particle* open = 0;

      for (uint32_t j = 0; j < system->capacity; ++j) {
        if (system->list[j].life <= 0.f) {
          open = &system->list[j];
          break;
        }
      }

      if (!open) {
        break;
      }

      open->life = system->particle_life;
      vec2_dup(open->size, system->particle_size);
      vec2_dup(open->velocity, system->particle_velocity);
      vec4_dup(open->color, system->color);

      if (system->use_spawner) {
        system->spawner_func(system, open);
      } else {
        open->position[0] = fmodf((float)rand(), system->size[0]);
        open->position[1] = fmodf((float)rand(), system->size[1]);

        open->layer = system->particle_layer;

        if (system->type == PARTICLE_ANIMATED) {
          open->frame = 0;
        } else if (system->type == PARTICLE_TEXTURED) {
          open->frame = system->render.subtex;
        }
      }

      ++system->emission_count;
      ++system->count;
    }
  }

  if (system->count > 0) {
    // Update particles
    for (uint32_t i = 0; i < system->capacity; ++i) {
      r_particle* particle = &system->list[i];
      if (particle->life > 0.f) {
        particle->life -= (float)delta;
      } else {
        continue;
      }

      if (particle->life <= 0.f) {
        particle->life = 0.f;
        --system->count;
        continue;
      }

      vec2 movement = {0.f, 0.f};
      vec2_scale(movement, particle->velocity, (const float)delta);
      vec2_add(particle->position, particle->position, movement);

      if (system->use_animator) {
        (*system->animator_func)(system, &system->list[i]);
      } else if (system->type == PARTICLE_ANIMATED) {
        float lifespan  = system->particle_life - particle->life;
        particle->frame = r_anim_frame_at(system->render.anim.anim, lifespan);
        if (particle->frame > system->render.anim.count) {
          particle->frame = system->render.anim.count;
        }
      }
    }
  }
}

void r_particles_set_anim(r_particles* particles, r_anim* anim) {
  if (!particles)
    return;

  particles->sheet       = anim->sheet;
  particles->render.anim = r_anim_create_viewer(anim);
}

void r_particles_set_subtex(r_particles* particles, r_sheet* sheet,
                            uint32_t subtex) {
  if (!particles)
    return;

  particles->sheet         = sheet;
  particles->render.subtex = subtex;
}

void r_particles_set_color(r_particles* particles, vec4 color,
                           uint8_t color_only) {
  if (!particles || !color)
    return;

  vec4_dup(particles->color, color);

  if (color_only) {
    particles->type = PARTICLE_COLORED;
  }
}

void r_particles_destroy(r_particles* particles) {
  free(particles->list);

  if (particles->calculate) {
    free(particles->colors);
    free(particles->coords);
    free(particles->mats);
  }

  particles->capacity = 0;
  particles->count    = 0;
}

static void r_particles_render(r_ctx* ctx, r_particles* particles,
                               r_shader shader) {
  r_shader_bind(shader);
  if ((particles->type == PARTICLE_ANIMATED ||
       particles->type == PARTICLE_TEXTURED) &&
      particles->sheet) {
    r_tex_bind(particles->sheet->id);
    r_set_uniformi(shader, "use_tex", 1);
  } else {
    r_set_uniformi(shader, "use_tex", 0);
  }

  r_set_m4(shader, "view", ctx->camera.view);
  r_set_m4(shader, "projection", ctx->camera.projection);
  // r_set_m4(shader, "model", system->model);

  r_set_v4x(shader, particles->uniform_count, "coords", particles->coords);
  r_set_v4x(shader, particles->uniform_count, "colors", particles->colors);
  r_set_m4x(shader, particles->uniform_count, "mats", particles->mats);

  glBindVertexArray(ctx->default_quad.vao);
  glBindBuffer(GL_ARRAY_BUFFER, ctx->default_quad.vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->default_quad.vboi);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (const void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (const void*)12);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0,
                          particles->uniform_count);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  glBindVertexArray(0);
  r_tex_bind(0);
  r_shader_bind(0);

  // Clear out the uniforms for the next draw call
  memset(particles->mats, 0, sizeof(mat4x4) * particles->uniform_count);
  memset(particles->colors, 0, sizeof(vec4) * particles->uniform_count);
  memset(particles->coords, 0, sizeof(vec4) * particles->uniform_count);
  particles->uniform_count = 0;
}

void r_particles_draw(r_ctx* ctx, r_particles* particles, r_shader shader) {
  if (particles->calculate) {
    r_sheet* sheet = particles->sheet;

    mat4x4_translate(particles->model, particles->position[0],
                     particles->position[1], 0);

    for (uint32_t i = 0; i < particles->capacity; ++i) {
      r_particle* particle = &particles->list[i];

      if (particle->life > 0.f) {
        mat4x4* mat = &particles->mats[particles->uniform_count];

        mat4x4_identity(*mat);
        mat4x4_translate(*mat, particles->position[0] + particle->position[0],
                         particles->position[1] + particle->position[1],
                         particle->layer * ASTERA_RENDER_LAYER_MOD);
        mat4x4_scale_aniso(*mat, *mat, particle->size[0], particle->size[1],
                           1.f);
        mat4x4_rotate_z(*mat, *mat, particle->rotation);

        vec4_dup(particles->colors[particles->uniform_count], particle->color);

        if (sheet) {
          if (particles->type == PARTICLE_TEXTURED ||
              particles->type == PARTICLE_ANIMATED) {
            vec4_dup(particles->coords[particles->uniform_count],
                     sheet->subtexs[particle->frame].coords);
          }
        }

        ++particles->uniform_count;
      }

      if (particles->uniform_count == particles->uniform_cap) {
        r_particles_render(ctx, particles, shader);
      }
    }

    if (particles->uniform_count != 0) {
      r_particles_render(ctx, particles, shader);
    }
  } else {
    r_particles_render(ctx, particles, shader);
  }
}

void r_particles_set_spawner(r_particles* system, r_particle_spawner spawner) {
  system->use_spawner  = 1;
  system->spawner_func = spawner;
}

void r_particles_set_animator(r_particles*        system,
                              r_particle_animator animator) {
  system->use_animator  = 1;
  system->animator_func = animator;
}

void r_particles_remove_spawner(r_particles* system) {
  system->use_spawner  = 0;
  system->spawner_func = 0;
}

void r_particles_remove_animator(r_particles* system) {
  system->use_animator  = 0;
  system->animator_func = 0;
}

void r_sprite_move(r_sprite* sprite, vec2 dist) {
  vec2_add(sprite->position, sprite->position, dist);
}

void r_sprite_draw(r_ctx* ctx, r_sprite* sprite) {
  if (!sprite->visible) {
    return;
  }

  r_shader_bind(sprite->shader);

  r_sheet* sheet = sprite->sheet;
  r_tex_bind(sheet->id);

  vec2 sheet_size = {(float)sheet->width, (float)sheet->height};
  r_set_v2(sprite->shader, "sheet_size", sheet_size);

  r_set_m4(sprite->shader, "view", ctx->camera.view);
  r_set_m4(sprite->shader, "projection", ctx->camera.projection);

  r_set_uniformi(sprite->shader, "flip_x", sprite->flip_x);
  r_set_uniformi(sprite->shader, "flip_y", sprite->flip_y);

  r_set_v4(sprite->shader, "color", sprite->color);
  r_set_m4(sprite->shader, "model", sprite->model);

  if (sprite->animated) {
    r_set_v4(sprite->shader, "coords",
             sheet
                 ->subtexs[sprite->render.anim.anim
                               ->frames[sprite->render.anim.curr]]
                 .coords);
  } else {
    r_set_v4(sprite->shader, "coords",
             sheet->subtexs[sprite->render.tex].coords);
  }

  glBindVertexArray(ctx->default_quad.vao);
  glBindBuffer(GL_ARRAY_BUFFER, ctx->default_quad.vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->default_quad.vboi);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (const void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (const void*)12);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glBindVertexArray(0);
  r_tex_bind(0);
  r_shader_bind(0);
}

void r_sprite_draw_batch(r_ctx* ctx, r_sprite* sprite) {
  if (!sprite->visible) {
    return;
  }

  r_batch* batch = r_batch_get(ctx, sprite->sheet, sprite->shader);

  if (batch) {
    if (batch->count == batch->capacity) {
      r_batch_draw(ctx, batch);
    }

    r_batch_add(batch, sprite);
  }
}

uint32_t r_sprites_draw(r_ctx* ctx, r_sprite* sprites, uint32_t sprite_count) {
  if (!sprites || !sprite_count || !ctx) {
    ASTERA_FUNC_DBG("no sprites passed.\n");
    return 0;
  }

  r_batch* batch = r_batch_get(ctx, sprites[0].sheet, sprites[0].shader);

  if (batch) {
    uint32_t remaining = sprite_count, used = 0;
    uint32_t index = 0;
    while (1) {
      uint32_t open = batch->capacity - batch->count;
      used          = (open < remaining) ? open : remaining;

      uint32_t used_count = r_batch_add_multi(batch, &sprites[index], used);

      index += used_count;
      remaining -= used_count;

      if (remaining && used)
        r_batch_draw(ctx, batch);
      else
        break;
    }

    return index;
  }

  ASTERA_FUNC_DBG("No batch found\n");
  return 0;
}

uint8_t r_sprite_get_anim_state(r_sprite* sprite) {
  if (!sprite->animated) {
    return 0;
  }

  return sprite->render.anim.state;
}

void r_sprite_anim_play(r_sprite* sprite) {
  if (!sprite->animated)
    return;
  r_anim_play(&sprite->render.anim);
}

void r_sprite_anim_pause(r_sprite* sprite) {
  if (!sprite->animated)
    return;
  r_anim_pause(&sprite->render.anim);
}

void r_sprite_anim_stop(r_sprite* sprite) {
  if (!sprite->animated)
    return;
  r_anim_stop(&sprite->render.anim);
}

static GLuint r_shader_create_sub(unsigned char* data, int type) {
  GLint  success = 0;
  GLuint id      = glCreateShader(type);

  const char* ptr = (const char*)data;

  glShaderSource(id, 1, &ptr, NULL);
  glCompileShader(id);

  glGetShaderiv(id, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    int maxlen = 0;
    int len;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxlen);

    char* log = malloc(maxlen);

    glGetShaderInfoLog(id, maxlen, &len, log);
    printf("%s: %s\n", (type == GL_FRAGMENT_SHADER) ? "FRAGMENT" : "VERTEX",
           log);
    ASTERA_DBG("%s: %s\n", (type == GL_FRAGMENT_SHADER) ? "FRAGMENT" : "VERTEX",
               log);
    free(log);
  }

  return id;
}

r_shader r_shader_get(r_ctx* ctx, const char* name) {
  for (uint8_t i = 0; i < ctx->shader_count; ++i) {
    if (strcmp(name, ctx->shader_names[i]) == 0) {
      return ctx->shaders[i];
    }
  }
  return 0;
}

r_shader r_shader_create(unsigned char* vert_data, unsigned char* frag_data) {
  GLuint v = r_shader_create_sub(vert_data, GL_VERTEX_SHADER);
  GLuint f = r_shader_create_sub(frag_data, GL_FRAGMENT_SHADER);

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
    char* log = malloc(maxlen);
    glGetProgramInfoLog(id, maxlen, &len, log);
    ASTERA_FUNC_DBG("%s\n", log);
    printf("%s\n", log);
    free(log);
  }

  return (r_shader)id;
}

void r_shader_cache(r_ctx* ctx, r_shader shader, const char* name) {
  if (!shader) {
    ASTERA_FUNC_DBG("invalid shader (%i) passed.\n", shader);
    return;
  }

  if (ctx->shader_capacity == ctx->shader_count || ctx->shader_capacity == 0) {
    ASTERA_FUNC_DBG("no shader cache available.\n");
    return;
  }

  if (!ctx->shaders) {
    ASTERA_FUNC_DBG("no shader cache allocated.\n");
    return;
  }

  if (ctx->shader_count > 0) {
    for (uint8_t i = 0; i < ctx->shader_capacity; ++i) {
      if (ctx->shaders[i] == shader) {
        ASTERA_FUNC_DBG("shader %d already contained with an alias "
                        "of: %s\n",
                        shader, ctx->shader_names[i]);
        return;
      }
    }
  }

  ctx->shader_names[ctx->shader_count] = name;
  ctx->shaders[ctx->shader_count]      = shader;
  ++ctx->shader_count;
}

void r_shader_bind(r_shader shader) {
  glUseProgram(shader);
}

void r_shader_destroy(r_ctx* ctx, r_shader shader) {
  glDeleteProgram(shader);

  int8_t start = 0;
  for (uint8_t i = 0; i < ctx->shader_count - 1; ++i) {
    if (ctx->shaders[i] == shader) {
      start = 1;
    }

    if (start) {
      ctx->shaders[i]      = ctx->shaders[i + 1];
      ctx->shader_names[i] = ctx->shader_names[i + 1];
    }
  }

  ctx->shaders[ctx->shader_count]      = 0;
  ctx->shader_names[ctx->shader_count] = 0;

  --ctx->shader_count;
}

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

static int r_hex_multi(const char* v, int len) {
  if (len == 2) {
    return r_hex_number(v[0]) * 16 + r_hex_number(v[1]);
  } else if (len == 1) {
    return r_hex_number(v[0]) * 16 + r_hex_number(v[0]);
  }
  return -1;
}

void r_get_color3f(vec3 val, const char* v) {
  int len    = strlen(v);
  int offset = 0;
  if (len == 4) {
    offset = 1;
    len    = 3;
  } else if (len == 7) {
    offset = 1;
    len    = 6;
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

void r_get_color4f(vec4 val, const char* v) {
  r_get_color3f(val, v);
  val[3] = 1.f;
}

r_anim_viewer r_anim_create_viewer(r_anim* anim) {
  if (!anim)
    return (r_anim_viewer){0};

  return (r_anim_viewer){.anim   = anim,
                         .count  = anim->count,
                         .rate   = anim->rate,
                         .state  = R_ANIM_STOP,
                         .pstate = R_ANIM_STOP,
                         .loop   = anim->loop};
}

uint32_t r_anim_frame_at(r_anim* anim, time_s time) {
  if (anim->lengths && anim->rate <= 0.f) {
    time_s last = 0.f;
    for (uint32_t i = 0; i < anim->count; ++i) {
      time_s this = anim->lengths[i];
      if (last + this >= time) {
        return i;
      } else {
        last += this;
      }
    }

    if (anim->loop && time > last) {
#if defined(ASTERA_SYS_LOWP_TIME)
      time = fmodf(time, last);
#else
      time = fmod(time, last);
#endif

      return r_anim_frame_at(anim, time);
    }

    return anim->count;
  } else {
    return (uint32_t)(time / anim->rate) % anim->count;
  }
}

r_anim r_anim_create_fixed(r_sheet* sheet, uint32_t* frames, uint32_t count,
                           uint32_t rate) {
  uint32_t* cpy_frames = (uint32_t*)malloc(sizeof(uint32_t) * count);

  time_s _rate = MS_TO_SEC / rate;

  for (uint32_t i = 0; i < count; ++i) {
    cpy_frames[i] = frames[i];
  }

  return (r_anim){.id      = 0,
                  .frames  = cpy_frames,
                  .lengths = 0,
                  .count   = count,
                  .rate    = _rate,
                  .sheet   = sheet,
                  .loop    = 0};
}

r_anim r_anim_create(r_sheet* sheet, uint32_t* frames, time_s* lengths,
                     uint32_t count) {
  uint32_t* cpy_frames = (uint32_t*)malloc(sizeof(uint32_t) * count);
  time_s*   cpy_times  = (time_s*)malloc(sizeof(time_s) * count);

  for (uint32_t i = 0; i < count; ++i) {
    cpy_frames[i] = frames[i];
    cpy_times[i]  = lengths[i];
  }

  return (r_anim){.id      = 0,
                  .frames  = cpy_frames,
                  .lengths = cpy_times,
                  .rate    = 0.f,
                  .count   = count,
                  .sheet   = sheet,
                  .loop    = 0};
}

void r_anim_destroy(r_ctx* ctx, r_anim* anim) {
  free(anim->frames);
  ctx->anims[anim->id] = (r_anim){0};
  --ctx->anim_count;
}

r_anim* r_anim_cache(r_ctx* ctx, r_anim anim, const char* name) {
  if (ctx->anim_count >= ctx->anim_capacity) {
    ASTERA_FUNC_DBG("Animation cache at capacity.\n");
    return 0;
  }

  for (uint32_t i = 0; i < ctx->anim_capacity; ++i) {
    r_anim* slot = &ctx->anims[i];

    if (!slot->frames) {
      *slot              = anim;
      uint32_t len       = strlen(name);
      ctx->anim_names[i] = (char*)calloc(sizeof(char), len + 1);
      strcpy(ctx->anim_names[i], name);

      if (i > ctx->anim_high) {
        ctx->anim_high = i;
      }

      slot->id = i;

      ++ctx->anim_count;

      return slot;
    }
  }

  return 0;
}

void r_anim_list_cache(r_ctx* ctx) {
  if (ctx->anim_count == 0) {
    ASTERA_FUNC_DBG("No anims in cache\n");
    return;
  }
  for (uint32_t i = 0; i < ctx->anim_capacity; ++i) {
    r_anim* slot = &ctx->anims[i];

    if (slot->frames && ctx->anim_names[i]) {
      ASTERA_DBG("%i: %s\n", i, ctx->anim_names[i]);
    }
  }
}

void r_anim_play(r_anim_viewer* anim) {
  anim->pstate = anim->state;
  anim->state  = R_ANIM_PLAY;
}

void r_anim_stop(r_anim_viewer* anim) {
  anim->pstate = anim->state;
  anim->state  = R_ANIM_STOP;
  anim->time   = 0;
  anim->curr   = 0;
}

void r_anim_pause(r_anim_viewer* anim) {
  anim->pstate = anim->state;
  anim->state  = R_ANIM_PAUSE;
}

void r_anim_reset(r_anim_viewer* anim) {
  anim->pstate = 0;
  anim->state  = 0;
  anim->time   = 0;
  anim->curr   = 0;
}

r_anim* r_anim_get(r_ctx* ctx, uint32_t id) {
  if (ctx->anim_count < id) {
    return 0;
  }

  if (ctx->anims[id].frames) {
    return &ctx->anims[id];
  }

  return 0;
}

r_anim* r_anim_get_name(r_ctx* ctx, const char* name) {
  if (ctx->anim_count == 0) {
    ASTERA_FUNC_DBG("no animations in cache.\n");
    return 0;
  }

  for (uint32_t i = 0; i < ctx->anim_capacity; ++i) {
    if (ctx->anim_names[i]) {
      if (strcmp(ctx->anim_names[i], name) == 0) {
        return &ctx->anims[i];
      }
    }
  }

  return 0;
}

r_anim r_anim_remove(r_ctx* ctx, uint32_t id) {
  if (ctx->anim_count < id) {
    ASTERA_FUNC_DBG("no animations in cache.\n");
    return (r_anim){0};
  }

  if (ctx->anims[id].frames) {
    r_anim ret = ctx->anims[id];

    ctx->anims[id].id     = 0;
    ctx->anims[id].frames = 0;

    if (id >= ctx->anim_high) {
      // recurse down to the next available animation
      for (uint32_t i = ctx->anim_high; i > 0; --i) {
        if (ctx->anims[i].frames) {
          ctx->anim_high = (uint16_t)i;
          break;
        }
      }
    }

    return ret;
  }

  return (r_anim){0};
}

r_anim r_anim_remove_name(r_ctx* ctx, const char* name) {
  r_anim* anim = r_anim_get_name(ctx, name);
  return r_anim_remove(ctx, anim->id);
}

r_subtex* r_subtex_create_tiled(r_sheet* sheet, uint32_t id, uint32_t width,
                                uint32_t height, uint32_t width_pad,
                                uint32_t height_pad) {
  if (!sheet) {
    ASTERA_FUNC_DBG("no sheet passed.\n");
    return 0;
  }

  if (sheet->count >= sheet->capacity - 1) {
    ASTERA_FUNC_DBG("no free space in sheet.\n");
    return 0;
  }

  r_subtex* tex = &sheet->subtexs[sheet->count];

  uint32_t per_width = sheet->width / width;
  uint32_t x         = id % per_width;
  uint32_t y         = id / per_width;

  *tex = (r_subtex){.x      = x * width + width_pad,
                    .y      = y * height + height_pad,
                    .width  = width - (width_pad * 2),
                    .height = height - (height_pad * 2)};

  tex->coords[0] = (float)(tex->x / sheet->width);
  tex->coords[1] = (float)(tex->y / sheet->height);
  tex->coords[2] = (float)(tex->coords[0] + (tex->width / sheet->width));
  tex->coords[3] = (float)(tex->coords[1] + (tex->height / sheet->height));

  tex->sub_id = sheet->count;
  ++sheet->count;

  return tex;
}

r_subtex* r_subtex_create(r_sheet* sheet, uint32_t x, uint32_t y,
                          uint32_t width, uint32_t height) {
  if (!sheet) {
    ASTERA_FUNC_DBG("no sheet passed.\n");
    return 0;
  }

  if (sheet->count >= sheet->capacity - 1) {
    ASTERA_FUNC_DBG("no free space in sheet.\n");
    return 0;
  }

  r_subtex* tex = &sheet->subtexs[sheet->count];

  *tex = (r_subtex){
      .x      = x,
      .y      = y,
      .width  = width,
      .height = height,
  };

  tex->coords[0] = (float)(x / sheet->width);
  tex->coords[1] = (float)(y / sheet->height);
  tex->coords[2] = tex->coords[0] + (float)(width / sheet->width);
  tex->coords[3] = tex->coords[1] + (float)(height / sheet->height);

  tex->sub_id = sheet->count;
  ++sheet->count;

  return tex;
}

void r_sprite_set(r_sprite* sprite, uint8_t layer, uint8_t flip_x,
                  uint8_t flip_y) {
  sprite->layer  = layer;
  sprite->flip_x = flip_x;
  sprite->flip_y = flip_y;
}

void r_sprite_set_pos(r_sprite* sprite, vec2 pos) {
  vec2_dup(sprite->position, pos);
}

void r_sprite_get_pos(vec2 dst, r_sprite* sprite) {
  vec2_dup(dst, sprite->position);
}

void r_sprite_set_anim(r_sprite* sprite, r_anim* anim) {
  sprite->render.anim = r_anim_create_viewer(anim);
  sprite->animated    = 1;
  sprite->sheet       = anim->sheet;
}

void r_sprite_set_tex(r_sprite* sprite, r_sheet* sheet, uint32_t id) {
  sprite->animated   = 0;
  sprite->render.tex = id;
  sprite->sheet      = sheet;
}

void r_sprite_set_colori(r_sprite* sprite, uint8_t index, float value) {
  sprite->color[index] = value;
}

void r_sprite_set_color(r_sprite* sprite, vec4 color) {
  vec4_dup(sprite->color, color);
}

r_sprite r_sprite_create(r_shader shader, vec2 pos, vec2 size) {
  r_sprite sprite = (r_sprite){0};

  if (pos) {
    vec2_dup(sprite.position, pos);
  }

  if (size) {
    vec2_dup(sprite.size, size);
  }

  mat4x4_identity(sprite.model);
  mat4x4_translate(sprite.model, sprite.position[0], sprite.position[1], 0.f);
  mat4x4_scale_aniso(sprite.model, sprite.model, sprite.size[0], sprite.size[1],
                     1.f);

  sprite.layer = 0;

  for (uint8_t i = 0; i < 4; ++i) {
    sprite.color[i] = 1.f;
  }

  sprite.flip_x = 0;
  sprite.flip_y = 0;

  sprite.visible = 1;
  sprite.shader  = shader;

  return sprite;
}

void r_sprite_update(r_sprite* sprite, long delta) {
  if (sprite->animated) {
    r_anim_viewer* view = &sprite->render.anim;

    if (view->state == R_ANIM_PLAY) {
      time_s frame_time = 0.f;
      if (view->anim->rate > 0.f) {
        frame_time = view->rate;
      } else {
        frame_time = view->anim->lengths[view->curr];
      }

      if (view->time + delta >= frame_time) {
        if (view->curr >= view->count - 1) {
          if (!view->loop) {
            view->state  = R_ANIM_STOP;
            view->pstate = R_ANIM_PLAY;
          }
          view->curr = 0;
        } else {
          view->curr++;
        }

        view->time -= frame_time;
      } else {
        view->time += delta;
      }
    }
  }

  mat4x4_translate(sprite->model, sprite->position[0], sprite->position[1],
                   (sprite->layer * ASTERA_RENDER_LAYER_MOD));
  mat4x4_scale_aniso(sprite->model, sprite->model, sprite->size[0],
                     sprite->size[0], sprite->size[1]);

  sprite->change = 0;
}

void r_set_uniformf(r_shader shader, const char* name, float value) {
  glUniform1f(glGetUniformLocation(shader, name), value);
}

void r_set_uniformfi(int loc, float value) {
  glUniform1f(loc, value);
}

void r_set_uniformi(r_shader shader, const char* name, int value) {
  glUniform1i(glGetUniformLocation(shader, name), value);
}

void r_set_uniformii(int loc, int val) {
  glUniform1i(loc, val);
}

void r_set_v4(r_shader shader, const char* name, vec4 value) {
  glUniform4f(glGetUniformLocation(shader, name), value[0], value[1], value[2],
              value[3]);
}

void r_set_v4i(int loc, vec4 value) {
  glUniform4f(loc, value[0], value[1], value[2], value[3]);
}

void r_set_v3(r_shader shader, const char* name, vec3 value) {
  glUniform3f(glGetUniformLocation(shader, name), value[0], value[1], value[2]);
}

void r_set_v3i(int loc, vec3 val) {
  glUniform3f(loc, val[0], val[1], val[2]);
}

void r_set_v2(r_shader shader, const char* name, vec2 value) {
  glUniform2f(glGetUniformLocation(shader, name), value[0], value[1]);
}

void r_set_v2i(int loc, vec2 val) {
  glUniform2f(loc, val[0], val[1]);
}

void r_set_m4(r_shader shader, const char* name, mat4x4 value) {
  glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE,
                     (GLfloat*)value);
}

void r_set_m4i(int loc, mat4x4 val) {
  glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)val);
}

int r_get_loc(r_shader shader, const char* name) {
  return glGetUniformLocation(shader, name);
}

void r_set_m4x(r_shader shader, uint32_t count, const char* name,
               mat4x4* values) {
  if (!count)
    return;
  glUniformMatrix4fv(glGetUniformLocation(shader, name), count, GL_FALSE,
                     (const GLfloat*)values);
}

void r_set_ix(r_shader shader, uint32_t count, const char* name, int* values) {
  if (!count)
    return;
  glUniform1iv(glGetUniformLocation(shader, name), count, (const GLint*)values);
}

void r_set_fx(r_shader shader, uint32_t count, const char* name,
              float* values) {
  if (!count)
    return;
  glUniform1fv(glGetUniformLocation(shader, name), count,
               (const GLfloat*)values);
}

void r_set_v2x(r_shader shader, uint32_t count, const char* name,
               vec2* values) {
  if (!count)
    return;

  glUniform2fv(glGetUniformLocation(shader, name), count,
               (const GLfloat*)values);
}

void r_set_v3x(r_shader shader, uint32_t count, const char* name,
               vec3* values) {
  if (!count)
    return;

  glUniform3fv(glGetUniformLocation(shader, name), count,
               (const GLfloat*)values);
}

void r_set_v4x(r_shader shader, uint32_t count, const char* name,
               vec4* values) {
  if (!count)
    return;

  glUniform4fv(glGetUniformLocation(shader, name), count,
               (const GLfloat*)values);
}

void r_set_m4xi(int loc, uint32_t count, mat4x4* values) {
  if (!count)
    return;
  glUniformMatrix4fv(loc, count, GL_FALSE, (const GLfloat*)values);
}

void r_set_fxi(int loc, uint32_t count, float* values) {
  if (!count)
    return;
  glUniform1fv(loc, count, (const GLfloat*)values);
}

void r_set_ixi(int loc, uint32_t count, int* values) {
  if (!count)
    return;
  glUniform1iv(loc, count, (const GLint*)values);
}

void r_set_v2xi(int loc, uint32_t count, vec2* values) {
  if (!count)
    return;
  glUniform2fv(loc, count, (const GLfloat*)values);
}

void r_set_v3xi(int loc, uint32_t count, vec3* values) {
  if (!count)
    return;
  glUniform3fv(loc, count, (const GLfloat*)values);
}

void r_set_v4xi(int loc, uint32_t count, vec4* values) {
  if (!count)
    return;
  glUniform4fv(loc, count, (const GLfloat*)values);
}

void r_window_get_size(r_ctx* ctx, int* w, int* h) {
  *w = ctx->window.params.width;
  *h = ctx->window.params.height;
}

void r_window_get_vsize(r_ctx* ctx, vec2 vec) {
  vec[0] = (float)ctx->window.params.width;
  vec[1] = (float)ctx->window.params.height;
}

uint8_t r_window_set_size(r_ctx* ctx, uint32_t width, uint32_t height) {
  if (ctx->window.params.fullscreen) {
    return 0;
  }

  glfwSetWindowSize(ctx->window.glfw, width, height);

  return 1;
}

GLFWvidmode* r_get_vidmodes_by_usize(r_ctx* ctx, uint8_t* count) {
  if (!ctx)
    return 0;

  uint16_t     ucount = 0, ucapacity = 8;
  GLFWvidmode* umodes = (GLFWvidmode*)calloc(ucapacity, sizeof(GLFWvidmode));
  for (uint16_t i = 0; i < ctx->mode_count; ++i) {
    GLFWvidmode current_mode = ctx->modes[i];
    uint8_t     contained    = 0;

    for (uint16_t j = 0; j < ucount; ++j) {
      if (umodes[j].width == current_mode.width &&
          umodes[j].height == current_mode.height) {
        contained = 1;
        break;
      }
    }

    if (!contained) {
      if (ucount == ucapacity) {
        umodes = (GLFWvidmode*)realloc(umodes,
                                       sizeof(GLFWvidmode) * (ucapacity + 8));
        ucapacity += 8;
      }

      umodes[ucount] = current_mode;
      ucount++;
    }
  }

  if (ucount < ucapacity) {
    umodes = (GLFWvidmode*)realloc(umodes, sizeof(GLFWvidmode) * ucount);
  }

  if (count)
    *count = ucount;

  return umodes;
}

GLFWvidmode* r_get_vidmode_options(r_ctx* ctx, uint8_t* count, uint32_t width,
                                   uint32_t height) {
  if (!ctx)
    return 0;

  uint8_t unique = 0;
  for (uint8_t i = 0; i < ctx->mode_count; ++i) {
    GLFWvidmode current_mode = ctx->modes[i];
    if (current_mode.width == (int)width && current_mode.height == (int)height)
      ++unique;
  }

  if (!unique)
    return 0;

  GLFWvidmode* modes      = (GLFWvidmode*)malloc(sizeof(GLFWvidmode) * unique);
  uint8_t      mode_index = 0;

  for (uint8_t i = 0; i < ctx->mode_count; ++i) {
    GLFWvidmode current_mode = ctx->modes[i];
    if (current_mode.width == (int)width &&
        current_mode.height == (int)height) {
      modes[mode_index] = current_mode;
      ++mode_index;
    }
  }

  if (count)
    *count = unique;

  return modes;
}

int16_t r_get_vidmode_index(r_ctx* ctx, uint32_t width, uint32_t height,
                            uint8_t refresh_rate) {
  if (!ctx)
    return -1;

  for (uint8_t i = 0; i < ctx->mode_count; ++i) {
    GLFWvidmode current_mode = ctx->modes[i];
    if (current_mode.width == (int)width &&
        current_mode.height == (int)height) {
      if (refresh_rate) {
        if (current_mode.refreshRate == refresh_rate) {
          return i;
        }
      } else {
        return i;
      }
    }
  }

  return -1;
}

uint8_t r_get_vidmode_str_simplei(r_ctx* ctx, char* dst, uint32_t max_length,
                                  uint8_t index) {
  if (index >= ctx->mode_count) {
    index = 0;
  }

  index = (ctx->mode_count - 1) - index;
  return (uint8_t)r_get_vidmode_str_simple(dst, max_length, ctx->modes[index]);
}

uint8_t r_get_vidmode_str_simple(char* dst, uint32_t max_length,
                                 GLFWvidmode mode) {
  int str_len = snprintf(dst, max_length, "%i x %i", mode.width, mode.height);
  return (uint8_t)str_len;
}

uint8_t r_get_vidmode_stri(r_ctx* ctx, char* dst, uint32_t max_length,
                           uint8_t index) {
  if (index >= ctx->mode_count) {
    index = 0;
  }

  index = (ctx->mode_count - 1) - index;
  return r_get_vidmode_str(dst, max_length, ctx->modes[index]);
}

uint8_t r_get_vidmode_str(char* dst, uint32_t max_length, GLFWvidmode mode) {
  int str_len = snprintf(dst, max_length, "%ix%i@%i", mode.width, mode.height,
                         mode.refreshRate);
  return (uint8_t)str_len;
}

uint8_t r_select_vidmode(r_ctx* ctx, GLFWvidmode mode, int8_t fullscreen,
                         int8_t vsync, int8_t borderless) {
  if (!fullscreen && borderless != ctx->window.params.borderless)
    ctx->allowed = 0;

  if (fullscreen) {
    glfwWindowHint(GLFW_RED_BITS, mode.redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode.greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode.blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode.refreshRate);

    ctx->window.params.refresh_rate = mode.refreshRate;

    ctx->resolution[0]        = (float)mode.width;
    ctx->resolution[1]        = (float)mode.height;
    ctx->window.params.width  = mode.width;
    ctx->window.params.height = mode.height;

    glfwSetWindowMonitor(ctx->window.glfw, glfwGetPrimaryMonitor(), 0, 0,
                         mode.width, mode.height, mode.refreshRate);
  } else {
    if (ctx->window.params.borderless != borderless) {
      ctx->window.params.borderless = borderless;
      glfwSetWindowAttrib(ctx->window.glfw, GLFW_DECORATED,
                          (borderless == 0) ? GLFW_TRUE : GLFW_FALSE);
      ASTERA_FUNC_DBG("Setting borderless to: %i\n", borderless);
    }

    if ((uint32_t)mode.width != ctx->window.params.width ||
        (uint32_t)mode.height != ctx->window.params.height) {
      glfwSetWindowSize(ctx->window.glfw, mode.width, mode.height);
      ctx->resolution[0] = (float)mode.width;
      ctx->resolution[1] = (float)mode.height;

      ctx->window.params.width  = mode.width;
      ctx->window.params.height = mode.height;

      r_window_center(ctx);
    }

    if (fullscreen != ctx->window.params.fullscreen) {
      int x, y;
      glfwGetWindowPos(ctx->window.glfw, &x, &y);
      glfwSetWindowMonitor(ctx->window.glfw, 0, x, y, mode.width, mode.height,
                           mode.refreshRate);
    }
  }

  ctx->window.params.fullscreen = fullscreen;
  ctx->window.params.vsync      = vsync;
  if (vsync) {
    glfwSwapInterval(1);
  }

  if (!fullscreen) {
    const GLFWvidmode* monitor_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(ctx->window.glfw,
                     (monitor_mode->width - ctx->window.params.width) / 2,
                     (monitor_mode->height - ctx->window.params.height) / 2);
  }

  ctx->allowed = 1;
  return 1;
}

uint8_t r_select_mode(r_ctx* ctx, uint8_t index, int8_t fullscreen,
                      int8_t vsync, int8_t borderless) {
  if (index > ctx->mode_count) {
    ASTERA_FUNC_DBG("Invalid video mode index, not setting.\n");
    return 0;
  }

  index = (ctx->mode_count - 1) - index;

  const GLFWvidmode selected_mode = ctx->modes[index];
  return r_select_vidmode(ctx, selected_mode, fullscreen, vsync, borderless);
}

GLFWwindow* r_window_get_glfw(r_ctx* ctx) {
  if (!ctx) {
    return 0;
  }

  return ctx->window.glfw;
}

uint8_t r_get_vidmode_count(r_ctx* ctx) {
  return ctx->mode_count;
}

uint8_t r_can_render(r_ctx* ctx) {
  return ctx->allowed;
}

void r_set_can_render(r_ctx* ctx, uint8_t allowed) {
  ctx->allowed = allowed;
}

uint8_t r_is_vsync(r_ctx* ctx) {
  return ctx->window.params.vsync;
}

uint8_t r_is_fullscreen(r_ctx* ctx) {
  return ctx->window.params.fullscreen;
}

uint8_t r_is_borderless(r_ctx* ctx) {
  return ctx->window.params.borderless;
}

static void r_window_get_modes(r_ctx* ctx) {
  int count;
  ctx->modes      = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
  ctx->mode_count = count;
}

static const GLFWvidmode* r_find_closest_mode(r_ctx*          ctx,
                                              r_window_params params) {
  if (ctx->mode_count == 0) {
    r_window_get_modes(ctx);
  } else if (ctx->mode_count == 1) {
    return ctx->modes;
  }

  int32_t distance = 60000;

  const GLFWvidmode* closest = 0;
  for (int i = 0; i < ctx->mode_count; ++i) {
    const GLFWvidmode* cursor = &ctx->modes[i];

    int32_t cur_dist = (params.width - cursor->width) +
                       (params.height - cursor->height) - cursor->refreshRate;

    if (cur_dist < distance) {
      closest  = cursor;
      distance = cur_dist;
    }
  }

  return closest;
}

static const GLFWvidmode* r_find_best_mode(r_ctx* ctx) {
  if (ctx->mode_count == 0) {
    r_window_get_modes(ctx);
  } else if (ctx->mode_count == 1) {
    return ctx->modes;
  }

  int32_t            value    = -100;
  const GLFWvidmode* selected = 0;

  for (uint8_t i = 0; i < ctx->mode_count; ++i) {
    const GLFWvidmode* cursor = &ctx->modes[i];
    int cur_val = cursor->width + cursor->height * (cursor->refreshRate * 2);
    if (cur_val > value) {
      selected = cursor;
      value    = cur_val;
    }
  }

  return selected;
}

uint8_t r_window_create(r_ctx* ctx, r_window_params params) {
  glfwSetErrorCallback(glfw_err_cb);

  if (!glfwInit()) {
    ASTERA_FUNC_DBG("Unable to initialize GLFW\n");
    return 0;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

#if defined(ASTERA_DEBUG_GL)
  ASTERA_FUNC_DBG("Debug GL Enabled.\n");
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

#if defined(__APPLE__)
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
#endif

  GLFWwindow* window = NULL;
  ctx->window        = (r_window){0};

  // Make sure that gamma isn't 0'd
  if (params.gamma == 0.f) {
    params.gamma = 1.0f;
  }

  ctx->window.params = params;

  if (params.fullscreen) {
    const GLFWvidmode* selected_mode;

    if (params.width > 0 && params.height > 0 && params.refresh_rate > 0) {
      selected_mode = r_find_closest_mode(ctx, params);
    } else {
      selected_mode = r_find_best_mode(ctx);
    }

    glfwWindowHint(GLFW_RED_BITS, selected_mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, selected_mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, selected_mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, selected_mode->refreshRate);

    ctx->window.params.refresh_rate = selected_mode->refreshRate;
    ctx->window.params.width        = selected_mode->width;
    ctx->window.params.height       = selected_mode->height;

    vec2_dup(ctx->resolution,
             (vec2){selected_mode->width, selected_mode->height});

    window = glfwCreateWindow(selected_mode->width, selected_mode->height,
                              params.title, glfwGetPrimaryMonitor(), NULL);
  } else {
    if (params.resizable) {
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    } else {
      glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    }

    glfwWindowHint(GLFW_DECORATED,
                   (params.borderless == 0) ? GLFW_TRUE : GLFW_FALSE);

    vec2_dup(ctx->resolution,
             (vec2){(float)params.width, (float)params.height});

    window =
        glfwCreateWindow(params.width, params.height, params.title, NULL, NULL);
  }

  if (!window) {
    ASTERA_FUNC_DBG("Error: Unable to create GLFW window.\n");
    glfwTerminate();
    return 0;
  }

  if (params.resizable) {
    if (params.max_width > 1 && params.max_height > 1) {
      glfwSetWindowSizeLimits(window, params.min_width, params.min_height,
                              params.max_width, params.max_height);
    }
  }

  ctx->window.glfw = window;

  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);

  if (params.vsync) {
    glfwSwapInterval(1);
  } else {
    glfwSwapInterval(0);
  }

  ctx->allowed = 1;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glfwGetWindowPos(ctx->window.glfw, &ctx->window.params.x,
                   &ctx->window.params.y);

#if !defined(CUSTOM_GLFW_CALLBACKS)
  glfwSetWindowPosCallback(ctx->window.glfw, glfw_window_pos_cb);
  glfwSetWindowSizeCallback(ctx->window.glfw, glfw_window_size_cb);
  glfwSetWindowCloseCallback(ctx->window.glfw, glfw_window_close_cb);
  glfwSetKeyCallback(ctx->window.glfw, glfw_key_cb);
  glfwSetCharCallback(ctx->window.glfw, glfw_char_cb);
  glfwSetMouseButtonCallback(ctx->window.glfw, glfw_mouse_button_cb);
  glfwSetCursorPosCallback(ctx->window.glfw, glfw_mouse_pos_cb);
  glfwSetScrollCallback(ctx->window.glfw, glfw_scroll_cb);
  glfwSetJoystickCallback(glfw_joy_cb);
#endif

  r_window_get_modes(ctx);

  return 1;
}

void r_window_center(r_ctx* ctx) {
  int           monitor_count;
  GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
  GLFWmonitor*  mon      = 0;

  if (monitor_count == 0) {
    return;
  } else if (monitor_count == 1) {
    const GLFWvidmode* mode = glfwGetVideoMode(monitors[0]);
    r_window_set_pos(ctx, (mode->width - ctx->window.params.width) / 2,
                     (mode->height - ctx->window.params.height) / 2);
    return;
  }

  int mon_x = 0, mon_y = 0;
  int mon_w = 0, mon_h = 0;

  for (int i = 0; i < monitor_count; ++i) {
    glfwGetMonitorPos(monitors[i], &mon_x, &mon_y);
    const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
    if (ctx->window.params.x > mon_x &&
        ctx->window.params.x < mon_x + mode->width) {
      if (ctx->window.params.y > mon_y &&
          ctx->window.params.y < mon_y + mode->height) {
        mon_w = mode->width;
        mon_h = mode->height;
        mon   = monitors[i];
        break;
      }
    }
  }

  if (mon)
    r_window_set_pos(ctx, (mon_w - ctx->window.params.width) / 2,
                     (mon_h - ctx->window.params.height) / 2);
}

void r_window_set_pos(r_ctx* ctx, int32_t x, int32_t y) {
  glfwSetWindowPos(ctx->window.glfw, x, y);
}

uint8_t r_window_set_icon(r_ctx* ctx, unsigned char* data, uint32_t length) {
  if (!data || !length) {
    return 0;
  }

  if (data) {
    int            w, h, ch;
    unsigned char* img = stbi_load_from_memory(data, length, &w, &h, &ch, 0);

    GLFWimage glfw_img = (GLFWimage){w, h, img};
    glfwSetWindowIcon(ctx->window.glfw, 1, &glfw_img);

    free(img);
  } else {
    ASTERA_FUNC_DBG("No window icon passed to set.\n");
    return 0;
  }

  return 1;
}

void r_window_destroy(r_ctx* ctx) {
  ctx->allowed = 0;
  glfwDestroyWindow(ctx->window.glfw);

  ctx->window.glfw                = NULL;
  ctx->window.params.width        = 0;
  ctx->window.params.height       = 0;
  ctx->window.params.refresh_rate = 0;
  ctx->window.params.fullscreen   = 0;
  ctx->window.params.vsync        = 0;
}

void r_window_request_close(r_ctx* ctx) {
  ctx->window.close_requested = 1;
}

uint8_t r_window_should_close(r_ctx* ctx) {
  return ctx->window.close_requested;
}

void r_window_swap_buffers(r_ctx* ctx) {
  glfwSwapBuffers(ctx->window.glfw);
}

void r_window_clear(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void r_window_clear_color(const char* str) {
  vec4 color;
  r_get_color4f(color, str);
  glClearColor(color[0], color[1], color[2], color[3]);
}

void r_window_clear_color_empty(void) {
  glClearColor(0.f, 0.f, 0.f, 0.f);
}

float r_window_get_gamma(r_ctx* ctx) {
  return ctx->window.params.gamma;
}

void r_window_set_gamma(r_ctx* ctx, float gamma) {
  ctx->window.params.gamma = gamma;
}

uint8_t r_window_is_resizable(r_ctx* ctx) {
  return ctx->window.params.resizable;
}

void r_window_hide(r_ctx* ctx) {
  glfwSetWindowAttrib(ctx->window.glfw, GLFW_VISIBLE, GLFW_FALSE);
  ctx->allowed = 0;
}

void r_window_show(r_ctx* ctx) {
  glfwSetWindowAttrib(ctx->window.glfw, GLFW_VISIBLE, GLFW_TRUE);
  glfwShowWindow(ctx->window.glfw);
  ctx->allowed = 1;
}

uint32_t r_window_max_width(r_ctx* ctx) {
  return ctx->window.params.max_width;
}

uint32_t r_window_max_height(r_ctx* ctx) {
  return ctx->window.params.max_height;
}

uint32_t r_window_min_width(r_ctx* ctx) {
  return ctx->window.params.min_width;
}

uint32_t r_window_min_height(r_ctx* ctx) {
  return ctx->window.params.min_height;
}

void r_window_max_bounds(r_ctx* ctx, int* width, int* height) {
  if (width) {
    *width = ctx->window.params.max_width;
  }

  if (height) {
    *height = ctx->window.params.max_height;
  }
}

void r_window_min_bounds(r_ctx* ctx, int* width, int* height) {
  if (width) {
    *width = ctx->window.params.min_width;
  }

  if (height) {
    *height = ctx->window.params.min_height;
  }
}

void r_window_set_size_bounds(r_ctx* ctx, int min_width, int min_height,
                              int max_width, int max_height) {
  ctx->window.params.max_width  = max_width;
  ctx->window.params.max_height = max_height;
  ctx->window.params.min_width  = min_width;
  ctx->window.params.min_height = min_height;

  glfwSetWindowSizeLimits(ctx->window.glfw, ctx->window.params.min_width,
                          ctx->window.params.min_height,
                          ctx->window.params.max_width,
                          ctx->window.params.max_height);
}

void r_window_set_min_bounds(r_ctx* ctx, int width, int height) {
  ctx->window.params.min_width  = width;
  ctx->window.params.min_height = height;

  glfwSetWindowSizeLimits(ctx->window.glfw, ctx->window.params.min_width,
                          ctx->window.params.min_height,
                          ctx->window.params.max_width,
                          ctx->window.params.max_height);
}

void r_window_set_max_bounds(r_ctx* ctx, int width, int height) {
  ctx->window.params.max_width  = width;
  ctx->window.params.max_height = height;

  glfwSetWindowSizeLimits(ctx->window.glfw, ctx->window.params.min_width,
                          ctx->window.params.min_height,
                          ctx->window.params.max_width,
                          ctx->window.params.max_height);
}

void r_window_request_attention(r_ctx* ctx) {
  glfwRequestWindowAttention(ctx->window.glfw);
}

uint8_t r_window_is_focused(r_ctx* ctx) {
  return glfwGetWindowAttrib(ctx->window.glfw, GLFW_FOCUSED);
}

uint16_t r_get_refresh_rate(r_ctx* ctx) {
  return ctx->window.params.refresh_rate;
}
