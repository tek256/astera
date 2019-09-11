#ifndef INPUT_C
#define INPUT_C

#include "input.h"
#include "render.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char  chars[MAX_CHARS];
static u16 char_count = 0;
static char char_track = 0;

static unsigned char joystick_id = 0;
static unsigned char joy_exists = 0;

i_states joy_b;
i_states mouse_b;
i_states keyboard;

u32 screen_width, screen_height;

u16* current_keys;
u16 current_key_count;

i_statesf joy_a;

//current
i_positions mouse_p;
i_positions mouse_l;
//scroll
i_positions mouse_s;

key_binding* key_bindings;
key_binding* tracked_key_binding;
int key_binding_count;

u16 i_init(){
	mouse_b = i_create_s(MAX_MOUSE_BUTTONS);
	if(!mouse_b.curr || !mouse_b.prev){
		_e("Unable to malloc space for mouse.\n");
		i_exit();
		return 0;
	}

	keyboard = i_create_s(MAX_KEYS);
	if(!keyboard.curr || !keyboard.prev){
		_e("Unable to malloc space for keyboard.\n");
		i_exit();
		return 0;
	}

	current_keys = malloc(sizeof(u16) * MAX_KEYS);
	if(!current_keys){
		_e("Unable to malloc space for keyboard.\n");
		i_exit();
		return 0;
	}
	current_key_count = 0;
	
	mouse_p = i_create_p();
	mouse_s = i_create_p();

	key_binding_count = 0;
	tracked_key_binding = 0;
	key_bindings = malloc(sizeof(key_binding) * MAX_KEY_BINDINGS);
	memset(key_bindings, 0, sizeof(key_binding) * MAX_KEY_BINDINGS);
	if(!key_bindings){
		_e("Unable to malloc space for key bindings.\n");
		i_exit();
		return 0;
	}
	return 1;
}

i_positions i_create_p(){
	return (i_positions){ 0, 0, 0, 0};
}

i_states i_create_s(u16 size){
	void* a, *b;
	a = malloc(size * sizeof(u16));
	b = malloc(size * sizeof(u16));

	memset(a, 0, sizeof(u16) * size);
	memset(b, 0, sizeof(u16) * size);

	return (i_states){
		a, b, 0, 0, size
	};
}

i_statesf i_create_sf(u16 size){
	void* a, *b;
	a = malloc(size * sizeof(f32));
	b = malloc(size * sizeof(f32));

	memset(a, 0, sizeof(f32) * size);
	memset(b, 0, sizeof(f32) * size);

	return (i_statesf){
		a, b, 0, 0, size
	};
}

void i_exit(){
	if(mouse_b.curr)
		free(mouse_b.curr);
	if(mouse_b.prev)
		free(mouse_b.prev);
	if(keyboard.curr)
		free(keyboard.curr);
	if(keyboard.prev)
		free(keyboard.prev);
	if(current_keys)
		free(current_keys);

	if(joy_exists){
		if(joy_b.curr)
			free(joy_b.curr);
		if(joy_b.prev)
			free(joy_b.prev);
		if(joy_a.curr)
			free(joy_a.curr);
		if(joy_a.prev)
			free(joy_a.prev);	
	}
}

u16 i_contains(u16 val, u16* arr, int count){
	for(int i=0;i<count;++i){
		if(arr[i] == val){
			return 1;
		}	
	}
	return 0;
}

void i_create_joy(u16 joy_id){
	if(!joy_exists){
		int present = glfwJoystickPresent(joy_id);

		if(!present){
			return;
		}

		joy_a = i_create_sf(MAX_JOY_AXES);
		joy_b = i_create_s(MAX_JOY_BUTTONS);

		joystick_id = joy_id;
	}
}

f32 i_joy_axis_delta(u16 axis){
	if(!joy_exists) return 0;
	return joy_a.curr[axis] - joy_a.prev[axis];
}

void i_get_joy_buttons(u16* dst, int count){
	if(!joy_exists) return;
	int cpy_count = (count > MAX_JOY_BUTTONS) ? MAX_JOY_BUTTONS : count;
	memcpy(dst, joy_b.curr, cpy_count * sizeof(u16));
}

void i_get_joy_axes(f32* dst, int count){
	if(!joy_exists) return;
	int cpy_count = (count > MAX_JOY_AXES) ? MAX_JOY_AXES : count;
	memcpy(dst, joy_a.curr, cpy_count * sizeof(f32));
}

const char* i_get_joy_name(u16 joy){
	return glfwGetJoystickName(joy);
}

u16 i_get_joy_type(u16 joy){	
	if(!glfwJoystickPresent(joy)){
		return -1;
	}

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

void i_destroy_joy(u16 joy_id){
	if(joy_id == joystick_id){
		if(joy_exists){
			joy_exists = 0;
		}
	}
}

f32 i_joy_axis(u16 axis){
	if(!joy_exists) return 0.f;
	return joy_a.curr[axis];
}

u16 i_joy_button_down(u16 button){
	if(!joy_exists) return 0;
	return joy_b.curr[button];
}

u16 i_joy_button_up(u16 button){
	if(!joy_exists) return 0;
	return !joy_b.curr[button];
}

u16 i_joy_button_clicked(u16 button){
	if(!joy_exists) return 0;
	return i_joy_button_down(button) && !joy_b.prev[button];
}

u16 i_joy_button_released(u16 button){
	if(!joy_exists) return 0;
	return !joy_b.curr[button] && joy_b.prev[button];
}

void i_rm_key(u16 key){
	int index = -1;
	for(int i=0;i<current_key_count;++i){
		if(current_keys[i] == key){
			index = i;
			break;
		}
	}

	int cap = (current_key_count >= MAX_KEYS-1) ? MAX_KEYS : current_key_count;

	for(int i=index;i<cap-1;++i){
		current_keys[i] = current_keys[i+1];
	}
	current_keys[cap-1] = 0;
	--current_key_count;
}

void i_key_callback(int key, int scancode, int toggle){
	if(toggle){
		if(key_binding_track){
			i_binding_track_callback(key, BINDING_KEY);
		}

		if(keyboard.curr_count == keyboard.capacity){
			return;
		}

		if(i_contains((u16)key, keyboard.curr, keyboard.curr_count)){
			return;	
		}

		keyboard.curr[keyboard.curr_count] = key;
		++keyboard.curr_count;

		if(i_contains((u16)key, current_keys, current_key_count) && current_key_count < MAX_KEYS){
			return;
		}

		current_keys[current_key_count] = key;
		++current_key_count;
	}else{
		i_rm_key((u16)key);
	}
}

void i_set_screensize(u32 width, u32 height){
	screen_width = width;
	screen_height = height;
}

void i_set_char_tracking(int tracking){
	char_track = tracking;
}

void i_char_callback(u32 c){
	if(!char_track){
		return;
	}

	chars[char_count] = c;
	char_count ++;
}

void i_get_chars(char* dst, u16 count){
	if(!char_count || !count){
		return;
	}
	u16 cpy_count = (count > char_count) ? char_count : count;
	memcpy(dst, chars, sizeof(char) * cpy_count);
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

void i_mouse_button_callback(u16 button){
	if(key_binding_track){
		i_binding_track_callback(button, BINDING_MB);
	}
	mouse_b.curr[mouse_b.curr_count] = button;
	++mouse_b.curr_count;
}

void i_mouse_pos_callback(double x, double y){
	mouse_p.x = x;
	mouse_p.y = y;
}

void i_mouse_scroll_callback(double sx, double sy){
	mouse_s.x += sx;
	mouse_s.y += sy;
	mouse_s.dx = sx;
	mouse_s.dy = sy;
}

void i_get_scroll(double* x, double* y){
	*x = mouse_s.dx;
	*y = mouse_s.dy;
}

u16 i_mouse_down(u16 button){
	return i_contains(button, mouse_b.curr, mouse_b.curr_count);
}

u16 i_mouse_up(u16 button){
	return !i_contains(button, mouse_b.curr, mouse_b.curr_count);
}

u16 i_mouse_clicked(u16 button){
	return i_mouse_down(button) && !i_contains(button, mouse_b.prev, mouse_b.prev_count);
}

u16 i_mouse_released(u16 button){
	return i_mouse_up(button) && !i_contains(button, mouse_b.prev, mouse_b.prev_count);
}

double i_get_scroll_x(){
	return mouse_s.x;
}

double i_get_scroll_y(){
	return mouse_s.y;
}

u16 i_mouse_within_normalized(vec2 min, vec2 max){
	double n_x = mouse_p.x / screen_width;
	double n_y = mouse_p.y / screen_height;

	if(min[0] <= n_x && max[0] >= n_x){
		if(min[1] <= n_y && max[1] >= n_y){
			return 1;
		}
	}
	return 0;	

}

u16 i_mouse_within(vec2 min, vec2 max){
	if(min[0] <= mouse_p.x && max[0] >= mouse_p.x){
		if(min[1] <= mouse_p.y && max[1] >= mouse_p.y){
			return 1;
		}
	}
	return 0;	
}


void i_get_mouse_pos(double* x, double* y){
	*x = mouse_p.x;
	*y = mouse_p.y;
}

double i_get_mouse_x(){
	return mouse_p.x;
}

double i_get_mouse_y(){
	return mouse_p.y;
}

void i_get_mouse_delta(double* x, double* y){
	*x = mouse_p.dx;
	*y = mouse_p.dy;
}

double i_get_delta_x(){
	return mouse_p.dx;
}

double i_get_delta_y(){
	return mouse_p.dy;
}

u16 i_key_down(u16 key){
	return i_contains(key, current_keys, current_key_count);
}

u16 i_key_up(u16 key){
	return !i_contains(key, current_keys, current_key_count);
}

u16 i_key_clicked(u16 key){
	return i_contains(key, keyboard.curr, keyboard.curr_count) && !i_contains(key, keyboard.prev, keyboard.prev_count);
}

u16 i_key_released(u16 key){
	return i_key_up(key) && i_contains(key, keyboard.prev, keyboard.prev_count);
}

void i_add_binding(const char* name, int value, int type){
	if(key_binding_count > 0){
		for(int i=0;i<key_binding_count;++i){
			int len = strlen(key_bindings[i].name);
			len = (len > 8) ? 8 : len;
			if(strncmp(key_bindings[i].name, name, len) == 0){
				key_bindings[i].value = value;
				key_bindings[i].type = type;
				return;
			}	
		}

		if(key_binding_count == MAX_KEY_BINDINGS){
			_l("Unable to add more key bindings.\n");
			return;
		}
	}

	int len = strlen(name);
	len = (len > 8) ? 8 : len;

	strncpy(key_bindings[key_binding_count].name, name, len);
	key_bindings[key_binding_count].value = value;
	key_bindings[key_binding_count].type = type;
	++key_binding_count;
}

void i_add_binding_alt(const char* name, int value, int type){
	for(int i=0;i<key_binding_count;++i){
		int len = strlen(key_bindings[i].name);
		len = (len > 8) ? 8 : len;
		if(strncmp(name, key_bindings[i].name, len) == 0){
			key_bindings[i].alt = value;
			key_bindings[i].alt_type = type;
			return;
		}
	}
}

void i_enable_binding_track(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		int len = strlen(key_bindings[i].name);
		len = (len > 8) ? 8 : len;
		if(!strncmp(key_bindings[i].name, key_binding, 8)){
			tracked_key_binding = &key_bindings[i];
			break;
		}
	}

	if(tracked_key_binding != NULL){
		key_binding_track = 1;
	}
}

u16 i_binding_track(){
	return key_binding_track;
}

void i_binding_track_callback(int value, int type){
	if(tracked_key_binding != NULL){
		tracked_key_binding->value = value;
		tracked_key_binding->type = type;
	}

	key_binding_track = 0;
}

u16 i_get_binding_type(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		int len = strlen(key_bindings[i].name);
		len = (len > 8) ? 8 : len;
		if(!strncmp(key_bindings[i].name, key_binding, 8)){
			return key_bindings[i].type;
		}
	}
	return 0;
}

u16 i_get_binding_alt_type(const char* key_binding){
	for(int i=0;i<key_binding_count;++i){
		int len = strlen(key_bindings[i].name);
		len = (len > 8) ? 8 : len;
		if(!strncmp(key_bindings[i].name, key_binding, 8)){
			return key_bindings[i].alt_type;
		}
	}
	return 0;
}

u16 i_binding_clicked(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		int len = strlen(key_bindings[i].name);
		len = (len > 8) ? 8 : len;
		if(!strncmp(key_bindings[i].name, key_binding, len)){
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

u16 i_binding_released(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		int len = strlen(key_bindings[i].name);
		len = (len > 8) ? 8 : len;
		if(!strncmp(key_bindings[i].name, key_binding, len)){
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

u16 i_binding_down(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		int len = strlen(key_bindings[i].name);
		len = (len > 8) ? 8 : len;
		if(!strncmp(key_bindings[i].name, key_binding, len)){
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

u16 i_binding_up(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		int len = strlen(key_bindings[i].name);
		len = (len > 8) ? 8 : len;
		if(!strncmp(key_bindings[i].name, key_binding, len)){
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

f32 i_binding_val(const char* key_binding){
	for(int i=0;i<key_binding_count;i++){
		int len = strlen(key_bindings[i].name);
		len = (len > 8) ? 8 : len;
		if(!strncmp(key_bindings[i].name, key_binding, len)){
			f32 val;
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

u16 i_binding_defined(const char* key_binding){
	if(!key_binding){
		return 0;
	}

	if(key_binding_count > 0){
		for(int i=0;i<key_binding_count;++i){
			if(key_bindings[i].name){
				int len = strlen(key_bindings[i].name);
				len = (len > 8) ? 8 : len;
				if(!strncmp(key_bindings[i].name, key_binding, len)){
					return 1;
				}
			}
		}
	}
	return 0;
}

f32 i_opposing(const char* prim, const char* sec){
	f32 prim_f = i_binding_down(prim) ? 2.f : 0.f;
	f32 sec_f = i_binding_down(sec) ? -1.f : 0.f;
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

	if(!i_binding_defined("do")){
		i_add_binding("do", GLFW_KEY_F, BINDING_KEY);
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
	mouse_p.dx = mouse_p.x - mouse_l.x;
	mouse_p.dy = mouse_p.y - mouse_l.y;

	mouse_l.x = mouse_p.x;
	mouse_l.y = mouse_p.y;

	if(!joy_exists){
		for(int i=0;i<16;++i){
			if(glfwJoystickPresent(i)){
				i_create_joy(i);
				_l("Joystick [%i] found.\n", i);
				joy_exists = 1;
				break;
			}
		}
	}

	
	int overlap = (keyboard.curr_count > keyboard.prev_count) ? keyboard.curr_count : keyboard.prev_count;
	memset(keyboard.prev, 0, sizeof(u16) * keyboard.prev_count);
	memcpy(keyboard.prev, keyboard.curr, sizeof(u16) *  keyboard.curr_count);
	keyboard.prev_count = keyboard.curr_count;

	memset(keyboard.curr, 0, keyboard.curr_count * sizeof(u16));
	memcpy(keyboard.curr, current_keys, current_key_count * sizeof(u16)); 
	keyboard.curr_count = current_key_count;

	overlap = (mouse_b.curr_count > mouse_b.prev_count) ? mouse_b.curr_count : mouse_b.prev_count;
	memcpy(mouse_b.prev, mouse_b.curr, sizeof(u16) * overlap);
	mouse_b.prev_count = mouse_b.curr_count;
	memset(mouse_b.curr, 0, sizeof(u16) * mouse_b.curr_count); 
	mouse_b.curr_count = 0;
	
	if(joy_exists){
		int count;
		const f32* axes = glfwGetJoystickAxes(joystick_id, &count);

		//Joy axes in variable
		memcpy(joy_a.prev, joy_a.curr, sizeof(f32) * joy_a.curr_count);
		memset(joy_a.curr, 0, sizeof(f32) * joy_a.capacity);
		
		overlap = (count > joy_a.capacity) ? joy_a.capacity : count;
		memcpy(joy_a.curr, axes, sizeof(f32) * overlap);		

		for(int i=0;i<joy_a.capacity;++i){
			if(joy_a.curr[i] != 0.f){
				i_binding_track_callback(i, BINDING_JOYA);
			}
		}

		//Really, not needed just for safety.
		joy_a.prev_count = joy_a.curr_count;
		joy_a.curr_count = count;

		const unsigned char* buttons = glfwGetJoystickButtons(joystick_id, &count);

		overlap = (joy_b.curr_count > joy_b.prev_count) ? joy_b.curr_count : joy_b.prev_count;
		memcpy(joy_b.prev, joy_b.curr, sizeof(u16) * overlap);
		memset(joy_b.curr, 0, sizeof(u16) * joy_b.capacity);
	
		for(int i=0;i<count;++i){
			if(i >= joy_b.capacity){
				break;
			}

			if(buttons[i] == GLFW_PRESS || buttons[i] == GLFW_REPEAT){
				if(key_binding_track){
					i_binding_track_callback(i, BINDING_JOYB);
				}

				joy_b.curr[i] = 1;
			}else{
				joy_b.curr[i] = 0;
			}
		}

		joy_b.prev_count = joy_b.curr_count;
		joy_b.curr_count = count;	
	}
}

#endif
