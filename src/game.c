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
	drawable = r_get_drawable(&anim, (v2){16.f, 16.f}, (v2){1.f, 1.f});

	buffer = a_create_buffer("res/snd/test.ogg");

	for(int i=0;i<10;++i){
		r_get_drawable(&anim, (v2){16.f, 16.f}, (v2){1.f * i, 1.f * i});
	}

	return 1;	
}

void g_exit(){
	_l("Exiting game.");
}

void g_input(long delta){
	if(i_key_clicked('P')){
		a_play_sfx(&buffer, 1.f, (v2){0.f, 0.f}); 
	}

	float change_x = 0.f; 
	float change_y = 0.f;
	if(i_key_down('D')){
		change_x += 6.f * delta;
	}else if(i_key_down('A')){
		change_x -= 6.f * delta;
	}
	
	if(i_key_down('W')){
		change_y += 6.f * delta;
	}else if(i_key_down('S')){
		change_y -= 6.f * delta;
	}

	if(change_x != 0 || change_y != 0){
		drawable->position.x += change_x;
		drawable->position.y += change_y;
		drawable->change = 1;
	}

	if(i_key_clicked(GLFW_KEY_ESCAPE)){
		r_request_close();	
	}
}

void g_update(long delta){
	a_update(delta);
	r_update(delta);
	//r_update_batch(shader, &sheet);
}

void g_render(long delta){
	r_simple_draw(shader, drawable, &sheet);
}
