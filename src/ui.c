#include "ui.h"

#ifdef INC_UI

#include "input.h"
#include <string.h>

static u_ctx _ctx;

//non-zero starting IDs to allow for null pointing
static u16 element_id = 1;
static u_section* active_section;
static u_section* sections;
static u16 section_count = 0;

static u_section_con* con_sec;
static u_row_con* con_row;

static u16 element_events[16];
static u16 event_count;

//create the arrays
void u_init(struct nk_context* handle){
	_ctx = (u_ctx){handle};
	u_set_style();
	memset(element_events, 0, sizeof(u16) * 16);
	event_count = 0;
}

void u_con_start(){
	con_sec = (u_section_con*)malloc(sizeof(u_section_con));
	con_sec->rows = (u_row_t*)malloc(sizeof(u_row_con) * 4);
	con_sec->row_capacity = 4;
	con_sec->focus_linkage = (u_focus_link*)malloc(sizeof(u_focus_link) * 8);
	con_sec->focus_count = 0;
	con_sec->focus_capacity = 8;
	
	con_row = (u_row_con*)malloc(sizeof(u_row_con));
	con_row->ratios = (float*)malloc(sizeof(float) * 8);
	con_row->ratio_capacity = 8;
	con_row->elements = (u_element*)malloc(sizeof(u_element) * 8);
	con_row->element_capacity = 8;
}

void u_con_end(){
	free(con_sec->rows);
	free(con_sec->focus_linkage);
	free(con_sec);
	con_sec = 0;

	free(con_row->ratios);
	free(con_row->elements);
	free(con_row);
	con_row = 0;
}

u16 u_check_add(){
	if(con_row->element_count - con_row->element_capacity == 0){
		u_element* elements =(u_element*)malloc(sizeof(u_element) * (con_row->element_capacity + 4));
		if(!elements){
			return 0;
		}

		memcpy(elements, con_row->elements, sizeof(u_element) * (con_row->element_capacity));
		con_row->element_capacity += 4;
		free(con_row->elements);
		con_row->elements = elements;



		float* ratios = (float*)malloc(sizeof(float) * (con_row->ratio_capacity + 4));
		if(!ratios){
			return 0;
		}

		memcpy(ratios, con_row->ratios, sizeof(float) * con_row->ratio_capacity);
		con_row->ratio_capacity += 4;
		free(con_row->ratios);
		con_row->ratios = ratios;
	}		
	return 1;
}

u16 u_row_add(u_element element){
	if(!u_check_add()){
		return 0;
	}

	u16 index = con_row->element_count;

	con_row->elements[index] = element;
	++con_row->element_count;

	if(con_row->ratios[index] == 0){
		con_row->ratios[index] = -1.f;
	}

	return 1;	
}

void u_set_style(){
	 
}

u16 u_start(){

}

u16 u_end(){
	nk_end(_ctx.nk);
}

u16  u_section_start(const char* name){
	if(!con_sec){
		return 0;
	}	
	u16 sec_id = section_count + 1;
	con_sec->uid = sec_id;
	strncpy(con_sec->name, name, 8);
}

u16 u_section_name(const char* name){
	u16 length = strlen(name);
	strncpy(con_sec->name, name, (length > 8) ? 8 : length);	
}

u16 u_section_end(){
	u_row_t* rows = (u_row_t*)malloc(sizeof(u_row_t) * con_sec->row_count);
	memcpy(rows, con_sec->rows, sizeof(u_row_t) * con_sec->row_count);

	u_focus_link* focus_links = (u_focus_link*)malloc(sizeof(u_focus_link) * con_sec->focus_count);
	memcpy(focus_links, con_sec->focus_linkage, sizeof(u_focus_link) * con_sec->focus_count);
	u_focus_group focus = (u_focus_group){
		focus_links,
		focus_links,
		con_sec->focus_count,
		0
	};

	u_section sec;
	sec.uid = section_count;
	memcpy(sec.name, con_sec->name, sizeof(char) * 8); 
	sec.rows = rows;
	sec.count = con_sec->row_count;
	sec.focus = focus;

	u_section* secs = (u_section*)malloc(sizeof(u_section) * (section_count + 1));
	memcpy(secs, sections, sizeof(u_section) * section_count);
	secs[section_count] = sec;	

	free(sections);
	sections = secs;

	memset(con_sec->focus_linkage, 0, sizeof(u_focus_link) * con_sec->focus_count);
	con_sec->focus_count = 0;
	
	return sec.uid;	
}

void u_draw(){
	nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERT_BUFFER, MAX_ELEMENT_BUFFER); 
}

void u_focus_add(u16 uid, u16 prev, u16 next){
	
}

u16 u_focused(){
	if(active_section){
		if(active_section->focus.selected){
			return active_section->focus.selected->uid;
		}
	}
	return 0;
}

u16 u_button(const char* name, const char* msg){
	u16 uid = element_id;
	++element_id;

	u_element element;
	
	element.column = con_row->element_count;	
	element.type = U_BUTTON;
	element.name = name;
	element.data.button = (u_button_t){msg};
	element.uid = uid;

	return u_row_add(element);	 
}

u16 u_window(const char* name, float x, float y, float w, float h){

}
u16 u_window_t(const char* name, const char* title, float x, float y, float w, float h){

}

u16 u_checkbox(const char* name, const char* label, int value){
	u16 uid = element_id;
	++element_id;

	u_element element;	
	element.column = con_row->element_count;	
	element.type = U_CHECK;
	element.name = name;
	element.data.check = (u_check_t){label, value};
	element.uid = uid;
	return u_row_add(element);	 
}

u16 u_option(const char* name, const char* label, int value){
 	u16 uid = element_id;
	++element_id;

	u_element element;	
	element.column = con_row->element_count;	
	element.type = U_OPTION;
	element.name = name;
	element.data.option = (u_option_t){label, value};
	element.uid = uid;

	return u_row_add(element);	 
}

u16 u_radio(const char* name, const char* label, int* value){
  	u16 uid = element_id;
	++element_id;

	u_element element;	
	element.column = con_row->element_count;	
	element.type = U_RADIO;
	element.name = name;
	element.data.radio = (u_radio_t){label, value};
	element.uid = uid;

	return u_row_add(element);	 
}

void u_text(const char* text, u16 text_length, u16 flags){
  	u16 uid = element_id;
	++element_id;

	u_element element;	
	element.column = con_row->element_count;	
	element.type = U_TEXT;
	element.data.text = (u_text_t){text, text_length, flags};
	element.uid = uid;

	u_row_add(element);
}

void u_color_text(const char* text, u16 text_length, u16 flags, vec3 color){
  	u16 uid = element_id;
	++element_id;

	u_element element;	
	element.column = con_row->element_count;	
	element.type = U_TEXT_COLOR;
	u_text_color_t text_color;
	text_color.str = text;
	text_color.str_len= text_length;
	text_color.flags = flags;
	vec3_dup(text_color.color, color);
	element.data.text_color = text_color;
	
	element.uid = uid;

	u_row_add(element);
}

u16  u_slider(const char* name, float min, float* value, float max, float step){
  	u16 uid = element_id;
	++element_id;

	u_element element;	
	element.column = con_row->element_count;	
	element.type = U_SLIDER;
	element.name = name;
	element.data.slider = (u_slider_t){min, value, max, step};
	element.uid = uid;

	return u_row_add(element);	 
}

u16 u_progress(const char* name, u16* value, u16 end, u16 modifiable){
  	u16 uid = element_id;
	++element_id;

	u_element element;	
	element.column = con_row->element_count;	
	element.type = U_PROGRESS;
	element.name = name;
	element.data.progress= (u_progress_t){value, end, modifiable};
	element.uid = uid;

	return u_row_add(element);	 
}

/* TODO implement if needed
void u_edit_str(char* str_buff, u16* length, u16 max_length, u16 flags){

}*/

void u_row(float height, int columns){
	u_check_add();
}

void u_check_row_add(){
	if(con_sec->row_count == con_sec->row_capacity){
		u_row_t* rows = (u_row_t*)malloc(sizeof(u_row_t) * (con_sec->row_capacity + 4));
		if(!rows){
			_e("Unable to malloc added 4 rows.\n");
			return 0;
		}
		memcpy(rows, con_sec->rows, sizeof(u_row_t)	* con_sec->capacity);
		free(con_sec->rows);
		con_sec->rows = rows;
		con_sec->row_capacity += 4;
	}
	return 1;
}

void u_row_padding(float height){
	if(!u_check_row_add()){
		return;
	}	
	
	u_row_t padding = (u_row_t){1, U_ROW_PADDING, height, NULL, 0, NULL, 0};			
	con_sec->rows[con_sec->row_count] = padding;
	++con_sec->row_count;	
}

void u_row_start(float height, int columns){
	if(!u_check_row_add()){
		return;
	}

	con_row->element_count = 0;
	con_row->ratio_count = 0;

	memset(con_row->ratios, 0, sizeof(float) * con_row->ratio_capacity);
	memset(con_row->elements, 0, sizeof(u_element) * con_row->element_capacity);
}

void u_row_push(float ratio){
	if(!u_add_check()){
		return;	
	}
	con_sec->ratios[con_sec->ratio_count] = ratio;
	++con_sec->ratio_count;
}

void u_row_end(){

}

void u_space(int columns){

}

u16 u_combo_start(const char* text, float width, float height){

}

u16 u_combo_label(const char* text, u16 flags){

}

void u_combo_end(){

}

u16 u_width(){

}

//static u16 element_events[16];
//static u16 event_count;

void u_push_event(u16 uid){
	if(event_count == 16){
		return;
	}

	for(int i=0;i<event_count;++i){
		if(element_events[i] == uid){
			return;
		}
	}

	element_events[event_count] = uid;
	++event_count;
}

u16 u_has_event(u16 uid){
	for(int i=0;i<event_count;++i){
		if(element_events[i] == uid){
			return 1;
		}
	}
	return 0;
}

void u_update(){
	memset(element_events, 0, sizeof(u16) * event_count);
	event_count = 0;
}

void u_draw_section(const char* name){
	u_section* sect;
	for(int i=0;i<section_count;++i){
		if(strncmp(name, sections[i].name, 8) == 0){
			sect = &sections[i];
			break;	
		}
	}

	u16 select_down = i_binding_val("select");
	s16 horizontal = i_binding_val("horizontal");
	s16 vertical = i_binding_val("vertical");

	if(active_section){
		u_focus_group* focus = &active_section->focus;

		if(horizontal != 0 || select_down){
			u_push_event(focus->selected->uid);
		}

		if(vertical != 0){
			if(vertical > 0){
				if(focus->selected->next){
					for(int i=0;i<focus->length;++i){
						if(focus->start[i].uid == focus->selected->next){
							focus->selected = &focus->start[i];
							break;
						}	
					}
				}else if(focus->loop){
					focus->selected = focus->start;
				}	
			}else{
				if(focus->selected->prev){
					for(int i=0;i<focus->length;++i){
						if(focus->start[i].uid == focus->selected->prev){
							focus->selected = &focus->start[i];
							break;
						}	
					}
				}else if(focus->loop){
					focus->selected = focus->start;
				}	
			}	
		}
	}

	for(int i=0;i<sect->count;++i){
		u_row_t* row = &sect->rows[i];
		if(row->standard == U_ROW_STANDARD){
			u_row(row->height, row->columns);
		}else if(row->standard == U_ROW_CUSTOM){
			u_row_start(row->height, row->columns);
		}else if(row->standard == U_ROW_PADDING){
			u_row_padding(row->height);
			continue;
		}

		for(int j=0;j<row->element_count;++j){
			if(j < row->ratio_count && !row->standard){
				u_row_push(row->ratios[j]);
			}
			u_element* element = &row->elements[j];
			switch(element->type){
				case U_TEXT:
					{
					u_text_t data_text = element->data.text;
					nk_text(_ctx.nk, data_text.str, data_text.str_len, data_text.flags); 
					}
					break;
				case U_TEXT_COLOR:
					{
					u_text_color_t data_text_c = element->data.text_color;
					nk_text_colored(_ctx.nk, data_text_c.str, data_text_c.str_len, data_text_c.flags, nk_rgb(data_text_c.color[0] * 255, data_text_c.color[1] * 255, data_text_c.color[2] * 255));
					}
					break;
				case U_BUTTON:
					if(nk_button_label(_ctx.nk, element->data.button.text)){
						u_push_event(element->uid);
					}
					break;
				case U_OPTION:
					if(nk_option_label(_ctx.nk, element->data.option.text, element->data.option.value)){
						u_push_event(element->uid);
					}	
					break;
				case U_RADIO:
					if(nk_radio_label(_ctx.nk, element->data.radio.text, element->data.radio.value)){
						u_push_event(element->uid);
					}
					break;
				case U_CHECK:
					if(nk_check_label(_ctx.nk, element->data.check.text, element->data.check.value)){
						u_push_event(element->uid);	
					}
					break;
				case U_SLIDER:
					if(nk_slider_float(_ctx.nk, element->data.slider.min, element->data.slider.value, element->data.slider.max, element->data.slider.step)){
						u_push_event(element->uid);
					}
					break;
				case U_PROGRESS:
					if(nk_progress(_ctx.nk, element->data.progress.value, element->data.progress.end, element->data.progress.modifiable)){
						u_push_event(element->uid);
					}
					break;
				case U_EDIT_STR:
					//TODO: implement this if needed
					break;
			}
		}	
			
	}	
}
#endif
