// TODO animation loading from file
// TODO Sprite sheet auto loading (pixel bounding)
// TODO Sprite sheet layout from file
// TODO Baked_sheet update/rebuffer from modifications??
// TODO Remove `fix` from shaders since we now have internal padding

#ifndef ASTERA_RENDER_HEADER
#define ASTERA_RENDER_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <GLFW/glfw3.h>

#include <astera/linmath.h>

// For i_ctx definition
#include <astera/input.h>

#include <astera/sys.h>
#include <stdint.h>

#if !defined(ASTERA_RENDER_LAYER_MOD)
#define ASTERA_RENDER_LAYER_MOD 0.01
#endif

typedef struct {
  /* vao - OpenGL Vertex Array object
   * vbo - OpenGL Vertex Buffer Object
   * vboi - OpenGL Vertex Buffer Object Indicies
   * vto - OpenGL Vertex Texture Object (texture coords)
   * width - the width of the quad created
   * height - the height of the quad created
   * use_vto - if using a separate buffer for texcoords (1) or
               not (interleaved) (0) */
  uint32_t vao, vbo, vboi, vto;
  float    width, height;
  int8_t   use_vto;
} r_quad;

typedef struct {
  /* width - the width of the window
   * height - the height of the window
   * min_width - the minimum width of the window
   * min_height - the minimum height of the window
   * max_width - the max width of the window
   * max_height - the max height of the window */
  uint32_t width, height;
  uint32_t min_width, min_height;
  uint32_t max_width, max_height;
  /* x - the x position of the window (0 if fullscreen)
   * y - the y poisition of the window (0 if fullscreen)
   * resizable - if the window is able to be resized
   * fullscreen - if the window should be drawn as fullscreen
   * vsync - if the window should use vsync (1), double (2), or none (0)
   * borderless - if the window should render without a border (decorations) */
  int32_t x, y;
  int8_t  resizable, fullscreen, vsync, borderless;
  /* refresh_rate - the refresh rate of the window (only matters if fullscreen)
   * gamma - the gamma set for the window
   * title - the title of the window */
  uint16_t    refresh_rate;
  float       gamma;
  const char* title;
} r_window_params;

typedef struct {
  /* params - the parameters for the window
   * close_requested - if the window is being requested to close
   * glfw - the glfw handle for the window */
  r_window_params params;
  int8_t          close_requested;
  GLFWwindow*     glfw;
} r_window;

// Just for sanity's sake
typedef uint32_t r_shader;

typedef struct {
  /* fbo - the OpenGL Framebuffer Object handle
   * tex - the OpenGL Texture handle (for fbo)
   * rbo - the OpenGL Renderubffer handle (for fbo)
   * width - the width of the framebuffer's texture
   * height - the height of the framebuffer's texture */
  uint32_t fbo, tex, rbo;
  uint32_t width, height;
  /* vao - the vertex array object of the framebuffer's quad
   * vbo - the vertex data for the framebuffer's quad
   * shader - the shader program for the framebuffer */
  uint32_t vao, vbo, vboi;
  r_shader shader;
  /* model - the model matrix for the fbo's quad */
  mat4x4 model;
} r_framebuffer;

// Note: This is a basic orthographic camera
typedef struct {
  /* position - the position of the camera
   * size - the size of the camera */
  vec3 position;
  vec2 size;
  /* rotation - the rotation of the camera (z axis) */
  float rotation;

  /* view - the calculated view matrix of the camera
   * projection - the calculated projection matrix of the camera */
  mat4x4 view;
  mat4x4 projection;

  /* near - the closest to render to the camera (can be negative)
   * far - the furthest to render to the camera
   *
   * NOTE: the amount of layers available is determined by the difference of far
   * and near. You can calculate it by dividing the difference of far & near by
   * whatever ASTERA_RENDER_LAYER_MOD is set to. Keep in mind these are
   * considered single precision floats and ample range should be given between
   * values */
  float near;
  float far;
} r_camera;

typedef struct {
  /* id - the OpenGL handle for the texture
   * width - the width of the texture
   * height - the height of the texture */
  uint32_t id;
  uint32_t width, height;
} r_tex;

typedef struct {
  /* sub_id - the ID in the sheet's array
   * x - the x value in pixels of the texture
   * y - the y value in pixels of the texture
   * width - the width in pixels of the sub texture
   * height - the height in pixels of the sub texture*/
  uint32_t sub_id;
  uint32_t x, y, width, height;

  /* coords - the min max values of the sub texture
   *          [min_x, min_y, max_x, max_y] */
  vec4 coords;
} r_subtex;

typedef struct {
  /* id - the OpenGL ID for the original texture
   * width - the width in pixels of the image
   * height - the height in pixels of the image */
  uint32_t id;
  uint32_t width, height;

  /* subtexs - the array of defined sub textures
   * count - the amount of sub textures in the array
   * capacity - the capacity (length) of the sub textures array allocated */
  r_subtex* subtexs;
  uint32_t  count, capacity;
} r_sheet;

typedef struct {
  /* x - the x offset in relative worldspace
   * y - the y offset in relative worldspace
   * width - the width of the quad
   * height - the height of the quad */
  float x, y;
  float width, height;

  /* subtex - the sub texture to use
   * layer - the layer (z index) to place the quad
   * flip_x - flips the texture along the x axis (1 = on, 0 = off)
   * flip_y - flips the texture along the y axis (1 = on, 0 = off) */
  uint32_t subtex;
  uint8_t  layer;
  uint8_t  flip_x;
  uint8_t  flip_y;
} r_baked_quad;

typedef struct {
  /* vao - the OpenGL Vertex Array handle
   * vbo - the OpenGL Vertex Buffer handle
   * vto - the OpenGL Texcoord Buffer handle
   * vboi - the OpenGL Vertex Index buffer handle */
  uint32_t vao, vbo, vto, vboi;
  /* quad_count - the number of quads contained in the OpenGL Buffers */
  uint32_t quad_count;
  /* sheet - a pointer to the texture sheet used */
  r_sheet* sheet;

  /* position - the position to offset everything
   * size - the size in world units of the sheet
   * scale - the amount to scale the sheet*/
  vec2 position, size, scale;
  /* model - just the OpenGL Model Matrix to render with */
  mat4x4 model;
} r_baked_sheet;

/* I think this is relatively self explanatory */
typedef enum {
  R_ANIM_STOP  = 0,
  R_ANIM_PLAY  = 1,
  R_ANIM_PAUSE = 2
} r_anim_state;

// NOTE: Animation IDs will be non-zero if valid
typedef struct {
  // id - the position in r_ctx's cache of this animation
  // NOTE: It will not be changed after it's assigned
  uint32_t id;

  // frames - each individual sub texture (frame)
  uint32_t* frames;
  // time - the internal timer
  float time;

  // curr - current index of frame
  // count - number of frames
  // rate - the amount of frames per second
  // sheet - the texture sheet the animation uses
  uint32_t curr, count, rate;
  r_sheet* sheet;

  // state - playing, pause, stop
  // pstate - previous state
  uint8_t state, pstate;

  // loop - 1 = yes, 0 = no
  int8_t loop;
} r_anim;

typedef struct {
  /* position - the position of the sprite in world units
   * size - the size of the sprite in world units */
  vec2 position, size;

  /* shader - the shader program to draw the sprite with
   * sheet - the sheet to use for the sprite */
  r_shader shader;
  r_sheet* sheet;
  union {
    r_anim   anim;
    uint32_t tex;
  } render;

  vec4 color;

  uint8_t layer;
  int8_t  flip_x, flip_y;
  mat4x4  model;

  int change : 1;
  int animated : 1;
  int visible : 1;
} r_sprite;

typedef struct {
  r_shader shader;
  r_sheet* sheet;

  // Uniform Arrays
  mat4x4*  mats;
  uint8_t* flip_x;
  uint8_t* flip_y;
  vec4*    colors;
  vec4*    coords;

  uint32_t count, capacity;
} r_batch;

typedef struct {
  float   life;
  float   rotation;
  vec2    position, size, velocity, direction;
  uint8_t layer;

  uint32_t frame;
  vec4     color;
} r_particle;

typedef enum {
  PARTICLE_COLORED,
  PARTICLE_TEXTURED,
  PARTICLE_ANIMATED
} r_particle_type;

typedef struct r_particles r_particles;

typedef void (*r_particle_animator)(r_particles*, r_particle*);
typedef void (*r_particle_spawner)(r_particles*, r_particle*);

struct r_particles {
  // list - the array of particles
  r_particle* list;

  // capacity - the max amount of particles to buffer for
  // count - the amount of particles within the system currently
  // max_emission - the max amount of particles to emit (0 = infinite)
  // emission_count - the amount of particles emitted
  uint32_t capacity, count;
  uint32_t max_emission;
  uint32_t emission_count;

  // particle_layer - the base layer to set a particle to
  uint8_t particle_layer;

  // particle_life - the lifetime of the particle
  // system_life - the lifetype of the system (0 = infinite)
  // spawn_rate - the amount of particles to spawn per second
  float particle_life, system_life;
  float spawn_rate;

  // time - the internal timer of the particle system
  // spawn_time - the time remaining to next particle spawn
  float time, spawn_time;

  // position - The center position of the particle system
  // size - the size (width, height) of the particle system
  vec2 position, size;
  // particle_size - the size of particles (width, height)
  // particle_velocity - the default velocity of particles
  vec2 particle_size, particle_velocity;

  // sheet - the texture sheet to use (note: only needed for PARTICLE_TEXTURED)
  r_sheet* sheet;
  union {
    r_anim   anim;
    uint32_t subtex;
  } render;

  // Overall color of the system
  vec4 color;

  // GL Uniforms
  mat4x4*  mats;
  vec4*    colors;
  vec4*    coords;
  uint32_t uniform_count, uniform_cap;

  // For custom movement
  r_particle_animator animator_func;
  r_particle_spawner  spawner_func;

  // Flags
  // Calculate -- Whether to calculate data for render
  // Type -- Render type colored (yes | no) && (textured | animated)
  // Note: (Can color both other types)
  // Use Animator -- Whether to use custom animator
  //    function of type `void xxx(r_particles*, r_particle*)`
  // Use Spawner -- Whether to use custom spawning function
  //    of type `void xxx(r_particles*, r_particle* particle)
  int8_t calculate, type, use_animator, use_spawner;
};

typedef struct r_ctx r_ctx;

/* Create a basic version of the window params structure for context creation
 * width - the width of the window
 * height - the height of the window
 * resizable - if the window should be resizable (0 = no, 1 = yes)
 * fullscreen - if the window should be fullscreen (0 = no, 1 = yes)
 * vsync = if the window should use vsync (0 = no, 1 = yes)
 * borderless = if the window should be borderless (0 = no, 1 = yes)
 * refresh_rate = the refresh rate the should be used (if fullscreen)
 * title - the title of the window
 * returns: formatted r_window_params struct */
r_window_params r_window_params_create(uint32_t width, uint32_t height,
                                       uint8_t resizable, uint8_t fullscreen,
                                       uint8_t vsync, uint8_t borderless,
                                       uint16_t    refresh_rate,
                                       const char* title);

/* Create the initial render context
 * NOTE: this is needed to render with astera
 *
 * params - the window parameters for the game window
 * use_fbo - to use a framebuffer to render to or not (post-processing)
 * batch_count - the number of batches to create for different draw types
 * batch_size - the max amount of sprites to store in each given batch
 * anim_map_size - the amount of animations to allow to be cached / mapped
 * shader_map_size - the amount of shaders to allow to be cached / mapped */
r_ctx* r_ctx_create(r_window_params params, uint8_t use_fbo,
                    uint32_t batch_count, uint32_t batch_size,
                    uint32_t anim_map_size, uint32_t shader_map_size);

/* Get the current set camera for the context */
r_camera* r_ctx_get_camera(r_ctx* ctx);

/* Make a specific context the primary context used for callbacks */
void r_ctx_make_current(r_ctx* ctx);

/* Set the input context for callbacks
 * ctx - the render context to set input callback for
 * input - the input context to set */
void r_ctx_set_i_ctx(r_ctx* ctx, i_ctx* input);

/* Set the shader that the context's framebuffer will be rendered with
 * ctx - the context to modify
 * shader - the shader to use
 * TODO: Document fbo shader layout */
void r_ctx_set_fbo_shader(r_ctx* ctx, r_shader shader);

/* Free all resources related to a specific context */
void r_ctx_destroy(r_ctx* ctx);

// TODO probably a better description of this
/* Update the context */
void r_ctx_update(r_ctx* ctx);

/* Call for the context to draw it's contents */
void r_ctx_draw(r_ctx* ctx);

/* Check if OpenGL has thrown an error */
uint32_t r_check_error(void);

/* Check if OpenGL has thrown and error, if it has, print
 * out the loc message & error code
 * loc - location to output if an error has occured */
uint32_t r_check_error_loc(const char* loc);

/* Create a quad in OpenGL
 * width - the width of the quad in units
 * height - the height of the quad in units
 * use_vto - if to use a separate buffer for texture coordinates
 *            aka (0 = interleaved, 1 = separate buffers) */
r_quad r_quad_create(float width, float height, uint8_t use_vto);

/* Issue a simple draw call for a quad
 * quad - the quad to issue a draw call for */
void r_quad_draw(r_quad quad);

/* Issue an instanced draw call for a quad
 * quad - the base quad to draw (usually unit quad)
 * count - the amount of times to draw that quad */
void r_quad_draw_instanced(r_quad quad, uint32_t count);
/* Delete the OpenGL buffers for a quad
 * quad - the quad to delete */
void r_quad_destroy(r_quad* quad);

/* Parse a string to get the hex color value
 * If # is passed, it'll skip it
 * Works on 3 & 6 character colorcodes
 * i.e FFF = white (255, 255, 255),
 * val - the destination to place the value
 * v - the color value string */
void r_get_color3f(vec3 val, const char* v);

/* Run r_get_color3f and set the alpha to 1.0 */
void r_get_color4f(vec4 val, const char* v);

/* Create an OpenGL Framebuffer & quad to draw it on
 * width - the width in pixels
 * height - the height in pixels
 * shader - the shader program to render it with */
r_framebuffer r_framebuffer_create(uint32_t width, uint32_t height,
                                   r_shader shader);

/* Destroy the OpenGL Framebuffer and it's quad (the shader is unaffected)
 * fbo - the framebuffer to destroy */
void r_framebuffer_destroy(r_framebuffer fbo);

/* Bind a framebuffer for OpenGL to draw to
 * fbo - the framebuffer to bind */
void r_framebuffer_bind(r_framebuffer fbo);

/* Draw a framebuffer to it's quad
 * ctx - the context to get the gamma parameter from
 * fbo - the framebuffer to draw */
void r_framebuffer_draw(r_ctx* ctx, r_framebuffer fbo);

/* Create an OpenGL Width data
 * data - the unformatted raw data of the texture file
 * length - the length of the image data */
r_tex r_tex_create(unsigned char* data, uint32_t length);

/* Free the OpenGL Texture buffer passed
 * tex - the texture to destroy */
void r_tex_destroy(r_tex* tex);

/* Bind the OpenGL Texture buffer passed
 * tex - the texture ID to bind */
void r_tex_bind(uint32_t tex);

/* Create a texture sheet with predetermined subsprites
 * data - the image data
 * length - the length of the image data
 * sub_sprites - an array of the bounding boxes in pixels for each sub sprite
 * subsprite_count - the number of subsprites in the sub_sprites array */
r_sheet r_sheet_create(unsigned char* data, uint32_t length, vec4* sub_sprites,
                       uint32_t subsprite_count);

/* Automatically create sprites by automatically detecting pixel bounds
 * NOTE: Padding between subsprites is highly recommended for this, also this
 * method isn't provento work in higher resolutions
 *
 * data - the image data
 * length - the length of the image data
 * tollerance - the amount of pixels you can go before breaking a subsprite
 *
 * NOTE: CURRENTLY NOT WORKING, TO BE IMPLEMENTED */
r_sheet r_sheet_create_auto(unsigned char* data, uint32_t length,
                            uint32_t tollerance);

/* Automatically create sprites based on a grid size
 * data - the image data
 * length - the length of the image data
 * sub_width - the width of the subsprite
 * sub_height - the height of the subsprite
 * width_pad - the internal padding between sprites on each X axis side
 * height_pad - the internal padding between sprites on each Y axis side */
r_sheet r_sheet_create_tiled(unsigned char* data, uint32_t length,
                             uint32_t sub_width, uint32_t sub_height,
                             uint32_t width_pad, uint32_t height_pad);

/* Destroy a texture sheet's OpenGL Buffer & free it's subsprite contents
 *
 * sheet - the sheet to destroy */
void r_sheet_destroy(r_sheet* sheet);

/* Create a baked sheet (series of quads) to render
 *
 * sheet - the texture sheet you want to use
 * quads - the quads you want to put within the baked_sheet
 * quad_count - the number of quads
 * position - the offset of the baked sheet (top-left)
 * layer - the layer (z index) of the baked_sheet overall
 * NOTE: After initialization, you're able to free the `quads` array */
r_baked_sheet r_baked_sheet_create(r_sheet* sheet, r_baked_quad* quads,
                                   uint32_t quad_count, vec2 position);
/* Draw the baked sheet
 *
 * shader - the shader to use
 * sheet - the baked sheet to draw
 * TODO add uniform spec for drawing  */
void r_baked_sheet_draw(r_ctx* ctx, r_shader shader, r_baked_sheet* sheet);

/* Destroy a baked sheet
 * NOTE: This will not destroy shaders & textures,
 *       just the baked sheet's vertex data */
void r_baked_sheet_destroy(r_baked_sheet* sheet);

/* Create a particle system
 * emit_rate - the amount of particles to emit per second
 * particle_capacity - the maximum amount of particles alive at any given
 * moment emit_count - the max amount of particles to emit (0 = infinite)
 * particle_type - reference r_particle_type
 *                 (i.e PARTICLE_COLORED |  PARTICLE_ANIMATED)
 * calculate - whether or not to automatically calculate uniform arrays
 *             NOTE: You should only disable this if you want to use your own
 *                   methods of rendering  */
r_particles r_particles_create(uint32_t emit_rate, float particle_life,
                               uint32_t particle_capacity, uint32_t emit_count,
                               int8_t particle_type, int8_t calculate,
                               uint16_t uniform_cap);

/* Set particle system variables related to individual particles
 * NOTE: vectors can be passed as 0/NULL, and they won't be set
 *
 * system - the particle system to affect
 * color - the color to set particles by default
 * particle_life - the duration of each particle's lifespan in milliseconds
 *                 NOTE: if passed 0 the value won't be set
 * particle_size - the size of particles in size unit
 * particle_velocity - the velocity of a particle (units per second) */
void r_particles_set_particle(r_particles* system, vec4 color,
                              float particle_life, vec2 particle_size,
                              vec2 particle_velocity);

/* This function uses the assumed uniforms for rendering,
 * TODO add documentation to this bit */
void r_particles_draw(r_ctx* ctx, r_particles* particles, r_shader shader);

/* Set the function to spawn particles with
 * system - the system to set the spawner
 * spawner - the spawn function to use
 *            (r_particles* partcile, r_particle particle) */
void r_particles_set_spawner(r_particles* system, r_particle_spawner spawner);

/* Set the function to animate particles with
 * system - the system to set the spawner
 * animator - the animation function to use
 *            (r_particles* partcile, r_particle particle) */
void r_particles_set_animator(r_particles*        system,
                              r_particle_animator animator);

/* Remove a spawner function from a particle system
 * system - the system to affect */
void r_particles_remove_spawner(r_particles* system);

/* Remove an animator function from a particle system
 * system - the system to affect */
void r_particles_remove_animator(r_particles* system);

/* Update the simulation of the particles */
void r_particles_update(r_particles* system, time_s delta);

/* Destroy all resources for the particles
 * NOTE: This will not destroy the textures / anims & shaders used */
void r_particles_destroy(r_particles* particles);

/* Set a particle system's default particle animation
 *
 * particles - the particle system to affect
 * anim - the animation to set as default */
void r_particles_set_anim(r_particles* particles, r_anim anim);

/* Set a particle system's default sub texture
 *
 * particles - the particle system to affect
 * sheet - the texture sheet to use
 * subtex - the subtex to set as default */
void r_particles_set_subtex(r_particles* particles, r_sheet* sheet,
                            uint32_t subtex);

/* Create an animation
 * sheet - the sheet to use for the animation
 * frames - the IDs of each subtex (frame)
 * count - the amount of frames
 * rate - the amount of frames to be played per second */
// TODO check / test if we want fractional frame rates
r_anim r_anim_create(r_sheet* sheet, uint32_t* frames, uint32_t count,
                     uint32_t rate);

/* Free all of the contents for animations
 * NOTE: this doesn't affect any texture buffers */
void r_anim_destroy(r_ctx* ctx, r_anim* anim);

/* cache an animation in a context
 * anim - the animation to cache
 * name - a name to cache it with (optional)
 * returns - the pointer to the cached animation */
r_anim* r_anim_cache(r_ctx* ctx, r_anim anim, const char* name);

/* Get an animation from cache by name
 * ctx - the context to search cache for
 * name - the name to search for */
r_anim* r_anim_get_name(r_ctx* ctx, const char* name);

/* Get an animation from cache by index
 * ctx - the context to search cache for
 * id - the index / ID of the animation */
r_anim* r_anim_get(r_ctx* ctx, uint32_t id);

/* Remove an animation from cache by ID
 * ctx - the context to remove the animation from
 * id - the index / ID of the animation
 * Returns - the original pointer of the animation to be freed
 * NOTE: This original pointer is its place in memory, cache is an array of
 *        pointeres */
r_anim r_anim_remove(r_ctx* ctx, uint32_t id);

/* Remove an animation from cache by name
 * ctx - the context to remove the animation from
 * name - the name of the animation
 * Returns - the original pointer of the animation to be freed
 * NOTE: This animation value that has been removed */
r_anim r_anim_remove_name(r_ctx* ctx, const char* name);

/* Set an animation's state to play */
void r_anim_play(r_anim* anim);

/* Set an animation's state to stopped */
void r_anim_stop(r_anim* anim);

/* Set an animation's state to paused */
void r_anim_pause(r_anim* anim);

/* Reset an animation's state & time */
void r_anim_reset(r_anim* anim);

/* Create a sprite to draw
 * shader - the shader program to draw it with
 * pos - the position of the sprite
 * size - the size of the sprite in units */
r_sprite r_sprite_create(r_shader shader, vec2 pos, vec2 size);

/* Create a subtexture in a sheet based on a tilesheet  */
r_subtex* r_subtex_create_tiled(r_sheet* sheet, uint32_t id, uint32_t width,
                                uint32_t height, uint32_t width_pad,
                                uint32_t height_pad);
/* Create a subtexture based around pixel coordinates
 * sheet - the sheet to use
 * x - the x offset in pixels
 * y - the y offset in pixels
 * width - the width of the sub texture
 * height - the height of the sub texture */
r_subtex* r_subtex_create(r_sheet* sheet, uint32_t x, uint32_t y,
                          uint32_t width, uint32_t height);

/* Set a sprite to draw an animation
 * sprite - the sprite to affect
 * anim - the animation to set it to draw */
void r_sprite_set_anim(r_sprite* sprite, r_anim anim);

/* Set a sprite's texture
 * sprite - the sprite to affect
 * sheet - the texture sheet to use
 * tex - the subtexture ID to use */
void r_sprite_set_tex(r_sprite* sprite, r_sheet* sheet, uint32_t tex);

/* Update a sprite for drawing
 * sprite - the sprite to update
 * delta - the time since last update / frame */
void r_sprite_update(r_sprite* sprite, long delta);

/* Call for a sprite to be drawn in the next batch
 * ctx - the context to draw the sprite in
 * sprite - the sprite to draw */
void r_sprite_draw(r_ctx* ctx, r_sprite* sprite);

/* Get the current state of a sprite's animation
 * sprite - the sprite to check
 * returns: 0 = STOPPED, 1 = PLAY, 2 = PAUSE */
uint8_t r_sprite_get_anim_state(r_sprite* sprite);

/* Call for a sprite to play its animation
 * sprite - the sprite to affect */
void r_sprite_anim_play(r_sprite* sprite);

/* Call for a sprite to pause its animation
 * sprite - the sprite to affect */
void r_sprite_anim_pause(r_sprite* sprite);

/* Call for a sprite to stop its animation
 * sprite - the sprite to affect */
void r_sprite_anim_stop(r_sprite* sprite);

/* Create a camera according to passed parameters
 * position - the offset of the camera in worldspace
 *  NOTE: the Z of this should  be negative since layers start their
 *        Z position at 0 and go towards positive
 * infinity) size - the size of the camera (units) near - the nearest depth to
 * draw (Z depth) far - the furthest depth to draw (Z depth) */
r_camera r_camera_create(vec3 position, vec2 size, float near, float far);

/* Update a camera's view matrix
 * camera - the camera to update */
void r_camera_update(r_camera* camera);

/* Move a camera by distance
 * camera - the camera to move
 * dist - the distance to move the camera */
void r_camera_move(r_camera* camera, vec3 dist);

/* Get the camera's size
 * camera - the camera to get the size of
 * dst - the destination to store the size */
void r_camera_get_size(vec2 dst, r_camera* camera);

/* Set the camera's size
 * camera - the camera to set the size
 * size - the size to set the camera to */
void r_camera_set_size(r_camera* camera, vec2 size);

/* dst is the vector to store the worldpsace to
 * camera is the camera to translate the point from
 * point is the point within the camera [0,1] on each axis */
void r_camera_screen_to_world(vec2 dst, r_camera* camera, vec2 point);

/* dst is the vector to store the screenspace point to
 * (translated to 0...1 scale of camera size)
 * camera is the camera to translate the point from
 * point is the point in worldspace to translate */
void r_camera_screen_to_world(vec2 dst, r_camera* camera, vec2 point);

/* vert - the vertex shader program's data
 * frag - the fragment shader program's data */
r_shader r_shader_create(unsigned char* vert, unsigned char* frag);

/* Get a shader from the context's map by name */
r_shader r_shader_get(r_ctx* ctx, const char* name);

/* Bind the shader in OpenGL
 * NOTE: r_shader is just typedefed uint32_t */
void r_shader_bind(r_shader shader);
/* Destroy the OpenGL Shader & remove it from context */
void r_shader_destroy(r_ctx* ctx, r_shader shader);
/* Add a shader to the context's cache */
void r_shader_cache(r_ctx* ctx, r_shader shader, const char* name);

void r_set_uniformf(r_shader shader, const char* name, float value);
void r_set_uniformi(r_shader shader, const char* name, int value);
void r_set_v4(r_shader shader, const char* name, vec4 value);
void r_set_v3(r_shader shader, const char* name, vec3 value);
void r_set_v2(r_shader shader, const char* name, vec2 value);
void r_set_m4(r_shader shader, const char* name, mat4x4 value);

void r_set_m4x(r_shader shader, uint32_t count, const char* name,
               mat4x4* values);
void r_set_ix(r_shader shader, uint32_t count, const char* name, int* values);
void r_set_fx(r_shader shader, uint32_t count, const char* name, float* values);
void r_set_v2x(r_shader shader, uint32_t count, const char* name, vec2* values);
void r_set_v3x(r_shader shader, uint32_t count, const char* name, vec3* values);
void r_set_v4x(r_shader shader, uint32_t count, const char* name, vec4* values);

void r_window_get_size(r_ctx* ctx, int32_t* w, int32_t* h);

uint8_t r_get_videomode_str(r_ctx* ctx, char* dst, uint8_t index);
uint8_t r_select_mode(r_ctx* ctx, uint8_t index, int8_t fullscreen,
                      int8_t vsync, int8_t borderless);
uint8_t r_get_vidmode_count(r_ctx* ctx);

uint8_t r_can_render(r_ctx* ctx);
void    r_set_can_render(r_ctx* ctx, uint8_t allowed);
uint8_t r_is_vsync(r_ctx* ctx);
uint8_t r_is_fullscreen(r_ctx* ctx);
uint8_t r_is_borderless(r_ctx* ctx);

uint8_t r_window_create(r_ctx* ctx, r_window_params params);
void    r_window_destroy(r_ctx* ctx);
void    r_window_request_close(r_ctx* ctx);
uint8_t r_window_set_icon(r_ctx* ctx, unsigned char* data, uint32_t length);

void r_window_center(r_ctx* ctx);
void r_window_set_pos(r_ctx* ctx, int32_t x, int32_t y);

uint8_t r_window_should_close(r_ctx* ctx);

uint8_t r_window_is_resizable(r_ctx* ctx);
void    r_window_swap_buffers(r_ctx* ctx);
void    r_window_clear(void);
void    r_window_clear_color(const char* str);

/* Get the gamma value for the current window
 * ctx - the context containing the window
 * returns: gamma */
float r_window_get_gamma(r_ctx* ctx);

/* Set the gamma value for the current window
 * ctx - the context containing the window
 * gamma - the gamma value */
void r_window_set_gamma(r_ctx* ctx, float gamma);

/* Hide a window
 * ctx - the context containing the window */
void r_window_hide(r_ctx* ctx);

/* Show a window
 * ctx - the context containing the window */
void r_window_show(r_ctx* ctx);

/* Get the set max width of a window
 * ctx - the context containing the window */
uint32_t r_window_get_max_width(r_ctx* ctx);

/* Get the set max height of a window
 * ctx - the context containing the window */
uint32_t r_window_get_max_height(r_ctx* ctx);

/* Get the set max width and height of the window
 * ctx - the context containing the window */
void r_window_get_max_bounds(r_ctx* ctx, int* width, int* height);

/* Set the max bounds of a window
 * ctx - the context containing the window
 * width - the minimum width of the window
 * height - the mimumum height of the window*/
void r_window_set_max_bounds(r_ctx* ctx, int width, int height);

/* Get the set min width of a window
 * ctx - the context containing the window */
uint32_t r_window_get_min_width(r_ctx* ctx);
/* Get the set min height of a window
 * ctx - the context containing the window */
uint32_t r_window_get_min_height(r_ctx* ctx);

/* Get the set max width and height of the window
 * ctx - the context containing the window */
void r_window_get_min_bounds(r_ctx* ctx, int* width, int* height);

/* Set the minimum bounds of a window
 * ctx - the context containing the window
 * width - the minimum width of the window
 * height - the mimumum height of the window*/
void r_window_set_min_bounds(r_ctx* ctx, int width, int height);

/* Set the min-max sizes of the window
 * ctx - the context to affect
 * min_width - the minimum width of the window
 * min_height - the minimum height of the window
 * max_width - the maximum width of the window
 * max_height - the maximum height of the window */
void r_window_set_size_bounds(r_ctx* ctx, int min_width, int min_height,
                              int max_width, int max_height);

/* Have the window request attention
 * ctx - the context containing the window */
void r_window_request_attention(r_ctx* ctx);

/* Check if a window is focused
 * ctx - the context containing the window */
uint8_t r_window_is_focused(r_ctx* ctx);

/* Get the set refresh rate of a window
 * NOTE: 0 likely means it is being drawn in windowed mode
 * ctx - the context containing the window */
uint16_t r_get_refresh_rate(r_ctx* ctx);

#ifdef __cplusplus
}
#endif
#endif

