#include <linmath.h>

#include "game.h"
#include "audio.h"
#include "debug.h"
#include "render.h"
#include "input.h"

static a_buf buffer;

static r_shader shader;
static r_sheet sheet;
static r_anim anim;
static r_drawable* drawable;

static r_drawable draws[10];

int g_init(){
	sheet  = r_get_sheet("res/tex/icon.png", 16, 16);
	shader = r_get_shader("res/shd/test.v", "res/shd/test.f");
	r_map_shader(shader, "default");

	int frames[5] = {0,1,2,3,1};	
	anim = r_get_anim(&sheet, frames, 5, 24);
	drawable = r_get_drawable(&anim, (vec2){16.f, 16.f}, (vec2){1.f, 1.f});

	buffer = a_create_buffer("res/snd/test.ogg");

	for(int i=0;i<10;++i){
		r_get_drawable(&anim, (vec2){160.f, 160.f}, (vec2){1.f * i, 1.f * i});
	}

	return 1;	
}

void g_exit(){
	_l("Exiting game.");
}

void g_input(long delta){
	if(i_key_clicked('P')){
		a_play_sfx(&buffer, 1.f, (vec2){0.f, 0.f}); 
	}

	float change_x = 0.f; 
	float change_y = 0.f;
	if(i_key_down('D')){
		change_x += delta;
	}else if(i_key_down('A')){
		change_x -= delta;
	}
	
	if(i_key_down('W')){
		change_y += delta;
	}else if(i_key_down('S')){
		change_y -= delta;
	}

	if(change_x != 0 || change_y != 0){
		drawable->position[0] += change_x;
		drawable->position[1] += change_y;
		drawable->change = 1;
	}

	if(i_key_clicked(GLFW_KEY_ESCAPE)){
		r_request_close();	
	}
}

void g_update(long delta){
	a_update(delta);
	r_update(delta);

	if(drawable->change){
		mat4x4_identity(drawable->model);	
		mat4x4_translate(drawable->model, drawable->position[0], drawable->position[1], 0.f);
		mat4x4_scale_aniso(drawable->model, drawable->model, drawable->size[0] * 100.f, drawable->size[1] * 100.f, 1.f);
		
	}
	//r_update_batch(shader, &sheet);
}

void g_render(long delta){
	r_simple_draw(shader, drawable, &sheet);
}
