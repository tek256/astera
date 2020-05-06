// TODO: Redo all of this
#if !defined ASTERA_COL_H
#define ASTERA_COL_H

#include <astera/export.h>
#include <astera/linmath.h>

typedef struct {
  vec2 center;
  vec2 halfsize;
} c_aabb;

typedef struct {
  vec2  center;
  float radius;
} c_circle;

typedef struct {
  vec2* verts;
  int   count;
  float angle;

  c_aabb bounds;
} c_comp;

typedef struct {
  vec2  normal;
  float pen;
} c_man;

ASTERA_API c_aabb   c_aabb_get(vec2 pos, vec2 size);
ASTERA_API c_circle c_circ_get(vec2 pos, float rad);
ASTERA_API c_comp   c_comp_get(vec2* verts, int count);

ASTERA_API int c_aabb_vs_aabb(c_man* man, c_aabb a, c_aabb b);
ASTERA_API int c_aabb_vs_circ(c_man* man, c_aabb a, c_circle b);
ASTERA_API int c_circ_vs_circ(c_man* man, c_circle a, c_circle b);
ASTERA_API int c_aabb_vs_comp(c_man* man, c_aabb a, c_comp b);
ASTERA_API int c_circ_vs_comp(c_man* man, c_circle a, c_comp b);
ASTERA_API int c_comp_vs_comp(c_man* man, c_comp a, c_comp b);

#endif
