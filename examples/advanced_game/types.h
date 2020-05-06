#if !defined TYPES_H
#define TYPES_H

#define TILE_WORLD_SIZE 16.f

typedef enum {
  TILE_EMPTY = 0,
  TILE_STONE,
  TILE_WALL,
  TILE_PIT,
  TILE_WATER,
  TILE_RUG,
  TILE_START,
  TILE_END,
} tile_type_t;

typedef enum {
  DECO_NONE = 0,
  DECO_TORCH,
  DECO_CRATE,
  DECO_POTS,
  DECO_POT,
  DECO_TABLE,
  DECO_FIREPIT
} decoration_type_t;

typedef enum {
  OBJ_NONE = 0,
  OBJ_TREASURE,
  OBJ_COIN,
} obj_type_t;

typedef enum {
  ROOM_EMPTY = 0,
  ROOM_FIGHT,
  ROOM_TREASURE,
  ROOM_PIT,   // just a big ol pit
  ROOM_DINER, // dinner hall, because why not
  ROOM_RIVER, // river area because why not
  ROOM_START,
  ROOM_END
} room_type_t;

typedef enum {
  ENM_NONE = 0,
  ENM_ZOMBIE,
  ENM_SKELETON,
  ENM_BAT,
  ENM_SPIDER,
} enemy_type_t;

typedef enum {
  STATE_IDLE = 0,
  STATE_SEEK,
  STATE_ATTACK,
  STATE_CD,
  STATE_PATROL
} ememy_state_t;

typedef struct {
  DIR_NORTH = 0, DIR_EAST, DIR_SOUTH, DIR_WEST,
} direction_t;

#endif
