#include "ui.h"
#include "render.h"

#include <misc/linmath.h>
#include <misc/microui.h>
#include <misc/stb_truetype.h>
#include <string.h>

#define BUFFER_SIZE 16384

static float           ui_tex_buff[BUFFER_SIZE * 8];
static float          ui_vert_buff[BUFFER_SIZE * 8];
static unsigned char ui_color_buff[BUFFER_SIZE * 16];
static float         ui_index_buff[BUFFER_SIZE * 6];
static int           ui_buffer_index;

static r_shader ui_shader;

/*
 *static unsigned char tmp_font[512*512];
r_font r_load_font(const char* name, unsigned char* data, unsigned int length){
	r_font font;
	
	stbtt_BakeFontBitmap(data, 0, 16.0, tmp_font, 512, 512, 32, 96, font.data);
	font.name = name;
	
	glGenTextures(1, &font.tex_id);
	glBindTexture(GL_TEXTURE_2D, font.tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tmp_font);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return font;
}
 */

void ui_init(){
	
}

void ui_start(){
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

void ui_end(){
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void ui_flush(){

}

static void push_quad(mu_Rect dst, mu_Rect tex, mu_Color color){
	if(ui_buffer_index == BUFFER_SIZE) ui_flush();

	int texvert_index = ui_buffer_index * 8;
	int color_index = ui_buffer_index * 16;
	int element_index = ui_buffer_index * 4;
	int index_index = ui_buffer_index * 6;
	++ui_buffer_index;

	ui_tex_buff[texvert_index + 0] = tex.x;
	ui_tex_buff[texvert_index + 1] = tex.y;
	ui_tex_buff[texvert_index + 2] = tex.x + tex.w;
	ui_tex_buff[texvert_index + 3] = tex.y;
	ui_tex_buff[texvert_index + 4] = tex.x;
	ui_tex_buff[texvert_index + 5] = tex.y + tex.h;
	ui_tex_buff[texvert_index + 6] = tex.x + tex.w;
	ui_tex_buff[texvert_index + 7] = tex.y + tex.h;

	ui_vert_buff[texvert_index    ] = dst.x;
	ui_vert_buff[texvert_index + 1] = dst.y;
	ui_vert_buff[texvert_index + 2] = dst.x + dst.w;
	ui_vert_buff[texvert_index + 3] = dst.y;
	ui_vert_buff[texvert_index + 4] = dst.x;
	ui_vert_buff[texvert_index + 5] = dst.y + dst.h;
	ui_vert_buff[texvert_index + 6] = dst.x + dst.w;
	ui_vert_buff[texvert_index + 7] = dst.y + dst.h;

	memcpy(ui_color_buff + color_index +  0, &color, sizeof(unsigned char) * 4);
	memcpy(ui_color_buff + color_index +  4, &color, sizeof(unsigned char) * 4);
	memcpy(ui_color_buff + color_index +  8, &color, sizeof(unsigned char) * 4);
	memcpy(ui_color_buff + color_index + 12, &color, sizeof(unsigned char) * 4);

	ui_index_buff[index_index    ] = element_index + 0;
	ui_index_buff[index_index + 1] = element_index + 1;
	ui_index_buff[index_index + 2] = element_index + 2;
	ui_index_buff[index_index + 3] = element_index + 2;
	ui_index_buff[index_index + 4] = element_index + 3;
	ui_index_buff[index_index + 5] = element_index + 1;
}


