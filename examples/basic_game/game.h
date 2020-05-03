#if !defined BASIC_GAME_GAME_HEADER_GUARD_DEFINE_THATS_TOO_LONG
#define BASIC_GAME_GAME_HEADER_GUARD_DEFINE_THATS_TOO_LONG

#define DUNGEON_WIDTH     128
#define DUNGEON_HEIGHT    128
#define DUNGEON_MAX_ROOMS 6
#define DUNGEON_MIN_ROOMS 3

#define ROOM_MAX_TRIES 16

#include <astera/col.h>
#include <astera/sys.h>
#include <astera/linmath.h>

typedef enum {
  TILE_EMPTY = 0,
  TILE_STONE,
  TILE_START,
  TILE_END,
  TILE_WALL,
} tile_type_t;

typedef enum {
  ENEMY_NONE     = 0,
  ENEMY_SPIDER   = 1,
  ENEMY_SKELETON = 2,
} enemy_type_t;

typedef enum {
  STATE_IDLE     = 0,
  STATE_SEEK     = 1,
  STATE_ATTACK   = 2,
  STATE_COOLDOWN = 3,
  STATE_DIE      = 4
} enemy_state_t;

typedef enum {
  DIR_NORTH = 0,
  DIR_EAST  = 1,
  DIR_SOUTH = 2,
  DIR_WEST  = 3
} direction_t;

typedef struct {
  int   type;
  int   uid;
  int   state;
  vec2  position;
  vec2  size;
  float timer;
} enemy_t;

typedef enum {
  ROOM_EMPTY  = 0,
  ROOM_START  = 1,
  ROOM_END    = 2,
  ROOM_PIT    = 3,
  ROOM_AMBUSH = 4
} room_type_t;

typedef struct {
  int x, y;
} door_t;

typedef struct {
  int x, y, width, height;
  int type;

  door_t* doors;
  // split out for rnd_except call
  int* door_walls;
  int  door_count;

  c_aabb* walls;
  int     wall_count;
} room_t;

typedef struct {
  int* tiles;
  int  tile_count;

  int width, height;

  room_t* rooms;
  int     room_count;
} level_t;

typedef struct {
  vec2 position, size;

  // movement
  // NOTE: movement speed is normalized as to not allow diagonals to give 1.5x
  // movespeed; magnitude is great
  float move_speed;
  float run_speed;
  float walk_speed;

  float timer;
} player_t;

typedef struct {
  level_t  level;
  player_t player;

  // basic world timer (checking)
  int tick;
} world_t;

typedef enum {
  GAME_START = 0,
  GAME_GAME  = 1,
  GAME_PAUSE = 2,
  GAME_EXIT  = 3
} game_state_t;

world_t g_world;
int     game_state;

int rnd_except(int min, int max, int* except, int except_count);
int rnd_range(int min, int max);

void game_start();
void game_exit();

void level_exit();
void level_new(long seed);

void world_update(time_s delta);
void world_render(time_s delta);
void world_exit();

#endif
