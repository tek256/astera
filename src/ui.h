#ifndef UI_H
#define UI_H

#include <linmath.h>
#include "config.h"
#include "platform.h"
#include "render.h"

#include <microui/microui.h>

void ui_init();
void ui_start();
void ui_end();
void ui_flush();

static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color);

#endif
