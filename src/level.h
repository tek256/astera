// TODO Broadphase Detection
// TODO Box / SAT Implementation
// TODO GJK Implementation
// TODO Plane Implementation
// TODO Level Save
// TODO Level Load
#if !defined(ASTERA_NO_LEVEL)
#if !defined(ASTERA_LEVEL_HEADER)
#define ASTERA_LEVEL_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define LEVEL_IS_FLAG(value, offset) ((value) & (1 >> offset))
#define LEVEL_IS_TYPE(value, type)   ((value) & (type)) == type

#include "asset.h"
#include "sys.h"

#include <misc/linmath.h>
#include <stdint.h>

typedef struct {
  vec2  center, normal;
  float length;
} l_plane;

typedef struct {
  vec2 center, halfsize;
  vec4 bounds;
} l_aabb;

typedef struct {
  vec2  center, halfsize;
  float angle; // radians
  vec4  bounds;
} l_box;

typedef struct {
  float radius;
  vec2  center;
  vec4  bounds;
} l_circle;

typedef struct {
  vec4 bounds;
  vec2 center;

  vec2*    verts;
  uint32_t count;
} l_complex;

typedef struct {
  vec2  normal;
  float pen;
} l_manifold;

typedef enum {
  L_AABB    = 0,
  L_BOX     = 1,
  L_CIRCLE  = 2,
  L_COMPLEX = 3,
  L_PLANE   = 4
} l_col_type;

typedef struct {
  void*      data;
  l_col_type type;

  uint32_t layer;
  uint32_t obj_id;
} l_col;

typedef struct {
  uint32_t uid, rid;
  l_col    col;

  uint32_t type, flags;
} l_obj;

typedef struct l_quad_leaf l_quad_leaf;
struct l_quad_leaf {
  uint32_t uid; // uid == index in array

  l_aabb       aabb;
  l_quad_leaf *nw, *ne, *sw, *se;

  l_obj**  objs;
  uint32_t count, capacity, level;

  int8_t is_set;
};

typedef struct {
  l_obj**  values;
  uint32_t count, capacity;
} l_obj_query;

typedef struct {
  l_quad_leaf* leafs;
  uint32_t     count, capacity, max_level, leaf_capacity;
  l_aabb       range;
  l_obj_query  query;
} l_quad_tree;

typedef struct {
  uint32_t uid;

  l_obj*   objs;
  uint32_t obj_count, obj_capacity;

  l_quad_tree quad_tree;

  uint32_t step;
  int8_t   active;
} l_level;

l_col l_col_create(void* data, l_col_type type, uint32_t layer);
l_obj l_obj_create();
void  l_obj_move(l_obj* obj, vec2 dist);
void  l_obj_get_position(l_obj* obj, vec2 dst);

l_quad_tree l_tree_create(l_aabb range, uint32_t leaf_capacity);
void        l_tree_destroy(l_quad_tree* tree);

static void l_tree_query_r(l_quad_tree* tree, l_quad_leaf* leaf,
                           l_obj_query* query, l_aabb range, uint32_t layers);

l_obj_query l_tree_query(l_quad_tree* tree, l_aabb range, uint32_t layer,
                         uint32_t capacity);
void        l_tree_insert(l_quad_tree* tree, l_obj* obj);
void        l_tree_remove(l_quad_tree* tree, l_obj* obj);

// static void l_tree_check_leaf(l_quad_tree* tree, l_quad_leaf* leaf,
//                              int32_t* result) {

static void l_tree_check_leaf(l_quad_leaf* leaf, l_obj* obj, int32_t* res);
int32_t     l_tree_check_leafs(l_quad_tree* tree, l_obj* obj);

void l_leaf_subdivide(l_quad_leaf* leaf);

int8_t l_aabb_contains(l_aabb box, vec2 point);

static float distpow(vec2 a, vec2 b);
static float distsqrt(vec2 a, vec2 b);

// --- creation functions  ---
l_aabb    l_aabb_create(vec2 center, vec2 size);
l_circle  l_circle_create(vec2 center, float radius);
l_box     l_box_create(vec2 center, vec2 size, float angle);
l_complex l_comp_create(vec2 center, vec2* verts, uint32_t vert_count,
                        float angle);

//---  movement functions  ---
void l_plane_move(l_plane* a, vec2 dist);
void l_aabb_move(l_aabb* a, vec2 dist);
void l_box_move(l_box* a, vec2 dist);
void l_circ_move(l_circle* a, vec2 dist);
void l_comp_move(l_complex* a, vec2 dist);

//---  point tests  ---
int8_t l_plane_cont(l_plane a, vec2 point);
int8_t l_aabb_cont(l_aabb a, vec2 point);
int8_t l_box_cont(l_box a, vec2 point);
int8_t l_cir_cont(l_circle a, vec2 point);
int8_t l_comp_cont(l_complex a, vec2 point);

//---  intersection tests  ---

// this will be the general precursor function,
// it's just aabb vs aabb but it'll be used for
// lesser detail it'll help optimize out GJK /
// SAT Calls overall
int8_t l_bounds_sect_bounds(l_manifold* man, vec4 a, vec4 b);

int8_t l_plane_vs_plane(l_manifold* man, l_plane a, l_plane b);
int8_t l_plane_vs_aabb(l_manifold* man, l_plane a, l_aabb b);
int8_t l_plane_vs_box(l_manifold* man, l_plane a, l_box b);
int8_t l_plane_vs_cir(l_manifold* man, l_plane a, l_circle b);
int8_t l_plane_vs_comp(l_manifold* man, l_plane a, l_complex b);

int8_t l_aabb_vs_aabb(l_manifold* man, l_aabb a, l_aabb b);
int8_t l_aabb_vs_box(l_manifold* man, l_aabb a, l_box b);
int8_t l_aabb_vs_cir(l_manifold* man, l_aabb a, l_circle b);
int8_t l_aabb_vs_complex(l_manifold* man, l_aabb a, l_complex b);

int8_t l_box_vs_box(l_manifold* man, l_box a, l_box b);
int8_t l_box_vs_cir(l_manifold* man, l_box a, l_circle b);
int8_t l_box_vs_comp(l_manifold* man, l_box a, l_complex b);

int8_t l_cir_vs_cir(l_manifold* man, l_circle a, l_circle b);
int8_t l_cir_vs_comp(l_manifold* man, l_circle a, l_complex b);

int8_t l_comp_vs_comp(l_manifold* man, l_complex a, l_complex b);

#ifdef __cplusplus
}
#endif
#endif
#endif
