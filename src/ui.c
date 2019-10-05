#include "ui.h"
#include "render.h"

#include <stb/stb_truetype.h>

#include <string.h>

void u_push_frame(mat4x4 proj){
	glBindVertexArray(u_context.quad);
}

void u_render(){
	u_colors_t colors = u_colors;
	for(int i=0;i<u_context.count;++i){
		u_window_t* window = &u_context.windows[i];
		
		//geometry pass
		for(int j=0;j<window->drop_count;++j){
			u_dropdown_t* drop = &window->dropdowns[j];
			if(drop->active){
				u_bounds_t bounds;
				bounds.x = drop->bounds.x;
				bounds.y = drop->bounds.y;
				bounds.width = drop->bounds.width;
				bounds.height = drop->bounds.height * drop->rows;
				bounds.layer += 1;
				u_add_quad(bounds, 0, 0, colors.dropdown_focus_bg);

				int rows_remaining = drop->option_count - drop->selected; 
				int start_row = -1;
				if(rows_remaining < drop->rows){
					start_row = drop->option_count - drop->rows;
				}

				for(int k=0;k<drop->rows;++k){
					u_bounds_t bounds = drop->bounds;
					bounds.layer += 2;
					bounds.y += k * bounds.height;
					u_add_text(bounds, drop->options[k], colors.dropdown_focus_fg);	
				}	
			}else{
				u_add_quad(drop->bounds, 0, 0, colors.dropdown_bg);
				u_add_text(drop->bounds, drop->options[drop->selected], colors.dropdown_fg); 
			}
		}

		for(int j=0;j<window->check_count;++j){
			u_check_t* check = &window->checks[j];
			u_add_quad(check->bounds, 0, 0, (check->state) ? colors.check_fg : colors.check_bg);	
		}

		for(int j=0;j<window->button_count;++j){
			u_button_t* button = &window->buttons[j];
			if(u_context.selected_element == button->uid){
				u_add_quad(button->bounds, 0, 0, colors.button_focus_bg);
				u_add_text(button->bounds, button->text, colors.button_focus_fg);	
			}else{
				u_add_quad(button->bounds, 0, 0, colors.button_bg);
				u_add_text(button->bounds, button->text, colors.button_fg);
			}
		}

		for(int j=0;j<window->box_count;++j){
			u_box_t* box = &window->boxes[j];
			u_add_quad(box->bounds, 0, 0, box->color); 
		}

		for(int j=0;j<window->img_count;++j){
			u_img_t* img = &window->imgs[j];
			u_add_quad(img->bounds, img->min, img->max, 0);			
		}

		for(int j=0;j<window->text_count;++j){
			u_text_t* text = &window->texts[j];
			u_add_text(text->bounds, text->text, colors.text_fg);
		}

		for(int j=0;j<window->slider_count;++j){
			u_slider_t* slider = &window->sliders[j];
			u_bounds_t sub_bounds = slider->bounds;
			float percentage = slider->value / slider->max;
			sub_bounds.width *= percentage;
			sub_bounds.layer += 1;

			if(u_context.selected_element == slider->uid){
				u_add_quad(slider->bounds, 0, 0, colors.slider_focus_bg);
				u_add_quad(sub_bounds, 0, 0, colors.slider_focus_fg);
			}else{
				u_add_quad(slider->bounds, 0, 0, colors.slider_bg);
				u_add_quad(sub_bounds, 0, 0, colors.slider_fg);	
			}
		}
	}	
}

u16 u_init(){
	f32 verts[16] = {
		//pos       //tex
		-0.5f, -0.5f,   0.f, 0.f,
		-0.5f,  0.5f,   0.f, 1.f,
		0.5f,  0.5f,   1.f, 1.f,
		0.5f, -0.5f,   1.f, 0.f
	};

	u16 inds[6] = { 
		0, 1, 2,
		2, 3, 0
	};

	unsigned int vao, vbo, vboi;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &vboi);

	glBindVertexArray(vao);	

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(f32), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(u16), &inds[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	u_context.quad = vao;
}

void u_add_text(u_bounds_t bounds, const char* text, vec3 color){
	r_font font = u_colors.default_font;
	float x, y;
	float x_offset, y_offset;

	x_offset = bounds.x;
	y_offset = bounds.y;
	float last_x = 0;

	while(*text){
		if(*text >= 32 && *text < 128){
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(font.data, 512, 512, *text-32, &x, &y, &q, 1);
				
			vec4 text_bounds;	
			text_bounds[0] = q.x0 + x_offset;
			text_bounds[1] = q.y0 + y_offset;
			text_bounds[2] = q.x1 + x_offset;
			text_bounds[3] = q.y1 + y_offset;
			vec4_dup(u_context.text_bounds[u_context.text_count], text_bounds);

			vec4 text_texcoords;
			text_texcoords[0] = q.s0;
			text_texcoords[1] = q.t0;
			text_texcoords[2] = q.s1;
			text_texcoords[3] = q.t1;
			vec4_dup(u_context.text_texcoords[u_context.text_count], text_texcoords);
			
			vec3_dup(u_context.text_colors[u_context.text_count], color);

			if(x_offset + x > bounds.width){
				y_offset += y;
				x_offset = bounds.x;
			}else{
				x_offset += x;
			}

			last_x = x;

			++u_context.text_count;	
		}else if(*text == ' '){
			x_offset += last_x; 
		}

		++text;
	}	
}

void u_add_quad(u_bounds_t bounds, vec2 sub_size, u32 sub_tex, vec3 color){
	u_context_t* ctx = &u_context;

	vec4 position;
	position[0] = bounds.x;
	position[1] = bounds.y;
	position[2] = bounds.width;
	position[3] = bounds.height;

	vec4 texc;
	texc[0] = sub_size[0];
	texc[1] = sub_size[1];
	texc[2] = sub_tex;
	ctx->texcs;

	if(!color){
		ctx->colors[ctx->quad_count][0] = 1.f;
		ctx->colors[ctx->quad_count][1] = 1.f;
		ctx->colors[ctx->quad_count][2] = 1.f;
	}else{
		vec3_dup(ctx->colors[ctx->quad_count], color);
	}

	++ctx->quad_count;
}

void u_set_color(u_colors_t* colors, const char* str, const char* color){
	if(!strcmp(str, "dropdown_focus_bg")){
		r_get_color(colors->dropdown_focus_bg, color); 
	}else if(!strcmp(str, "dropdown_focus_fg")){
		r_get_color(colors->dropdown_focus_fg, color); 
	}else if(!strcmp(str, "dropdown_bg")){
		r_get_color(colors->dropdown_bg, color); 
	}else if(!strcmp(str, "dropdown_fg")){
		r_get_color(colors->dropdown_fg, color); 
	}else if(!strcmp(str, "window_bg")){
		r_get_color(colors->window_bg, color); 
	}else if(!strcmp(str, "button_focus_bg")){
		r_get_color(colors->button_focus_bg, color); 
	}else if(!strcmp(str, "button_focus_fg")){
		r_get_color(colors->button_focus_fg, color); 
	}else if(!strcmp(str, "button_bg")){
		r_get_color(colors->button_bg, color); 
	}else if(!strcmp(str, "button_fg")){
		r_get_color(colors->button_bg, color); 
	}else if(!strcmp(str, "text_fg")){
		r_get_color(colors->text_fg, color); 
	}else if(!strcmp(str, "slider_bg")){	
		r_get_color(colors->slider_bg, color); 
	}else if(!strcmp(str, "slider_fg")){
		r_get_color(colors->slider_fg, color); 
	}else if(!strcmp(str, "slider_focus_bg")){
		r_get_color(colors->slider_focus_bg, color); 
	}else if(!strcmp(str, "slider_focus_fg")){
		r_get_color(colors->slider_focus_fg, color); 
	}else if(!strcmp(str, "check_bg")){
		r_get_color(colors->check_bg, color);
	}else if(!strcmp(str, "check_focus_bg")){
		r_get_color(colors->check_focus_bg, color);
	}
}

s16 u_check_buffer(void** buffer, u16 element_size, u16* count, u16* capacity, u16 grow){
	if(count == capacity){
		void* new_buffer = malloc(element_size * (*capacity + grow));
		if(!new_buffer){
			return -1;	
		}
		memcpy(new_buffer, *buffer, element_size * *capacity);
		free(*buffer);
		*capacity += grow;	
		*buffer = new_buffer;
		return 1;	
	}
	return 0;
}

u16 u_select_window(u_window_t* window){
	if(!window) return 0;

	u_context.selected_window = window->uid;
	return window->uid;
}

u_window_t* u_get_selected_window(){
	if(u_context.capacity == 0){
		return 0;
	}

	for(int i=0;i<u_context.count;++i){
		if(u_context.windows[i].uid == u_context.selected_window){
			return &u_context.windows[i];
		}
	}

	return 0;
}

u_window_t* u_window(u_context_t* context, u_bounds_t bounds){
	if(!context){
		_e("No u_context.passed.\n");
		return 0;
	}

	if(context->capacity == 0){
		context->windows = (u_window_t*)malloc(sizeof(u_window_t) * 2);
		context->capacity = 2;
	}else if(context->capacity == context->count){
		u_window_t* nu = (u_window_t*)malloc(sizeof(u_window_t) * context->capacity + 2);
		memcpy(nu, context->windows, sizeof(u_window_t) * context->capacity);
		free(context->windows);
		context->windows = nu;
		context->capacity += 2;		
	}

	u_window_t* window = &context->windows[context->count];
	window->bounds = bounds;

	++context->count;

	return window;
}	

u_button_t* u_button(u_bounds_t bounds, const char* text){
	u_window_t* window = u_get_selected_window();
	if(!window) return 0;

	if(u_check_buffer(&window->buttons, sizeof(u_button_t), &window->button_count, &window->button_cap, 2) < 0){
		_e("Uhoh\n");
		return NULL;
	}

	u_button_t* button = &window->buttons[window->button_count];
	++u_context.element_count;
	button->uid = u_context.element_count;
	button->bounds = bounds;
	button->text = text;
	++window->button_count;
	return button;
}

u_text_t* u_text(u_bounds_t bounds, const char* msg){
	u_window_t* window = u_get_selected_window();
	if(!window) return 0;

	if(u_check_buffer(&window->texts, sizeof(u_text_t), &window->text_count, &window->text_cap, 2) < 0){
		_e("Uhoh\n");
		return NULL;
	}

	u_text_t* text = &window->texts[window->text_count];
	++u_context.element_count;
	text->uid = u_context.element_count;
	text->bounds = bounds;
	text->text = msg; 
	++window->text_count;
	return text;
}

u_check_t* u_check(u_bounds_t bounds, u16 state){
	u_window_t* window = u_get_selected_window();
	if(!window) return 0;

	if(u_check_buffer(&window->checks, sizeof(u_check_t), &window->check_count, &window->check_cap, 2) < 0){
		_e("Uhoh\n");
		return NULL;
	}

	u_check_t* check = &window->checks[window->check_count];
	++u_context.element_count;
	check->uid = u_context.element_count;
	check->bounds = bounds;
	check->state = state;
	++window->check_count;
	return check;
}

u_slider_t* u_slider(u_bounds_t bounds, int step, int max, int value){
	u_window_t* window = u_get_selected_window();
	if(!window) return 0;

	if(u_check_buffer(&window->sliders, sizeof(u_slider_t), &window->slider_count, &window->slider_cap, 2) < 0){
		_e("Uhoh\n");
		return NULL;
	}

	u_slider_t* slider = &window->sliders[window->slider_count];
	slider->bounds = bounds;
	++u_context.element_count;
	slider->uid = u_context.element_count;
	++window->check_count;
	return slider;
}

u_dropdown_t* u_dropdown(u_bounds_t bounds, const char** options, u16 option_count, u16 selected_option){
	u_window_t* window = u_get_selected_window();
	if(!window) return 0;

	if(u_check_buffer(&window->dropdowns, sizeof(u_dropdown_t), &window->drop_count, &window->drop_cap, 2) < 0){
		_e("Uhoh\n");
		return NULL;
	}

	u_dropdown_t* drop = &window->dropdowns[window->drop_count];
	++u_context.element_count;
	drop->uid = u_context.element_count;
	drop->bounds = bounds;
	drop->options = options;
	drop->option_count = option_count;
	drop->selected = selected_option;

	++window->drop_count;
	return drop;
}

u_img_t* u_img(u_bounds_t bounds, r_sheet sheet, unsigned int sub_tex){
	u_window_t* window = u_get_selected_window();
	if(!window) return 0;

	if(u_check_buffer(&window->dropdowns, sizeof(u_img_t), &window->img_count, &window->img_cap, 2) < 0){
		_e("Uhoh\n");
		return NULL;
	}

	u_img_t* img = &window->imgs[window->img_count];

	int sub_x, sub_y, per_width;
	vec2 scale;

	per_width = img->sheet.width / img->sheet.subwidth;
	
	sub_x = img->sub_img % per_width;
	sub_y = img->sub_img / per_width;

	scale[0] = img->sheet.subwidth / img->sheet.width;
	scale[1] = img->sheet.subheight / img->sheet.height;

	img->min[0] = scale[0] * sub_x;
	img->min[1] = scale[1] * sub_y;

	img->max[0] = scale[0] * (sub_x + 1);
	img->max[1] = scale[1] * (sub_y + 1);

	++u_context.element_count;
	img->uid = u_context.element_count;

	img->bounds = bounds;
	img->sheet = sheet;
	img->sub_img = sub_tex;	
	
	return img;
}

u_box_t* u_box(u_bounds_t bounds, vec3 color){
	u_window_t* window = u_get_selected_window();
	if(!window) return 0;

	if(u_check_buffer(&window->dropdowns, sizeof(u_box_t), &window->box_count, &window->box_cap, 2) < 0){
		_e("Uhoh\n");
		return NULL;
	}

	u_box_t* box = &window->boxes[window->box_count];
	vec3_dup(box->color, color);

	++u_context.element_count;
	box->uid = u_context.element_count;

	box->bounds = bounds;

	return box;
}
