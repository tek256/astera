#define DEBUG_OUTPUT
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>

#include "conf.h"

#include "debug.h"
#include "audio.h"
#include "sys.h"
#include "input.h"
#include "render.h"
#include "game.h"
#include "mem.h"

#define MS_PER_SEC 1000
#define NS_PER_SEC MS_PER_SEC * 1000

int target_fps = 60;
int max_fps = 60;

static m_zone mem_zone;
static c_map conf_map;

int main(int argc, char** argv){
	#ifdef __MINGW32__
		#ifndef DEBUG_OUTPUT
			FreeConsole();
		#endif
	#endif

	dbg_enable_log(1, "log.txt");

	c_parse_args(argc, argv);
	c_alloc_map(&conf_map, 128);
	c_parse_file("test.toml", &conf_map);


	if(!r_init()){
		_fatal("Unable to initialize rendering system.\n");	
	}

	if(!a_init()){
		_fatal("Unable to initialize audio system.\n");
	}

	if(!g_init()){
		_fatal("Unable to initialize game runtime.\n");
	}

	double timeframe = MS_PER_SEC / (double) target_fps;
	double curr = t_get_time();
	double last = curr;
	double check;

	double delta;
	double accum = timeframe;

	while(!r_should_close() && !d_fatal){
		last = curr;
		curr = t_get_time(); 
		delta = curr - last;
		accum = timeframe;

		r_clear_window();
		g_render(delta);
		r_swap_buffers();

		i_update();
		glfwPollEvents();
		g_input(delta);
		
		g_update(delta);

		check = t_get_time(); 
		accum = (long)(check - curr);

		double n_time_frame = timeframe;
		int t_fps;
		int l_fps = target_fps;

		if(accum > 0){
			n_time_frame -= accum;
			t_fps = (int)((double)MS_PER_SEC / n_time_frame);

			if(t_fps > max_fps){
				t_fps = max_fps;
			}else if(t_fps > 0){
				target_fps = t_fps;
			}

			timeframe = (double)(MS_PER_SEC / (double)(target_fps));

			struct timespec sleep_req, sleep_rem;
			sleep_req.tv_sec = 0;
			sleep_req.tv_nsec = accum * NS_PER_MS;

			nanosleep(&sleep_req, &sleep_rem);
		}else{
			n_time_frame += accum;
			t_fps = (int)((double)MS_PER_SEC / n_time_frame);

			if(t_fps < max_fps){
				target_fps = max_fps;
			}else if(t_fps < 0){
				target_fps = 1;
			}else{
				target_fps = t_fps;
			}

			timeframe = (double)(MS_PER_SEC / (double)(target_fps));
		}
	}

	g_exit();

	r_exit();
	a_exit();
	
	if(d_fatal){
		dbg_post_to_err();
	}else{
		dbg_cleanup();
	}

	return EXIT_SUCCESS;
}
