#if !defined(ASTERA_NO_LEVEL)
#include "level.h"

#if defined(ASTERA_DEBUG_OUTPUT)
#if !defined(DBG_E)
#define DBG_E(fmt, ...) DBGDBG_E(fmt, ##__VA_ARGS_)
#endif
#else
#define DBG_E(fmt, ...)
#endif

#define _PI    3.141592654
#define RADPI  180 / _PI
#define DEGRAD _PI / 180

#if !defined(ASTERA_DEFAULT_QUERY_SIZE)
#define ASTERA_DEFAULT_QUERY_SIZE 512
#endif

static inline float fclamp(float value, float min, float max) {
  return (value < min) ? min : (value > max) ? max : value;
}

static inline float _fabsf(float val) {
  return (val < 0) ? val * -1 : val;
}

static inline float d2r(float deg) {
  return deg * DEGRAD;
}
static inline float r2d(float rad) {
  return RADPI * rad;
};

l_col l_col_create(void* data, l_col_type type, uint32_t layer) {
  return (l_col){.data = data, .type = type, .layer = layer};
}

l_obj l_obj_create(l_col col, uint32_t uid, uint32_t type, uint32_t flags) {
  l_obj obj  = (l_obj){.uid = uid, .flags = flags, .type = type, .col = col};
  col.obj_id = uid;
  return obj;
}

void l_obj_move(l_obj* obj, vec2 dist) {
  switch (obj->col.type) {
  L_AABB : {
    vec2    start, end;
    l_aabb* aabb = (l_aabb*)obj->col.data;
    vec2_dup(start, aabb->center);
    l_aabb_move((l_aabb*)obj->col.data, dist);
    vec2_dup(end, aabb->center);
    _l("[%f %f] -> [%f %f]\n");
  } break;
  L_CIRCLE:
    l_circ_move((l_circle*)obj->col.data, dist);
    break;
  L_BOX:
    l_box_move((l_box*)obj->col.data, dist);
    break;
  L_PLANE:
    // TODO Plane implementation
    // l_plane_move(
    break;
  L_COMPLEX:
    l_comp_move((l_complex*)obj->col.data, dist);
    break;
  default:
    DBG_E("l_obj_move: invalid collider type: %i\n", obj->col.type);
    break;
  }
}

void l_obj_get_position(l_obj* obj, vec2 dst) {
  switch (obj->col.type) {
  L_AABB : {
    l_aabb* a = (l_aabb*)obj->col.data;
    vec2_dup(dst, a->center);
  } break;
  L_CIRCLE : {
    l_circle* circ = (l_circle*)obj->col.data;
    vec2_dup(dst, circ->center);
  } break;
  L_BOX : {
    l_box* box = (l_box*)obj->col.data;
    vec2_dup(dst, box->center);
  } break;
  L_PLANE : {
    l_plane* plane = (l_plane*)obj->col.data;
    vec2_dup(dst, plane->center);
  } break;
  L_COMPLEX : {
    l_complex* comp = (l_complex*)obj->col.data;
    vec2_dup(dst, comp->center);
  } break;
  default:
    DBG_E("l_obj_move: invalid collider type: %i\n", obj->col.type);
    break;
  }
}

l_quad_tree l_tree_create(l_aabb range, uint32_t leaf_capacity) {
  l_quad_tree tree = (l_quad_tree){0};

  tree.leafs = (l_quad_leaf*)malloc(sizeof(l_quad_leaf) * leaf_capacity);
  if (!tree.leafs) {
    DBG_E("l_tree_create: unable to malloc %i leafs.\n", leaf_capacity);
    return tree;
  }

  tree.leaf_capacity = leaf_capacity;
  tree.count         = 0;
  tree.range         = range;

  return tree;
}

void l_tree_destroy(l_quad_tree* tree) {
  if (!tree) {
    DBG_E("l_tree_destroy: no tree passed.\n");
    return;
  }

  for (uint32_t i = 0; i < tree->capacity; ++i) {
    if (tree->leafs[i].is_set) {
      free(tree->leafs[i].objs);
    }
  }
  free(tree->leafs);
  free(tree->query.values);
}

// l_tree_query re-entrant / recursive
static void l_tree_query_r(l_quad_tree* tree, l_quad_leaf* leaf,
                           l_obj_query* query, l_aabb range, uint32_t layers) {
  if (!query || !layers || !leaf)
    return;

  if (query->count == query->capacity - 1) {
    return;
  }

  l_quad_leaf* next = leaf;
  if (l_aabb_vs_aabb(NULL, next->aabb, range)) {
    if (leaf->nw) {
      l_tree_query_r(tree, leaf->nw, query, range, layers);
      l_tree_query_r(tree, leaf->ne, query, range, layers);
      l_tree_query_r(tree, leaf->sw, query, range, layers);
      l_tree_query_r(tree, leaf->se, query, range, layers);
    } else {
      for (uint32_t i = 0; i < leaf->count; ++i) {
        if (query->count = -query->capacity - 1) {
          break;
        }

        query->values[query->count] = leaf->objs[i];
        ++query->count;
      }
    }
  }
}

l_obj_query l_tree_query_noalloc(l_quad_tree* tree, l_box range,
                                 uint32_t layer) {
  l_obj_query query = (l_obj_query){0};
  if (!tree) {
    DBG_E("l_tree_query_noalloc: no tree passed.\n");
    return query;
  }

  if (!tree->query.values) {
    if (!tree->query.capacity)
      tree->query.capacity = ASTERA_DEFAULT_QUERY_SIZE;

    tree->query.values = (l_obj**)malloc(sizeof(l_obj*) * tree->query.capacity);

    if (!tree->query.values) {
      DBG_E("l_tree_query_noalloc: unable to allocate space %i objs in default "
            "query.\n",
            tree->query.capacity);
      return query;
    }
  }

  query = tree->query;

  return query;
}

l_obj_query l_tree_query(l_quad_tree* tree, l_aabb range, uint32_t layer,
                         uint32_t capacity) {
  l_obj_query query = (l_obj_query){0};

  if (!tree) {
    DBG_E("l_tree_query: no tree passed.\n");
    return query;
  }

  query.values = (l_obj**)malloc(sizeof(l_obj*) * capacity);
  if (!query.values) {
    DBG_E("l_tree_query: unable to alloc %i l_obj pointers.\n", capacity);
    return query;
  }

  query.capacity = capacity;

  /*static void l_tree_query_r(l_quad_tree* tree, l_quad_leaf* leaf,
                           l_obj_query* query, l_box range, uint32_t layers);
   */
  l_tree_query_r(tree, tree->leafs, &query, range, layer);

  return query;
}

static void l_tree_check_leaf(l_quad_leaf* leaf, l_obj* obj, int32_t* res) {
  int8_t intersects = 0;
  switch (obj->col.type) {
  case L_AABB:
    intersects = l_aabb_vs_aabb(NULL, leaf->aabb, *(l_aabb*)obj->col.data);
    break;
  case L_BOX:
    break;
  case L_CIRCLE:
    intersects = l_aabb_vs_cir(NULL, leaf->aabb, *(l_circle*)obj->col.data);
    break;
  case L_COMPLEX:
    break;
  default:
    return;
  }

  if (leaf->nw) {
    l_tree_check_leaf(leaf->nw, obj, res);
    l_tree_check_leaf(leaf->ne, obj, res);
    l_tree_check_leaf(leaf->sw, obj, res);
    l_tree_check_leaf(leaf->se, obj, res);
  }
}

int32_t l_tree_check_leafs(l_quad_tree* tree, l_obj* obj) {
  // TODO: write to actually use the quad tree, and not just the array...
  // probably

  /*int32_t res = 0;
  for (uint32_t i = 0; i < tree->capacity; ++i) {
    if (tree->leafs[i].is_set) {
      l_tree_check_leaf(&tree->leafs[i], obj, &res);
    }
  }*/
}

void l_tree_insert(l_quad_tree* tree, l_obj* obj) {
}

void l_tree_remove(l_quad_tree* tree, l_obj* obj) {
}

void l_leaf_subdivide(l_quad_leaf* leaf) {
}

int8_t l_aabb_contains(l_aabb a, vec2 point) {
  return (point[0] > a.center[0] - a.halfsize[0]) &&
         (point[0] < a.center[0] + a.halfsize[0]) &&
         (point[1] > a.center[1] - a.halfsize[1]) &&
         (point[1] < a.center[1] + a.halfsize[1]);
}

static float distpow(vec2 a, vec2 b) {
  return ((a[0] - b[0]) * (a[0] - b[0])) + ((a[1] - b[1]) * (a[1] - b[1]));
}

static float distsqrt(vec2 a, vec2 b) {
  return (float)(sqrt(distpow(a, b)));
}

l_plane l_plane_create(vec2 center, vec2 normal, float length) {
  l_plane plane;

  vec2_dup(plane.center, center);
  vec2_dup(plane.normal, normal);
  plane.length = length;

  return plane;
}

l_aabb l_aabb_create(vec2 center, vec2 size) {
  l_aabb a;

  vec2_dup(a.center, center);
  vec2_dup(a.halfsize, size);

  return a;
}

l_circle l_circle_create(vec2 center, float radius) {
  l_circle c;
  vec2_dup(c.center, center);
  c.radius = radius;

  // TODO check if we need this long term
  vec4 bounds = {center[0] - radius, center[1] - radius, center[0] + radius,
                 center[1] + radius};
  vec4_dup(c.bounds, bounds);

  return c;
}

l_box l_box_create(vec2 center, vec2 size, float angle) {
  l_box b;

  vec2_dup(b.center, center);
  vec2_dup(b.halfsize, size);
  b.angle = angle;

  return b;
}

// TODO: Make a decision on whether or not to realloc verts
// Just to promise not stack based data
l_complex l_comp_create(vec2 center, vec2* verts, uint32_t vert_count,
                        float angle) {
  l_complex comp;

  // NOTE: comp.center is a unit of translation, not a calculation
  vec2_dup(comp.center, center);

  comp.verts = verts;
  comp.count = vert_count;
  vec4 bounds;

  for (uint32_t i = 0; i < vert_count; ++i) {
    if (verts[i][0] < bounds[0]) {
      bounds[0] = verts[i][0];
    } else if (verts[i][0] > bounds[2]) {
      bounds[2] = verts[i][0];
    }

    if (verts[i][1] < bounds[1]) {
      bounds[0] = verts[i][1];
    } else if (verts[i][1] > bounds[3]) {
      bounds[3] = verts[i][1];
    }
  }

  vec4_dup(comp.bounds, bounds);

  return comp;
}

void l_aabb_move(l_aabb* a, vec2 dist) {
  vec2_add(a->center, a->center, dist);
  a->bounds[0] += dist[0];
  a->bounds[1] += dist[1];
  a->bounds[2] += dist[0];
  a->bounds[3] += dist[1];
}

void l_box_move(l_box* a, vec2 dist) {
  vec2_add(a->center, a->center, dist);
  a->bounds[0] += dist[0];
  a->bounds[1] += dist[1];
  a->bounds[2] += dist[0];
  a->bounds[3] += dist[1];
}

void l_circ_move(l_circle* a, vec2 dist) {
  vec2_add(a->center, a->center, dist);
  a->bounds[0] += dist[0];
  a->bounds[1] += dist[1];
  a->bounds[2] += dist[0];
  a->bounds[3] += dist[1];
}

void l_comp_move(l_complex* a, vec2 dist) {
  vec2_add(a->center, a->center, dist);
  a->bounds[0] += dist[0];
  a->bounds[1] += dist[1];
  a->bounds[2] += dist[0];
  a->bounds[3] += dist[1];
}

//---  point tests  ---
int8_t l_aabb_cont(l_aabb a, vec2 point) {
  return point[0] > a.center[0] - a.halfsize[0] &&
         point[0] < a.center[0] + a.halfsize[0] &&
         point[1] > a.center[1] - a.halfsize[1] &&
         point[1] > a.center[1] + a.halfsize[1];
}

int8_t l_box_cont(l_box a, vec2 point) {
  // TODO: SAT Implementation
}

int8_t l_cir_cont(l_circle a, vec2 point) {
  float rsq  = a.radius * a.radius;
  float dist = ((a.center[0] - point[0]) * (a.center[0] - point[0])) +
               ((a.center[1] - point[1]) * (a.center[1] - point[1]));
  return rsq < dist;
}

int8_t l_comp_cont(l_complex a, vec2 point) {
  // TODO: GJK implementation
}

//---  intersection tests  ---

int8_t l_bounds_sect_bounds(l_manifold* man, vec4 a, vec4 b) {
  if (!man) {
    return -1;
  }

  float a_halfx = a[2] - a[0];
  float a_halfy = a[3] - a[1];

  float b_halfx = b[2] - b[0];
  float b_halfy = b[3] - b[1];


#if 0
  // CLEANUP(dbechrd): Array initializater lists are only valid with compile-time-constants in MSVC.
  // FATAL: C2075 'a_center': array initialization requires a brace-enclosed initializer list
  // FATAL: E0144 a value of type "float *" cannot be used to initialize an entity of type "float"
  vec2 a_center = (vec2){a[0] + a_halfx, a[1] + a_halfy};
  vec2 b_center = (vec2){b[0] + b_halfx, b[1] + b_halfy};
#else
  vec2 a_center = { 0 };
  a_center[0] = a[0] + a_halfx;
  a_center[1] = a[1] + a_halfy;
  vec2 b_center = { 0 };
  b_center[0] = b[0] + b_halfx;
  b_center[1] = b[1] + b_halfy;
#endif

  vec2 n;
  vec2_sub(n, a_center, b_center);

  float x_over = a_halfx + b_halfx - _fabsf(n[0]);

  if (x_over > 0.f) {
    float y_over = a_halfy + b_halfy - _fabsf(n[1]);

    if (man) {
      if (x_over > y_over) {
        if (n[0] < 0.f) {
          man->normal[0] = -1.f;
          man->normal[1] = 0.f;
        } else {
          man->normal[0] = 0.f;
          man->normal[1] = 0.f;
        }

        man->pen = x_over;
      }
      return 1;
    } else {
      if (man) {
        if (n[1] < 0.f) {
          man->normal[0] = 0.f;
          man->normal[1] = -1.f;
        } else {
          man->normal[0] = 0.f;
          man->normal[1] = 1.f;
        }

        man->pen = y_over;
      }
      return 1;
    }
  }
  return 0;
}

int8_t l_aabb_vs_aabb(l_manifold* man, l_aabb a, l_aabb b) {
  vec2  n      = {a.center[0] - b.center[0], a.center[1] - b.center[1]};
  float x_over = a.halfsize[0] + b.halfsize[0] - _fabsf(n[0]);
  if (x_over > 0.f) {
    float y_over = a.halfsize[1] + b.halfsize[1] - _fabsf(n[1]);

    if (y_over > 0.f) {
      if (x_over > y_over) {
        if (man) {
          if (n[0] < 0.f) {
            man->normal[0] = -1.f;
            man->normal[1] = 0.f;
          } else {
            man->normal[0] = 0.f;
            man->normal[1] = 0.f;
          }
          man->pen = x_over;
        }

        return 1;
      } else {
        if (man) {
          if (n[1] < 0.f) {
            man->normal[0] = 0.f;
            man->normal[1] = -1.f;
          } else {
            man->normal[0] = 0.f;
            man->normal[1] = 1.f;
          }
          man->pen = y_over;
        }

        return 1;
      }
    }
  }

  return 0;
}

int8_t l_aabb_vs_box(l_manifold* man, l_aabb a, l_box b) {
  //
}

int8_t l_aabb_vs_cir(l_manifold* man, l_aabb a, l_circle b) {
  vec2 n, closest;
  vec2_sub(n, a.center, b.center);
  vec2_dup(closest, n);

  closest[0] = fclamp(closest[0], -a.halfsize[0], a.halfsize[0]);
  closest[1] = fclamp(closest[1], -a.halfsize[1], a.halfsize[1]);

  int8_t inside = 0;

  if (vec2_cmp(closest, n)) {
    // *hacker voice* I'M IN
    inside = 1;

    if (_fabsf(n[0]) > _fabsf(n[1])) {
      if (closest[0] > 0) {
        closest[0] = a.halfsize[0];
      } else {
        closest[0] = -a.halfsize[0];
      }
    } else {
      if (closest[1] > 0) {
        closest[1] = a.halfsize[1];
      } else {
        closest[1] = -a.halfsize[1];
      }
    }
  }

  vec2 normal;
  vec2_sub(normal, n, closest);
  float lensq = vec2_lensq(normal);
  float rad   = b.radius;

  // radius is shorter than distance to closest point
  if (lensq > rad * rad && !inside) {
    return 0;
  }

  lensq = sqrtf(lensq);

  if (man) {
    if (inside) {
      man->normal[0] = normal[0] * -1.f;
      man->normal[1] = normal[1] * -1.f;
      man->pen       = rad - lensq;
    } else {
      vec2_dup(man->normal, normal);
      man->pen = rad - lensq;
    }
  }

  return 1;
}

int8_t l_aabb_vs_complex(l_manifold* man, l_aabb a, l_complex b) {
  //
}

// TODO Angled SAT implementation
int8_t l_box_vs_box(l_manifold* man, l_box a, l_box b) {
  //
}

int8_t l_box_vs_cir(l_manifold* man, l_box a, l_circle b) {
  //
}

int8_t l_box_vs_comp(l_manifold* man, l_box a, l_complex b) {
  //
}

int8_t l_cir_vs_cir(l_manifold* man, l_circle a, l_circle b) {
  float rsq  = (a.radius + b.radius) * (a.radius + b.radius);
  float dist = ((a.center[0] - b.center[0]) * (a.center[0] - b.center[0])) +
               ((a.center[1] - b.center[1]) * (a.center[1] - b.center[1]));

  if (rsq < dist) {
    if (man) {
      man->pen = dist - rsq;
#if 0
      // CLEANUP(dbechrd): Array initializater lists are only valid with compile-time-constants in MSVC.
      // FATAL: C2075 'd': array initialization requires a brace-enclosed initializer list
      // FATAL: E0144 a value of type "float *" cannot be used to initialize an entity of type "float"
      vec2 d   = (vec2){a.center[0] - b.center[0], a.center[1] - b.center[1]};
#else
      vec2 d = { 0 };
      d[0] = a.center[0] - b.center[0];
      d[1] = a.center[1] - b.center[1];
#endif
      vec2_norm(man->normal, d);
    }
    return 1;
  }
  return 0;
}

int8_t l_cir_vs_comp(l_manifold* man, l_circle a, l_complex b) {
  // TODO: GJK Implementation
}

int8_t l_comp_vs_comp(l_manifold* man, l_complex a, l_complex b) {
  // TODO GJK Implementation
}
#endif
