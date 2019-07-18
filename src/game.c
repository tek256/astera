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

int g_init(){
	sheet  = r_get_sheet("res/tex/test_sheet.png", 16, 16);
	shader = r_get_shader("res/shd/main.v", "res/shd/main.f");
	r_map_shader(shader, "default");
	int frames[5] = { 0, 1, 2, 3, 1};
	anim = r_get_anim(&frames, 5, 24);

	buffer = a_create_buffer("res/snd/test.ogg");
	
	float offset_x = 100.f;
	float offset_y = 100.f;
	for(int i=0;i<16;++i){
		int x = i % 4;
		int y = i / 4;
		r_drawable* d = r_get_drawable(anim, (vec2){16.f, 16.f}, (vec2){(32.f * x) + offset_x, (32.f * y) + offset_y});
		r_anim_p(&d->anim);
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
		r_move_cam(change_x, change_y);
	}

	if(i_key_clicked(GLFW_KEY_ESCAPE)){
		r_request_close();	
	}
}

void g_update(long delta){
	a_update(delta);
	r_update(delta);
	r_update_batch(shader, &sheet);
}

void g_render(long delta){
	r_draw_call(shader, &sheet);
}
