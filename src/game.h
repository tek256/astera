#ifndef GAME_H
#define GAME_H 

#include <linmath.h>

int g_init(void);
void g_exit(void); 
void g_input(long delta);
void g_update(long delta);
void g_render(long delta);

#endif
