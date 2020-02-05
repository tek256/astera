#ifndef ASTERA_LEVEL_HEADER
#define ASTERA_LEVEL_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "asset.h"
#include "sys.h"

#include <misc/linmath.h>
#include <stdint.h>

#define LEVEL_IS_FLAG(value, offset) ((value) & (1 >> offset))
#define LEVEL_IS_TYPE(value, type) ((value) & (type)) == type

#define LEVEL_PTR_MOVED = 1 << 2;

#if !defined(LEVEL_SECTION_TEST_CAPACITY)
#define LEVEL_SECTION_TEST_CAPACITY 16
#endif

typedef enum { KINEMATIC = 0, DYNAMIC = 1, STATIC = 2 } collision_type;

typedef struct {
  vec2 position, size, velocity;
  float rotation;
  int8_t type;
  int32_t flags;
} l_box;

typedef struct {
  vec2 position, velocity;
  float radius;
  int8_t type;
  int32_t flags;
} l_circle;

typedef struct {
  vec2 start, end, velocity;
  vec2 direction; // applied if semi permiable
  int8_t semi;    // semi permiable (single direction pass thru)
} l_line;

typedef struct {
  vec2 position, velocity;
  float rotation; // Not sure if I want to have to calculate this, but it might
                  // prove useful
  vec2 *points;
  uint32_t point_count;
} l_complex;

typedef enum { BOX = 0, CIRCLE = 1, LINE = 2, COMPLEX = 3 } l_col_type;

typedef enum { OBJECT = 0, ENTITY = 1, OTHER = 2 } l_owner_type;

typedef struct {
  l_owner_type type;
  void *data;
} l_col_owner;

typedef struct {
  l_col_type col_type;
  l_col_owner owner;
  int8_t active;
  void *data;
} l_col;

typedef struct {
  l_col *a, *b;
  uint32_t step;
} l_col_action;

typedef struct {
  l_col_action *actions;
  uint32_t capacity, count;
} l_col_history;

typedef struct {
  uint32_t uid;
  uint32_t type;

  vec2 position, velocity, size;

  l_col col;

  int8_t is_static : 1;
  int8_t pos_change : 1;
  int8_t active : 1;
} l_object;

typedef struct {
  uint32_t uid;
  uint32_t type;

  vec2 position, velocity, size;

  l_col col;

  int8_t is_static : 1;
  int8_t pos_change : 1;
  int8_t active : 1;
} l_entity;

typedef struct {
  uint32_t uid;
  uint32_t step;
  int8_t obj : 1;
} l_change;

// Psuedo-Camera for tracking streaming data
typedef struct {
  vec2 position, size, velocity;
  int8_t change;
} l_viewer;

typedef struct {
  vec2 position, size;

  // Relative positions in grid
  uint32_t x, y;

  l_object **objs;
  uint32_t obj_count, obj_capacity;

  l_entity **ents;
  uint32_t ent_count, ent_capacity;

  l_change **additions;
  uint32_t add_count, add_cap;

  uint32_t step;
} l_section;

// Level Delta type
// This type is to store specific flags globally for an object
typedef struct {
  uint32_t uid;
  uint16_t flags;
  int8_t in_file;
} l_delta;

// Level header struct, for post-file read
typedef struct {
  uint32_t *sections;
  int32_t *section_offsets;
  int32_t section_count;
  const char *path;

  asset_t *data;
} l_header;

// This is the overall level storage part
typedef struct {
  l_object *objs;
  uint32_t obj_count, obj_capacity;

  l_entity *ents;
  uint32_t ent_count, ent_capacity;

  l_delta *deltas;
  uint32_t delta_count, delta_capacity;
  uint32_t new_delta_index;

  l_section *sections;
  uint32_t section_count, section_capacity;

  l_viewer viewer;
  l_header header;

  uint32_t step;
} l_level;

// Overall update function for level context
void level_update(l_level *level, time_s delta);
void level_update_col(l_col *col, vec2 *pos, vec2 vel, time_s delta);

// Generic switch case thing
int8_t level_col_resolve(l_col *a, l_col *b, int8_t allow_resolution);

l_entity *level_get_entity(l_level *level, uint32_t uid);
l_object *level_get_object(l_level *level, uint32_t uid);

int8_t level_obj_is_current(l_section *section, l_object *obj);
int8_t level_ent_is_current(l_section *section, l_entity *obj);

l_section *level_get_section_relative(l_level *level, uint32_t x, uint32_t y);
l_section *level_section_by_point(l_level *level, vec2 point);
uint32_t level_sections_within_grid(l_section ***dst, uint32_t dst_max,
                                    l_level *level, uint32_t x_min,
                                    uint32_t y_min, uint32_t x_max,
                                    uint32_t y_max);
uint8_t level_section_overlaps(l_section *section, vec4 bounds);
uint32_t level_sections_within_bounds(l_section ***dst, uint32_t dst_max,
                                      l_level *levl, vec4 bounds);
int8_t level_section_check_add_obj(l_section *section, l_object *obj);
int8_t level_section_check_add_ent(l_section *section, l_object *ent);

#ifdef __cplusplus
}
#endif
#endif
