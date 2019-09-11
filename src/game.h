#ifndef GAME_H
#define GAME_H 

#include <linmath.h>

#define G_CONF_DISPLAY 0x01
#define G_CONF_AUDIO   0x10
#define G_CONF_GAME    0x11

typedef struct {
	//if updated
	int is_set;
	//states
	int sub_window;
		
	//rendering
	int vidmode_max, vidmode_select;
	int fullscreen, vsync, borderless;
	
	int current_width, current_height;
	int ui_width, ui_height;
	int ui_x, ui_y;

	//audio
	float master, sfx, music;
	
	//game
	char* save_dir;
} g_configer;

void g_set_configer();
void g_update_configer();
void g_show_configer();

int g_init();
void g_exit(); 
void g_input(long delta);
void g_update(long delta);
void g_render(long delta);

#endif
