//NOTE: this file is for configuring features, nothing else.

#ifndef CONFIG_H
#define CONFIG_H

// --- DISPLAY CONFIG ---
#define WINDOW_TITLE "demo"

//Configuration INI File path
//#define CONF_PATH "res/conf.ini"

//Enable / Disable the resources for level editor usage
//#define CREATE_MODE

//Enable / Disable the initial output from startup
//#define INIT_DEBUG

#define DEBUG_OUTPUT

//Define whether or not to use files or packed files
#define FILE_MODE
//#define PACK_MODE
//Level of compression used for the pack file
#define COMPRESSION_LEVEL 0

#define MAX_THREADS 4

//Asset Configurations
#define MAX_ASSET_CACHE 256
#define MIN_ASSET_CACHE 16
#define ASSET_CACHE_GROWTH 16

//Max amount of map_t's
#define ASSET_MAX_MAPS 4

// ---- RENDER CONFIG ----

//max number of quads to draw at once
#define RENDER_BATCH_SIZE 128

//max length of a uniform string
#define SHADER_STR_SIZE 32

//number of animations to cache at max
#define RENDER_ANIM_CACHE 64

//number of shaders to store in map at max
#define RENDER_SHADER_CACHE 2

//number of uniform locations to store
#define RENDER_SHADER_VALUE_CACHE 1024

//max length of an animation in frames
#define RENDER_ANIM_MAX_FRAMES 32

//uniform types
typedef enum {
	r_vec2 = 0,
   	r_vec3,
   	r_vec4,
   	r_float,
   	r_int,
   	r_uint,
   	r_mat
} uniform_type;

//animation states
#define R_ANIM_STOP  0x00
#define R_ANIM_PLAY  0x01
#define R_ANIM_PAUSE 0x10


#endif
