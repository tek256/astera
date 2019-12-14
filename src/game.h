#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "sys.h"

int g_init(void);
void g_exit(void);
void g_input(time_s delta);
void g_update(time_s delta);
void g_render(time_s delta);
void g_frame_start();
void g_frame_end();

#endif
