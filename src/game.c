#include <misc/linmath.h>

#include "game.h"

#include "sys.h"
#include "audio.h"
#include "debug.h"
#include "render.h"
#include "input.h"
#include "conf.h"

#include "asset.h"

int g_init(void){
	asset_t* window_icon = asset_get("sys", "res/tex/icon.png");
	r_window_set_icon(window_icon);
	asset_free(window_icon);
	window_icon = 0;	

	return 1;
}

void g_exit(void){

}

void g_input(long delta){
	if(i_key_clicked(GLFW_KEY_ESCAPE)){
		r_window_request_close();	
	}
}

void g_update(long delta){

}

void g_render(long delta){
}
