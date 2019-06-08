#include "game.h"
#include "debug.h"
#include "render.h"

void g_init(){
	dbg_log("Starting game.");
}

void g_exit(){
	dbg_log("Exiting game.");
}

void g_input(long delta){
	if(i_key_down('a')){
		dbg_log("AAAAAAAAAAAAAAAAAH\n");
	}

	if(i_key_clicked(GLFW_KEY_ESCAPE)){
		r_request_close();	
	}
}

void g_update(long delta){

}

void g_render(long delta){
	
}
