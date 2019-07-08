//TODO
//
// 1. Optimize tree allocation into heap & contiguous array
// 2. Implement proper shader cache
// 3. Implement area based culling of renderables (game.c maybe)

#include "render.h"

#include <glad_gl.c>

#define RENDER_BATCH_SIZE 128
#define SHADER_STR_SIZE 64


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "input.h"
#include "sys.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>

typedef struct r_flags {
	int allowed : 1;
	int scaled  : 1;
	unsigned char video_mode_count : 6;
} r_flags;

static r_flags flags;

static r_window_info g_window_info;
static GLFWmonitor* r_default_monitor;
static const GLFWvidmode* r_vidmodes;
static int r_vidmode_count;
static v2 r_res;
static const char* r_window_title = "Yolo";

static unsigned int default_quad_vao;

static char* shader_value_buff[SHADER_STR_SIZE];

int r_init(){
	if(!r_create_window((r_window_info){1280, 720, 0, 1, 0, 60, r_window_title})){ 
		_e("Unable to create window.\n");
		return 0;
	}
	
	r_init_quad();
	return 1;
}

r_camera r_create_camera(v2 size, v2 position){
	m4 proj, view;
	m4_identity(&view);	
	m4_identity(&proj);

	m4_ortho(&proj, 0, size.x, size.y, 0, 0.01f, 10.f);
	m4_translate(&view, position.x, position.y, 0.f);

	return (r_camera){
		(v3){position.x, position.y, 0.f}, (v3){0.f}, view, proj, size, 60.f, 0.01f, 10.f};	
}

void r_update_camera(r_camera* camera){
	m4_identity(&camera->view);
	m4_identity(&camera->proj);

	m4_ortho(&camera->proj, 0, camera->size.x, camera->size.y, 0, camera->near, camera->far);
	m4_translate(&camera->view, camera->pos.x, camera->pos.y, 0.f);
}

void r_update(long delta){

}

r_tex r_get_tex(const char* fp){
	int w, h, ch;
	unsigned char* img = stbi_load(fp, &w, &h, &ch, 0);
	unsigned int id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	
	stbi_image_free(img);
	return (r_tex){id, (unsigned int)w,(unsigned int)h};	
}

r_sheet r_get_sheet(const char* fp, unsigned int subwidth, unsigned int subheight){
	r_tex tex = r_get_tex(fp);
	return (r_sheet){tex.id, tex.width, tex.height, subwidth, subheight};
}

static int r_init_quad() {
	int vao, vbo, vboi;
	
	float verts[16] = {
		//pos           //tex
		-0.5f,  0.5f,   0.f, 1.f
		-0.5f, -0.5f,   0.f, 0.f,
		 0.5f,  0.5f,   1.f, 0.f,
		 0.5f, -0.5f,   1.f, 1.f
	};

	int inds[6] = { 
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &vboi);
	
	glBindVertexArray(vao);	

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(int), &inds[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
			
	return 1;
}

void r_draw_call(r_shader shader, unsigned int count){
	if(!count) return;

	r_set_m4x(shader, count, "models",  g_r_cache.models);
  	r_set_ix(shader, count, "tex_ids", g_r_cache.tex_ids);

	//draw instanced
	glBindVertexArray(default_quad_vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, count);
}

void r_exit(){
	r_destroy_window(g_window);
	glfwTerminate();
}

v3 r_get_hex_color(const char* hex){
    int len = strlen(hex);
    int start = 0;

    if(len == 4){
        len = 3;
        start = 1;
    }else if(len == 7){
        len = 6;
        start = 1;
    }

    v3 val = {0.f};

    if(len == 6){
        for(int i=start;i<len;i+=2){
            int res = (int)strtol(&hex[i], NULL, 16);
            val.v[(i-start) / len] = res / 255.f;
        }
    }else if(len == 3){
        for(int i=start;i<len;++i){
            int res = (int)strtol(&hex[i], NULL, 8);
            val.v[(i-start) / len] = res / 255.f;
        }
    }else{
        printf("Incorrect length of hex string: %i\n", len);
    }
}

static GLuint r_get_sub_shader(const char* fp, int type){
	FILE* file;
	unsigned char* data = NULL;
	int count = 0;
	file = fopen(fp, "rt");
	if(file != NULL){
		fseek(file, 0, SEEK_END);
		count = ftell(file);
		rewind(file);

		if(count > 0){
			data = malloc(sizeof(unsigned char)*(count+1));
			count = fread(data, sizeof(unsigned char), count, file);
			data[count] = '\0';
		}

		fclose(file);
	}else{
		_l("Unable to open file: %s\n", fp);
		return 0;
	}

	GLint success = 0;
	GLuint id = glCreateShader(type);

	const char* ptr = data;

	glShaderSource(id, 1, &ptr, NULL);
	glCompileShader(id);

	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if(success != GL_TRUE){
		int maxlen = 0;
		int len;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxlen);

		char* log = malloc(maxlen);

		glGetShaderInfoLog(id, maxlen, &len, log);

		_l("%s\n", log);
		free(log);
	}

	free(data);

	return id;	
}

r_shader r_get_shader(const char* vert, const char* frag){
    GLuint v = r_get_sub_shader(vert, GL_VERTEX_SHADER);
    GLuint f = r_get_sub_shader(frag, GL_FRAGMENT_SHADER);

    GLuint id = glCreateProgram();

    glAttachShader(id, v);
    glAttachShader(id, f);

    glLinkProgram(id);

    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if(success != GL_TRUE){
        int maxlen = 0;
        int len;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxlen);
        char* log = malloc(maxlen);
        glGetProgramInfoLog(id, maxlen, &len, log);
        printf("%s\n", log);
        free(log);
    }

#ifdef DEBUG_OUTPUT
    printf("r_shader program loaded: %i\n", id);
#endif
    return (r_shader){id};
}

void r_destroy_shader(r_shader shader){
   	glDeleteProgram(shader);
}

void r_bind_tex(r_tex* tex){
	glBindTexture(GL_TEXTURE_2D, tex->id);
}

void r_bind_shader(r_shader shader){
    if(shader == NULL){
        glUseProgram(0);
    }else{
        glUseProgram(shader);
    }
}

int r_hex_number(char v){
  if(v >= '0' && v <= '9'){
    return v - 0x30;
  }else{
    switch(v){
        case 'A': case 'a': return 10;
        case 'B': case 'b': return 11;
        case 'C': case 'c': return 12;
        case 'D': case 'd': return 13;
        case 'E': case 'e': return 14;
        case 'F': case 'f': return 15;
        default: return 0;
    }
  }
}

int r_hex_multi(char* v, int len){
  if(len == 2){
      return r_hex_number(v[0])*16+r_hex_number(v[1]);
  }else if(len == 1){
      return r_hex_number(v[0])*16+r_hex_number(v[0]);
  }
}

v3 r_get_color(char* v){
  int len = strlen(v);
  int offset = 0;
  if(len == 4){
      offset = 1;
      len = 3;
  }else if(len == 7){
      offset = 1;
      len = 6;
  }

  v3 val = {0.f};

  if(len == 3){
      val.x = r_hex_multi(&v[offset], 1)   / 255.f;
      val.y = r_hex_multi(&v[offset+1], 1) / 255.f;
      val.z = r_hex_multi(&v[offset+2], 1) / 255.f;
  }else if(len == 6){
      val.x = r_hex_multi(&v[offset], 2)   / 255.f;
      val.y = r_hex_multi(&v[offset+2], 2) / 255.f;
      val.z = r_hex_multi(&v[offset+4], 2) / 255.f;
  }

  return val;
}

inline int r_get_uniform_loc(r_shader shader, const char* uniform){
	int loc = glGetUniformLocation(shader, uniform);
	//r_cache_check(shader, uniform, loc);
	return loc;
}

inline void r_set_uniformf(r_shader shader, const char* name, float value){
    glUniform1f(r_get_uniform_loc(shader, name), value);
}

inline void r_set_uniformfi(int loc, float value){
	glUniform1f(loc, value);
}

inline void r_set_uniformi(r_shader shader, const char* name, int value){
    glUniform1i(r_get_uniform_loc(shader, name), value);
}

inline void r_set_uniformii(int loc, int val){
	glUniform1i(loc, val);
}

inline void r_set_v4(r_shader shader, const char* name, v4 value){
    glUniform4f(r_get_uniform_loc(shader, name), value.x, value.y, value.z, value.w);
}

inline void r_set_v4i(int loc, v4 value){
	glUniform4f(loc, value.x, value.y, value.z, value.w);
}

inline void r_set_v3(r_shader shader, const char* name, v3 value){
    glUniform3f(r_get_uniform_loc(shader, name), value.x, value.y, value.z);
}

inline void r_set_v3i(int loc, v3 val){
	glUniform3f(loc, val.x, val.y, val.z);
}

inline void r_set_v2(r_shader shader, const char* name, v2 value){
    glUniform2f(r_get_uniform_loc(shader, name), value.x, value.y);
}

inline void r_set_v2i(int loc, v2 val){
	glUniform2f(loc, val.x, val.y);
}

inline void r_set_quat(r_shader shader, const char* name, quat value){
    glUniform4f(r_get_uniform_loc(shader, name), value.x, value.y, value.z, value.w);
}

inline void r_set_quati(int loc, quat val){
	glUniform4f(loc, val.x, val.y, val.z, val.w);
}

inline void r_set_m4(r_shader shader, const char* name, m4 value){
    glUniformMatrix4fv(r_get_uniform_loc(shader, name), 1, GL_FALSE, &value.v[0][0]);
}

inline void r_set_m4i(int loc, m4 val){
	glUniformMatrix4fv(loc, 1, GL_FALSE, &val.v[0][0]);
}

void r_set_m4x(r_shader shader, unsigned int count, const char* name, m4* values){
	if(!count) return;

	for(int i=0;i<count;++i){
		sprintf(shader_value_buff, "%s[%d]", name, i);
		glUniformMatrix4fv(r_get_uniform_loc(shader, shader_value_buff), 1, GL_FALSE, &values[i].v[0][0]);

		memset(shader_value_buff, 0, sizeof(char) * SHADER_STR_SIZE);
	}
}

void r_set_ix(r_shader shader, unsigned int count, const char* name, int* values){
	if(!count) return;

	for(int i=0;i<count;++i){
		sprintf(shader_value_buff, "%s[%d]", name, i);
		glUniform1i(r_get_uniform_loc(shader, shader_value_buff), values[i]);

		memset(shader_value_buff, 0, sizeof(char) * SHADER_STR_SIZE);
	}
}

void r_set_fx(r_shader shader, unsigned int count, const char* name, float* values){
	if(!count) return;

	for(int i=0;i<count;++i){
		sprintf(shader_value_buff, "%s[%d]", name, i);
		glUniform1f(r_get_uniform_loc(shader, shader_value_buff), values[i]);

		memset(shader_value_buff, 0, sizeof(char) * SHADER_STR_SIZE);
	}
}

void r_set_v2x(r_shader shader, unsigned int count, const char* name, v2* values){
	if(!count) return;

	for(int i=0;i<count;++i){
		sprintf(shader_value_buff, "%s[%d]", name, i);
		glUniform2f(r_get_uniform_loc(shader, shader_value_buff), values[i].x, values[i].y);

		memset(shader_value_buff, 0, sizeof(char) * SHADER_STR_SIZE);
	}
}

void r_set_v3x(r_shader shader, unsigned int count, const char* name, v3* values){
	if(!count) return;

	for(int i=0;i<count;++i){
		sprintf(shader_value_buff, "%s[%d]", name, i);
		glUniform3f(r_get_uniform_loc(shader, shader_value_buff), values[i].x, values[i].y, values[i].z);

		memset(shader_value_buff, 0, sizeof(char) * SHADER_STR_SIZE);
	}
}

void r_set_v4x(r_shader shader, unsigned int count, const char* name, v4* values){
	if(!count) return;

	for(int i=0;i<count;++i){
		sprintf(shader_value_buff, "%s[%d]", name, i);
		glUniform4f(r_get_uniform_loc(shader, shader_value_buff), values[i].x, values[i].y, values[i].z, values[i].w);

		memset(shader_value_buff, 0, sizeof(char) * SHADER_STR_SIZE);
	}
}

static void r_create_modes(){
    if(r_default_monitor == NULL){
        r_default_monitor = glfwGetPrimaryMonitor();
    }

    r_vidmodes = glfwGetVideoModes(r_default_monitor, &r_vidmode_count);
}

static int r_window_info_valid(r_window_info info){
    return info.width > 0 && info.height > 0;
}

static const GLFWvidmode* r_find_closest_mode(r_window_info info){
    if(r_vidmode_count == 0){
        r_create_modes();
    }else if(r_vidmode_count == 1){
        return r_vidmodes;
    }

    const GLFWvidmode* closest = &r_vidmodes[0];
    int distance = (abs(info.width - r_vidmodes[0].width) + abs(info.height - r_vidmodes[0].height) - r_vidmodes[0].refreshRate);

    for(int i=0;i<r_vidmode_count;++i){
        int d2 = (abs(info.width - r_vidmodes[i].width) + abs(info.height - r_vidmodes[i].height) - r_vidmodes[i].refreshRate);
        if(d2 < distance){
            closest = &r_vidmodes[i];
            distance = d2;
        }
    }

    return closest;
}

static const GLFWvidmode* r_find_best_mode(){
    if(r_vidmode_count == 0){
        r_create_modes();
    }else if(r_vidmode_count == 1){
        return r_vidmodes;
    }

    const GLFWvidmode* selected = &r_vidmodes[0];
    int value = selected->width + selected->height * (selected->refreshRate * 2);

    for(int i=0;i<r_vidmode_count;++i){
        int v2 = r_vidmodes[i].width + r_vidmodes[i].height * (r_vidmodes[i].refreshRate * 2);
        if(v2 > value){
            selected = &r_vidmodes[i];
            value = v2;
        }
    }

    return selected;
}

int r_create_window(r_window_info info){
    if(!info.fullscreen && !r_window_info_valid(info)){
        return 0;
    }

	_l("Loading GLFW.\n");

    if(!glfwInit()){
        printf("Unable to initialize GLFW\n");
        return 0;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = NULL;
	g_window = (r_window){0};
	
	_l("Loading Window Settings.\n");

    if(info.fullscreen){
        const GLFWvidmode* selected_mode;
        if(r_window_info_valid(info)){
            selected_mode = r_find_closest_mode(info);
        }else{
            selected_mode = r_find_best_mode();
        }

        glfwWindowHint(GLFW_RED_BITS, selected_mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, selected_mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, selected_mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, selected_mode->refreshRate);

        g_window.refreshRate = selected_mode->refreshRate;
        g_window.width = selected_mode->width;
        g_window.height = selected_mode->height;
        g_window.fullscreen = 1;
        window = glfwCreateWindow(selected_mode->width, selected_mode->height, info.title, r_default_monitor, NULL);
    }else{
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, info.borderless ? GLFW_FALSE : GLFW_TRUE);

        if(info.refreshRate > 0){
            glfwWindowHint(GLFW_REFRESH_RATE, info.refreshRate);
            g_window.refreshRate = info.refreshRate;
        }

        g_window.width = info.width;
        g_window.height = info.height;

        r_res = (v2){info.width, info.height};

        g_window.fullscreen = 0;
        g_window.vsync = 0;

        window = glfwCreateWindow(info.width, info.height, info.title, NULL, NULL);
    }

	_l("Loaded window settings.\n");

    if(!window){
        _e("Error: Unable to create GLFW window.\n");
        glfwTerminate();
        return 0;
    }

    g_window.glfw = window;
	
	_l("Attaching GL Context.\n");
    glfwMakeContextCurrent(window);

	_l("Loading GL\n");
	gladLoadGL(glfwGetProcAddress);

    flags.allowed = 1;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glEnable(GL_TEXTURE_2D);
    glViewport(0, 0, g_window.width, g_window.height);

    glfwGetWindowPos(g_window.glfw, &g_window.x, &g_window.y);

    glDisable(GL_CULL_FACE);

    v3 color = r_get_color("FFF");
    glClearColor(color.r, color.g, color.b, 1.f);
	
	_l("Setting Callbacks.\n");

    glfwSetWindowPosCallback(g_window.glfw, glfw_window_pos_cb);
    glfwSetWindowSizeCallback(g_window.glfw, glfw_window_size_cb);
    glfwSetWindowCloseCallback(g_window.glfw, glfw_window_close_cb);
    glfwSetKeyCallback(g_window.glfw, glfw_key_cb);
    glfwSetCharCallback(g_window.glfw, glfw_char_cb);
    glfwSetMouseButtonCallback(g_window.glfw, glfw_mouse_button_cb);
    glfwSetCursorPosCallback(g_window.glfw, glfw_mouse_pos_cb);
    glfwSetScrollCallback(g_window.glfw, glfw_scroll_cb);
    glfwSetJoystickCallback(glfw_joy_cb);
	
	_l("Setting default bindings.\n");
    i_default_bindings();

    return 1;
}

void r_center_window(){
    GLFWmonitor* mon = NULL;
    int monitor_count;
    GLFWmonitor** monitors =  glfwGetMonitors(&monitor_count);

    if(monitor_count == 0){
        return;
    }else if(monitor_count == 1){
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[0]);
        r_set_window_pos((mode->width - g_window.width) / 2, (mode->height - g_window.height) / 2);
        return;
    }

    int mon_x, mon_y;
    int mon_w, mon_h;

    for(int i=0;i<monitor_count;++i){
        glfwGetMonitorPos(monitors[i], &mon_x, &mon_y);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        if(g_window.x > mon_x && g_window.x < mon_x + mode->width){
            if(g_window.y > mon_y && g_window.y < mon_y + mode->height){
                mon_w = mode->width;
                mon_h = mode->height;
                mon = monitors[i];
                break;
            }
        }
    }

    if(mon != NULL){
        r_set_window_pos((mon_w - g_window.width) / 2, (mon_h - g_window.height) / 2);
    }
}

void r_set_window_pos(int x, int y){
    glfwSetWindowPos(g_window.glfw, x, y);
}

void r_destroy_window(){
    flags.allowed = 0;

    glfwDestroyWindow(g_window.glfw);

    g_window.glfw = NULL;
    g_window.width = -1;
    g_window.height = -1;
    g_window.refreshRate = -1;
    g_window.fullscreen = 0;
    g_window.vsync = 0;
}

void r_request_close(){
    g_window.close_requested = 1;
}

int r_should_close(){
    return g_window.close_requested;
}

void r_swap_buffers(){
    glfwSwapBuffers(g_window.glfw);
}

void r_clear_window(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void glfw_err_cb(int error, const char* msg){
    printf("ERROR: %i %s\n", error, msg);
}

static void glfw_window_pos_cb(GLFWwindow* window, int x, int y){
    g_window.x = x;
    g_window.y = y;
}

static void glfw_window_size_cb(GLFWwindow* window, int w, int h){
    g_window.width = w;
    g_window.height = h;
    glViewport(0, 0, w, h);
    flags.scaled = 1;
}

static void glfw_window_close_cb(GLFWwindow* window){
    g_window.close_requested = 1;
}

static void glfw_key_cb(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(action == GLFW_PRESS || action == GLFW_REPEAT){
        i_key_callback(key, scancode, 1);
        if(i_binding_track()){
            i_binding_track_callback(key, KEY_BINDING_KEY);
        }
    }else if(action == GLFW_RELEASE){
        i_key_callback(key, scancode, 0);
    }
}

static void glfw_char_cb(GLFWwindow* window, unsigned int c){
    i_char_callback(c);
}

static void glfw_mouse_pos_cb(GLFWwindow* window, double x, double y){
    i_mouse_pos_callback(x, y);
}

static void glfw_mouse_button_cb(GLFWwindow* window, int button, int action, int mods){
    if(action == GLFW_PRESS || action == GLFW_REPEAT){
        i_mouse_button_callback(button);
        if(i_binding_track()){
            i_binding_track_callback(button, KEY_BINDING_MOUSE_BUTTON);
        }
    }
}

static void glfw_scroll_cb(GLFWwindow* window, double dx, double dy){
    i_mouse_scroll_callback(dx, dy);
}

static void glfw_joy_cb(int joystick, int action){
    if(action == GLFW_CONNECTED){
        i_create_joy(joystick);
    }else if(action == GLFW_DISCONNECTED){
        i_destroy_joy(joystick);
    }
}
