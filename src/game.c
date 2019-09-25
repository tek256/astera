#include <linmath.h>

#include "game.h"

#include "sys.h"
#include "audio.h"
#include "debug.h"
#include "render.h"
#include "input.h"
#include "conf.h"

#define U_ENTER_CON_ON_INIT
#include "ui.h"

#include <time.h>

static a_buf buffer;
static a_music* music;

static r_shader shader;
static r_sheet sheet;
static r_anim anim;

static int dir_x[16];
static int dir_y[16];

static int ui_active = 1;
static int ui_state = 0;

static g_configer confer;

int g_init(){
	sheet  = r_get_sheet("res/tex/test_sheet.png", 16, 16);
	shader = r_get_shader("res/shd/main.v", "res/shd/main.f");
	r_map_shader(shader, "default");
	unsigned int frames[5] = { 4, 5, 6, 7, 4};
	unsigned int frames2[5] = { 0, 1, 2, 3, 1};

	time_t t;
	srand((unsigned) time(&t));

	anim = r_get_anim(sheet, frames, 5, 24);
	r_anim anim2 = r_get_anim(sheet, frames2, 5, 24);

	r_cache_anim(anim, "test");
	r_cache_anim(anim2, "test2");

	r_anim* _anim = r_get_anim_n("test");

	//buffer = a_create_buffer("res/snd/test.ogg");

	int data_len;
	unsigned char* data = c_get_file_contents("res/snd/test.ogg", &data_len);

	if(!data){
		_l("No data loaded for music file.\n");
	}

	//buffer = a_get_buf(data, data_len);
	//music = a_create_music(data, data_len, NULL);
	//a_play_music(music);

	for(int i=0;i<16;++i){
		dir_x[i] = (rand() % 3) - 1;
		dir_y[1] = (rand() % 3) - 1;
	}

	float offset_x = 100.f;
	float offset_y = 100.f;
	for(int i=0;i<16;++i){
		int x = i % 4;
		int y = i / 4;
		r_drawable* d = r_get_drawable(_anim, shader, (vec2){16.f, 16.f}, (vec2){(32.f * x) + offset_x, (32.f * y) + offset_y});
		if(!dir_x[i]){
			d->flip_x = 1;	
		}

		if(dir_y[i]){
			d->flip_y = 1;
		}
		r_anim_p(&d->anim);
	}

	i_create_joy(0);
	int type = i_get_joy_type(0);
	if(type == XBOX_360_PAD){
		printf("XBOX FOUND!\n");
	}

	g_set_configer();

	_l("Initialized game.\n");
	return 1;	
}


void g_init_ui(){
	/*u_con_start();


	u_con_end();*/
}

void g_update_ui(){
	
}

void g_set_configer(){
	g_update_configer();
	
	confer.sub_window = G_CONF_DISPLAY;
	confer.vidmode_select = 0;

	confer.master = a_get_vol_master() * 100.f;
	confer.sfx = a_get_vol_sfx() * 100.f;
	confer.music = a_get_vol_music() * 100.f;

	confer.start_time = t_get_time();
}

void g_update_configer(){/*
	r_window_get_size(&confer.current_width, &confer.current_height);
	
	confer.ui_width = 720;
	confer.ui_height = 360;

	//max to size of window
	if(confer.ui_width > confer.current_width) confer.ui_width = confer.current_width;
	if(confer.ui_height > confer.current_height) confer.ui_height = confer.current_height;

	confer.ui_x = (confer.current_width - confer.ui_width) / 2;
	confer.ui_y = (confer.current_height - confer.ui_height) / 2;

	confer.fullscreen = r_is_fullscreen();
	confer.vsync = r_is_vsync();
	confer.borderless = r_is_borderless();

	confer.vidmode_max = r_get_vidmode_count();*/
}

void g_show_configer(g_configer* confer){/*
	static float master_vol = 0.f;
	static float sfx_vol = 0.f;
	static float music_vol = 0.f;
	static char digit_text[6];
	static int digit_text_length = 6;

	if(r_ui_window(confer->ui_x, confer->ui_y, confer->ui_width, confer->ui_height)){
		//header padding
		r_ui_row(16, 1);
		//option row
		r_ui_row_start(25, 4);
		r_ui_row_push(0.1f);
		r_ui_spacing(1);

		r_ui_row_push(0.32f);
		if(confer->sub_window != G_CONF_DISPLAY){
			if(r_ui_button("DISPLAY")){
				confer->sub_window = G_CONF_DISPLAY;
			}
		}else {
			r_ui_text("DISPLAY", 7, TEXT_ALIGN_CENTERED | TEXT_ALIGN_MIDDLE);
		}

		r_ui_row_push(0.32f);
		if(confer->sub_window != G_CONF_AUDIO){
			if(r_ui_button("AUDIO")){
				confer->sub_window = G_CONF_AUDIO;
			}
		}else{
			r_ui_text("AUDIO", 5, TEXT_ALIGN_CENTERED | TEXT_ALIGN_MIDDLE);
		}

		r_ui_row_push(0.32f);
		if(confer->sub_window != G_CONF_GAME){
			if(r_ui_button("GAME")){
				confer->sub_window = G_CONF_GAME;
			}
		}else{
			r_ui_text("GAME", 4, TEXT_ALIGN_CENTERED | TEXT_ALIGN_MIDDLE);
		}
		r_ui_row_end();

		r_ui_row(15, 1);

		switch(confer->sub_window){
			case G_CONF_DISPLAY:
				r_ui_row_start(32, 4);
				r_ui_row_push(0.35f);
				r_ui_spacing(1);

				r_ui_row_push(0.2f);
				if(r_ui_radio("Fullscreen", &confer->fullscreen)){
					_l("Toggling fullscreen to: %i\n", confer->fullscreen);
				}

				r_ui_row_push(0.2f);
				if(r_ui_radio("Borderless", &confer->borderless)) {
					_l("Toggling borderless to: %i\n", confer->borderless);
				}

				r_ui_row_push(0.2f);
				if(r_ui_radio("Vsync", &confer->vsync)){
					_l("Toggling vsync to: %i\n", confer->vsync);
				}

				r_ui_row_end();

				r_ui_row_start(32, 2);
				
				r_ui_row_push(0.35f);
				r_ui_spacing(1);
				r_ui_row_push(0.5f);

				static char resolution_text[32];
				int res_text_length = r_get_videomode_str(resolution_text, confer->vidmode_select);
				//combo box resolution selector
				if(r_ui_combo_start(resolution_text, r_ui_width(), 150)){
					r_ui_row(25, 1);
					for(int i=0;i<confer->vidmode_max;++i){
						int res_text_length = r_get_videomode_str(resolution_text, i);
						if(i != confer->vidmode_select){
							if(r_ui_combo_item_label(resolution_text, TEXT_ALIGN_CENTERED | TEXT_ALIGN_MIDDLE)){
								confer->vidmode_select = i;
							}
						}else{
							r_ui_text(resolution_text, res_text_length, TEXT_ALIGN_CENTERED | TEXT_ALIGN_MIDDLE); 
						}
					}	
					r_ui_combo_end();	
				}

				r_ui_row_end();

				//r_ui_property(resolution_text, 0, &confer.vidmode_select, confer.vidmode_max-1, 1, 1);

				//10px bottom padding	
				r_ui_row(10, 1);
				r_ui_row_start(30.f, 2);
				r_ui_row_push(0.4f);
				r_ui_spacing(1);
				r_ui_row_push(0.2f);
				if(r_ui_button("Apply")){
					r_select_mode(confer->vidmode_select, confer->fullscreen, confer->vsync, confer->borderless);
				}
				r_ui_row_end();

				r_ui_row(30, 1);

				break;
			case G_CONF_AUDIO:
				//digit conversion strings	
				r_ui_row_start(16, 4);
				r_ui_row_push(0.1f);
				r_ui_spacing(1);

				r_ui_row_push(0.1f);
				r_ui_text("MASTER", 6, TEXT_ALIGN_LEFT | TEXT_ALIGN_MIDDLE);

				r_ui_row_push(0.1f);
				memset(digit_text, 0, sizeof(char) * 6);
				digit_text_length = sprintf(digit_text, "%i", (int)confer->master);	
				r_ui_text(digit_text, digit_text_length, TEXT_ALIGN_RIGHT | TEXT_ALIGN_MIDDLE);

				r_ui_row_push(0.4f);
				if(r_ui_slider(0, &confer->master, 100.f, 1.f)){
					a_set_vol_master((unsigned int)confer->master);
					//_l("Updating master volume: %d\n", (unsigned int)confer->master);
				}

				r_ui_row_end();
				r_ui_row_start(16, 4);
				
				r_ui_row_push(0.1f);
				r_ui_spacing(1);
				
				r_ui_row_push(0.1f);
				r_ui_text("SFX", 3, TEXT_ALIGN_LEFT | TEXT_ALIGN_MIDDLE);

				r_ui_row_push(0.1f);
				memset(digit_text, 0, sizeof(char) * 6);
				digit_text_length = sprintf(digit_text, "%i", (int)confer->sfx);
				r_ui_text(digit_text, digit_text_length, TEXT_ALIGN_RIGHT | TEXT_ALIGN_MIDDLE);

				r_ui_row_push(0.4f);
				if(r_ui_slider(0, &confer->sfx, 100.f, 1.f)){
					a_set_vol_sfx((unsigned int)confer->sfx);
				}

				r_ui_row_end();
				r_ui_row_start(16, 4);

				r_ui_row_push(0.1f);
				r_ui_spacing(1);
				
				r_ui_row_push(0.1f);
				r_ui_text("MUSIC", 5, TEXT_ALIGN_LEFT | TEXT_ALIGN_MIDDLE);

				r_ui_row_push(0.1f);
				memset(digit_text, 0, sizeof(char) * 6);
				digit_text_length = sprintf(digit_text, "%i", (int)confer->music);
				r_ui_text(digit_text, digit_text_length, TEXT_ALIGN_RIGHT | TEXT_ALIGN_MIDDLE);

				r_ui_row_push(0.4f);
				if(r_ui_slider(0, &confer->music, 100.f, 1.f)){
					a_set_vol_music((unsigned int)confer->music);	
				}

				break;
			case G_CONF_GAME:
				r_ui_row_start(32, 3);
				r_ui_row_push(0.1f);
				r_ui_spacing(1);
				r_ui_row_push(0.3f);
				double time_difference = t_get_time() - confer->start_time;

				int seconds = ((int)time_difference / (int)MS_PER_SEC) % 60;
			   	int minutes = ((int)time_difference / (int)(MS_PER_SEC * 60)) % 60;
				int hours = ((int)time_difference / (int)(MS_PER_HR * 60 * 60)) % 24;	
				
				static char time_text[32];
				static int time_text_length;
				memset(time_text, 0, sizeof(char) * 16);
				time_text_length = sprintf(time_text, "Time In Game: %.2d:%.2d:%.2d", hours, minutes, seconds);
				r_ui_text(time_text, time_text_length, TEXT_ALIGN_LEFT | TEXT_ALIGN_MIDDLE);
				r_ui_row_end();

				break;
			default:
				break;
		}
	}*/
}

void g_exit(){

}
void g_input(long delta){
	//r_update_ui();
	if(ui_active){
		g_show_configer(&confer);
		//r_ui_end();
	}else{
		if(i_key_clicked('P')){
			a_play_sfx(&buffer, NULL); 
		}

		if(i_key_clicked('R')){
			int r = (rand() % 15) + 1;//non-zero uid
			r_drawable* d = r_get_drawablei(r);
			r_anim* anim = r_get_anim_n("test2");
			r_drawable_set_anim(d, anim);
		}

		if(i_joy_button_down(0)){
			_l("Down!\n");
		}

		float change_x = 0.f; 
		float change_y = 0.f;

		float _x = i_joy_axis_delta(XBOX_L_X);
		float _y = i_joy_axis_delta(XBOX_L_Y);
		float x_axis = i_joy_axis(XBOX_L_X);
		float y_axis = -1.f * i_joy_axis(XBOX_L_Y);

		if(_x != 0 || x_axis >= 0.75f || x_axis <= -0.75f){
			_x = x_axis;
		}

		if(_y != 0 || y_axis >= 0.75f || y_axis <= -0.75f){
			_y = y_axis;
		}

		if(i_key_down('D')){
			change_x += delta;
		}else if(i_key_down('A')){
			change_x -= delta;
		}else if(_x > 0.f){
			change_x += delta * _x;
		}else if(_x < 0.f){
			change_x += delta * _x;
		}

		if(i_key_down('W')){
			change_y += delta;
		}else if(i_key_down('S')){
			change_y -= delta;
		}else if(_y > 0) {
			change_y += delta * _y;
		}else if(_y < 0){
			change_y += delta * _y;
		}

		if(change_x != 0 || change_y != 0){
			r_move_cam(change_x, change_y);
		}
	}

	if(i_key_clicked(GLFW_KEY_TAB)){
		ui_active = (ui_active) ? 0 : 1;
	}

	if(i_key_clicked(GLFW_KEY_ESCAPE)){
		if(ui_active){
			ui_active = 0;
		}else{
			r_request_close();	
		}
	}

}

void g_update(long delta){
	a_update(delta);
	r_update(delta);
	r_update_batch(shader, &sheet);
}

void g_render(long delta){
	for(int i=1;i<17;++i){
		r_drawable* d = r_get_drawablei(i);
		d->position[0] += dir_x[i] * delta * 0.05f;
		d->position[1] += dir_y[i] * delta * 0.05f;
		d->change = 1;		
	}

	r_draw_call(shader, &sheet);
	
	//r_draw_ui();
}
