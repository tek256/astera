#include "render.h"

#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <glad/glad_gl.c>

#define STB_IMAGE_IMPLEMENTATION
#include <misc/stb_image.h>


#define STB_TRUETYPE_IMPLEMENTATION 
#include <misc/stb_truetype.h>

#include "input.h"
#include "sys.h"
#include "game.h"
#include "debug.h"

static r_flags flags;

static GLFWmonitor* r_default_monitor;
static const GLFWvidmode* r_vidmodes;
static vec2 r_res;
static u32 default_quad_vao, default_quad_vbo, default_quad_vboi;

#ifndef EXCLUDE_CREATE
static r_resource_map r_res_map;
r_resource_map* r_get_map(){
	return &r_res_map;
}
#endif

#ifndef CUSTOM_GLFW_CALLBACKS
static void glfw_err_cb(int error, const char* msg){
	_e("ERROR: %i %s\n", error, msg);
}

static void glfw_window_pos_cb(GLFWwindow* window, int x, int y){
	if(g_window.glfw == window){
		g_window.x = x;
		g_window.y = y;
	}
}

static void glfw_window_size_cb(GLFWwindow* window, int w, int h){
	if(g_window.glfw == window){
		g_window.width = w;
		g_window.height = h;
		glViewport(0, 0, w, h);
		flags.scaled = 1;
		i_set_screensize(w, h);
	}
}

static void glfw_window_close_cb(GLFWwindow* window){
	if(g_window.glfw == window)
		g_window.close_requested = 1;
}

static void glfw_key_cb(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(g_window.glfw != window) return;
	
	if(action == GLFW_PRESS || action == GLFW_REPEAT){
		i_key_callback(key, scancode, 1);
		if(i_key_binding_track()){
			i_binding_track_callback(key, BINDING_KEY);
		}
	}else if(action == GLFW_RELEASE){
		i_key_callback(key, scancode, 0);
	}
}

static void glfw_char_cb(GLFWwindow* window, u32 c){
	if(window == g_window.glfw)
		i_char_callback(c);
}

static void glfw_mouse_pos_cb(GLFWwindow* window, double x, double y){
	if(window == g_window.glfw)
		i_mouse_pos_callback(x, y);
}

static void glfw_mouse_button_cb(GLFWwindow* window, int button, int action, int mods){
	//UI callback
	if(action == GLFW_PRESS || action == GLFW_REPEAT){
		i_mouse_button_callback(button);
		if(i_key_binding_track()){
			i_binding_track_callback(button, BINDING_MB);
		}
	}
}

static void glfw_scroll_cb(GLFWwindow* window, double dx, double dy){
	if(g_window.glfw == window)
		i_mouse_scroll_callback(dx, dy);
}

static void glfw_joy_cb(int joystick, int action){
	if(action == GLFW_CONNECTED){
		i_create_joy(joystick);
	}else if(action == GLFW_DISCONNECTED){
		i_destroy_joy(joystick);
	}
}
#endif 

int r_init(r_window_info info){
	if(!r_window_create(info)){ 
		_e("Unable to create window.\n");
		return 0;
	}

	r_window_center();

	r_cam_create(&g_camera, (vec2){r_res[0], r_res[1]}, (vec2){0.f, 0.f});

	g_anim_cache.capacity = RENDER_ANIM_CACHE;
	g_shader_map.capacity = RENDER_SHADER_CACHE;
	g_drawable_cache.capacity = RENDER_BATCH_SIZE;

	f32 verts[16] = {
		//pos       //tex
		-0.5f, -0.5f,   0.f, 0.f,
		-0.5f,  0.5f,   0.f, 1.f,
	 	 0.5f,  0.5f,   1.f, 1.f,
	 	 0.5f, -0.5f,   1.f, 0.f
	};

	u16 inds[6] = { 
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &default_quad_vao);
	glGenBuffers(1, &default_quad_vbo);
	glGenBuffers(1, &default_quad_vboi);

	glBindVertexArray(default_quad_vao);	

	glBindBuffer(GL_ARRAY_BUFFER, default_quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(f32), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, default_quad_vboi);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(u16), &inds[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	return 1;
}

void r_cam_create(r_camera* cam, vec2 size, vec2 position){
	vec3_dup(cam->pos, (vec3){position[0], position[1], 0.f});
	vec2_dup(cam->size, size);

	cam->near = -10.f;
	cam->far = 10.f;

	f32 x, y;
	x = floorf(cam->pos[0]);
	y = floorf(cam->pos[1]);

	mat4x4_ortho(cam->proj, 0, cam->size[0], cam->size[1], 0, cam->near, cam->far);

	mat4x4_translate(cam->view, x, y, 0.f);
	mat4x4_rotate_x(cam->view, cam->view, 0.0);
	mat4x4_rotate_y(cam->view, cam->view, 0.0);
	mat4x4_rotate_z(cam->view, cam->view,  0.0);
}

void r_cam_move(f32 x, f32 y){
	g_camera.pos[0] -= x;
	g_camera.pos[1] += y;
}

void r_cam_update(void){
	f32 x, y;
	x = floorf(g_camera.pos[0]);
	y = floorf(g_camera.pos[1]);
	mat4x4_translate(g_camera.view, x, y, 0.f);
}

void r_update(long delta){
	for(unsigned int i=0;i<g_drawable_cache.count;++i){
		r_drawable_update(&g_drawable_cache.drawables[i], delta);
	}

	r_cam_update();	
}

r_tex r_get_tex(asset_t* asset){
	int w, h, ch;
	unsigned char* img = stbi_load_from_memory(asset->data, asset->data_length, &w, &h, &ch, 0);
	u32 id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);

#if defined(CREATE_MODE)
	int count = r_res_map.tex_count;
	r_res_map.tex_ids[count] = id;
	r_res_map.tex_paths[count] = strdup(asset->name);
	++r_res_map.tex_count;
#endif

	stbi_image_free(img);
	asset->req_free = 1;
	return (r_tex){id, (u32)w,(u32)h};	
}

r_sheet r_get_sheet(asset_t* asset, u32 subwidth, u32 subheight){
	r_tex tex = r_get_tex(asset);
#if defined(CREATE_MODE)
	int count = r_res_map.sheet_count;

	r_res_map.sheet_ids[count] = tex.id;
	r_res_map.sheet_subwidths[count] = subwidth;
	r_res_map.sheet_subheights[count] = subheight;
	r_res_map.sheet_paths[count] = strdup(asset.name);

	++ r_res_map.sheet_count;	
#endif
	return (r_sheet){tex.id, tex.width, tex.height, subwidth, subheight};
}
/*
 *typedef struct {
	r_particle* list;
	unsigned int capacity, high;
	unsigned int max_emission;
	
	float particle_life, system_life;

	float time;

	vec2 position, size, velocity;

	r_particle_spawn spawn_type;

	r_keyframes fade_frames;
	r_keyframes size_frames;

	union {
		r_subtex tex;
		r_animv anim;
		vec4   color;
	};

	int animated : 1;
	int texture  : 1;
	int color    : 1;
} r_particle_system;
 */

float r_keyframe_get_value(r_keyframes frames, float point){
	int start_index = 0;
	for(int i=0;i<frames.count;++i){
		if(frames.list[i].point > point){
			start_index = -1;
			break;
		}
	}

	if(start_index < 0){
		return frames.list[0].value;
	}

	if(frames.count-1 == start_index){
		return frames.list[start_index].value;
	}

	switch(frames.list[start_index].curve){
		case LINEAR: {
			float point_dist = frames.list[start_index + 1].point - frames.list[start_index].point;
			float value_dist = frames.list[start_index + 1].value - frames.list[start_index].value;
			float avg = (point - frames.list[start_index].point) / point_dist;
			return frames.list[start_index].value + (value_dist * avg);
		 }
		case EASE_IN:
			return frames.list[start_index].value;
		case EASE_EASE:
			return frames.list[start_index].value;
		case EASE_OUT:
			return frames.list[start_index].value;
		default:
			return frames.list[start_index].value;
	}	
}

void r_particle_system_init(r_particle_system* system, unsigned int particle_capacity){
	if(system->list){
		free(system->list);
	}

	if(system->max_emission > 1 && system->max_emission < particle_capacity){
		particle_capacity = system->max_emission;
		system->capacity = particle_capacity;
	}

	r_particle* particles = (r_particle*)malloc(sizeof(r_particle) * particle_capacity);
	if(!particles){
		_e("Unable to allocate sizeo of %i for a particle list.\n", (sizeof(r_particle) * particle_capacity));
		system->capacity = 0;
		system->list = 0;
		return;
	}

	system->list = particles;
	system->capacity = particle_capacity;
	system->time = 0;
}

void r_particle_system_update(r_particle_system* system, double delta){
	int last = 0;

	vec2 adj_vel;

	for(int i=0;i<system->capacity;++i){
		if(system->list[i].alive){
			r_particle* particle = &system->list[i];
			particle->life += delta;

			if(particle->life > system->particle_life){
				particle->alive = 0;
			}else{
				last = i;
				adj_vel[0] = system->velocity[0] * delta;	
				adj_vel[1] = system->velocity[1] * delta;	
				vec2_add(particle->position, particle->position, adj_vel);
			}
		}		
	}

	system->high = last;

	int to_spawn = system->spawn_time / (system->spawn_rate / MS_PER_SEC);
	if(to_spawn > (system->max_emission - system->emission_count) && system->max_emission > 0){
		to_spawn = system->max_emission - system->emission_count; 
	}

	for(int i=0;i<system->high;++i){
		if(!system->list[i].alive){
			r_particle* particle;
			switch(system->spawn_type){
				case POINT:
					vec2_dup(particle->position, system->position);
					break;
				case CIRCLE:
					particle->position[0] = fmodf(rand(), system->size[0]);
					particle->position[1] = fmodf(rand(), system->size[0]);
					break;
				case BOX:
					particle->position[0] = fmodf(rand(), system->size[0]);
					particle->position[1] = fmodf(rand(), system->size[1]);
					break;
			}

			vec2_dup(particle->size, system->particle_size);
			
			if(system->animated){
				particle->anim = system->anim;	
				particle->anim.time = 0;
				particle->anim.frame = 0;
			}else if(system->texture){
				particle->tex = system->tex;
			}else if(system->color){
				vec4_dup(particle->color, system->color);
			}

			particle->life = 0;

			particle->animated = system->animated;
			particle->tex = system->tex;
			particle->colored = system->colored;

			--to_spawn;
			++system->emission_count;
		}
	}

	if(to_spawn > system->capacity - system->high){
		to_spawn = system->capacity - system->high;
	}
	
	for(int i=0; i<to_spawn;++i) {
		r_particle* particle = &system->list[system->high + i];
		switch(system->spawn_type){
			case POINT:
				vec2_dup(particle->position, system->position);
				break;
			case CIRCLE:
				particle->position[0] = fmodf(rand(), system->size[0]);
				particle->position[1] = fmodf(rand(), system->size[0]);
				break;
			case BOX:
				particle->position[0] = fmodf(rand(), system->size[0]);
				particle->position[1] = fmodf(rand(), system->size[1]);
				break;
		}

		vec2_dup(particle->size, system->particle_size);

		if(system->animated){
			particle->anim = system->anim;	
			particle->anim.time = 0;
			particle->anim.frame = 0;
		}else if(system->texture){
			particle->tex = system->tex;
		}else if(system->color){
			vec4_dup(particle->color, system->color);
		}

		particle->life = 0;

		particle->animated = system->animated;
		particle->tex = system->tex;
		particle->colored = system->colored;

		--to_spawn;
		++system->emission_count;
	}
}

void r_update_batch(r_shader shader, r_sheet* sheet){
	int cache_index = -1;
	for(unsigned int i=0;i<g_shader_map.count;++i){
		if(g_shader_map.batches[i].shader == shader){
			cache_index = i;
			break;
		}
	}

	if(cache_index == -1){
		_e("Unable to find cache to update batch: %d\n", shader);
		return;
	}

	r_shader_batch* cache = &g_shader_map.batches[cache_index];
	for(unsigned int i=0;i<g_drawable_cache.count;++i){
		if(cache->count >= cache->capacity){
			_e("Reached cache limit for shader: %d\n", shader);
			break;
		}
		if(g_drawable_cache.drawables[i].shader == shader){
			if(g_drawable_cache.drawables[i].anim.sheet.id == sheet->id){
				r_shader_add_to_array(shader, "models", &g_drawable_cache.drawables[i].model,  1);

				unsigned int id = 0;
				if(g_drawable_cache.drawables[i].animated){
					id = (u32)g_drawable_cache.drawables[i].anim.frames[g_drawable_cache.drawables[i].anim.frame];
				}else{
					id = (u32)g_drawable_cache.drawables[i].tex.sub_id;
				}
				
				r_shader_add_to_array(shader, "tex_ids", &id, 1);
				r_shader_add_to_array(shader, "flip_x", &g_drawable_cache.drawables[i].flip_x, 1); 
				r_shader_add_to_array(shader, "flip_y", &g_drawable_cache.drawables[i].flip_x, 1);
				
				cache->count ++;
			}
		}
	}
}

void r_draw_call(r_shader shader){
	if(!shader || !g_drawable_cache.count) return;

	r_shader_batch* batch = r_shader_get_batch(shader);
	r_sheet* sheet = batch->sheet;

	r_shader_bind(shader);
	r_shader_set_arrays(shader);

	vec2 sub_size, tex_size;
	vec2_dup(sub_size, (vec2){ sheet->subwidth, sheet->subheight });
	vec2_dup(tex_size, (vec2){ sheet->width, sheet->height });

	r_set_v2(shader, "sub_size", sub_size);
	r_set_v2(shader, "tex_size", tex_size);	

	r_set_m4(shader, "proj", g_camera.proj);
	r_set_m4(shader, "view", g_camera.view);	

	r_tex_bind(sheet->id);

	glBindVertexArray(default_quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, default_quad_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, default_quad_vboi);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(0);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, batch->count);

	r_shader_clear_arrays(shader);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void r_exit(void){
	for(int i=0;i<g_anim_cache.count;++i){
		free(g_anim_cache.anims[i].frames);
	}

	glDeleteVertexArrays(1, &default_quad_vao);
	
	r_window_destroy();
	glfwTerminate();
}

static GLuint r_shader_create_sub(asset_t* asset, int type){
	GLint success = 0;
	GLuint id = glCreateShader(type);

	const char* ptr = asset->data;

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

	return id;	
}

r_shader r_get_shadern(const char* name){
	for(unsigned int i=0;i<g_shader_map.count;++i){
		if(strcmp(name, g_shader_map.names[i]) == 0){
			return g_shader_map.batches[i].shader;
		}		
	}	
	return 0; 
}

r_shader r_shader_create(asset_t* vert, asset_t* frag){
	GLuint v = r_shader_create_sub(vert, GL_VERTEX_SHADER);
	GLuint f = r_shader_create_sub(frag, GL_FRAGMENT_SHADER);

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
		_l("%s\n", log);
		free(log);
	}
#if defined(CREATE_MODE)
	int shader_count = r_res_map.shader_count;
	r_res_map.vertex_shaders[shader_count] = strdup(vert->name);
	r_res_map.fragment_shaders[shader_count] = strdup(frag->name);
	r_res_map.shader_ids[shader_count] = id;
	r_res_map.shader_count ++;
#endif
#ifdef DEBUG_OUTPUT
	_l("r_shader program loaded: %i\n", id);
#endif

	return (r_shader){id};
}

void r_shader_setup(r_shader shader, const char* name){
	if(g_shader_map.capacity == 0){
		_e("No shader cache available.\n");
		return;
	}

	if(g_shader_map.capacity == g_shader_map.count){
		//TODO implement cache overflow
		_e("No shader cache open to use.\n");
		return;
	}

	for(unsigned int i=0;i<g_shader_map.count;++i){
		if(g_shader_map.batches[i].shader == shader){
			_l("Shader: %d already contained with alias of: %s\n", shader, g_shader_map.names[i]);
			return;
		}
	}

	u32 count = g_shader_map.count;

	g_shader_map.names[count] = name;
	g_shader_map.batches[count].shader = shader;
	g_shader_map.count ++;
}

void r_shader_clear_arrays(r_shader shader){
	r_shader_batch* batch = r_shader_get_batch(shader);
	if(!batch) return;

	for(unsigned int i=0;i<batch->uniform_array_count;++i){
		r_uniform_array* array = &batch->uniform_arrays[i];	
		memset(array->data, 0, array->stride * array->capacity);
		array->count = 0;
	}

	batch->count = 0;
}

r_shader_batch* r_shader_get_batch(r_shader shader){
	for(unsigned int i=0;i<g_shader_map.count;++i){
		if(g_shader_map.batches[i].shader == shader){
			return &g_shader_map.batches[i];
		}
	}
	return 0;
}

void r_shader_destroy_batch(r_shader shader){
	r_shader_batch* batch = r_shader_get_batch(shader);

	for(unsigned int i=0;i<batch->uniform_array_count;++i){
		free(batch->uniform_arrays[i].data);
	}

	free(batch->uniform_arrays);
	batch->uniform_array_count = 0;	
	batch->shader = 0;
	batch->count = 0;
	batch->capacity = 0;
}

void r_shader_destroy(r_shader shader){
	r_shader_destroy_batch(shader);
	glDeleteProgram(shader);
}

void r_tex_bind(u32 tex){
	glBindTexture(GL_TEXTURE_2D, tex);
}

void r_shader_bind(r_shader shader){
	//if shader id is negative
	if(!shader){
		glUseProgram(0);
	}else{
		glUseProgram(shader);
	}
}

static int r_hex_number(char v){
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

static int r_hex_multi(char* v, int len){
	if(len == 2){
		return r_hex_number(v[0])*16+r_hex_number(v[1]);
	}else if(len == 1){
		return r_hex_number(v[0])*16+r_hex_number(v[0]);
	}
	return -1;
}

void r_get_color(vec3 val, char* v){
	int len = strlen(v);
	int offset = 0;
	if(len == 4){
		offset = 1;
		len = 3;
	}else if(len == 7){
		offset = 1;
		len = 6;
	}

	if(len == 3){
		val[0] = r_hex_multi(&v[offset], 1)   / 255.f;
		val[1] = r_hex_multi(&v[offset+1], 1) / 255.f;
		val[2] = r_hex_multi(&v[offset+2], 1) / 255.f;
	}else if(len == 6){
		val[0] = r_hex_multi(&v[offset], 2)   / 255.f;
		val[1] = r_hex_multi(&v[offset+2], 2) / 255.f;
		val[2] = r_hex_multi(&v[offset+4], 2) / 255.f;
	}
}

r_anim r_get_anim(r_sheet sheet, u32* frames, int frame_count, int frame_rate){
	u32* _frames = malloc(sizeof(u32) * frame_count);
	memcpy(_frames, frames, sizeof(u32) * frame_count);
	return (r_anim){
		_frames, frame_count, frame_rate, 0, sheet.id
	};
}

void r_cache_anim(r_anim anim, const char* name){
	if(g_anim_cache.count >= g_anim_cache.capacity){
		_e("Animation cache at capacity.\n");
		//TODO overflow caching
		return;	
	}

	for(int i=0;i<g_anim_cache.count;++i){
		if(strcmp(name, g_anim_cache.names[i]) == 0){
			_l("Animation cache already contains animation named: %s\n", name);
			return;
		}
	}

	g_anim_cache.uid ++;
	anim.uid = g_anim_cache.uid;

	g_anim_cache.names[g_anim_cache.count] = name;
	g_anim_cache.anims[g_anim_cache.count] = anim;

	g_anim_cache.count ++;

#ifndef EXCLUDE_CREATE
	int count = r_res_map.anim_count;
	r_res_map.anim_ids[count] = g_anim_cache.uid;
	r_res_map.anims[count] = anim;
	r_res_map.anim_count++;
#endif
}

r_anim* r_get_anim_n(const char* name){
	if(g_anim_cache.count == 0){
		_l("No animations in cache to check for.\n");
		return NULL;
	}

	for(int i=0;i<g_anim_cache.count;++i){
		if(strcmp(g_anim_cache.names[i], name) == 0){
			return &g_anim_cache.anims[i];	
		}
	}

	_l("No animations matching: %s in cache.\n", name);
	return NULL;
}

r_anim* r_get_anim_i(u32 uid){
	for(int i=0;i<g_anim_cache.count;++i){
		if(g_anim_cache.anims[i].uid == uid){
			return &g_anim_cache.anims[i];
		}
	}
	return NULL;
}

void r_drawable_set_anim(r_drawable* drawable, r_anim* anim){
	r_animv* v = &drawable->anim;

	memcpy(v->frames, anim->frames, sizeof(u32) * anim->frame_count);
	v->frame = 0;
	v->frame_count = anim->frame_count;
	v->frame_rate = anim->frame_rate;
	v->anim_id = anim->uid;
	v->time = 0L;
}

r_animv r_v_anim(r_anim* anim){
	r_animv animv;

	animv.frames = malloc(RENDER_ANIM_MAX_FRAMES * sizeof(u32)); 
	memcpy(animv.frames, anim->frames, anim->frame_count * sizeof(u32));
	animv.sheet = anim->sheet;
	animv.anim_id = anim->uid;

	animv.frame_count = anim->frame_count;
	animv.frame_rate = anim->frame_rate;
	animv.time = 0L;
	animv.state = R_ANIM_STOP;
	animv.pstate = R_ANIM_STOP;
	animv.loop = 1;

	return animv;
}

void r_destroy_animv(r_animv* animv){
	free(animv->frames);
	free(animv);	
}

void r_anim_p(r_animv* anim){
	anim->pstate = anim->state;
	anim->state = R_ANIM_PLAY;
}

void r_anim_s(r_animv* anim){
	anim->pstate = anim->state;
	anim->state = R_ANIM_STOP;
}

void r_anim_h(r_animv* anim){
	anim->pstate = anim->state;
	anim->state = R_ANIM_PAUSE;
}

r_drawable* r_get_drawable_t(r_subtex sub_tex, r_shader shader, vec2 size, vec2 pos){
	r_drawable* draw;

	if(g_drawable_cache.count < RENDER_BATCH_SIZE){
		draw = &g_drawable_cache.drawables[g_drawable_cache.count];
		g_drawable_cache.count ++;
		g_drawable_cache.uid ++;
		draw->uid = g_drawable_cache.uid;
	}else{
		return NULL;
	}

	mat4x4_identity(draw->model);	
	mat4x4_translate(draw->model, pos[0], pos[1], 0.f);
	mat4x4_scale_aniso(draw->model, draw->model, size[0], size[1], 1.f);

	vec2_dup(draw->size, size);
	vec2_dup(draw->position, pos);
	draw->tex = sub_tex;
	draw->layer = 0;
	draw->animated = 0;
	draw->visible = 1;
	draw->change = 0;
	draw->flip_x = 0;
	draw->flip_y = 0;
	draw->shader = shader;

	return draw;	
}

r_drawable* r_drawable_create(r_anim* anim, r_shader shader, vec2 size, vec2 pos){
	r_drawable* draw;

	if(g_drawable_cache.count < RENDER_BATCH_SIZE){
		draw = &g_drawable_cache.drawables[g_drawable_cache.count];
		g_drawable_cache.count ++;
		g_drawable_cache.uid ++;
		draw->uid = g_drawable_cache.uid;
	}else{
		return NULL;
	}

	mat4x4_identity(draw->model);	
	mat4x4_translate(draw->model, pos[0], pos[1], 0.f);
	mat4x4_scale_aniso(draw->model, draw->model, size[0], size[1], 1.f);

	vec2_dup(draw->size, size);
	vec2_dup(draw->position, pos);
	draw->anim = r_v_anim(anim);
	draw->layer = 0;
	draw->visible = 1;
	draw->change = 0;
	draw->flip_x = 0;
	draw->flip_y = 0;
	draw->animated = 1;
	draw->shader = shader;

	return draw;	
}

r_drawable* r_get_drawablei(u32 uid){
	for(unsigned int i=0;i<g_drawable_cache.count;++i){
		if(g_drawable_cache.drawables[i].uid == uid){
			return &g_drawable_cache.drawables[i];
		}
	}

	return NULL;
}

void r_destroy_drawable(r_drawable* drawable){
	unsigned int index = 0, found = 0;
	for(unsigned int i=0;i<g_drawable_cache.count;++i){
		if(g_drawable_cache.drawables[i].uid == drawable->uid){
			index = i;
			found = 1;
			break;	
		}
	}

	if(drawable->animated){
		free(drawable->anim.frames);
	}

	if(!found){
		return;
	}

	for(unsigned int i=index;i<g_drawable_cache.count-1;++i){
		g_drawable_cache.drawables[i] = g_drawable_cache.drawables[i+1];
	}

	if(g_drawable_cache.count == g_drawable_cache.capacity){
		memset(&g_drawable_cache.drawables[g_drawable_cache.count], 0, sizeof(r_drawable));
	}

	g_drawable_cache.count --;
}

void r_destroy_drawables(r_drawable* drawable_list, unsigned int list_size){
	int drawable_ids[list_size];
	int drawable_indexes[list_size];

	int found_index = 0;

	for(unsigned int i=0;i<g_drawable_cache.count;++i){
		for(unsigned int j=0;j<list_size;++j){
			if(drawable_list[j].uid == g_drawable_cache.drawables[i].uid){
				drawable_ids[found_index] = drawable_list->uid;
				drawable_indexes[found_index] = i;
				++found_index;
				break;
			}	
		}
	}

	//TODO single pass remove
	//int remove_index = 0;
	for(unsigned int i=0;i<g_drawable_cache.count;++i){
		for(int j=0;j<found_index;++j){
			if(i == drawable_indexes[j]){
				for(unsigned int k=drawable_indexes[j];k<g_drawable_cache.count-1;++k){
					g_drawable_cache.drawables[k] = g_drawable_cache.drawables[k+1];
				}	
				
				memset(&g_drawable_cache.drawables[g_drawable_cache.count-1], 0, sizeof(r_drawable));
				//g_drawable_cache.drawables[g_drawable_cache.count-1] = 0;
				g_drawable_cache.count --;
				break;
			}		
		}	
	}		
}

void r_drawable_update(r_drawable* drawable, long delta){
	if(drawable->change){
		f32 x, y;
		x = floorf(drawable->position[0]);
		y = floorf(drawable->position[1]);
		mat4x4_translate(drawable->model, x, y, 0.f);
		mat4x4_scale_aniso(drawable->model, drawable->model, drawable->size[0], drawable->size[0], drawable->size[1]);

		drawable->change = 0;
	}

	if(drawable->animated){
		if(drawable->anim.state == R_ANIM_PLAY && drawable->visible){
			r_animv* anim = &drawable->anim;

			f32 frame_time = MS_PER_SEC / anim->frame_rate;	
			if(anim->time + delta >= frame_time){
				if(anim->frame >= anim->frame_count-1){
					if(!anim->loop){
						anim->state = R_ANIM_STOP;
						anim->pstate = R_ANIM_PLAY;
					}

					anim->frame = 0;
				}else{
					anim->frame ++;
				}

				anim->time -= frame_time;
			}else{
				anim->time += delta;
			}

			if(anim->frame > anim->frame_count-1){
				anim->frame = 0;
			}
		}
	}
}

void r_drawable_remove(u32 uid){
	int index = 0;
	for(unsigned int i=0;i<g_drawable_cache.count;++i){
		if(g_drawable_cache.drawables[i].uid == uid){
			index = i;
			break;
		}
	}

	unsigned int range = (g_drawable_cache.count == RENDER_BATCH_SIZE) ? RENDER_BATCH_SIZE - 1 : g_drawable_cache.count;
	for(unsigned int i=index;i<range;++i){
		g_drawable_cache.drawables[i] = g_drawable_cache.drawables[i+1];
	}

	memset(&g_drawable_cache.drawables[range], 0, sizeof(r_drawable));
}

r_uniform_array* r_shader_get_array(r_shader shader, const char* name){
	r_shader_batch* batch = r_shader_get_batch(shader);
	if(!batch){
		return 0;
	}

	for(unsigned int i=0;i<batch->uniform_array_count;++i){
		if(strcmp(batch->uniform_arrays[i].name, name) == 0){
			return &batch->uniform_arrays[i];
		}
	}

	return 0;
}

void r_shader_setup_array(r_shader shader, const char* name, int stride, int capacity, int type){
	r_shader_batch* batch = r_shader_get_batch(shader); 

	if(!batch){
		return;
	}

	r_uniform_array array = (r_uniform_array){0};
	void* data = malloc(stride * capacity);
	array.data = data;
	array.type = type;
	array.name = name;

	r_uniform_array* new_array = (r_uniform_array*)malloc(sizeof(r_uniform_array) * (batch->uniform_array_count + 1));

	memcpy(new_array, batch->uniform_arrays, sizeof(r_uniform_array) * batch->uniform_array_count);
	free(batch->uniform_arrays);
	batch->uniform_arrays = new_array;
	batch->uniform_array_count++;
}

void r_shader_add_to_array(r_shader shader, const char* name, void* data, int count){
	r_uniform_array* array = r_shader_get_array(shader, name);

	void* data_ptr = array->data + (array->stride * array->count);
	void* array_ptr = data;

	for(int i=0;i<count;++i){
		if(!data_ptr || !array_ptr || array->count == array->capacity) break;
	
		memcpy(data_ptr, array_ptr, array->stride);
		
		array->count ++;
		array_ptr += array->stride;
	   	data_ptr += array->stride;
	}	
}

int r_shader_clear_array(r_shader shader, const char* name){
	r_uniform_array* array = r_shader_get_array(shader, name);
	memset(array->data, 0, array->stride * array->capacity);

	int count = array->count;	
	array->count = 0;

	return count;
}

void r_set_array(r_shader shader, const char* name){
	r_uniform_array* array = r_shader_get_array(shader, name);
	switch(array->type){
		case r_vec2:
			r_set_v2x(shader, array->count, name, array->data);
		break;
   		case r_vec3:
			r_set_v3x(shader, array->count, name, array->data);
		break;
	   	case r_vec4:
			r_set_v4x(shader, array->count, name, array->data);
	   	break;
		case r_float:
			r_set_fx(shader, array->count, name, array->data);
		break;
	   	case r_int:
			r_set_ix(shader, array->count, name, array->data);
		break;
   		case r_uint:
			//TODO implement uint into shaders (maybe not needed?)
		break;
		case r_mat:
			r_set_m4x(shader, array->count, name, array->data);
		break;
		default:
		break;
	}
}

void r_shader_set_arrays(r_shader shader){
	r_shader_batch* batch = r_shader_get_batch(shader);
	if(!batch) return;

	for(unsigned int i=0;i<batch->uniform_array_count;++i){
		r_uniform_array* array = &batch->uniform_arrays[i];
		switch(array->type){
			case r_vec2:
				r_set_v2x(shader, array->count, array->name, array->data);
				break;
			case r_vec3:
				r_set_v3x(shader, array->count, array->name, array->data);
				break;
			case r_vec4:
				r_set_v4x(shader, array->count, array->name, array->data);
				break;
			case r_float:
				r_set_fx(shader, array->count, array->name, array->data);
				break;
			case r_int:
				r_set_ix(shader, array->count, array->name, array->data);
				break;
			case r_uint:
				//TODO implement uint into shaders (maybe not needed?)
				break;
			case r_mat:
				r_set_m4x(shader, array->count, array->name, array->data);
				break;
			default:
				break;
		}

		memset(array->data, 0, array->stride * array->capacity);
		array->count = 0;
	}
}
inline void r_set_uniformf(r_shader shader, const char* name, f32 value){
	glUniform1f(glGetUniformLocation(shader, name), value);
}

inline void r_set_uniformfi(int loc, f32 value){
	glUniform1f(loc, value);
}

inline void r_set_uniformi(r_shader shader, const char* name, int value){
	glUniform1i(glGetUniformLocation(shader, name), value);
}

inline void r_set_uniformii(int loc, int val){
	glUniform1i(loc, val);
}

inline void r_set_v4(r_shader shader, const char* name, vec4 value){
	glUniform4f(glGetUniformLocation(shader, name), value[0], value[1], value[2], value[3]);
}

inline void r_set_v4i(int loc, vec4 value){
	glUniform4f(loc, value[0], value[1], value[2], value[3]);
}

inline void r_set_v3(r_shader shader, const char* name, vec3 value){
	glUniform3f(glGetUniformLocation(shader, name), value[0], value[1], value[2]);
}

inline void r_set_v3i(int loc, vec3 val){
	glUniform3f(loc, val[0], val[1], val[2]);
}

inline void r_set_v2(r_shader shader, const char* name, vec2 value){
	glUniform2f(glGetUniformLocation(shader, name), value[0], value[1]);
}

inline void r_set_v2i(int loc, vec2 val){
	glUniform2f(loc, val[0], val[1]);
}

inline void r_set_m4(r_shader shader, const char* name, mat4x4 value){
	glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE, (GLfloat*)value);
}

inline void r_set_m4i(int loc, mat4x4 val){
	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)val);
}

void r_set_m4x(r_shader shader, u32 count, const char* name, mat4x4* values){
	if(!count) return;
	glUniformMatrix4fv(glGetUniformLocation(shader, name), count, GL_FALSE, (const GLfloat*)values);
}

void r_set_ix(r_shader shader, u32 count, const char* name, int* values){
	if(!count) return;
	glUniform1iv(glGetUniformLocation(shader, name), count, (const GLint*)values);
}

void r_set_fx(r_shader shader, u32 count, const char* name, f32* values){
	if(!count) return;
	glUniform1fv(glGetUniformLocation(shader, name), count, (const GLfloat*)values);
}

void r_set_v2x(r_shader shader, u32 count, const char* name, vec2* values){
	if(!count) return;

	glUniform2fv(glGetUniformLocation(shader, name), count, (const GLfloat*)values);
}

void r_set_v3x(r_shader shader, u32 count, const char* name, vec3* values){
	if(!count) return;

	glUniform3fv(glGetUniformLocation(shader, name), count, (const GLfloat*)values);
}

void r_set_v4x(r_shader shader, u32 count, const char* name, vec4* values){
	if(!count) return;

	glUniform4fv(glGetUniformLocation(shader, name),  count, (const GLfloat*)values);
}

void r_window_get_size(int* w, int* h){
	*w = g_window.width;
	*h = g_window.height;
}

int r_get_videomode_str(char* dst, int index){
	if(index >= flags.video_mode_count){
		index = 0;	
	}	
	index = (flags.video_mode_count-1) - index;
	int str_len = sprintf(dst, "%ix%i@%i", r_vidmodes[index].width, r_vidmodes[index].height, r_vidmodes[index].refreshRate);
	return str_len;
}

void r_select_mode(int index, int fullscreen, int vsync, int borderless){
	if(index > flags.video_mode_count){
		_l("Invalid video mode index, not setting.\n");
		return;
	}

	index = (flags.video_mode_count-1) - index;

	const GLFWvidmode* selected_mode = &r_vidmodes[index];

	if(!fullscreen && borderless != g_window.borderless)
	flags.allowed = 0;

	if(fullscreen){
		glfwWindowHint(GLFW_RED_BITS, selected_mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, selected_mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, selected_mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, selected_mode->refreshRate);

		g_window.refreshRate = selected_mode->refreshRate;

		vec2_dup(r_res, (vec2){selected_mode->width, selected_mode->height});

		glfwSetWindowMonitor(g_window.glfw, r_default_monitor, 0, 0, selected_mode->width, selected_mode->height, selected_mode->refreshRate); 
	}else{
		//TODO fix borderless switching back to decorated
		if(g_window.borderless != borderless){
			g_window.borderless = borderless;
			glfwSetWindowAttrib(g_window.glfw, GLFW_DECORATED, (borderless == 0) ? GLFW_TRUE : GLFW_FALSE);
			_l("Setting borderless to: %i\n", borderless);
		}

		if(selected_mode->width != g_window.width || selected_mode->height != g_window.height){
			glfwSetWindowSize(g_window.glfw, selected_mode->width, selected_mode->height);
			vec2_dup(r_res, (vec2){selected_mode->width, selected_mode->height});

			r_window_center();
		}

		if(fullscreen != g_window.fullscreen){	
			int x, y;
			glfwGetWindowPos(g_window.glfw, &x, &y);
			glfwSetWindowMonitor(g_window.glfw, 0, x, y, selected_mode->width, selected_mode->height, selected_mode->refreshRate); 
		}

	}

	g_window.fullscreen = fullscreen;
	g_window.vsync = vsync;
	if(vsync){
		glfwSwapInterval(1);
	}

	if(!fullscreen){
		const GLFWvidmode* monitor_mode = glfwGetVideoMode(r_default_monitor);
		glfwSetWindowPos(g_window.glfw, (monitor_mode->width - g_window.width) / 2, (monitor_mode->height - g_window.height) / 2);
	}

	flags.allowed = 1;
}

int r_get_vidmode_count(void){
	return flags.video_mode_count;
}

int r_allow_render(void){
	return flags.allowed;	
}

int r_is_vsync(void){
	return g_window.vsync;
}

int r_is_fullscreen(void){
	return g_window.fullscreen;
}

int r_is_borderless(void){
	return g_window.borderless; 	
}

static void r_window_get_modes(void){
	if(r_default_monitor == NULL){
		r_default_monitor = glfwGetPrimaryMonitor();
	}
	int count;
	r_vidmodes = glfwGetVideoModes(r_default_monitor,&count);
	flags.video_mode_count = count;
}

static const GLFWvidmode* r_find_closest_mode(r_window_info info){
	if(flags.video_mode_count == 0){
		r_window_get_modes();
	}else if(flags.video_mode_count == 1){
		return r_vidmodes;
	}

	const GLFWvidmode* closest = &r_vidmodes[0];
	int distance = (abs(info.width - r_vidmodes[0].width) + abs(info.height - r_vidmodes[0].height) - r_vidmodes[0].refreshRate);

	for(int i=0;i<flags.video_mode_count;++i){
		int d2 = (abs(info.width - r_vidmodes[i].width) + abs(info.height - r_vidmodes[i].height) - r_vidmodes[i].refreshRate);
		if(d2 < distance){
			closest = &r_vidmodes[i];
			distance = d2;
		}
	}

	return closest;
}

static const GLFWvidmode* r_find_best_mode(void){
	if(flags.video_mode_count == 0){
		r_window_get_modes();
	}else if(flags.video_mode_count == 1){
		return r_vidmodes;
	}

	const GLFWvidmode* selected = &r_vidmodes[0];
	int value = selected->width + selected->height * (selected->refreshRate * 2);

	for(int i=0;i<flags.video_mode_count;++i){
		int vec2 = r_vidmodes[i].width + r_vidmodes[i].height * (r_vidmodes[i].refreshRate * 2);
		if(vec2 > value){
			selected = &r_vidmodes[i];
			value = vec2;
		}
	}

	return selected;
}

int r_window_create(r_window_info info){
#if defined(INIT_DEBUG)
	_l("Creating window.\n");
#endif

	glfwSetErrorCallback(glfw_err_cb);

	if(!glfwInit()){
		_e("Unable to initialize GLFW\n");
		return 0;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	GLFWwindow* window = NULL;
	g_window = (r_window){0};

	if(info.fullscreen){
		const GLFWvidmode* selected_mode;
		
		if(info.width > 0 && info.height > 0 && info.refreshRate > 0){
			selected_mode = r_find_closest_mode(info);
		}else{
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

		window = glfwCreateWindow(selected_mode->width, selected_mode->height, info.title, r_default_monitor, NULL);
	}else{
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_DECORATED, (info.borderless == 0) ? GLFW_TRUE : GLFW_FALSE);
		g_window.borderless = info.borderless;

		if(info.refreshRate > 0){
			glfwWindowHint(GLFW_REFRESH_RATE, info.refreshRate);
			g_window.refreshRate = info.refreshRate;
		}

		g_window.width = info.width;
		g_window.height = info.height;

		vec2_dup(r_res, (vec2){info.width, info.height});

		g_window.fullscreen = 0;
		g_window.vsync = info.vsync;

		window = glfwCreateWindow(info.width, info.height, info.title, NULL, NULL);
	}

	if(!window){
		_e("Error: Unable to create GLFW window.\n");
		glfwTerminate();
		return 0;
	}

	if(info.icon){
		asset_t* icon = asset_get("sys", info.icon);
		r_window_set_icon(icon);
	}

	g_window.glfw = window;

	glfwMakeContextCurrent(window);

	gladLoadGL(glfwGetProcAddress);

	if(g_window.vsync){
		glfwSwapInterval(1);
	}

#if defined(INIT_DEBUG)
	_l("Window context created successfully.\n");
#endif

	flags.allowed = 1;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	glEnable(GL_SCISSOR_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, g_window.width, g_window.height);

	glfwGetWindowPos(g_window.glfw, &g_window.x, &g_window.y);

	vec3 color;
	r_get_color(color, "222");
	glClearColor(color[0], color[1], color[2], 1.f);

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

void r_window_center(void){
	GLFWmonitor* mon = NULL;
	int monitor_count;
	GLFWmonitor** monitors =  glfwGetMonitors(&monitor_count);

	if(monitor_count == 0){
		return;
	}else if(monitor_count == 1){
		const GLFWvidmode* mode = glfwGetVideoMode(monitors[0]);
		r_window_set_pos((mode->width - g_window.width) / 2, (mode->height - g_window.height) / 2);
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
		r_window_set_pos((mon_w - g_window.width) / 2, (mon_h - g_window.height) / 2);
	}
}

void r_window_set_pos(int x, int y){
	glfwSetWindowPos(g_window.glfw, x, y);
}

int r_window_set_icon(asset_t* asset){
	if(asset->filled){
		int w, h, ch;
		unsigned char* img = stbi_load_from_memory(asset->data, asset->data_length, &w, &h, &ch, 0);

		GLFWimage glfw_img = (GLFWimage){ w, h, img };
		glfwSetWindowIcon(g_window.glfw, 1, &glfw_img);

		free(img);

		asset->req_free = 1;
	}else{
		_l("No window icon passed to set.\n");
		return 0;
	}

	return 1;
}

void r_window_destroy(void){
	flags.allowed = 0;

	glfwDestroyWindow(g_window.glfw);

	g_window.glfw = NULL;
	g_window.width = -1;
	g_window.height = -1;
	g_window.refreshRate = -1;
	g_window.fullscreen = 0;
	g_window.vsync = 0;
}

void r_window_request_close(void){
	g_window.close_requested = 1;
}

int r_window_should_close(void){
	return g_window.close_requested;
}

void r_window_swap_buffers(void){
	glfwSwapBuffers(g_window.glfw);
}

void r_window_clear(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
