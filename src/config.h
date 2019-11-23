// NOTE: this file is for configuring features, nothing else.

#ifndef CONFIG_H
#define CONFIG_H

// --- DISPLAY CONFIG ---
#define WINDOW_TITLE "demo"

// --- SYSTEM STANDARDS ---
// Configuration INI File path
#define CONF_PATH "res/conf.ini"

// Enable / Disable the resources for level editor usage
//#define CREATE_MODE

// Enable / Disable the initial output from startup
//#define INIT_DEBUG

#define DEBUG_OUTPUT
// Exclude all create mode things
//#define EXCLUDE_CREATE

// Define whether or not to use files or packed files
#define FILE_MODE
//#define PACK_MODE
// Level of compression used for the pack file
#define COMPRESSION_LEVEL 0

#define MAX_THREADS 4

// ---- RENDER CONFIG ----

// uniform types
// float = 3

// animation states

// --- STB CONFIGURATION ---

// STB_IMAGE

// --- SPECIFIC RENDERING LOCATIONS

#define RENDER_USE_LOCATIONS
#define RENDER_FLIP_X 0
#define RENDER_FLIP_Y 1
#define RENDER_MODEL_MAT 2
#define RENDER_TEX_ID 3

#endif
