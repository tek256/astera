#define ASTERA_NO_LEVEL
#if !defined(ASTERA_NO_LEVEL)
#include "level.h"

#define _PI    3.141592654
#define RADPI  180 / _PI
#define DEGRAD _PI / 180

static inline float fclamp(float value, float min, float max) {
  return (value < min) ? min : (value > max) ? max : value;
}

static inline float fabsf(float val) {
  return (val < 0) ? val * -1 : val;
}

static inline float d2r(float deg) {
  return deg * DEGRAD;
}
static inline float r2d(float rad){return RADPI * rad};

l_quad_tree l_tree_create(l_box range, vec2 min_size, uint32_t leaf_capacity) {
  l_quad_tree  tree = (l_quad_tree){0};
  l_quad_leaf* leafs;
  uint32_t     count, capacity, max_level, leaf_capacity;

  tree.leafs = (l_quad_leaf*)malloc(sizeof(l_quad_leaf) * leaf_capaicty);
  if (!tree.leafs) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("l_tree_create: unable to malloc %i leafs.\n", leaf_capacity);
#endif
    return tree;
  }

  tree.leaf_capacity = leaf_capacity;
  tree.leaf_count    = 0;

  return tree;
}

void l_tree_destroy(l_quad_tree* tree) {
  if (!tree) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("l_tree_destroy: no tree passed.\n");
#endif
    return;
  }

  for (uint32_t i = 0; i < tree->capacity; ++i) {
    if (tree->leafs[i].isset) {
      free(tree->leafs[i].objs);
    }
  }
  free(tree->leafs);
  free(tree->query->values);
}

// l_tree_query re-entrant / recursive
static void l_tree_query_r(l_quad_tree* tree, l_quad_leaf* leaf,
                           l_obj_query* query, l_box range, uint32_t layers) {
  if (!query || !layers || !leaf)
    return;

  if (query->count == query->capacity - 1) {
    return;
  }

  l_tree_leaf* next = leaf;
  if (l_box_intersects(next->box, range)) {
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
  l_obj_query query = (l_tree_query){0};
  if (!tree) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("l_tree_query_noalloc: no tree passed.\n");
#endif
    return query;
  }

  if (!tree.query.values) {
    if (!tree.query.capacity)
      tree.query.capacity = ASTERA_DEFAULT_QUERY_SIZE;

    tree.query.values = (l_obj**)malloc(sizeof(l_obj*) * tree.query.capacity);

    if (!tree.query.values) {
#if defined(ASTERA_DEBUG_OUTPUT)
      _e("l_tree_query_noalloc: unable to allocate space %i objs in default "
         "query.\n",
         tree.query.capacity);
#endif
      return query;
    }
  }

  query = tree.query;

  return query;
}

l_obj_query l_tree_query(l_quad_tree* tree, l_box range, uint32_t layer,
                         uint32_t capacity) {
  l_obj_query query = (l_obj_query){0};

  if (!tree) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("l_tree_query: no tree passed.\n");
#endif
    return query;
  }

  query.values = (l_obj**)malloc(sizeof(l_obj*) * capacity);
  if (!query.values) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("l_tree_query: unable to alloc %i l_obj pointers.\n", capacity);
#endif
    return query;
  }

  query.capacity = capacity;

  l_tree_query_r(tree, tree->start, range, layer);

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
    intersects = l_aabb_vs_circle(NULL, leaf->aabb, *(l_circle*)obj->col.data);
    break;
  case L_COMPLEX:
    break;
  default:
    return 0;
  }

  if (leaf->nw) {
    l_tree_check_leaf(leaf->nw, obj, res);
    l_tree_check_leaf(leaf->ne, obj, res);
    l_tree_check_leaf(leaf->sw, obj, res);
    l_tree_check_leaf(leaf->se, obj, res);
  }
}

int32_t l_tree_check_leafs(l_quad_tree* tree, l_obj* obj) {
  int32_t res = 0;
  for (uint32_t i = 0; i < tree->capacity; ++i) {
    if (tree->leafs[i].isset) {
      res += l_tree_check_leaf(&tree->leafs[i], obj, &res);
    }
  }
}

void l_tree_insert(l_quad_tree* tree, l_obj* obj) {
  l_tree_check_leafs(
}

void l_tree_remove(l_quad_tree* tree, l_obj* obj) {
}

void l_leaf_subdivide(l_quad_leaf* leaf) {
}

int8_t l_box_contains(l_box box, vec2 point) {
}

int8_t l_box_intersects(l_box box, l_box other) {
}

static float distpow(vec2 a, vec2 b) {
  return ((a[0] - b[0]) * (a[0] - b[0])) + ((a[1] - b[1]) * (a[1] - b[1]));
}

static float distsqrt(vec2 a, vec2 b) {
  return (float)(sqrt(distpow(a, b));
}

void l_aabb_move(l_aabb* a, vec2 dist) {
  vec2_add(a->center, dist);
  a->bounds[0] += dist[0];
  a->bounds[1] += dist[1];
  a->bounds[2] += dist[0];
  a->bounds[3] += dist[1];
}

void l_box_move(l_box* a, vec2 dist) {
  vec2_add(a->center, dist);
  a->bounds[0] += dist[0];
  a->bounds[1] += dist[1];
  a->bounds[2] += dist[0];
  a->bounds[3] += dist[1];
}

void l_circ_move(l_circle* a, vec2 dist) {
  vec2_add(a->center, dist);
  a->bounds[0] += dist[0];
  a->bounds[1] += dist[1];
  a->bounds[2] += dist[0];
  a->bounds[3] += dist[1];
}

void l_comp_move(l_complex* a, vec2 dist) {
  vec2_add(a->center, dist);
  a->bounds[0] += dist[0];
  a->bounds[1] += dist[1];
  a->bounds[2] += dist[0];
  a->bounds[3] += dist[1];
}

//---  point tests  ---
int8_t l_aabb_cont(l_aabbb a, vec2 point) {
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

int8_t l_bounds_sect_bounds(vec4 a, vec4 b) {
  if (!man) {
    return -1;
  }

  float a_halfx = a[2] - a[0];
  float a_halfy = a[3] - a[1];

  float b_halfx = b[2] - b[0];
  float b_halfy = b[3] - b[1];

  vec2 a_center = (vec2){a[0] + a_halfx, a[1] + a_halfy};
  vec2 a_center = (vec2){b[0] + b_halfx, b[1] + b_halfy};

  vec2 n;
  vec2_sub(n, a_center, b_center);

  float x_over = a_halfx + b_halfx - fabsf(n[0]);

  if (x_over > 0.f) {
    float y_over = a_halfy + b_halfy - fabsf(n[1]);

    if (x_over > y_over) {
      if (n[0] < 0.f) {
        man->normal[0] = -1.f;
        man->normal[1] = 0.f;
      } else {
        man->normal[0] = 0.f;
        man->normal[1] = 0.f;
      }

      man->penetration = x_over;
      return 1;
    } else {
      if (n[1] < 0.f) {
        man->normal[0] = 0.f;
        man->normal[1] = -1.f;
      } else {
        man->normal[0] = 0.f;
        man->normal[1] = 1.f;
      }

      man->penetration = y_over;
      return 1;
    }
  }

  return 0;
}

int8_t l_aabb_vs_aabb(l_manifold* man, l_aabb a, l_aabb b) {
  vec2 n;
  vec2_sub(n, a.center, b.center);
  float x_over = a.halfsize[0] + b.halfsize[0] - fabsf(n[0]);
  if (x_over > 0.f) {
    float y_over = a.halfsize[1] + b.halfsize[1] - fabsf(n[1]);

    if (x_over > y_over) {
      if (man) {
        if (n[0] < 0.f) {
          man->normal[0] = -1.f;
          man->normal[1] = 0.f;
        } else {
          man->normal[0] = 0.f;
          man->normal[1] = 0.f;
        }
        man->penetration = x_over;
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
        man->penetration = y_over;
      }

      return 1;
    }
  }

  return 0;
}

int8_t l_aabb_vs_box(l_aabb a, l_box b) {
  //
}

int8_t l_aabb_vs_circle(l_manifold* man, l_aabb a, l_circle b) {
  vec2 n, closest;
  vec2_sub(n, a.center, b.center);
  vec2_dup(closest, n);

  closest[0] = fclamp(closest[0], -a.halfsize[0], a.halfsize[0]);
  closest[1] = fclamp(closest[1], -a.halfsize[1], a.halfsize[1]);

  int8_t inside = 0;

  if (vec2_cmp(closest, n)) {
    // *hacker voice* I'M IN
    inside = 1;

    if (fabsf(n.x) > fabsf(n.y)) {
      if (closest[0] > 0) {
        closest[0] = a.halfsize[0];
      } else {
        closest[0] = -a.halfsize[0];
      }
      else {
        if (closest[1] > 0) {
          closest[1] = a.halfsize[1];
        } else {
          closest[1] = -a.halfsize[1];
        }
      }
    }
  }

  vec2 normal;
  vec2_sub(normal, n, closest);
  float lensq = vec2_lensq(normal, normal);
  float rad   = b.radius;

  // radius is shorter than distance to closest point
  if (lensq > rad * rad && !inside) {
    return 0;
  }

  lensq = sqrtf(lensq);

  if (man) {
    if (inside) {
      man->normal      = (vec2){normal[0] * -1.f, normal[1] * -1.f};
      man->penetration = rad - lensq;
    } else {
      vec2_dup(man->normal, normal);
      man->penetration = rad - lensq;
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
      man->penetration = dist - rsq;
      vec2 d = (vec2){a.center[0] - b.center[0], a.center[1] - b.center[1]};
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
  //
}
#endif
