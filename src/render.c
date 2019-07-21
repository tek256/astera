#include "render.h"

#include <string.h>

#include <glad_gl.c>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "input.h"
#include "sys.h"
#include "debug.h"

static r_flags flags;

static const char* r_window_title = "demo";

static r_window_info g_window_info;
static GLFWmonitor* r_default_monitor;
static const GLFWvidmode* r_vidmodes;
static vec2 r_res;
static unsigned int default_quad_vao;

static char* shader_value_buff[SHADER_STR_SIZE];

int r_init(c_conf conf){
	r_window_info info;
	info.width = conf.width;
	info.height = conf.height;
	info.fullscreen = conf.fullscreen;
	info.vsync = conf.vsync;
	info.borderless = conf.borderless;
	info.refreshRate = conf.refreshRate;
	info.title = r_window_title; 

	if(!r_create_window(info)){ 
		_e("Unable to create window.\n");
		return 0;
	}

	r_center_window();

	r_create_camera(&g_camera, (vec2){r_res[0], r_res[1]}, (vec2){0.f, 0.f});

	g_anim_cache.capacity = RENDER_ANIM_CACHE;
	g_shader_map.capacity = RENDER_SHADER_CACHE;
	g_drawable_cache.capacity = RENDER_BATCH_SIZE;

	default_quad_vao = r_init_quad();

	return 1;
}

void r_create_camera(r_camera* cam, vec2 size, vec2 position){
	vec3_dup(cam->pos, (vec3){position[0], position[1], 0.f});
	vec2_dup(cam->size, size);

	cam->near = -10.f;
	cam->far = 10.f;

	float x, y;
	x = floorf(cam->pos[0]);
	y = floorf(cam->pos[1]);

	mat4x4_ortho(&cam->proj, 0, cam->size[0], cam->size[1], 0, cam->near, cam->far);

	mat4x4_translate(&cam->view, x, y, 0.f);
	mat4x4_rotate_x(&cam->view, cam->view, 0.0);
	mat4x4_rotate_y(&cam->view, cam->view, 0.0);
	mat4x4_rotate_z(&cam->view, cam->view,  0.0);
}

void r_move_cam(float x, float y){
	g_camera.pos[0] -= x;
	g_camera.pos[1] += y;
}

void r_update_camera(){
	float x, y;
	x = floorf(g_camera.pos[0]);
	y = floorf(g_camera.pos[1]);
	mat4x4_translate(g_camera.view, x, y, 0.f);
}

void r_update(long delta){
	for(int i=0;i<g_drawable_cache.count;++i){
		r_update_drawable(&g_drawable_cache.drawables[i], delta);
	}

	r_update_camera();	
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

int r_init_quad() {
	int vao, vbo, vboi;

	float verts[16] = {
		//pos       //tex
		-0.5f, -0.5f,   0.f, 0.f,
		-0.5f,  0.5f,   0.f, 1.f,
		0.5f,  0.5f,   1.f, 1.f,
		0.5f, -0.5f,   1.f, 0.f
	};

	unsigned short inds[6] = { 
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned short), &inds[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	return vao;
}

void r_update_batch(r_shader shader, r_sheet* sheet){
	int cache_index = -1;
	for(int i=0;i<g_shader_map.count;++i){
		if(g_shader_map.shaders[i] == shader){
			cache_index = i;
			break;
		}
	}

	if(cache_index == -1){
		_e("Unable to find cache to update batch: %d\n", shader);
		return;
	}

	r_shader_batch* cache = &g_shader_map.batches[cache_index];
	for(int i=0;i<g_drawable_cache.count;++i){
		if(cache->count >= cache->capacity){
			_e("Reached cache limit for shader: %d\n", shader);
			break;
		}
		if(g_drawable_cache.drawables[i].shader == shader){
			if(g_drawable_cache.drawables[i].anim.sheet_id == sheet->id){
				mat4x4_dup(cache->models[cache->count], g_drawable_cache.drawables[i].model);
				cache->tex_ids[cache->count] = (unsigned int)g_drawable_cache.drawables[i].anim.frames[g_drawable_cache.drawables[i].anim.frame];
				cache->flip_x[cache->count] = g_drawable_cache.drawables[i].flip_x;
				cache->flip_y[cache->count] = g_drawable_cache.drawables[i].flip_y;
				cache->count ++;
			}
		}
	}
}

void r_draw_call(r_shader shader, r_sheet* sheet){
	if(!shader || !g_drawable_cache.count) return;

	int cache_index = -1;

	for(int i=0;i<g_shader_map.count;++i){
		if(g_shader_map.shaders[i] == shader){
			cache_index = i;
			break;
		}		
	}

	if(cache_index == -1){
		_e("Unable to find shader cache for: %d\n", shader);
		return;
	}

	r_shader_batch* cache = &g_shader_map.batches[cache_index];

	r_bind_shader(shader);

	r_set_m4x(shader, 128, "models",  cache->models);
	r_set_ix(shader, 128, "tex_ids", cache->tex_ids);
	r_set_ix(shader, 128, "flip_x", cache->flip_x);
	r_set_ix(shader, 128, "flip_y", cache->flip_y);

	vec2 sub_size;
	vec2_dup(sub_size, (vec2){ sheet->subwidth, sheet->subheight });
	vec2 tex_size;
	vec2_dup(tex_size, (vec2){ sheet->width, sheet->height });

	//set texture shit
	r_set_v2(shader, "sub_size", sub_size);
	r_set_v2(shader, "tex_size", tex_size);	

	r_set_m4(shader, "proj", g_camera.proj);
	r_set_m4(shader, "view", g_camera.view);	

	r_bind_tex(sheet->id);

	//draw instanced
	glBindVertexArray(default_quad_vao);
	glEnableVertexAttribArray(0);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, cache->count);

	cache->count = 0;
}

void r_destroy_anims(){
	for(int i=0;i<g_anim_cache.count;++i){
		free(g_anim_cache.anims[i].frames);
	}
}

void r_destroy_quad(unsigned int vao){
	glDeleteVertexArrays(1, &vao);
}

void r_exit(){
	r_destroy_anims();
	r_destroy_quad(default_quad_vao);
	r_destroy_window(g_window);
	glfwTerminate();

	if(c_prefs_located()){
		r_save_prefs();
	}
}

void r_save_prefs(){
			
}

int r_is_anim_cache(){
	return 1; 
}

int r_is_shader_cache(){
	return 1;	
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
		_l("%s\n", log);
		free(log);
	}

#ifdef DEBUG_OUTPUT
	_l("r_shader program loaded: %i\n", id);
#endif
	return (r_shader){id};
}

void r_map_shader(r_shader shader, const char* name){
	if(g_shader_map.capacity == 0){
		_e("No shader cache available.\n");
		return;
	}

	if(g_shader_map.capacity == g_shader_map.count){
		//TODO implement cache overflow
		_e("No shader cache open to use.\n");
		return;
	}

	for(int i=0;i<g_shader_map.count;++i){
		if(g_shader_map.shaders[i] == shader){
			_l("Shader: %d already contained with alias of: %s\n", shader, g_shader_map.names[i]);
			return;
		}
	}

	unsigned int count = g_shader_map.count;

	g_shader_map.shaders[count] = shader;
	g_shader_map.names[count] = name;

	g_shader_map.caches[count].shader = shader;
	g_shader_map.caches[count].capacity = RENDER_BATCH_SIZE;

	g_shader_map.batches[count].shader = shader;
	g_shader_map.batches[count].capacity = RENDER_BATCH_SIZE;

	g_shader_map.count ++;
}

void r_clear_cache(r_shader shader){
	r_shader_cache* specific;
	int index = 0;

	for(int i=0;i<g_shader_map.count;++i){
		if(g_shader_map.shaders[i] == shader){
			specific = &g_shader_map.caches[i];
			index = i;
		}
	}

	if(specific){
		memset(specific->values, 0, sizeof(unsigned int) * RENDER_SHADER_VALUE_CACHE);
		memset(specific->names, 0, sizeof(const char*) * RENDER_SHADER_VALUE_CACHE);	
	}	
}

void r_remove_from_cache(r_shader shader){
	if(!g_shader_map.count){
		_e("No shaders to remove from cache.\n");
		return; 
	}

	int index = -1;
	for(int i=0;i<g_shader_map.count;++i){
		if(g_shader_map.shaders[i] == shader){
			index = i;
			break;
		}
	}

	if(index){
		for(int i=index;i<g_shader_map.count-1;++i){
			g_shader_map.names[i] = g_shader_map.names[i+1];
			g_shader_map.shaders[i] = g_shader_map.shaders[i+1];
			g_shader_map.caches[i] = g_shader_map.caches[i+1];
		}

		g_shader_map.names[g_shader_map.count] == "";
		g_shader_map.shaders[g_shader_map.count] = 0;
		g_shader_map.caches[g_shader_map.count] = (r_shader_cache){0};
		g_shader_map.count --;
	}
}

void r_cache_uniform(r_shader shader, const char* uniform, unsigned int location){
	r_shader_cache* specific;

	for(int i=0;i<g_shader_map.count;++i){
		if(g_shader_map.shaders[i] == shader){
			specific = &g_shader_map.caches[i];
			break;
		}	
	}

	for(int i=0;i<specific->count;++i){
		if(strcmp(specific->names[i], uniform) == 0){
			_l("Uniform: %s for shader: %d already cached.\n", uniform, shader);
			return;
		}
	}

	if(specific->capacity <= specific->count){
		//TODO implement shader cache overflow
		_e("Shader uniform cache space unavailable.\n");
		return;
	}

	specific->values[specific->count] = location;
	specific->names[specific->count] = uniform;
	specific->count ++;
}

void r_destroy_shader(r_shader shader){
	r_remove_from_cache(shader);
	glDeleteProgram(shader);
}

void r_bind_tex(unsigned int tex){
	glBindTexture(GL_TEXTURE_2D, tex);
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

r_anim  r_get_anim(r_sheet sheet, unsigned int* frames, int frame_count, int frame_rate){
	unsigned int* _frames = malloc(sizeof(unsigned int) * frame_count);
	memcpy(_frames, frames, sizeof(unsigned int) * frame_count);
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

r_anim* r_get_anim_i(unsigned int uid){
	for(int i=0;i<g_anim_cache.count;++i){
		if(g_anim_cache.anims[i].uid == uid){
			return &g_anim_cache.anims[i];
		}
	}
	return NULL;
}

void r_drawable_set_anim(r_drawable* drawable, r_anim* anim){
	r_animv* v = &drawable->anim;

	memcpy(v->frames, anim->frames, sizeof(unsigned int) * anim->frame_count);
	v->frame = 0;
	v->frame_count = anim->frame_count;
	v->frame_rate = anim->frame_rate;
	v->anim_id = anim->uid;
	v->time = 0L;
}

r_animv r_v_anim(r_anim* anim){
	r_animv animv;

	animv.frames = malloc(RENDER_ANIM_MAX_FRAMES * sizeof(unsigned int)); 
	memcpy(animv.frames, anim->frames, anim->frame_count * sizeof(unsigned int));
	animv.sheet_id = anim->sheet_id;
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

r_drawable* r_get_drawable(r_anim* anim, r_shader shader,  vec2 size, vec2 pos){
	r_drawable* draw;

	if(g_drawable_cache.count < RENDER_BATCH_SIZE){
		draw = &g_drawable_cache.drawables[g_drawable_cache.count];
		g_drawable_cache.count ++;
		g_drawable_cache.uid ++;
		draw->uid = g_drawable_cache.uid;
	}else{
		return NULL;
	}

	mat4x4_identity(&draw->model);	
	mat4x4_translate(&draw->model, pos[0], pos[1], 0.f);
	mat4x4_scale_aniso(&draw->model, draw->model, size[0], size[1], 1.f);

	vec2_dup(draw->size, size);
	vec2_dup(draw->position, pos);
	draw->anim = r_v_anim(anim);
	draw->layer = 0;
	draw->visible = 1;
	draw->change = 0;
	draw->flip_x = 0;
	draw->flip_y = 0;
	draw->c_tex = 0;
	draw->shader = shader;

	return draw;	
}

r_drawable* r_get_drawablei(unsigned int uid){
	for(int i=0;i<g_drawable_cache.count;++i){
		if(g_drawable_cache.drawables[i].uid == uid){
			return &g_drawable_cache.drawables[i];
		}
	}

	return NULL;
}

void r_update_drawable(r_drawable* drawable, long delta){
	if(drawable->change){

		float x, y;
		x = floorf(drawable->position[0]);
		y = floorf(drawable->position[1]);
		mat4x4_translate(&drawable->model, x, y, 0.f);
		mat4x4_scale_aniso(drawable->model, drawable->model, drawable->size[0], drawable->size[1], drawable->size[2]);

		drawable->change = 0;
	}

	if(drawable->anim.state == R_ANIM_PLAY && drawable->visible){
		r_animv* anim = &drawable->anim;

		float frame_time = MS_PER_SEC / anim->frame_rate;	
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

void r_remove_drawable(unsigned int uid){
	int index = 0;
	for(int i=0;i<g_drawable_cache.count;++i){
		if(g_drawable_cache.drawables[i].uid == uid){
			index = i;
			break;
		}
	}

	int range = (g_drawable_cache.count == RENDER_BATCH_SIZE) ? RENDER_BATCH_SIZE - 1 : g_drawable_cache.count;
	for(int i=index;i<range;++i){
		g_drawable_cache.drawables[i] = g_drawable_cache.drawables[i+1];
	}

	memset(&g_drawable_cache.drawables[range], 0, sizeof(r_drawable));
}

int r_get_cached(r_shader shader, const char* uniform){
	if(shader && g_shader_map.count){
		r_shader_cache* specific;
		for(int i=0;i<g_shader_map.count;++i){
			if(g_shader_map.shaders[i] == shader){
				specific = &g_shader_map.caches[i];
				break;
			}
		}

		if(specific){
			for(int i=0;i<specific->count;++i){
				if(strcmp(uniform, specific->names[i]) == 0){
					return specific->values[i];
				}
			}
		}	
	}

	return -1;
}

inline int r_get_uniform_loc(r_shader shader, const char* uniform){
	/*
	   int cache_loc = r_get_cached(shader, uniform);

	   if(!cache_loc){
	   cache_loc = glGetUniformLocation(shader, uniform);
	   r_cache_uniform(shader, uniform, cache_loc);
	   }	

	   return cache_loc;
	   */
	return glGetUniformLocation(shader, uniform);
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

inline void r_set_v4(r_shader shader, const char* name, vec4 value){
	glUniform4f(r_get_uniform_loc(shader, name), value[0], value[1], value[2], value[3]);
}

inline void r_set_v4i(int loc, vec4 value){
	glUniform4f(loc, value[0], value[1], value[2], value[3]);
}

inline void r_set_v3(r_shader shader, const char* name, vec3 value){
	glUniform3f(r_get_uniform_loc(shader, name), value[0], value[1], value[2]);
}

inline void r_set_v3i(int loc, vec3 val){
	glUniform3f(loc, val[0], val[1], val[2]);
}

inline void r_set_v2(r_shader shader, const char* name, vec2 value){
	glUniform2f(r_get_uniform_loc(shader, name), value[0], value[1]);
}

inline void r_set_v2i(int loc, vec2 val){
	glUniform2f(loc, val[0], val[1]);
}

inline void r_set_m4(r_shader shader, const char* name, mat4x4 value){
	glUniformMatrix4fv(r_get_uniform_loc(shader, name), 1, GL_FALSE, value);
}

inline void r_set_m4i(int loc, mat4x4 val){
	glUniformMatrix4fv(loc, 1, GL_FALSE, val);
}

void r_set_m4x(r_shader shader, unsigned int count, const char* name, mat4x4* values){
	if(!count) return;
	glUniformMatrix4fv(r_get_uniform_loc(shader, name), count, GL_FALSE, values);
}

void r_set_ix(r_shader shader, unsigned int count, const char* name, int* values){
	if(!count) return;
	glUniform1iv(r_get_uniform_loc(shader, name), count, values);
}

void r_set_fx(r_shader shader, unsigned int count, const char* name, float* values){
	if(!count) return;
	glUniform1fv(r_get_uniform_loc(shader, name), count, values);
}

void r_set_v2x(r_shader shader, unsigned int count, const char* name, vec2* values){
	if(!count) return;

	glUniform2fv(r_get_uniform_loc(shader, name), count, values);
}

void r_set_v3x(r_shader shader, unsigned int count, const char* name, vec3* values){
	if(!count) return;

	glUniform3fv(r_get_uniform_loc(shader, name), count, values);
}

void r_set_v4x(r_shader shader, unsigned int count, const char* name, vec4* values){
	if(!count) return;

	glUniform4fv(r_get_uniform_loc(shader, name),  count, values);
}

static void r_create_modes(){
	if(r_default_monitor == NULL){
		r_default_monitor = glfwGetPrimaryMonitor();
	}
	int count;
	r_vidmodes = glfwGetVideoModes(r_default_monitor,&count);
	flags.video_mode_count = count;
}

static int r_window_info_valid(r_window_info info){
	return info.width > 0 && info.height > 0;
}

static const GLFWvidmode* r_find_closest_mode(r_window_info info){
	if(flags.video_mode_count== 0){
		r_create_modes();
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

static const GLFWvidmode* r_find_best_mode(){
	if(flags.video_mode_count == 0){
		r_create_modes();
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

static void _error_callback(int err, const char* msg){
	_e("GLFW Error (%d) %s\n", err, msg);
}

int r_create_window(r_window_info info){
	if(!r_window_info_valid(info)){
		return 0;
	}

	_l("Loading GLFW.\n");

	glfwSetErrorCallback(_error_callback);

	if(!glfwInit()){
		_e("Unable to initialize GLFW\n");
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

		r_default_monitor = glfwGetPrimaryMonitor();

		g_window.refreshRate = selected_mode->refreshRate;
		g_window.width = selected_mode->width;
		g_window.height = selected_mode->height;
		g_window.fullscreen = 1;

		vec2_dup(r_res, (vec2){selected_mode->width, selected_mode->height});

		window = glfwCreateWindow(selected_mode->width, selected_mode->height, info.title, r_default_monitor, NULL);
	}else{
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		_l("Borderless: %d\n", info.borderless);
		glfwWindowHint(GLFW_DECORATED, (info.borderless == 0) ? GLFW_TRUE : GLFW_FALSE);

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
		if(g_window.vsync){
			glfwSwapInterval(1);
		}
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

	//glDisable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, g_window.width, g_window.height);

	glfwGetWindowPos(g_window.glfw, &g_window.x, &g_window.y);

	vec3 color;
	r_get_color(color, "222");
	glClearColor(color[0], color[1], color[2], 1.f);

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
	_e("ERROR: %i %s\n", error, msg);
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
