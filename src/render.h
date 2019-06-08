#ifndef r_H
#define r_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

#include "geom.h"

typedef struct {
    int width, height;
    int fullscreen, vsync, borderless;
    int refreshRate;
    char* title;
} r_window_info;

typedef struct {
    int width, height;
    int x, y;
    int fullscreen, vsync;
    int close_requested;
    int refreshRate;
    GLFWwindow* glfw;
} r_window;

typedef struct {
    unsigned int vao;
    unsigned int vbo, vto, vboi;
    float width, height;
} r_quad;

typedef struct {
    v3 pos, rot;
    m4 view, proj;
    v2 size;
    float fov;
    float near;
    float far;
} r_camera;

typedef struct {
    unsigned int id;
} r_shader;

typedef struct {
    unsigned int id;
    unsigned int width, height;
} r_tex;

typedef struct {
    r_tex* tex;
    unsigned int subwidth, subheight;
} r_sheet;

typedef struct {
	r_sheet* sheet;
	int frame_count;
	int* frames;
	int frame_rate;	
} r_anim;

typedef struct {
	r_anim* anim;
	long time; 
	int state, pstate;
} r_animv;

typedef struct {
	r_animv* anim;
	v2 size;
	v3 position;
} r_drawable;

typedef struct {
	r_drawable* val;
	r_drawable* next;
} r_leaf;

typedef struct {
	r_leaf* root;
	int count;
} r_list;

static r_window g_window;
static r_quad quad;

int  r_init();
void r_exit();
void r_update(long delta, r_list* list, int r_count);
void r_draw(r_list* list, int r_count);

r_tex*      r_get_tex(char* name);
void        r_bind_tex(r_tex tex);

r_sheet r_create_tex_sheet(r_tex* tex, unsigned int subwidth, unsigned int subheight);
void r_get_sub_texcoords(r_sheet* sheet, unsigned int id, float* coords);

void r_draw_quad(r_quad* quad);

static GLuint r_get_sub_shader(const char* filePath, int type);
r_shader      r_create_shader(const char* vert, const char* frag);
void          r_assign_shader(r_shader* shader, char* name);
void          r_bind_shader(r_shader* shader);
void          r_destroy_shader(r_shader* shader);

int           r_get_uniform_loc(r_shader shader, const char* name);

int  r_hex_number(char v);
int  r_hex_multi(char* v, int len);
v3 r_get_color(char* v);

void r_set_uniformf(r_shader shader, const char* name, float value);
void r_set_uniformi(r_shader shader, const char* name, int value);
void r_set_v4(r_shader shader, const char* name, v4 value);
void r_set_v3(r_shader shader, const char* name, v3 value);
void r_set_v2(r_shader shader, const char* name, v2 value);
void r_set_quat(r_shader shader, const char* name, quat value);
void r_set_m4(r_shader shader, const char* name, m4 value);

void r_set_uniformfi(int loc, float val);
void r_set_uniformii(int loc, int val);
void r_set_v4i(int loc, v4 val);
void r_set_v3i(int loc, v3 val);
void r_set_v2i(int loc, v2 val);
void r_set_quati(int loc, quat val);
void r_set_m4i(int loc, m4 val);

static void r_create_modes();
static int  r_window_info_valid(r_window_info info);
static const GLFWvidmode* r_find_closest_mode(r_window_info info);
static const GLFWvidmode* r_find_best_mode();

static void r_window_resized();

static void glfw_err_cb(int error, const char* msg);
static void glfw_window_pos_cb(GLFWwindow* window, int x, int y);
static void glfw_window_size_cb(GLFWwindow* window, int w, int h);
static void glfw_window_close_cb(GLFWwindow* window);
static void glfw_key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);
static void glfw_mouse_pos_cb(GLFWwindow* window, double x, double y);
static void glfw_mouse_button_cb(GLFWwindow* window, int button, int action, int mods);
static void glfw_scroll_cb(GLFWwindow* window, double dx, double dy);
static void glfw_joy_cb(int joystick, int action);
static void glfw_char_cb(GLFWwindow* window, unsigned int c);

int  r_create_window(r_window_info info);
void r_destroy_window();
void r_request_close();

void r_center_window();
void r_set_window_pos(int x, int y);

int r_should_close();

void r_swap_buffers();
void r_clear_window();
#endif
