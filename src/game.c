#include "game.h"
#include "audio.h"
#include "debug.h"
#include "render.h"
#include "input.h"

static a_buf buffer;

static r_camera camera;
static r_shader shader;
static r_sheet sheet;
static r_drawable drawable;

int g_init(){
	camera = r_create_camera((v2){160.f, 90.f}, (v2){0.f, 0.f});
	sheet  = r_get_sheet("res/tex/test_sheet.png", 16, 16);
	shader = r_get_shader("res/shd/main.v", "res/shd/main.f");

	buffer = a_create_buffer("res/snd/test.ogg");

	return 1;	
}

void g_exit(){
	_l("Exiting game.");
}

void g_input(long delta){
	if(i_key_down('A')){
		_l("oh...\n");
	}

	if(i_key_clicked('P')){
		a_play_sfx(&buffer, 1.f, (v2){0.f, 0.f}); 
	}

	if(i_key_clicked(GLFW_KEY_ESCAPE)){
		r_request_close();	
	}
}

void g_update(long delta){	
	a_update(delta);
	r_update(delta);
}

void g_render(long delta){
	r_update_camera(&camera);
}
