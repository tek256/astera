#if !defined DUNGEON_H
#define DUNGEON_H

#include "types.h"

#include <astera/linmath.h>
#include <astera/sys.h>

typedef int tile_t;

typedef struct {
  int          x, y;
  enemy_type_t type;
} enemy_spawn_t;

typedef struct {
  vec2          position;
  enemy_type_t  type;
  enemy_state_t state;
} enemy_t;

typedef struct {
  int               x, y;
  decoration_type_t type;
  r_sprite          sprite;
} decoration_t;

typedef struct {
  int x, y;
  int room_a, room_b;
} door_t;

typedef struct {
  int id;

  int x, y;
  int width, height;

  room_type_t type;

  tile_t* tiles;
  int     tile_count;

  door_t* doors;
  int     door_count;

  decoration_t* decos;
  int           deco_count;

  enemy_spawn_t* spawns;
  int            spawn_count;

  enemy_t* enemies;
  int      enemy_count;
  int      enemy_cap;

  r_baked_sheet static_bg;

  c_aabb* walls;
  int     wall_count;
} room_t;

// TODO: actually make the player lol
typedef struct {
  vec2 position;
} player_t;

typedef struct {
  room_t* rooms;
  int     room_count;

  player_t player;
} dungeon_t;

typedef struct {
  int room_min_width, room_min_height;
  int room_max_width, room_max_height;
  int min_rooms, max_rooms;
  int max_room_tries;
} gen_config;

/// Runtime functions
void dungeon_update(dungeon_t* dungeon, time_s delta);
void room_update(room_t* room, time_s delta);

/// Render functions
void dungeon_render(dungeon_t* dungeon, time_s delta);
void room_render(room_t* room, time_s delta);

/// Exit functions
void room_destroy(room_t* room);
void dungeon_destroy(dungeon_t* dungeon);

/// Creation functions

// Just so we can track how we're passing things
gen_config gen_config_create(int room_min_width, int room_max_width,
                             int room_min_height, int room_max_height,
                             int min_rooms, int max_rooms);

/* Returns the direction of the wall it's on from center */
dungeon_t dungeon_new(gen_config conf, long seed);

void room_decorate(room_t* room);
void room_init_physics(room_t* room);

float room_closest_door(room_t* room, int x, int y);

/// Utility functions
int room_contains(room_t* room, vec2 point);
int room_tile_is_wall(room_t* room, int x, int y);
// A is the room being placed, B is the room already existing
int room_intersects_room(room_t* a, room_t* b, vec2 solve);
#endif
