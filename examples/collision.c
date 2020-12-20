#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <astera/asset.h>
#include <astera/render.h>
#include <astera/input.h>
#include <astera/col.h>
#include <astera/ui.h>

// render & input context
r_ctx* render_ctx;
i_ctx* input_ctx;

// ui stuff
ui_ctx* u_ctx;
ui_font font;

// actual collision code
c_aabb    box_a, box_b;
c_circle  circle;
c_ray     ray;
c_raycast raycast;

// formatted text buffer
char     fmt_text[128];
vec2     overlap_text_pos;
asset_t* font_data;

// Create the window to render with, ui context, and load in a font
void init_render(r_ctx* render_ctx) {
  r_window_params params = render_ctx->window.params;

  vec2 screen_size = {(float)params.width, (float)params.height};
  u_ctx            = ui_ctx_create(screen_size, 1.f, 0, 1, 0);

  font_data = asset_get("resources/fonts/monogram.ttf");

  font = ui_font_create(u_ctx, font_data->data, font_data->data_length,
                        "monogram");

  vec2_clear(overlap_text_pos);

  // offset 5 px from corner
  vec2 offset = {5.f, 5.f};
  ui_scale_offset_px(u_ctx, overlap_text_pos, overlap_text_pos, offset);
}

void setup_collision() {
  vec2 position = {0.f, 0.f};
  vec2 halfsize = {8.f, 8.f};

  // format size to halfsize
  box_a = c_aabb_create(position, halfsize);

  position[0] = 64.f;
  position[1] = 64.f;
  box_b       = c_aabb_create(position, halfsize);

  position[0] = 72.f;
  position[1] = 72.f;
  circle      = c_circle_create(position, 16.f);
}

void debug_text(vec2 start, char* text) {
  ui_im_text_draw_aligned(u_ctx, start, 16.f, font,
                          UI_ALIGN_LEFT | UI_ALIGN_TOP, text);
}

void debug_fmt_text(vec2 start, char* format, ...) {
  va_list args;
  va_start(args, format);

  vsprintf(fmt_text, format, args);

  debug_text(start, fmt_text);

  // clear out formatted text
  for (int i = 0; i < 128; ++i)
    fmt_text[i] = 0;

  va_end(args);
}

void debug_circle(vec2 center, float radius, float thickness, ui_color color) {
  vec2 center_screen;

  r_camera* camera = r_ctx_get_camera(render_ctx);
  r_camera_world_to_screen(center_screen, camera, center);

  ui_im_circle_draw(u_ctx, center_screen, radius * 3.14159, thickness, color);
}

// worldspace points
void debug_line(vec2 start, vec2 end, float thickness, ui_color color) {
  vec2 start_screen, end_screen;

  r_camera* camera = r_ctx_get_camera(render_ctx);
  r_camera_world_to_screen(start_screen, camera, start);
  r_camera_world_to_screen(end_screen, camera, end);

  ui_im_line_draw(u_ctx, start_screen, end_screen, thickness, color);
}

void debug_box(vec2 start, vec2 end, ui_color color) {
  vec2 start_screen, end_screen;

  r_camera* camera = r_ctx_get_camera(render_ctx);
  r_camera_world_to_screen(start_screen, camera, start);
  r_camera_world_to_screen(end_screen, camera, end);

  vec2 start_pos, size;
  vec2_min(start_pos, start, end);
  vec2_dup(size, end);
  vec2_sub(size, size, start);

  r_camera_world_to_screen(start_pos, camera, start_pos);
  r_camera_size_to_screen(size, camera, size);

  ui_im_box_draw(u_ctx, start_pos, size, color);
}

void input(time_s delta) {
  if (i_key_clicked(input_ctx, KEY_ESCAPE)) {
    r_window_request_close(render_ctx);
  }

  vec3 camera_move = {0.f, 0.f, 0.f};

  if (i_key_down(input_ctx, 'A')) {
    camera_move[0] -= 1.f;
  }
  if (i_key_down(input_ctx, 'D')) {
    camera_move[0] += 1.f;
  }

  if (i_key_down(input_ctx, 'W')) {
    camera_move[1] -= 1.f;
  }
  if (i_key_down(input_ctx, 'S')) {
    camera_move[1] += 1.f;
  }

  vec2_scale(camera_move, camera_move, delta * 0.1f);

  // r_camera_move(r_ctx_get_camera(render_ctx), camera_move);

  // Move AABB
  c_aabb_move(&box_a, camera_move);

  // Move Circle
  // c_circle_move(&circle, camera_move);
  // vec2_add(circle.center, circle.center, camera_move);
}

void update(time_s delta) {
  // Check for collision between boxes
  /* AABB vs AABB
  if (c_aabb_vs_aabb(box_a, box_b)) {
    c_manifold solution = c_aabb_vs_aabb_man(box_a, box_b);

    vec2 distance, old_pos;
    vec2_scale(distance, solution.direction, solution.distance);

    // NOTE this was modified to remove resolution
    c_aabb_move(&box_a, distance);

    static ui_color line_color;
    ui_get_color(line_color, "#0f0");

    vec2_clear(solution.direction);
    solution.distance = 0.f;

  }
  */

  // AABB vs Circle
  if (c_aabb_vs_circle(box_a, circle)) {
    // c_manifold man = c_aabb_vs_circle_man(box_b, circle);
    c_manifold man = c_circle_vs_aabb_man(circle, box_a);
    vec2       old_pos, distance;

    vec2_scale(distance, man.direction, man.distance);
    // vec2_sub(old_pos, box_b.max, box_b.min);
    // vec2_dup(old_pos, circle.center);

    debug_fmt_text(overlap_text_pos, "%.2f %.2f", distance[0], distance[1]);
    vec2_scale(distance, distance, -1.f);

    // vec2_add(.center, circle.center, distance);
    // printf("move %.2f %.2f\n", distance[0], distance[1]);
    // c_aabb_move(&box_a, distance);
    c_circle_move(&circle, distance);

    ui_color color;
    ui_get_color(color, "#f00");
    debug_circle(man.point, 1.f, 3.f, color);
  }
}

void render(time_s delta) {
  r_window_clear();

  r_ctx_update(render_ctx);

  ui_frame_start(u_ctx);

  update(delta);

  ui_color circle_color;
  ui_get_color(circle_color, "fff");
  vec2 circle_size, draw_size;

  r_camera* camera = r_ctx_get_camera(render_ctx);
  r_camera_size_to_screen(draw_size, camera, circle_size);
  debug_circle(circle.center, circle.radius, 2.f, circle_color);

  // printf("%.2f %.2f\n", circle.center[0], circle.center[1]);

  vec2 box_start, box_end;

  ui_color box_color;
  ui_get_color(box_color, "#00f");
  // debug_box(box_b.min, box_b.max, box_color);

  debug_box(box_a.min, box_a.max, box_color);

  ui_frame_end(u_ctx);

  r_ctx_draw(render_ctx);
}

int main(void) {
  r_window_params params =
      r_window_params_create(WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 0, 60,
                             "Collision Example - Astera 1.0");

  // Create an empty render ctx (just window) so we can draw with the UI
  // system
  render_ctx = r_ctx_create(params, 0, 0, 0, 0);

  // 16x9 * 20
  vec2 camera_size = {320, 180};
  r_camera_set_size(r_ctx_get_camera(render_ctx), camera_size);

  r_window_clear_color("#0A0A0A");

  if (!render_ctx) {
    printf("Render context failed.\n");
    return 1;
  }

  input_ctx = i_ctx_create(4, 8, 0, 1, 0);

  if (!input_ctx) {
    printf("Input context failed.\n");
    return 1;
  }

  // Set the window icon
  asset_t* icon = asset_get("resources/textures/icon.png");
  r_window_set_icon(render_ctx, icon->data, icon->data_length);
  asset_free(icon);

  // Set the render context as the global default
  r_ctx_make_current(render_ctx);
  r_ctx_set_i_ctx(render_ctx, input_ctx);

  init_render(render_ctx);
  setup_collision();

  s_timer timer = s_timer_create();

  while (!r_window_should_close(render_ctx)) {
    time_s delta = s_timer_update(&timer);

    i_ctx_update(input_ctx);

    input(delta);

    if (r_can_render(render_ctx)) {
      r_window_clear();
      render(delta);
      r_window_swap_buffers(render_ctx);
    }
  }

  asset_free(font_data);

  ui_ctx_destroy(u_ctx);
  r_ctx_destroy(render_ctx);
  i_ctx_destroy(input_ctx);

  return 0;
}
