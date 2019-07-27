#ifndef INPUT_C
#define INPUT_C

#include "input.h"
#include "render.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int joy_buttons[MAX_JOY_BUTTONS];
static int joy_buttons_this_frame[MAX_JOY_BUTTONS];
static int joy_buttons_last_frame[MAX_JOY_BUTTONS];

static int   joy_button_count = 0;
static int   joy_this_frame_count = 0;
static int   joy_last_frame_count = 0;

static float joy_axes[MAX_JOY_AXES];
static int   joy_axes_count = 0;

static float joy_axes_last_frame[MAX_JOY_AXES];
static float joy_axes_this_frame[MAX_JOY_AXES];

static int   joy_axes_last_frame_count = 0;
static int   joy_axes_this_frame_count = 0;

static int   joystick_id = 0;
static int   joy_exists = 0;

static int   char_track = 0;

static int   key_count = 0;
static int   char_count = 0;

static int   last_key_count = 0;
static int   concurrent_count = 0;

static int   concurrent_keys[MAX_KEYS] = {0};
static int   this_frame[MAX_KEYS] = {0};
static int   last_frame[MAX_KEYS] = {0};
static char  chars[MAX_CHARS] = {0};

static int   m_count = 0;
static int   m_l_count = 0;

static int   mouse_this_frame[MAX_MOUSE_BUTTONS] = {0};
static int   mouse_last_frame[MAX_MOUSE_BUTTONS] = {0};

typedef struct {
	const char*  name;
	int          value;
	int 		 alt;
	unsigned int type;
	unsigned int alt_type;
} key_binding;

static key_binding* tracked_key_binding = 0;
static key_binding  key_bindings[MAX_KEY_BINDINGS];
static int          key_binding_count = 0;
static int          has_key_bindings = 0;

//mouse
static double lfx = 0;
static double lfy = 0;

static double mx = 0;
static double my = 0;

//deltas
static double mdx = 0;
static double mdy = 0;

//scroll deltas
static double msx = 0;
static double msy = 0;

void i_create_joy(int joy_id){
	if(!joy_exists){
		int present = glfwJoystickPresent(joy_id);
		if(!present){
			return;
		}
		joystick_id = joy_id;
	}
}

float i_joy_axis_delta(int axis){
	if(axis < 0){
		return 0.f;
	}
	return joy_axes_this_frame[axis] - joy_axes_last_frame[axis];
}

void i_get_joy_buttons(int* dst, int count){
	int cpy_count = (count > MAX_JOY_BUTTONS) ? MAX_JOY_BUTTONS : count;
	for(int i=0;i<cpy_count;++i){
		dst[i] = joy_buttons_this_frame[i];
	}	
}

void i_get_joy_axes(float* dst, int count){
	int cpy_count = (count > MAX_JOY_AXES) ? MAX_JOY_AXES : count;
	for(int i=0;i<cpy_count;++i){
		dst[i] = joy_axes[i];
	}
}

const char* i_get_joy_name(int joy){
	return glfwGetJoystickName(joy);
}

int i_get_joy_type(int joy){
	const char* name = i_get_joy_name(joy);
	if(strstr(name, "Microsoft")){
		if(strstr(name, "360")){
			return XBOX_360_PAD;
		}else if(strstr(name, "One")){
			return XBOX_ONE_PAD;	
		}
	}else if(strstr(name, "Sony")){
		if(strstr(name, "3")){
			return PS3_PAD;	
		}else if(strstr(name, "4")){
			return PS4_PAD;
		}
	}else{
		return GENERIC_PAD;
	}		
}

void i_destroy_joy(int joy_id){
	if(joy_id == joystick_id){
		if(joy_exists){
			joy_exists = 0;
		}
	}
}

void i_add_joy_button(int button){
	if(joy_button_count == MAX_JOY_BUTTONS){
		return;
	}

	joy_buttons[joy_button_count] = button;
	joy_button_count ++;
}

void i_rm_joy_button(int button){
	if(!joy_button_count){
		return;
	}

	if(joy_button_count){
		joy_buttons[0] = 0;
		joy_button_count = 0;
		return;
	}

	int index = 0;
	for(int i=0;i<joy_button_count;i++){
		if(joy_buttons[i] == button){
			index = i;
			break;
		}
	}

	for(int i=index;i<joy_button_count-1;i++){
		joy_buttons[i] = joy_buttons[i+1];
	}

	joy_button_count --;
}

float i_joy_axis(int axis){
	if(axis > joy_axes_count){
		return 0.0f;
	}
	return joy_axes[axis];
}

int i_joy_button_down(int button){
	if(button > joy_button_count){
		return 0;
	}
	return joy_buttons[button];
}

int i_joy_button_up(int button){
	if(button > joy_button_count){
		return 1;
	}
	return !joy_buttons[button];
}

int i_joy_button_clicked(int button){
	if(button > joy_last_frame_count){
		return 0;
	}
	return i_joy_button_down(button);
}

int i_joy_button_released(int button){
	if(i_joy_button_down(button)){
		return 0;
	}
	if(button > joy_this_frame_count){
		return 0;
	}

	return joy_buttons_last_frame[button];
}

void i_rm_concurrent_key(int index){
	for(int i=index;i<concurrent_count - 1; i++){
		concurrent_keys[i] = concurrent_keys[i+1];
	}
}

void i_key_callback(int key, int scancode, int toggle){
	if(toggle){
		if(key_binding_track){
			i_binding_track_callback(key, BINDING_KEY);
		}

		if(concurrent_count == MAX_KEYS){
			return;
		}

		for(int i=0;i<concurrent_count;i++){
			if(concurrent_keys[i] == key){
				return;
			}
		}

		concurrent_keys[concurrent_count] = key;
		concurrent_count ++;
	}else{
		int index = 0;
		while(index < concurrent_count){
			if(concurrent_keys[index] == key){
				i_rm_concurrent_key(index);
				concurrent_count --;
			}
			index++;
		}
	}
}

void i_set_char_tracking(int tracking){
	char_track = tracking;
}

void i_char_callback(unsigned int c){
	if(!char_track){
		return;
	}

	chars[char_count] = c;
	char_count ++;
}

char* i_get_chars(){
	if(!char_count){
		return NULL;
	}

	char* chars = calloc(1, char_count);
	for(int i=0;i<char_count;i++){
		chars[i] = chars[i];
		chars[i] = 0;
	}
	char_count = 0;
	return chars;
}

void i_set_mouse_grab(int grab){
	if(grab){
		glfwSetInputMode(g_window.glfw, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}else{
		glfwSetInputMode(g_window.glfw, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

int i_get_mouse_grab(){
	int value = glfwGetInputMode(g_window.glfw, GLFW_CURSOR);

	if(value == GLFW_CURSOR_DISABLED){
		return 1;
	}else if(value == GLFW_CURSOR_NORMAL){
		return 0;
	}else{
		printf("Error: Invalid input mode for GLFW_CURSOR: %i\n", value);
		return 0;
	}
}

void i_mouse_button_callback(int button){
	if(key_binding_track){
		i_binding_track_callback(button, BINDING_MB);
	}
	mouse_this_frame[m_count] = button;
	m_count++;
}

void i_mouse_pos_callback(double x, double y){
	mx = x;
	my = y;
}

void i_mouse_scroll_callback(double sx, double sy){
	msx = sx;
	msy = sy;
}

void i_get_scroll(double* x, double* y){
	*x = msx;
	*y = msy;
}

int i_mouse_down(int button){
	for(int i=0;i<m_count;i++){
		if(mouse_this_frame[i] == button){
			return 1;
		}
	}
	return 0;
}

int i_mouse_up(int button){
	for(int i=0;i<m_count;i++){
		if(mouse_this_frame[i] == button){
			return 0;
		}
	}
	return 1;
}

int i_mouse_clicked(int button){
	for(int i=0;i<m_l_count;i++){
		if(mouse_last_frame[i] == button){
			return 0;
		}
	}

	for(int i=0;i<m_count;i++){
		if(mouse_this_frame[i] == button){
			return 1;
		}
	}
	return 0;
}

int i_mouse_released(int button){
	for(int i=0;i<m_count;i++){
		if(mouse_this_frame[i] == button){
			return 0;
		}
	}

	for(int i=0;i<m_l_count;i++){
		if(mouse_last_frame[i] == button){
			return 1;
		}
	}
	return 0;
}

double getScrollX(){
	return msx;
}

double i_get_scroll_y(){
	return msy;
}

void i_get_mouse_pos(double* x, double* y){
	*x = mx;
	*y = my;
}

double i_get_mouse_x(){
	return mx;
}

double i_get_mouse_y(){
	return my;
}

void i_get_mouse_delta(double* x, double* y){
	*x = mdx;
	_l("Exiting game.");
	*y = mdy;
}

double i_get_delta_x(){
	return mdx;
}

double i_get_delta_y(){
	return mdy;
}

int i_key_down(int key){
	for(int i=0;i<key_count;i++){
		if(this_frame[i] == key){
			return 1;
		}
	}
	return 0;
}

int i_key_up(int key){
	for(int i=0;i<key_count;i++){
		if(this_frame[i] == key){
			return 0;
		}
	}
	return 1;
}

int i_key_clicked(int key){
	for(int i=0;i<last_key_count;i++){
		if(last_frame[i] == key){
			return 0;
		}
	}

	for(int i=0;i<key_count;i++){
		if(this_frame[i] == key){
			return 1;
		}
	}
	return 0;
}

int i_key_released(int key){
	for(int i=0;i<key_count;i++){
		if(this_frame[i] == key){
			return 1;
		}
	}

	for(int i=0;i<last_key_count;i++){
		if(last_frame[i] == key){
			return 1;
		}
	}

	return 0;
}

void i_add_binding(const char* name, int value, int type){
	key_bindings[key_binding_count] = (key_binding){name, value, type};
	key_binding_count ++;
}

void i_add_binding_alt(const char* name, int value, int type){
	for(int i=0;i<key_binding_count;++i){
		if(strcmp(name, key_bindings[i].name) == 0){
			key_bindings[i].alt = value;
			key_bindings[i].alt_type = type;
			return;
		}
	}
}

void i_enable_binding_track(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		if(!strcmp(key_bindings[i].name, key_binding)){
			tracked_key_binding = &key_bindings[i];
			break;
		}
	}

	if(tracked_key_binding != NULL){
		key_binding_track = 1;
	}
}

int i_binding_track(){
	return key_binding_track;
}

void i_binding_track_callback(int value, int type){
	if(tracked_key_binding != NULL){
		tracked_key_binding->value = value;
		tracked_key_binding->type = type;
	}

	key_binding_track = 0;
}

int i_get_binding_type(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		if(!strcmp(key_bindings[i].name, key_binding)){
			return key_bindings[i].type;
		}
	}
	return 0;
}

int i_get_binding_alt_type(const char* key_binding){
	for(int i=0;i<key_binding_count;++i){
		if(!strcmp(key_bindings[i].name, key_binding)){
			return key_bindings[i].alt_type;
		}
	}
	return 0;
}

int i_binding_clicked(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		if(!strcmp(key_bindings[i].name, key_binding)){
			int val;
			switch(key_bindings[i].type){
				case BINDING_JOYB:
					val = i_joy_button_clicked(key_bindings[i].value);
					break;
				case BINDING_KEY:
					val = i_key_clicked(key_bindings[i].value);
					break;
				case BINDING_MB:
					val = i_mouse_clicked(key_bindings[i].value);
					break;
			}

			if(val){
				return 1;
			}

			switch(key_bindings[i].alt_type){
				case BINDING_JOYB:
					val = i_joy_button_clicked(key_bindings[i].alt);
					break;
				case BINDING_KEY:
					val = i_key_clicked(key_bindings[i].alt);
					break;
				case BINDING_MB:
					val = i_mouse_clicked(key_bindings[i].alt);
					break;
			}

			return val;
		}
	}

	return 0;
}

int i_binding_released(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		if(!strcmp(key_bindings[i].name, key_binding)){
			if(key_bindings[i].type == BINDING_JOYA){
				return 0;
			}

			int val;
			switch(key_bindings[i].type){
				case BINDING_JOYB:
					val = i_joy_button_released(key_bindings[i].value);
					break;
				case BINDING_KEY:
					val = i_key_released(key_bindings[i].value);
					break;
				case BINDING_MB:
					val = i_mouse_released(key_bindings[i].value);
					break;
			}

			if(val){
				return 1;
			}

			switch(key_bindings[i].alt_type){
				case BINDING_JOYB:
					val = i_joy_button_released(key_bindings[i].alt);
					break;
				case BINDING_KEY:
					val = i_key_released(key_bindings[i].alt);
					break;
				case BINDING_MB:
					val = i_mouse_released(key_bindings[i].alt);
					break;
			}

			return val;
		}
	}

	return 0;
}

int i_binding_down(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		if(!strcmp(key_bindings[i].name, key_binding)){
			if(key_bindings[i].type == BINDING_JOYA){
				return 0;
			}

			int val;
			switch(key_bindings[i].type){
				case BINDING_JOYB:
					val = i_joy_button_down(key_bindings[i].value);
					break;
				case BINDING_KEY:
					val = i_key_down(key_bindings[i].value);
					break;
				case BINDING_MB:
					val = i_mouse_down(key_bindings[i].value);
					break;
				case BINDING_JOYA:
					val = (i_joy_axis(key_bindings[i].value) < 0.f) ? 1 : 0;
					break;
			}
			if(val){
				return 1;
			}

			switch(key_bindings[i].alt_type){
				case BINDING_JOYB:
					val = i_joy_button_down(key_bindings[i].alt);
					break;
				case BINDING_KEY:
					val = i_key_down(key_bindings[i].alt);
					break;
				case BINDING_MB:
					val = i_mouse_down(key_bindings[i].alt);
					break;
				case BINDING_JOYA:
					val = (i_joy_axis(key_bindings[i].alt) < 0.f) ? 1 : 0;
					break;
			}

			return val;
		}
	}

	return 0;
}

int i_binding_up(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		if(!strcmp(key_bindings[i].name, key_binding)){
			int val;

			switch(key_bindings[i].type){
				case BINDING_JOYB:
					val = i_joy_button_up(key_bindings[i].value);
					break;
				case BINDING_MB:
					val = i_mouse_up(key_bindings[i].value);
					break;
				case BINDING_KEY:
					val = i_key_up(key_bindings[i].value);
					break;
				case BINDING_JOYA:
					val = (i_joy_axis(key_bindings[i].value) > 0.f) ? 1 : 0;
					break;
			}

			if(val){
				return 1;
			}

			switch(key_bindings[i].alt_type){
				case BINDING_JOYB:
					val = i_joy_button_up(key_bindings[i].alt);
					break;
				case BINDING_MB:
					val = i_mouse_up(key_bindings[i].alt);
					break;
				case BINDING_KEY:
					val = i_key_up(key_bindings[i].alt);
					break;
				case BINDING_JOYA:
					val = (i_joy_axis(key_bindings[i].value) > 0.f) ? 1 : 0;
					break;
			}

			return val;
		}
	}
	return 0;
}

float i_binding_val(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		if(!strcmp(key_bindings[i].name, key_binding)){
			float val;
			switch(key_bindings[i].type){
				case BINDING_MB:
					val = (i_mouse_down(key_bindings[i].value)) ? 1.0f : 0.0f;
					break;
				case BINDING_KEY:
					val = (i_key_down(key_bindings[i].value)) ? 1.0f : 0.0f;
					break;
				case BINDING_JOYA:
					val = i_joy_axis(key_bindings[i].value);
					break;
				case BINDING_JOYB:
					val =(i_joy_button_down(key_bindings[i].value)) ? 1.0f : 0.0f;
					break;
			}

			if(val != 0.f){
				return val;
			}

			switch(key_bindings[i].alt_type){
				case BINDING_MB:
					val = (i_mouse_down(key_bindings[i].alt)) ? 1.0f : 0.0f;
					break;
				case BINDING_KEY:
					val = (i_key_down(key_bindings[i].alt)) ? 1.0f : 0.0f;
					break;
				case BINDING_JOYA:
					val = i_joy_axis(key_bindings[i].alt);
					break;
				case BINDING_JOYB:
					val =(i_joy_button_down(key_bindings[i].alt)) ? 1.0f : 0.0f;
					break;
			}

			return val;
		}
	}

	return 0.0f;
}

int i_binding_defined(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		if(!strcmp(key_bindings[i].name, key_binding)){
			return 1;
		}
	}
	return 0;
}

float i_opposing(const char* prim, const char* sec){
	float prim_f = i_binding_down(prim) ? 2.f : 0.f;
	float sec_f = i_binding_down(sec) ? -1.f : 0.f;
	return prim_f + sec_f;
}

void i_default_bindings(){
	if(!i_binding_defined("left")){
		i_add_binding("left", GLFW_KEY_A, BINDING_KEY);
	}

	if(!i_binding_defined("right")){
		i_add_binding("right", GLFW_KEY_D, BINDING_KEY);
	}

	if(!i_binding_defined("up")){
		i_add_binding("up", GLFW_KEY_W, BINDING_KEY);
	}

	if(!i_binding_defined("down")){
		i_add_binding("down", GLFW_KEY_S, BINDING_KEY);
	}

	if(!i_binding_defined("interact")){
		i_add_binding("interact", GLFW_KEY_F, BINDING_KEY);
	}

	if(!i_binding_defined("start")){
		i_add_binding("start", GLFW_KEY_TAB, BINDING_KEY);
	}

	if(!i_binding_defined("jump")){
		i_add_binding("jump", GLFW_KEY_SPACE, BINDING_KEY);
	}
	
	//TODO: add other gamepad type support
	if(joy_exists){
		int type = i_get_joy_type(0);
		switch(type){
			case XBOX_360_PAD:
				i_add_binding("horizontal", XBOX_L_X, BINDING_JOYA);
				i_add_binding("vertical", XBOX_L_Y, BINDING_JOYA);
				i_add_binding_alt("interact", XBOX_X, BINDING_JOYB);
				i_add_binding_alt("start", XBOX_START, BINDING_JOYB);
				i_add_binding_alt("jump", XBOX_A, BINDING_JOYB);
				break;
			case XBOX_ONE_PAD:
				break;
			case PS3_PAD:
				break;
			case PS4_PAD:
				break;
			case GENERIC_PAD:
				break;
		}	
	}
}

void i_update(){
	mdx = mx - lfx;
	mdy = my - lfy;

	lfx = mx;
	lfy = my;

	if(!joy_exists){
		for(int i=0;i<16;++i){
			if(glfwJoystickPresent(i)){
				i_create_joy(i);
				printf("Joystick found\n");
				joy_exists = 1;
				break;
			}
		}
	}


	int continuable = 1;
	for(int i=0;i<MAX_KEYS;i++){
		if(i < key_count){
			last_frame[i] = this_frame[i];
			continuable = 1;
		}else if(i < last_key_count){
			last_frame[i] = 0;
			continuable = 1;
		}

		if(i < concurrent_count){
			this_frame[i] = concurrent_keys[i];
			continuable = 1;
		}else{
			continuable = 0;
		}

		if(!continuable){
			break;
		}
	}
	last_key_count = key_count;
	key_count = concurrent_count;

	for(int i=0;i<MAX_MOUSE_BUTTONS;i++){
		if(i < m_count){
			last_frame[i] = this_frame[i];
			this_frame[i] = 0;
		}else if(i < m_l_count){
			last_frame[i] = 0;
		}else{
			break;
		}
	}

	m_l_count = m_count;
	m_count = 0;

	if(joy_exists){
		int count;
		const float* axes = glfwGetJoystickAxes(joystick_id, &count);

		memset(joy_axes_last_frame, 0, sizeof(float) * joy_axes_last_frame_count);
		memcpy(joy_axes_last_frame, joy_axes_this_frame, sizeof(float) * joy_axes_this_frame_count);
		memcpy(joy_axes_this_frame, axes, sizeof(float) * joy_axes_count);

		joy_axes_last_frame_count = joy_axes_this_frame_count;
		joy_axes_this_frame_count = count;

		for(int i=0;i<MAX_JOY_AXES;i++){
			if(key_binding_track){
				if(axes[i] != 0.0f){
					i_binding_track_callback(i, BINDING_JOYA);
				}
			}

			if(i > count){
				joy_axes[i] = 0.0f;
			}else if(i < count){
				joy_axes[i] = axes[i];
			}
		}

		joy_axes_count = count;

		joy_button_count = 0;

		int button_count;
		const unsigned char* buttons = glfwGetJoystickButtons(joystick_id, &button_count);

		for(int i=0;i<MAX_JOY_BUTTONS;++i){
			if(i < joy_this_frame_count){
				joy_buttons_last_frame[i] = joy_buttons_this_frame[i];
			}else{
				joy_buttons_last_frame[i] = 0;
			}
		}

		joy_last_frame_count = joy_this_frame_count;

		for(int i=0;i<button_count;i++){
			if(key_binding_track){
				if(buttons[i] == GLFW_PRESS || buttons[i] == GLFW_REPEAT){
					i_binding_track_callback(i, BINDING_JOYB);
				}
			}

			if(buttons[i] == GLFW_PRESS || buttons[i] == GLFW_REPEAT){
				joy_buttons[i] = 1;
				joy_button_count ++;
			}else {
				joy_buttons[i] = 0;
			}

			joy_buttons_this_frame[i] = joy_buttons[i];
		}
		joy_this_frame_count = joy_button_count;
	}
}

#endif
