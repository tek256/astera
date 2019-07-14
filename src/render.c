//TODO
//
// 1. Optimize tree allocation into heap & contiguous array
// 2. Implement proper shader cache
// 3. Implement area based culling of renderables (game.c maybe)

#include "render.h"

#include <glad_gl.c>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "input.h"
#include "sys.h"
#include "debug.h"

#include <string.h>

static r_flags flags;

static const char* r_window_title = "demo";
static r_window_info g_window_info;
static GLFWmonitor* r_default_monitor;
static const GLFWvidmode* r_vidmodes;
static v2 r_res;

static r_drawable drawables[RENDER_BATCH_SIZE];
static int drawable_count = 0;
static unsigned int drawable_uid = 1;

static unsigned int default_quad_vao;

static char* shader_value_buff[SHADER_STR_SIZE];

int r_init(){
	r_create_camera(&g_camera, r_res, (v2){0.f, 0.f});

	if(!r_create_window((r_window_info){1280, 720, 0, 0, 0, 60, r_window_title})){ 
		_e("Unable to create window.\n");
		return 0;
	}

	#if defined(RENDER_ENABLE_ANIM_CACHE)	
	g_anim_cache.capacity = RENDER_ANIM_CACHE;
	#endif

	#if defined(RENDER_ENABLE_SHADER_CACHE)
	g_shader_map.capacity = RENDER_SHADER_CACHE;
	#endif

	default_quad_vao = r_init_quad();
	_l("Default quad VAO: %d\n", default_quad_vao);

	return 1;
}

void r_create_camera(r_camera* cam, v2 size, v2 position){
	m4_identity(&cam->view);	
	m4_identity(&cam->proj);

	m4_ortho(&cam->proj, 0, size.x, size.y, 0, -10.f, 10.f);
	m4_translate(&cam->view, position.x, position.y, 0.f);

	cam->pos = (v3){position.x, position.y, 0.f};
	cam->size = size;
	cam->near = -10.f;
	cam->far = 10.f;
}

void r_update_camera(r_camera* camera){
	m4_identity(&camera->view);
	m4_identity(&camera->proj);

	m4_ortho(&camera->proj, 0, camera->size.x, camera->size.y, 0, camera->near, camera->far);
	m4_translate(&camera->view, camera->pos.x, camera->pos.y, 0.f);
}

void r_update(long delta){
	for(int i=0;i<drawable_count;++i){
		r_update_drawable(&drawables[i], delta);
	}
	r_update_camera(&g_camera);	
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
	
	/*float verts[16] = {
		//pos           //tex
		-0.5f,  0.5f,   0.f, 1.f
		-0.5f, -0.5f,   0.f, 0.f,
		 0.5f,  0.5f,   1.f, 0.f,
		 0.5f, -0.5f,   1.f, 1.f
	};*/

	float verts[16] = {
	    //pos       //tex
		0.f, 0.f,  0.f, 1.f,
		0.f, 1.f,   0.f, 0.f,
		1.f, 1.f,   1.f, 0.f,
		1.f, 0.f,   1.f, 1.f
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

void r_update_batch(r_shader shader, r_sheet* sheet){
#if defined(RENDER_ENABLE_SHADER_CACHE)
	r_bind_shader(shader);
	r_bind_tex(sheet->id);

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
	for(int i=0;i<drawable_count;++i){
		if(cache->count >= cache->capacity){
			_e("Reached cache limit for shader: %d\n", shader);
			break;
		}

		cache->models[cache->count] = drawables[i].model;
		cache->tex_ids[cache->count] = drawables[i].c_tex;
		cache->count ++;
	}
#endif
}

void r_simple_draw(r_shader shader, r_drawable* draw, r_sheet* sheet){
	if(!shader || !drawable_count) return;
	
	r_bind_shader(shader);

	v2 sub_size = (v2){ sheet->subwidth, sheet->subheight };
	v2 tex_size = (v2){ sheet->width, sheet->height };

	//set texture shit
	//r_set_v2(shader, "sub_size", sub_size);
   	//r_set_v2(shader, "tex_size", tex_size);	
	
	r_set_m4(shader, "proj", g_camera.proj);
	r_set_m4(shader, "view", g_camera.view);
	r_set_m4(shader, "model", draw->model);	

	r_bind_tex(sheet->id);

	//draw instanced
	glBindVertexArray(default_quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

void r_draw_call(r_shader shader, r_sheet* sheet, r_camera* cam){
	if(!shader || !drawable_count) return;
	
#if defined(RENDER_ENABLE_SHADER_CACHE)
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

	r_set_m4x(shader, drawable_count, "models",  cache->models);
  	r_set_ix(shader, drawable_count, "tex_ids", cache->tex_ids);
#endif

	v2 sub_size = (v2){ sheet->subwidth, sheet->subheight };
	v2 tex_size = (v2){ sheet->width, sheet->height };

	//set texture shit
	r_set_v2(shader, "sub_size", sub_size);
   	r_set_v2(shader, "tex_size", tex_size);	
	
	r_set_m4(shader, "proj", cam->proj);
	r_set_m4(shader, "view", cam->view);	
	
	r_bind_tex(sheet->id);

	//draw instanced
	glBindVertexArray(default_quad_vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, drawable_count);

#if defined(RENDER_ENABLE_SHADER_CACHE)
	memset(cache->tex_ids, 0, sizeof(unsigned int) * cache->count);
	memset(cache->models, 0, sizeof(m4) * cache->count);
	cache->count = 0;
#endif
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

int r_is_anim_cache(){
#if defined(RENDER_ENABLE_ANIM_CACHE)
	return 1; 
#else
	return 0;
#endif
}

int r_is_shader_cache(){
#if defined(RENDER_ENABLE_SHADER_CACHE)
	return 1;	
#else
	return 0;
#endif
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

#if defined(RENDER_ENABLE_SHADER_CACHE)
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

#endif

void r_destroy_shader(r_shader shader){
#if defined(RENDER_ENABLE_SHADER_CACHE)
	r_remove_from_cache(shader);
#endif
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

r_anim  r_get_anim(r_sheet* sheet, int* frames, unsigned short frame_count, unsigned char frame_rate){
	return (r_anim){*sheet, frame_count, frames, frame_rate};	
}

#if defined(RENDER_ENABLE_ANIM_CACHE)
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

	g_anim_cache.names[g_anim_cache.count] = name;
	g_anim_cache.count ++;
}

r_anim r_get_anim_n(const char* name){
	if(g_anim_cache.count == 0){
		_l("No animations in cache to check for.\n");
		return (r_anim){0};
	}
	
	for(int i=0;i<g_anim_cache.count;++i){
		if(strcmp(g_anim_cache.names[i], name) == 0){
			return g_anim_cache.anims[i];	
		}
	}

	_l("No animations matching: %s in cache.\n", name);
	return (r_anim){0};
}

#endif

r_animv r_v_anim(r_anim* anim){
	return (r_animv){
		*anim, 0L, 0, R_ANIM_STOP, R_ANIM_STOP, 1	
	};
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

r_drawable* r_get_drawable(r_anim* anim, v2 size, v2 pos){
	r_drawable* draw;

	if(drawable_count < RENDER_BATCH_SIZE){
		draw = &drawables[drawable_count];
		drawable_count ++;
		draw->uid = drawable_uid;
		drawable_uid ++;	
	}
	
	m4_identity(&draw->model);
	
	m4_size_scale(&draw->model, draw->model, size.x * 100.f, size.y * 100.f, 1.f);
	m4_translate(&draw->model, pos.x, pos.y, 0.f);

	draw->size = size;
	draw->position = pos;
	draw->anim = r_v_anim(anim);
	draw->layer = 0;
	draw->visible = 1;
	draw->change = 0;

	return draw;	
}

void r_update_drawable(r_drawable* drawable, long delta){
	//matrix update check
	if(drawable->change){
		m4_identity(&drawable->model);
		m4_size_scale(&drawable->model, drawable->model, drawable->size.x * 100.f, drawable->size.y * 100.f, 1.f);
		m4_translate(&drawable->model, drawable->position.x, drawable->position.y, 0.f);

		//reset bit flag
		drawable->change = 0;
	}

	if(drawable->anim.state == R_ANIM_PLAY && drawable->visible){
		long time = drawable->anim.time;
		r_anim anim = drawable->anim.anim;
		
		long frame_time = anim.frame_rate / MS_PER_SEC;
		if(time + delta >= frame_time){
			int check = time / frame_time;
			//handle frameskip
			if(check > 1){
				//Optimize into one instruction?
				int rounds = check / anim.frame_count;
				int rem = check % anim.frame_count;

				if(rounds){
					//check if viewer is looping
					if(drawable->anim.loop){
						drawable->anim.frame = rem;
					}else{
						drawable->anim.frame = anim.frame_count-1;
						drawable->anim.state = R_ANIM_STOP;
						drawable->anim.pstate = R_ANIM_PLAY;
					}
				}
			}else{
				time = time % frame_time;
				drawable->anim.frame ++;

				if(drawable->anim.frame == anim.frame_count){
					if(drawable->anim.loop){
						drawable->anim.frame = 0;
					}else{
						drawable->anim.state = R_ANIM_STOP;
						drawable->anim.pstate = R_ANIM_PLAY;
					}
				}
			}
		}

		drawable->c_tex = anim.frames[drawable->anim.frame];
	}
}

void r_remove_drawable(unsigned int uid){
	int index = 0;
	for(int i=0;i<drawable_count;++i){
		if(drawables[i].uid == uid){
			index = i;
			break;
		}
	}

	int range = (drawable_count == RENDER_BATCH_SIZE) ? RENDER_BATCH_SIZE - 1 : drawable_count;
	for(int i=index;i<range;++i){
		drawables[i] = drawables[i+1];
	}

	memset(&drawables[range], 0, sizeof(r_drawable));
}

#if defined(RENDER_ENABLE_SHADER_CACHE)
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
#endif

inline int r_get_uniform_loc(r_shader shader, const char* uniform){
#if defined(RENDER_ENABLE_SHADER_CACHE)
	int cache_loc = r_get_cached(shader, uniform);

	if(!cache_loc){
		cache_loc = glGetUniformLocation(shader, uniform);
		r_cache_uniform(shader, uniform, cache_loc);
	}	

	return cache_loc;
#else
	return glGetUniformLocation(shader, uniform);
#endif
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
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
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

    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, g_window.width, g_window.height);

    glfwGetWindowPos(g_window.glfw, &g_window.x, &g_window.y);

    glDisable(GL_CULL_FACE);

    v3 color = r_get_color("EEE");
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
